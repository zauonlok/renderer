#include <assert.h>
#include <stdlib.h>
#include "darray.h"

/*
 * for type-safe dynamic array, see
 * https://github.com/nothings/stb/blob/master/stretchy_buffer.h
 */

#define DARRAY_RAW_DATA(darray) ((int*)(darray) - 2)
#define DARRAY_CAPACITY(darray) (DARRAY_RAW_DATA(darray)[0])
#define DARRAY_OCCUPIED(darray) (DARRAY_RAW_DATA(darray)[1])

int darray_size(void *darray) {
    return darray != NULL ? DARRAY_OCCUPIED(darray) : 0;
}

void darray_free(void *darray) {
    if (darray != NULL) {
        free(DARRAY_RAW_DATA(darray));
    }
}

void *darray_hold(void *darray, int count, int item_size) {
    int header_size = sizeof(int) * 2;
    assert(count > 0 && item_size > 0);
    if (darray == NULL) {
        int raw_size = header_size + item_size * count;
        int *base = (int*)malloc(raw_size);
        base[0] = count;  /* capacity */
        base[1] = count;  /* occupied */
        return base + 2;
    } else if (DARRAY_OCCUPIED(darray) + count <= DARRAY_CAPACITY(darray)) {
        DARRAY_OCCUPIED(darray) += count;
        return darray;
    } else {
        int needed_size = DARRAY_OCCUPIED(darray) + count;
        int double_curr = DARRAY_CAPACITY(darray) * 2;
        int capacity = needed_size > double_curr ? needed_size : double_curr;
        int occupied = needed_size;
        int raw_size = header_size + item_size * capacity;
        int *base = (int*)realloc(DARRAY_RAW_DATA(darray), raw_size);
        base[0] = capacity;
        base[1] = occupied;
        return base + 2;
    }
}
