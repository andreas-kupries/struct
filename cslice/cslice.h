#ifndef CSLICE_H
#define CSLICE_H 1

/*
 * A slice is an array of void* cells of some other data structure based on
 * such cells. Each cell either directly contains the data, or a pointer to
 * it. In contrast to other data structures a slice does not own the data in
 * its cells, therefore it doesn't need a function to release cells.
 * A slice may not even own the array of cells itself.
 *
 * Slices are expected to live only shortly, to provide access to parts of
 * other data structures.
 *
 * Forward declaration of slices (opaque handle).
 */

typedef struct CSLICE_* CSLICE;

/*
 * Cue to 'cslice_create' where to put the first element of the given array
 * slice in the returned slice object.
 */

typedef enum {
    cslice_normal,  /* The first element is at the left/beginning/first */
    cslice_revers   /* The first element is at the right/end/last */
} CSLICE_DIRECTION;

#endif /* CSLICE_H */

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
