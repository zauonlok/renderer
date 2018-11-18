#ifndef DARRAY_H
#define DARRAY_H

/*
 * for typesafe dynamic array, see
 * https://github.com/nothings/stb/blob/master/stretchy_buffer.h
 */

#define darray_push(darray, value)                                          \
    do {                                                                    \
        (darray) = darray_hold(darray, 1, sizeof(*(darray)));               \
        (darray)[darray_size(darray) - 1] = (value);                        \
    } while (0)

void *darray_hold(void *darray, int count, int itemsize);
int darray_size(void *darray);
void darray_free(void *darray);

#endif
