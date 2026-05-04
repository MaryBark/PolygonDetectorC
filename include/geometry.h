#ifndef POLYGON_DETECTOR_GEOMETRY_H_
#define POLYGON_DETECTOR_GEOMETRY_H_

#include <stdbool.h>
#include <stddef.h>

#define POLYGON_MAX_POINTS 100
#define EPSILON 1e-9

typedef struct {
  double x;
  double y;
} Point2D;

typedef struct {
  double x_min;
  double y_min;
  double x_max;
  double y_max;
} Rectangle;

typedef struct {
  Point2D points[POLYGON_MAX_POINTS];
  size_t num_points;
} Polygon;

// Point operations
Point2D point_create(double x, double y);
bool point_equal(const Point2D* p1, const Point2D* p2);

// Rectangle operations
Rectangle rect_create(double x1, double y1, double x2, double y2);
double rect_area(const Rectangle* rect);
bool rect_is_valid(const Rectangle* rect);
Point2D rect_center(const Rectangle* rect);

// Polygon operations
bool polygon_init(Polygon* poly, const Point2D* points, size_t num_points);
bool polygon_is_valid(const Polygon* poly);
bool polygon_contains_point(const Polygon* poly, const Point2D* point);
double polygon_area(const Polygon* poly);
Rectangle polygon_bounding_box(const Polygon* poly);

// Intersection operations
double polygon_rect_intersection_area(const Polygon* poly, const Rectangle* rect);
double polygon_rect_iou(const Polygon* poly, const Rectangle* rect);
bool rect_intersects_rect(const Rectangle* r1, const Rectangle* r2);
Rectangle rect_intersection(const Rectangle* r1, const Rectangle* r2);

#endif  // POLYGON_DETECTOR_GEOMETRY_H_