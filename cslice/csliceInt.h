#ifndef CSLICE_INT_H
#define CSLICE_INT_H 1

#include "c_slice/c_sliceDecls.h"

/*
 * Actual type of the slice data structure. Used only inside of the
 * package.
 */

typedef struct CSLICE_ {
    long int n;        /* Number of cells in the slice. */
    void**   cell;     /* Array of the cells in the slice. */
    int      dynamic;  /* Flag telling us if 'cell' is allocated on the heap, or not */
} CSLICE_;

/*
 * Allocation macros for common situations.
 */

#define ALLOC(type)    (type *) ckalloc (sizeof (type))
#define NALLOC(n,type) (type *) ckalloc ((n) * sizeof (type))
#define NREALLOC(n,type,p) (type *) ckrealloc ((p), (n) * sizeof (type))

/*
 * Assertions in general, and asserting the proper range of an array
 * index.
 */

#undef  CSLICE_DEBUG
#define CSLICE_DEBUG 1

#ifdef CSLICE_DEBUG
#define XSTR(x) #x
#define STR(x) XSTR(x)
#define RANGEOK(i,n) ((0 <= (i)) && (i < (n)))
#define ASSERT(x,msg) if (!(x)) { Tcl_Panic (msg " (" #x "), in file " __FILE__ " @line " STR(__LINE__));}
#define ASSERT_BOUNDS(i,n) ASSERT (RANGEOK(i,n),"array index out of bounds: " STR(i) " > " STR(n))
#else
#define ASSERT(x,msg)
#define ASSERT_BOUNDS(i,n)
#endif

#endif /* CSLICE_INT_H */

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
