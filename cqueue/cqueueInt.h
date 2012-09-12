#ifndef CQUEUE_INT_H
#define CQUEUE_INT_H 1

#include "c_queue/c_queueDecls.h"

/*
 * Actual type of the queue data structure.
 * Used only inside of the package.
 * The outside only seeq opaque CQUEUE handles.
 */

typedef struct CQUEUE_ {
    /*
     * Cell release function, and queue clientdata.
     */

    CQUEUE_CELL_FREE freeCell; 
    void*            clientData;

    /*
     * The queue is managed via several stacks holding the elements added at
     * the two sides and the middle.
     *
     * The logical organization and ordering of the stacks making up the queue
     * is shown below. The arrows point in the logical direction of growth,
     * i.e. from bottom to top.
     *
     * |HEAD..| |MIDDLE| |TAIL..|
     * |<-----| |----->| |----->|
     * |T    B| |B  ^ T| |B    T|
     *              AT - index into the middle.
     *
     * The HEAD contains all elements "prepend"ed to the front and not yet
     * removed from the front ("(remove|drop)_head").
     *
     * The TAIL contains all elements "append"ed to the end and not yet
     * removed from the end ("(remove|drop)_tail").
     *
     * The MIDDLE is filled from the TAIL when either HEAD or MIDDLE run out
     * of data taken from the front. To make removal fast (1) the index AT is
     * used to access it. It holds the location of the _next_ element to
     * return. Index 0 is bottom.
     *
     * (Ad 1): In the naive implementation we would shift the array of
     * elements down after removing from the bottom, making this an O(n)
     * operation, and repeated use O(n**2). With the index we avoid that, and
     * defer the actual release of cells to when the MIDDLE has been processed
     * completely.
     */

    CSTACK   head;
    CSTACK   middle;
    CSTACK   tail;
    long int at;

    /*
     * This field holds a stack slated for destruction, defering actual action
     * in the hope of reusing it again for one of the other stacks above.
     */

    CSTACK   hold;

} CQUEUE_;

/*
 * General utilities
 */

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

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
