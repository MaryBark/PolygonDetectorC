#ifndef POLYGON_DETECTOR_POLYGON_PROCESSOR_H_
#define POLYGON_DETECTOR_POLYGON_PROCESSOR_H_

#include "geometry.h"
#include "detector.h"
#include "geometry.h"
#include "detector.h"
#include <stdbool.h>

#define MAX_POLYGONS 32
#define MAX_CLASSES_PER_POLYGON 20
#define MAX_POLYGON_ID_LEN 64

typedef struct {
  char id[MAX_POLYGON_ID_LEN];
  Polygon polygon;
  int priority;
  bool is_inclusive;
  double iou_threshold;
  char target_classes[MAX_CLASSES_PER_POLYGON][MAX_CLASS_NAME_LEN];
  int num_target_classes;
} PolygonConfig;

typedef struct {
  Detection detection;
  char polygon_id[MAX_POLYGON_ID_LEN];
  bool is_included;
  int priority;
} ProcessedObject;

typedef struct {
  ProcessedObject objects[MAX_DETECTIONS];
  size_t num_objects;
} ProcessedResult;

typedef struct PolygonProcessor PolygonProcessor;

PolygonProcessor* processor_create(void);
void processor_destroy(PolygonProcessor* processor);
void processor_add_polygon(PolygonProcessor* processor, const PolygonConfig* config);
void processor_clear_polygons(PolygonProcessor* processor);
ProcessedResult processor_process_detections(PolygonProcessor* processor,
                                             const DetectionArray* detections);
void* processor_draw_results(PolygonProcessor* processor,
                             void* original_image,
                             const ProcessedResult* results);
const PolygonConfig* processor_get_polygons(PolygonProcessor* processor, int* count);

#endif  // POLYGON_DETECTOR_POLYGON_PROCESSOR_H_
