#ifndef CQUEUE_INT_H
#define CQUEUE_INT_H 1

#include "c_queue/c_queueDecls.h"

/*
 * Actual type of the queue data structure. Used only inside of the
 * package.
 */

typedef struct CQUEUE_ {
    /*
     * We are using a Banker's queue to keep the memory we use limited and
     * return of elements still fast (by index). All fields can be NULL or 0,
     * i.e. are allocated only when needed.
     *
     * The logical organization and ordering of the stack making up the queue
     * is:
     *
     * |unget.| |result| |append|
     * |<-----| |----->| |----->|
     * |T    B| |B  ^ T| |B    T|
     *              at
     *
     * The arrows above point in the direction of growth, i.e. bottom to top.
     */

    CSTACK   unget;  /* Stack of unget'ed elements */
    CSTACK   result; /* Buffer of elements to return */
    CSTACK   append; /* Stack of newly added elements */
    long int at;     /* Index of the next element to return, in result */

    CQUEUE_CELL_FREE freeCell; 
    void*            clientData;

    CSTACK hold; /* Saved from destruction for reuse */
} CQUEUE_;

/*
 * Allocation macros for common situations.
 */

#define ALLOC(type)    (type *) ckalloc (sizeof (type))
#define NALLOC(n,type) (type *) ckalloc ((n) * sizeof (type))

/*
 * Assertions in general, and asserting the proper range of an array
 * index.
 */

#undef  CQUEUE_DEBUG
#define CQUEUE_DEBUG 1

#ifdef CQUEUE_DEBUG
#define XSTR(x) #x
#define STR(x) XSTR(x)
#define RANGEOK(i,n) ((0 <= (i)) && (i < (n)))
#define ASSERT(x,msg) if (!(x)) { Tcl_Panic (msg " (" #x "), in file " __FILE__ " @line " STR(__LINE__));}
#define ASSERT_BOUNDS(i,n) ASSERT (RANGEOK(i,n),"array index out of bounds: " STR(i) " > " STR(n))
#else
#define ASSERT(x,msg)
#define ASSERT_BOUNDS(i,n)
#endif

#endif /* CQUEUE_INT_H */

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
