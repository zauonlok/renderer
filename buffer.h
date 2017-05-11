#ifndef BUFFER_H
#define BUFFER_H

void buffer_free(void *buffer);
int buffer_size(void *buffer);

#define buffer_hold(buffer, count)                                          \
    do {                                                                    \
        (buffer) = buffer_hold_helper(buffer, count, sizeof(*(buffer)));    \
    } while (0)

#define buffer_push(buffer, value)                                          \
    do {                                                                    \
        buffer_hold(buffer, 1);                                             \
        (buffer)[buffer_size(buffer) - 1] = (value);                        \
    } while (0)

/* private helper function */
void *buffer_hold_helper(void *buffer, int count, int itemsize);

#endif
