#ifndef CSET_INT_H
#define CSET_INT_H 1

#include "c_set/c_setDecls.h"
#include "tree.h"


/*
 * Actual type of the set data structure.
 * Used only inside of the package.
 * The outside only sees opaque CSET handles.
 */

typedef struct CSET_ {
    /*
     * Cell release function, and set clientdata.
     */

    CSET_CELL_DUP dup;
    CSET_CELL_REL release;
    CSET_CELL_CMP compare;
    void const*   clientData;

    /*
     * The set is managed through a binary tree whose keys are the elements.
     */

    jsw_tree_t* set;
} CSET_;

/*
 * General utilities
 */

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#define SWAP(a,b) { void* tmp = a ; a = b ; b = tmp; }


/*
 * Allocation macros for common situations.
 */

#define ALLOC(type)    (type *) ckalloc (sizeof (type))
#define NALLOC(n,type) (type *) ckalloc ((n) * sizeof (type))

/*
 * Assertions in general, and asserting the proper range of an array
 * index.
 */

#undef  CSET_DEBUG
#define CSET_DEBUG 1

#ifdef CSET_DEBUG
#define XSTR(x) #x
#define STR(x) XSTR(x)
#define RANGEOK(i,n) ((0 <= (i)) && (i < (n)))
#define ASSERT(x,msg) if (!(x)) { Tcl_Panic (msg " (" #x "), in file " __FILE__ " @line " STR(__LINE__));}
#define ASSERT_BOUNDS(i,n) ASSERT (RANGEOK(i,n),"array index out of bounds: " STR(i) " > " STR(n))
#else
#define ASSERT(x,msg)
#define ASSERT_BOUNDS(i,n)
#endif

#endif /* CSET_INT_H */

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
