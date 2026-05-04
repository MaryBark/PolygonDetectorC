#include "detector.h"
//#include <opencv2/dnn/dnn.h>  // не совместим opencv с С, только с С++
#include <opencv2/core/core_c.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INPUT_WIDTH 416
#define INPUT_HEIGHT 416
#define DETECTION_THRESHOLD 0.5f
#define NMS_THRESHOLD 0.4f

static const char* COCO_CLASSES[] = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck",
    "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench",
    "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra",
    "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
    "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove",
    "skateboard", "surfboard", "tennis racket", "bottle", "wine glass", "cup",
    "fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich", "orange",
    "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
    "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse",
    "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink",
    "refrigerator", "book", "clock", "vase", "scissors", "teddy bear", "hair drier",
    "toothbrush"
};

#define NUM_CLASSES (sizeof(COCO_CLASSES) / sizeof(COCO_CLASSES[0]))

struct ObjectDetector {
  CvDNNNet net;
  float confidence_threshold;
  float nms_threshold;
  int input_width;
  int input_height;
};

ObjectDetector* detector_create(const char* model_path, const char* config_path,
                                float confidence_threshold, float nms_threshold) {
  ObjectDetector* detector = (ObjectDetector*)malloc(sizeof(ObjectDetector));
  if (!detector) return NULL;
  
  memset(detector, 0, sizeof(ObjectDetector));
  
  detector->net = cv_dnn_read_net(model_path, config_path, NULL);
  if (!detector->net) {
    free(detector);
    return NULL;
  }
  
  detector->confidence_threshold = confidence_threshold;
  detector->nms_threshold = nms_threshold;
  detector->input_width = INPUT_WIDTH;
  detector->input_height = INPUT_HEIGHT;
  
  cv_dnn_net_set_preferable_backend(detector->net, CV_DNN_BACKEND_OPENCV);
  cv_dnn_net_set_preferable_target(detector->net, CV_DNN_TARGET_CPU);
  
  return detector;
}

void detector_destroy(ObjectDetector* detector) {
  if (detector) {
    if (detector->net) {
      cv_dnn_net_release(&detector->net);
    }
    free(detector);
  }
}

const char* detector_get_class_name(int class_id) {
  if (class_id >= 0 && class_id < NUM_CLASSES) {
    return COCO_CLASSES[class_id];
  }
  return "unknown";
}

int detector_get_num_classes(void) {
  return NUM_CLASSES;
}

typedef struct {
  CvRect bbox;
  float confidence;
  int class_id;
} RawDetection;

static int compare_detections(const void* a, const void* b) {
  const RawDetection* da = (const RawDetection*)a;
  const RawDetection* db = (const RawDetection*)b;
  if (db->confidence > da->confidence) return 1;
  if (db->confidence < da->confidence) return -1;
  return 0;
}

static int nms_filter(RawDetection* detections, int num_detections, float iou_threshold) {
  if (num_detections <= 0) return 0;
  
  qsort(detections, num_detections, sizeof(RawDetection), compare_detections);
  
  int* keep = (int*)calloc(num_detections, sizeof(int));
  int keep_count = 0;
  
  for (int i = 0; i < num_detections; ++i) {
    if (keep[i]) continue;
    
    keep[i] = 1;
    keep_count++;
    
    const CvRect* box_i = &detections[i].bbox;
    double area_i = (box_i->width + 1) * (box_i->height + 1);
    
    for (int j = i + 1; j < num_detections; ++j) {
      if (keep[j]) continue;
      if (detections[i].class_id != detections[j].class_id) continue;
      
      const CvRect* box_j = &detections[j].bbox;
      int x1 = box_i->x > box_j->x ? box_i->x : box_j->x;
      int y1 = box_i->y > box_j->y ? box_i->y : box_j->y;
      int x2 = (box_i->x + box_i->width) < (box_j->x + box_j->width) ?
               (box_i->x + box_i->width) : (box_j->x + box_j->width);
      int y2 = (box_i->y + box_i->height) < (box_j->y + box_j->height) ?
               (box_i->y + box_i->height) : (box_j->y + box_j->height);
      
      int intersection_width = x2 - x1 + 1;
      int intersection_height = y2 - y1 + 1;
      
      if (intersection_width > 0 && intersection_height > 0) {
        double intersection_area = intersection_width * intersection_height;
        double box_j_area = (box_j->width + 1) * (box_j->height + 1);
        double iou = intersection_area / (area_i + box_j_area - intersection_area);
        
        if (iou > iou_threshold) {
          keep[j] = 1;
        }
      }
    }
  }
  
  // Pack detections
  int write_idx = 0;
  for (int i = 0; i < num_detections; ++i) {
    if (keep[i]) {
      detections[write_idx++] = detections[i];
    }
  }
  
  free(keep);
  return write_idx;
}

