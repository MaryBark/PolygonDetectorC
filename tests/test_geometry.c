#include "../include/geometry.h"
#include "test_framework.h"
#include <math.h>

int test_point_creation(void) {
    Point2D p = point_create(10.5, 20.3);
    TEST_ASSERT_FLOAT_EQUAL(p.x, 10.5, EPSILON, "Point x coordinate");
    TEST_ASSERT_FLOAT_EQUAL(p.y, 20.3, EPSILON, "Point y coordinate");
    return 0;
}

int test_point_equality(void) {
    Point2D p1 = point_create(5.0, 5.0);
    Point2D p2 = point_create(5.0, 5.0);
    Point2D p3 = point_create(5.1, 5.0);
    
    TEST_ASSERT(point_equal(&p1, &p2), "Equal points should match");
    TEST_ASSERT(!point_equal(&p1, &p3), "Different points should not match");
    return 0;
}

int test_rectangle_creation(void) {
    Rectangle r = rect_create(10, 20, 30, 40);
    TEST_ASSERT_FLOAT_EQUAL(r.x_min, 10.0, EPSILON, "x_min");
    TEST_ASSERT_FLOAT_EQUAL(r.y_min, 20.0, EPSILON, "y_min");
    TEST_ASSERT_FLOAT_EQUAL(r.x_max, 30.0, EPSILON, "x_max");
    TEST_ASSERT_FLOAT_EQUAL(r.y_max, 40.0, EPSILON, "y_max");
    return 0;
}

int test_rectangle_area(void) {
    Rectangle r = rect_create(0, 0, 10, 20);
    double area = rect_area(&r);
    TEST_ASSERT_FLOAT_EQUAL(area, 200.0, EPSILON, "Rectangle area");
    return 0;
}

int test_rectangle_intersection(void) {
    Rectangle r1 = rect_create(0, 0, 10, 10);
    Rectangle r2 = rect_create(5, 5, 15, 15);
    Rectangle intersection = rect_intersection(&r1, &r2);
    
    TEST_ASSERT_FLOAT_EQUAL(intersection.x_min, 5.0, EPSILON, "Intersection x_min");
    TEST_ASSERT_FLOAT_EQUAL(intersection.y_min, 5.0, EPSILON, "Intersection y_min");
    TEST_ASSERT_FLOAT_EQUAL(intersection.x_max, 10.0, EPSILON, "Intersection x_max");
    TEST_ASSERT_FLOAT_EQUAL(intersection.y_max, 10.0, EPSILON, "Intersection y_max");
    
    double area = rect_area(&intersection);
    TEST_ASSERT_FLOAT_EQUAL(area, 25.0, EPSILON, "Intersection area");
    return 0;
}

int test_rectangle_no_intersection(void) {
    Rectangle r1 = rect_create(0, 0, 10, 10);
    Rectangle r2 = rect_create(20, 20, 30, 30);
    
    TEST_ASSERT(!rect_intersects_rect(&r1, &r2), "Non-intersecting rectangles");
    return 0;
}

int test_polygon_creation(void) {
    Point2D points[] = {
        point_create(0, 0),
        point_create(10, 0),
        point_create(10, 10),
        point_create(0, 10)
    };
    
    Polygon poly;
    bool result = polygon_init(&poly, points, 4);
    
    TEST_ASSERT(result, "Polygon creation");
    TEST_ASSERT(polygon_is_valid(&poly), "Polygon validity");
    TEST_ASSERT(poly.num_points == 4, "Polygon point count");
    return 0;
}

int test_polygon_contains_point(void) {
    Point2D points[] = {
        point_create(0, 0),
        point_create(10, 0),
        point_create(10, 10),
        point_create(0, 10)
    };
    
    Polygon poly;
    polygon_init(&poly, points, 4);
    
    Point2D inside = point_create(5, 5);
    Point2D outside = point_create(15, 15);
    Point2D on_edge = point_create(10, 5);
    
    TEST_ASSERT(polygon_contains_point(&poly, &inside), "Point inside polygon");
    TEST_ASSERT(!polygon_contains_point(&poly, &outside), "Point outside polygon");
    TEST_ASSERT(polygon_contains_point(&poly, &on_edge), "Point on edge");
    return 0;
}

