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

#endif /* CSLICE_H */

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
