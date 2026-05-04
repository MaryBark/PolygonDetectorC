#include "polygon_processor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct PolygonProcessor {
  PolygonConfig polygons[MAX_POLYGONS];
  int num_polygons;
};

PolygonProcessor* processor_create(void) {
  PolygonProcessor* p = (PolygonProcessor*)malloc(sizeof(PolygonProcessor));
  if (p) {
    memset(p, 0, sizeof(PolygonProcessor));
  }
  return p;
}

void processor_destroy(PolygonProcessor* processor) {
  free(processor);
}

static int compare_polygons(const void* a, const void* b) {
  const PolygonConfig* pa = (const PolygonConfig*)a;
  const PolygonConfig* pb = (const PolygonConfig*)b;
  return pb->priority - pa->priority;
}

void processor_add_polygon(PolygonProcessor* processor, const PolygonConfig* config) {
  if (!processor || processor->num_polygons >= MAX_POLYGONS) return;

  processor->polygons[processor->num_polygons++] = *config;
  qsort(processor->polygons, processor->num_polygons,
        sizeof(PolygonConfig), compare_polygons);
}

void processor_clear_polygons(PolygonProcessor* processor) {
  if (processor) {
    processor->num_polygons = 0;
  }
}

static bool is_class_targeted(const char* class_name, const PolygonConfig* polygon) {
  if (polygon->num_target_classes == 0) return true;

  for (int i = 0; i < polygon->num_target_classes; ++i) {
    if (strcmp(class_name, polygon->target_classes[i]) == 0) {
      return true;
    }
  }
  return false;
}

static bool should_include_object(const Detection* detection, const PolygonConfig* polygon) {
  Rectangle bbox_rect = rect_create(
      detection->x,
      detection->y,
      detection->x + detection->width,
      detection->y + detection->height
  );

  double iou = polygon_rect_iou(&polygon->polygon, &bbox_rect);

  if (!is_class_targeted(detection->class_name, polygon)) {
    if (polygon->is_inclusive) return false;
    return true;
  }

  if (polygon->is_inclusive) {
    return iou >= polygon->iou_threshold;
  } else {
    if (iou >= polygon->iou_threshold) {
      return false;
    }
    return true;
  }
}

ProcessedResult processor_process_detections(PolygonProcessor* processor,
                                             const DetectionArray* detections) {
  ProcessedResult result;
  memset(&result, 0, sizeof(result));

  if (!processor || !detections) return result;

  for (size_t d = 0; d < detections->num_detections; ++d) {
    const Detection* det = &detections->detections[d];
    bool final_inclusion = true;
    int highest_priority = -1;
    char responsible_polygon[MAX_POLYGON_ID_LEN] = {0};

    for (int p = 0; p < processor->num_polygons; ++p) {
      const PolygonConfig* poly = &processor->polygons[p];

      if (highest_priority >= poly->priority && highest_priority != -1) {
        break;
      }

      bool include = should_include_object(det, poly);

      if (poly->is_inclusive && include) {
        final_inclusion = true;
        highest_priority = poly->priority;
        strncpy(responsible_polygon, poly->id, MAX_POLYGON_ID_LEN - 1);
        break;
      } else if (!poly->is_inclusive && !include) {
        final_inclusion = false;
        highest_priority = poly->priority;
        strncpy(responsible_polygon, poly->id, MAX_POLYGON_ID_LEN - 1);
        break;
      }
    }

    if (final_inclusion) {
      ProcessedObject* obj = &result.objects[result.num_objects++];
      obj->detection = *det;
      obj->is_included = true;
      obj->priority = highest_priority;
      strncpy(obj->polygon_id, responsible_polygon, MAX_POLYGON_ID_LEN - 1);
    }
  }

  return result;
}

void* processor_draw_results(PolygonProcessor* processor,
                             void* original_image,
                             const ProcessedResult* results) {
  // Stub implementation - just return original image
  printf("Drawing results: %zu objects\n", results->num_objects);
  return original_image;
}

const PolygonConfig* processor_get_polygons(PolygonProcessor* processor, int* count) {
  if (!processor || !count) return NULL;
  *count = processor->num_polygons;
  return processor->polygons;
}
