#include "geometry.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MONTE_CARLO_SAMPLES 5000

Point2D point_create(double x, double y) {
  Point2D p = {x, y};
  return p;
}

bool point_equal(const Point2D* p1, const Point2D* p2) {
  return fabs(p1->x - p2->x) < EPSILON && fabs(p1->y - p2->y) < EPSILON;
}

Rectangle rect_create(double x1, double y1, double x2, double y2) {
  Rectangle r = {
      .x_min = fmin(x1, x2),
      .y_min = fmin(y1, y2),
      .x_max = fmax(x1, x2),
      .y_max = fmax(y1, y2)
  };
  return r;
}

double rect_area(const Rectangle* rect) {
  if (!rect_is_valid(rect)) return 0.0;
  return (rect->x_max - rect->x_min) * (rect->y_max - rect->y_min);
}

bool rect_is_valid(const Rectangle* rect) {
  return (rect->x_max > rect->x_min + EPSILON) &&
         (rect->y_max > rect->y_min + EPSILON);
}

Point2D rect_center(const Rectangle* rect) {
  return point_create(
      (rect->x_min + rect->x_max) / 2.0,
      (rect->y_min + rect->y_max) / 2.0
  );
}

bool rect_intersects_rect(const Rectangle* r1, const Rectangle* r2) {
  return !(r1->x_max <= r2->x_min + EPSILON ||
           r1->x_min >= r2->x_max - EPSILON ||
           r1->y_max <= r2->y_min + EPSILON ||
           r1->y_min >= r2->y_max - EPSILON);
}

Rectangle rect_intersection(const Rectangle* r1, const Rectangle* r2) {
  return rect_create(
      fmax(r1->x_min, r2->x_min),
      fmax(r1->y_min, r2->y_min),
      fmin(r1->x_max, r2->x_max),
      fmin(r1->y_max, r2->y_max)
  );
}

bool polygon_init(Polygon* poly, const Point2D* points, size_t num_points) {
  if (num_points < 3 || num_points > POLYGON_MAX_POINTS) {
    return false;
  }
  
  memset(poly->points, 0, sizeof(poly->points));
  memcpy(poly->points, points, num_points * sizeof(Point2D));
  poly->num_points = num_points;
  return true;
}

bool polygon_is_valid(const Polygon* poly) {
  return poly->num_points >= 3;
}

static double cross_product(const Point2D* o, const Point2D* a, const Point2D* b) {
  return (a->x - o->x) * (b->y - o->y) - (a->y - o->y) * (b->x - o->x);
}

bool polygon_contains_point(const Polygon* poly, const Point2D* point) {
  if (!polygon_is_valid(poly)) return false;
  
  bool inside = false;
  size_t n = poly->num_points;
  
  for (size_t i = 0, j = n - 1; i < n; j = i++) {
    const Point2D* p1 = &poly->points[i];
    const Point2D* p2 = &poly->points[j];
    
    if (((p1->y > point->y) != (p2->y > point->y)) &&
        (point->x < (p2->x - p1->x) * (point->y - p1->y) /
                    (p2->y - p1->y) + p1->x)) {
      inside = !inside;
    }
  }
  
  return inside;
}

double polygon_area(const Polygon* poly) {
  if (!polygon_is_valid(poly)) return 0.0;
  
  double area = 0.0;
  size_t n = poly->num_points;
  
  for (size_t i = 0; i < n; ++i) {
    const Point2D* p1 = &poly->points[i];
    const Point2D* p2 = &poly->points[(i + 1) % n];
    area += p1->x * p2->y - p2->x * p1->y;
  }
  
  return fabs(area) / 2.0;
}

Rectangle polygon_bounding_box(const Polygon* poly) {
  if (!polygon_is_valid(poly)) {
    return rect_create(0, 0, 0, 0);
  }
  
  double min_x = poly->points[0].x, max_x = poly->points[0].x;
  double min_y = poly->points[0].y, max_y = poly->points[0].y;
  
  for (size_t i = 1; i < poly->num_points; ++i) {
    min_x = fmin(min_x, poly->points[i].x);
    max_x = fmax(max_x, poly->points[i].x);
    min_y = fmin(min_y, poly->points[i].y);
    max_y = fmax(max_y, poly->points[i].y);
  }
  
  return rect_create(min_x, min_y, max_x, max_y);
}

static Rectangle clip_rect_to_bbox(const Rectangle* rect, const Rectangle* bbox) {
  return rect_create(
      fmax(rect->x_min, bbox->x_min),
      fmax(rect->y_min, bbox->y_min),
      fmin(rect->x_max, bbox->x_max),
      fmin(rect->y_max, bbox->y_max)
  );
}

double polygon_rect_intersection_area(const Polygon* poly, const Rectangle* rect) {
  if (!polygon_is_valid(poly) || !rect_is_valid(rect)) return 0.0;
  
  Rectangle bbox = polygon_bounding_box(poly);
  if (!rect_intersects_rect(rect, &bbox)) return 0.0;
  
  Rectangle clip_rect = clip_rect_to_bbox(rect, &bbox);
  if (!rect_is_valid(&clip_rect)) return 0.0;
  
  // Monte Carlo integration for intersection area
  static int seeded = 0;
  if (!seeded) {
    srand((unsigned int)time(NULL));
    seeded = 1;
  }
  
  int inside_count = 0;
  for (int i = 0; i < MONTE_CARLO_SAMPLES; ++i) {
    double x = clip_rect.x_min + (rand() / (double)RAND_MAX) *
                                   (clip_rect.x_max - clip_rect.x_min);
    double y = clip_rect.y_min + (rand() / (double)RAND_MAX) *
                                   (clip_rect.y_max - clip_rect.y_min);
    Point2D sample = point_create(x, y);
    
    if (polygon_contains_point(poly, &sample)) {
      inside_count++;
    }
  }
  
  double rect_area_val = rect_area(&clip_rect);
  return rect_area_val * inside_count / MONTE_CARLO_SAMPLES;
}

double polygon_rect_iou(const Polygon* poly, const Rectangle* rect) {
  double intersection = polygon_rect_intersection_area(poly, rect);
  if (intersection < EPSILON) return 0.0;
  
  double rect_area_val = rect_area(rect);
  double poly_area_val = polygon_area(poly);
  double union_area = rect_area_val + poly_area_val - intersection;
  
  if (union_area < EPSILON) return 0.0;
  return intersection / union_area;
}