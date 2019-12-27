#ifndef DARRAY_H
#define DARRAY_H

#define darray_push(darray, value)                                          \
    do {                                                                    \
        (darray) = darray_hold((darray), 1, sizeof(*(darray)));             \
        (darray)[darray_size(darray) - 1] = (value);                        \
    } while (0)

void *darray_hold(void *darray, int count, int item_size);
int darray_size(void *darray);
void darray_free(void *darray);

#endif