int test_polygon_area(void) {
    Point2D points[] = {
        point_create(0, 0),
        point_create(10, 0),
        point_create(10, 10),
        point_create(0, 10)
    };
    
    Polygon poly;
    polygon_init(&poly, points, 4);
    
    double area = polygon_area(&poly);
    TEST_ASSERT_FLOAT_EQUAL(area, 100.0, EPSILON, "Square area");
    return 0;
}

int test_polygon_triangle_area(void) {
    Point2D points[] = {
        point_create(0, 0),
        point_create(10, 0),
        point_create(5, 10)
    };
    
    Polygon poly;
    polygon_init(&poly, points, 3);
    
    double area = polygon_area(&poly);
    TEST_ASSERT_FLOAT_EQUAL(area, 50.0, EPSILON, "Triangle area");
    return 0;
}

int test_polygon_bounding_box(void) {
    Point2D points[] = {
        point_create(10, 20),
        point_create(30, 40),
        point_create(50, 10),
        point_create(20, 5)
    };
    
    Polygon poly;
    polygon_init(&poly, points, 4);
    
    Rectangle bbox = polygon_bounding_box(&poly);
    
    TEST_ASSERT_FLOAT_EQUAL(bbox.x_min, 10.0, EPSILON, "BBox x_min");
    TEST_ASSERT_FLOAT_EQUAL(bbox.y_min, 5.0, EPSILON, "BBox y_min");
    TEST_ASSERT_FLOAT_EQUAL(bbox.x_max, 50.0, EPSILON, "BBox x_max");
    TEST_ASSERT_FLOAT_EQUAL(bbox.y_max, 40.0, EPSILON, "BBox y_max");
    return 0;
}

int test_polygon_rect_iou(void) {
    Point2D points[] = {
        point_create(0, 0),
        point_create(10, 0),
        point_create(10, 10),
        point_create(0, 10)
    };
    
    Polygon poly;
    polygon_init(&poly, points, 4);
    
    // Rectangle completely inside polygon
    Rectangle inside = rect_create(2, 2, 8, 8);
    double iou1 = polygon_rect_iou(&poly, &inside);
    TEST_ASSERT(iou1 > 0.5 && iou1 < 0.7, "IoU for inside rectangle");
    
    // Rectangle partially overlapping
    Rectangle partial = rect_create(5, 5, 15, 15);
    double iou2 = polygon_rect_iou(&poly, &partial);
    TEST_ASSERT(iou2 > 0.2 && iou2 < 0.4, "IoU for partially overlapping");
    
    // Rectangle completely outside
    Rectangle outside = rect_create(20, 20, 30, 30);
    double iou3 = polygon_rect_iou(&poly, &outside);
    TEST_ASSERT_FLOAT_EQUAL(iou3, 0.0, EPSILON, "IoU for outside rectangle");
    
    return 0;
}

int test_polygon_concave_shape(void) {
    // Concave polygon (L-shape)
    Point2D points[] = {
        point_create(0, 0),
        point_create(10, 0),
        point_create(10, 5),
        point_create(5, 5),
        point_create(5, 10),
        point_create(0, 10)
    };
    
    Polygon poly;
    polygon_init(&poly, points, 6);
    
    Point2D inside = point_create(7, 2);
    Point2D in_hole = point_create(7, 7);
    Point2D outside = point_create(12, 12);
    
    TEST_ASSERT(polygon_contains_point(&poly, &inside), "Point in concave polygon");
    TEST_ASSERT(!polygon_contains_point(&poly, &in_hole), "Point in cavity");
    TEST_ASSERT(!polygon_contains_point(&poly, &outside), "Point outside concave polygon");
    
    return 0;
}

int main(void) {
    int passed = 0, failed = 0;
    
    printf("\n========================================\n");
    printf("Geometry Unit Tests\n");
    printf("========================================\n\n");
    
    RUN_TEST(test_point_creation);
    RUN_TEST(test_point_equality);
    RUN_TEST(test_rectangle_creation);
    RUN_TEST(test_rectangle_area);
    RUN_TEST(test_rectangle_intersection);
    RUN_TEST(test_rectangle_no_intersection);
    RUN_TEST(test_polygon_creation);
    RUN_TEST(test_polygon_contains_point);
    RUN_TEST(test_polygon_area);
    RUN_TEST(test_polygon_triangle_area);
    RUN_TEST(test_polygon_bounding_box);
    RUN_TEST(test_polygon_rect_iou);
    RUN_TEST(test_polygon_concave_shape);
    
    TEST_SUMMARY();
}
