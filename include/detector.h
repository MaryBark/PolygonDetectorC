#ifndef POLYGON_DETECTOR_DETECTOR_H_
#define POLYGON_DETECTOR_DETECTOR_H_

#include <stddef.h>
#include <stdbool.h>

// Forward declarations for OpenCV types (C interface)
#ifdef __cplusplus
extern "C" {
#endif

typedef struct CvMat CvMat;
typedef struct IplImage IplImage;

#ifdef __cplusplus
}
#endif

#define MAX_DETECTIONS 100
#define MAX_CLASS_NAME_LEN 64

typedef struct {
  char class_name[MAX_CLASS_NAME_LEN];
  float confidence;
  int x;
  int y;
  int width;
  int height;
} Detection;

typedef struct {
  Detection detections[MAX_DETECTIONS];
  size_t num_detections;
} DetectionArray;

typedef struct ObjectDetector ObjectDetector;

ObjectDetector* detector_create(const char* model_path, const char* config_path,
                                float confidence_threshold, float nms_threshold);
void detector_destroy(ObjectDetector* detector);
DetectionArray detector_detect(ObjectDetector* detector, void* image);
void detector_set_confidence_threshold(ObjectDetector* detector, float threshold);
void detector_set_nms_threshold(ObjectDetector* detector, float threshold);
const char* detector_get_class_name(int class_id);
int detector_get_num_classes(void);
#endif  // POLYGON_DETECTOR_DETECTOR_H_
