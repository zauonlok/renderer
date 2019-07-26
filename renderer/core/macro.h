#ifndef MACRO_H
#define MACRO_H

#define EPSILON 1e-6f
#define PI 3.1415927f

#define TO_RADIANS(degrees) ((PI / 180) * (degrees))
#define TO_DEGREES(radians) ((180 / PI) * (radians))

#define UNUSED(x) ((void)(x))
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#define MAX_PATH 2048
#define LINE_LENGTH 1024

#endif
