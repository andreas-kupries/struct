#ifndef CSET_H
#define CSET_H 1

/*
 * Sets are conceptually a hashtable of void* cells, with each cell
 * either directly containing the data, or a pointer to it.
 *
 * To handle the latter a pointer to a per-cell delete function is
 * maintained, enabling the set code to delete cells which are
 * pointers to the actual data.
 *
 * We also have to provide a function to compare set elements for
 * equality.
 *
 * Note however that the allocation of cell data is the responsibility
 * of the set's user.
 */

/*
 * Forward declaration of sets (opaque handle).
 * Type of cell release functions.
 * Type of cell comparison functions.
 */

typedef struct CSET_* CSET;

typedef void* (*CSET_CELL_DUP) (void* cell);
typedef void  (*CSET_CELL_REL) (void* cell);
typedef int   (*CSET_CELL_CMP) (const void* cella, const void* cellb);

#endif /* CSET_H */

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
