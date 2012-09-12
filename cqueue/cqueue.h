#ifndef CQUEUE_H
#define CQUEUE_H 1

/*
 * Import the slice and stack APIs, especially the data types.
 */

#include "c_slice/c_sliceDecls.h"
#include "c_stack/c_stackDecls.h"

/*
 * Queues are conceptually an array of void* cells, with each cell
 * either directly containing the data, or a pointer to it.
 *
 * To handle the latter a pointer to a per-cell delete function is
 * maintained, enabling the queue code to delete cells which are
 * pointers to the actual data.
 *
 * Note however that the allocation of cell data is the responsibility
 * of the queue's user.
 */

/*
 * Forward declaration of queues (opaque handle).
 * Type of cell release functions.
 */

typedef struct CQUEUE_* CQUEUE;

typedef void (*CQUEUE_CELL_FREE) (void* cell);

#endif /* CQUEUE_H */

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
