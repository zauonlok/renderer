#ifndef MACRO_H
#define MACRO_H

#define EPSILON 1e-5f
#define PI 3.1415927f

#define TO_RADIANS(degrees) ((PI / 180) * (degrees))
#define TO_DEGREES(radians) ((180 / PI) * (radians))

#define LINE_SIZE 256
#define PATH_SIZE 256

#define UNUSED_VAR(x) ((void)(x))
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#endif
