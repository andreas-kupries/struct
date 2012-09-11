# cslice.tcl --
#
#	Low-level slice data structure. Supporting data structure for
#	stacks, queues, and any other data structure making use of
#	arrays of cells. The slice represents a part of such arrays.
#
# Copyright (c) 2012 Andreas Kupries <andreas_kupries@users.sourceforge.net>

# # ## ### ##### ######## ############# #####################
## Requirements

package require Tcl 8.5
package require critcl 3 ;# stubs management

# # ## ### ##### ######## ############# #####################
## Administrivia

critcl::license {Andreas Kupries} BSD

critcl::summary {A C-level abstract datatype for slices}

critcl::description {
    This package implements an abstract data type for slices of cells,
    i.e. continuous parts of arrays, at the C-level.  No Tcl-binding
    is provided. It is a supporting data structure for others, for
    example stacks, queues, etc.
}

critcl::subject slice
critcl::subject {data structure}
critcl::subject structure
critcl::subject {abstract data structure}
critcl::subject {generic data structure}

# # ## ### ##### ######## ############# #####################
## Configuration

critcl::api header cslice.h
critcl::cheaders   csliceInt.h

# # ## ### ##### ######## ############# #####################
## Exported API
#
# - Create and initialize a slice. May copy the data.
# - Dispose of a slice.
# - Access the data in the slice.

critcl::api function CSLICE cslice_create {
    CSLICE_DIRECTION dir
    void**           cells
    {long int}       n
}

critcl::api function void cslice_destroy {
    CSLICE s
}

critcl::api function void cslice_get {
    CSLICE       s
    void***      cell
    {long int *} n
}

critcl::api function CSLICE cslice_concat {
    CSLICE a
    CSLICE b
}

# # ## ### ##### ######## ############# #####################
## Implementation. Inlined.

critcl::ccode {
    #include <csliceInt.h>

    CSLICE
    cslice_create ( CSLICE_DIRECTION dir,
		    void**           cells,
		    long int         n)
    {
	CSLICE s = ALLOC (CSLICE_);
	s->n = n;

	if (dir == cslice_normal) {
	    /* The slice is in the same direction as the input array.
	     * We simply copy the pointer, and remember that it is not
	     * allocated.
	     */

	    s->cell    = cells;
	    s->dynamic = 0;

	} else if (dir == cslice_revers) {
	    /* For the reverse direction we have make our own copy of
	     * the input. Can't use memcpy :(
	     * Is there a standard reverse memcpy ?
	     */

	    long int i;

	    s->cell = NALLOC (n, void*);
	    s->dynamic = 1;

	    for (i=0; i<n; i++) {
		 s->cell [i] = cells [n-i-1];
	     }
	} else {
	    ASSERT (0,"Bad slice direction");
	}

	return s;
    }

    void
    cslice_destroy (CSLICE s)
    {
	if (s->dynamic) {
	    ckfree ((char*) s->cell);
	}
	ckfree ((char*) s);
    }

    void
    cslice_get ( CSLICE    s,
		 void***   cells,
		 long int* n)
    {
	*cells = s->cell;
	*n     = s->n;
    }

    CSLICE
    cslice_concat (CSLICE a, CSLICE b)
    {
	long int n = a->n + b->n;

	if (a->dynamic) {
	    /* Extend a with b, destroy only b. */
	    void** new = NREALLOC (n, void*, a->cell);
	    ASSERT (new, "Reallocation failure");
	    a->cell = new;
	} else {
	    /* Make a dynamic, already in larger size */
	    void** new = NALLOC (n, void*);
	    ASSERT (new, "Allocation failure");
	    memcpy (new, a->cell, a->n * sizeof(void*));
	    a->cell = new;
	}

	memcpy (a->cell + a->n, b->cell, b->n * sizeof(void*));
	a->dynamic = 1;
	a->n = n;

	cslice_destroy (b);
	return a;
    }
}

# ### ### ### ######### ######### #########
## Ready
package provide c::slice 1
