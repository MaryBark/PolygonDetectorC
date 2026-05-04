#include "detector.h"
#include "detector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  float confidence_threshold;
  float nms_threshold;
};

ObjectDetector* detector_create(const char* model_path, const char* config_path,
                                float confidence_threshold, float nms_threshold) {
  printf("WARNING: Using stub detector (no real object detection)\n");
  printf("Model path: %s\n", model_path ? model_path : "none");
  printf("Config path: %s\n", config_path ? config_path : "none");
  
  ObjectDetector* detector = (ObjectDetector*)malloc(sizeof(ObjectDetector));
  if (detector) {
    detector->confidence_threshold = confidence_threshold;
    detector->nms_threshold = nms_threshold;
  }
  return detector;
}

void detector_destroy(ObjectDetector* detector) {
  free(detector);
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

DetectionArray detector_detect(ObjectDetector* detector, void* image) {
  DetectionArray result;
  memset(&result, 0, sizeof(result));
  
  if (!detector || !image) return result;
  
  // Generate fake detections for testing
  static int fake_count = 0;
  fake_count = (fake_count + 1) % 5;
  
  printf("Stub detector: generating %d fake detections\n", fake_count);
  
  for (int i = 0; i < fake_count && i < MAX_DETECTIONS; ++i) {
    Detection* det = &result.detections[result.num_detections++];
    snprintf(det->class_name, MAX_CLASS_NAME_LEN, "%s", COCO_CLASSES[i % NUM_CLASSES]);
    det->confidence = 0.5f + (i * 0.1f);
    det->x = 100 + i * 50;
    det->y = 100 + i * 50;
    det->width = 100;
    det->height = 100;
  }
  
  return result;
}

void detector_set_confidence_threshold(ObjectDetector* detector, float threshold) {
  if (detector) detector->confidence_threshold = threshold;
}

void detector_set_nms_threshold(ObjectDetector* detector, float threshold) {
  if (detector) detector->nms_threshold = threshold;
}

