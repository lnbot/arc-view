#pragma once
#include <pebble.h>

#define TRIG_7_32_ANGLE (7 * TRIG_MAX_ANGLE / 32)
#define TRIG_QUARTER_ANGLE (TRIG_MAX_ANGLE / 4)
#define TRIG_HALF_ANGLE (TRIG_MAX_ANGLE / 2)

uint32_t two_point_distance(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);
uint32_t isqrt(uint32_t n);
GPoint polar_to_point(int angle, int distance);
GPoint polar_to_point_native(int angle_native, int distance);
GPoint add_points(GPoint a, GPoint b);
GPoint polar_to_point_offset(GPoint offset, int angle, int distance);
GPoint polar_to_point_offset_native(GPoint offset, int angle_native, int distance);
double slope_from_two_points(GPoint a, GPoint b);

GPoint angle_to_rect_edge_native(GPoint center, int angle_native, GRect r);
GPoint angle_to_rect_edge(GPoint center, int angle_deg, GRect r);
GPoint angle_to_rounded_rect_edge(GPoint center, int angle_deg, int half_w, int half_h, int r);
GPoint angle_to_rounded_rect_edge_native(GPoint center, int angle_native, int half_w, int half_h, int r);
GPoint point_from_edge(GPoint origin, int angle_deg, GRect r, int inset);
