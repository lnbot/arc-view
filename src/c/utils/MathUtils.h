#pragma once
#include <pebble.h>

#define TRIG_7_32_ANGLE (7 * TRIG_MAX_ANGLE / 32)
#define TRIG_3_32_ANGLE (3 * TRIG_MAX_ANGLE / 32)
#define TRIG_QUARTER_ANGLE (TRIG_MAX_ANGLE / 4)
#define TRIG_HALF_ANGLE (TRIG_MAX_ANGLE / 2)

uint32_t isqrt(uint32_t n);
GPoint polar_to_point_native(int angle_native, int distance);
GPoint add_points(GPoint a, GPoint b);
GPoint polar_to_point_offset_native(GPoint offset, int angle_native, int distance);

GPoint angle_to_rect_edge_native(GPoint center, int angle_native, GRect r);
GPoint angle_to_rounded_rect_edge_native(GPoint center, int angle_native, int half_w, int half_h, int r);

int modulus(int a, int b);
GColor get_contrasting_color(GColor color);