DetectionArray detector_detect(ObjectDetector* detector, const IplImage* image) {
  DetectionArray result;
  memset(&result, 0, sizeof(result));
  
  if (!detector || !image) return result;
  
  // Preprocess image
  cv::Mat img_mat = cvCreateMat(image->height, image->width, CV_8UC3);
  cvCvtColor(image, &img_mat, CV_BGR2RGB);
  
  CvMat blob;
  cv_dnn_blob_from_image(&img_mat, &blob, 1.0/255.0,
                         cvSize(detector->input_width, detector->input_height),
                         cvScalar(0,0,0), 1, 0);
  
  // Forward pass
  cv_dnn_net_set_input_blob(detector->net, &blob);
  
  CvDNNOutput* outputs = cv_dnn_net_forward_detection(detector->net);
  if (!outputs) {
    cvReleaseMat(&blob);
    cvReleaseMat(&img_mat);
    return result;
  }
  
  // Parse outputs
  RawDetection raw_detections[MAX_DETECTIONS * 10];
  int raw_count = 0;
  
  for (int i = 0; i < outputs->num_detections && raw_count < MAX_DETECTIONS * 10; ++i) {
    float conf = outputs->detections[i].confidence;
    if (conf < detector->confidence_threshold) continue;
    
    int class_id = outputs->detections[i].class_id;
    if (class_id < 0 || class_id >= NUM_CLASSES) continue;
    
    float x = outputs->detections[i].x * image->width;
    float y = outputs->detections[i].y * image->height;
    float w = outputs->detections[i].width * image->width;
    float h = outputs->detections[i].height * image->height;
    
    int x_min = (int)(x - w/2);
    int y_min = (int)(y - h/2);
    int x_max = (int)(x + w/2);
    int y_max = (int)(y + h/2);
    
    x_min = x_min < 0 ? 0 : x_min;
    y_min = y_min < 0 ? 0 : y_min;
    x_max = x_max > image->width ? image->width : x_max;
    y_max = y_max > image->height ? image->height : y_max;
    
    if (x_max <= x_min || y_max <= y_min) continue;
    
    raw_detections[raw_count].bbox = cvRect(x_min, y_min, x_max - x_min, y_max - y_min);
    raw_detections[raw_count].confidence = conf;
    raw_detections[raw_count].class_id = class_id;
    raw_count++;
  }
  
  // Apply NMS
  int filtered_count = nms_filter(raw_detections, raw_count, detector->nms_threshold);
  
  // Convert to result
  for (int i = 0; i < filtered_count && i < MAX_DETECTIONS; ++i) {
    const char* class_name = detector_get_class_name(raw_detections[i].class_id);
    strncpy(result.detections[result.num_detections].class_name,
            class_name, MAX_CLASS_NAME_LEN - 1);
    result.detections[result.num_detections].confidence = raw_detections[i].confidence;
    result.detections[result.num_detections].bbox = raw_detections[i].bbox;
    result.num_detections++;
  }
  
  cv_dnn_output_release(&outputs);
  cvReleaseMat(&blob);
  cvReleaseMat(&img_mat);
  
  return result;
}

void detector_set_confidence_threshold(ObjectDetector* detector, float threshold) {
  if (detector) detector->confidence_threshold = threshold;
}

void detector_set_nms_threshold(ObjectDetector* detector, float threshold) {
  if (detector) detector->nms_threshold = threshold;
}
