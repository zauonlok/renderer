#ifndef MACRO_H
#define MACRO_H

#define EPSILON 1e-6f
#define PI 3.141592653589793f

#define TO_RADIANS(degrees) ((PI / 180) * degrees)
#define TO_DEGREES(radians) ((180 / PI) * radians)

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#endif
