#ifndef CSTACK_H
#define CSTACK_H 1

/*
 * Import the slice API, especially the data types.
 */

#include "c_slice/c_sliceDecls.h"

/*
 * Stacks are conceptually an array of void* cells, with each cell
 * either directly containing the data, or a pointer to it.
 *
 * To handle the latter a pointer to a per-cell delete function is
 * maintained, enabling the stack code to delete cells which are
 * pointers to the actual data.
 *
 * Note however that the allocation of cell data is the responsibility
 * of the stack's user.
 */

/*
 * Forward declaration of stacks (opaque handle).
 * Type of cell release functions.
 */

typedef struct CSTACK_* CSTACK;

typedef void (*CSTACK_CELL_FREE) (void* cell);

#endif /* CSTACK_H */

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
