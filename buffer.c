#include <stdlib.h>
#include "buffer.h"

#define BUFFER_RAW_DATA(buffer) ((int*)(buffer) - 2)
#define BUFFER_CAPACITY(buffer) (BUFFER_RAW_DATA(buffer)[0])
#define BUFFER_OCCUPIED(buffer) (BUFFER_RAW_DATA(buffer)[1])

void buffer_free(void *buffer) {
    if (buffer != NULL) {
        free(BUFFER_RAW_DATA(buffer));
    }
}

int buffer_size(void *buffer) {
    return buffer != NULL ? BUFFER_OCCUPIED(buffer) : 0;
}

void *buffer_hold_helper(void *buffer, int count, int itemsize) {
    if (buffer == NULL) {
        int *base = (int*)malloc(sizeof(int) * 2 + itemsize * count);
        base[0] = count;  /* capacity */
        base[1] = count;  /* occupied */
        return base + 2;
    } else if (BUFFER_OCCUPIED(buffer) + count <= BUFFER_CAPACITY(buffer)) {
        BUFFER_OCCUPIED(buffer) += count;
        return buffer;
    } else {
        int needed_size = BUFFER_OCCUPIED(buffer) + count;
        int double_curr = BUFFER_CAPACITY(buffer) * 2;
        int capacity = needed_size > double_curr ? needed_size : double_curr;
        int occupied = needed_size;
        int *base = (int*)realloc(BUFFER_RAW_DATA(buffer),
                                  sizeof(int) * 2 + itemsize * capacity);
        base[0] = capacity;
        base[1] = occupied;
        return base + 2;
    }
}
