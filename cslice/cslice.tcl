# cslice.tcl --
#
#	Low-level 'slice of cells' data structure. A supporting data
#	structure for stacks, queues, and any other data structure
#	making use of arrays of cells. The slice represents a part of
#	such arrays.
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

critcl::subject {slice of array} {array slice}
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
# - Create and initialize a slice.
# - Dispose of a slice.
# - Access the data in the slice.
# - Reverse the elements in a slice.
# - Concatenate 2 slices.
# - Convenience functions: To and from Tcl "list" Tcl_Obj.

critcl::api function CSLICE     cslice_create    {{long int} cc void** cv}
critcl::api function void       cslice_destroy   {CSLICE s}
critcl::api function void       cslice_get       {CSLICE s {long int *} cc void*** cv}
critcl::api function CSLICE     cslice_reverse   {CSLICE s}
critcl::api function CSLICE     cslice_concat    {CSLICE a CSLICE b}
critcl::api function CSLICE     cslice_from_list {Tcl_Interp* interp Tcl_Obj* l}
critcl::api function Tcl_Obj*   cslice_to_list   {CSLICE s}
critcl::api function {long int} cslice_size      {CSLICE s}
critcl::api function void*      cslice_at        {CSLICE s {long int} at}

# # ## ### ##### ######## ############# #####################
## Implementation. Inlined.

critcl::ccode {
    #include <csliceInt.h>
    #include <string.h>

    CSLICE
    cslice_create ( long int cc,
		    void**   cv )
    {
	CSLICE s = ALLOC (CSLICE_);
	s->n = cc;

	/* Initial slices are always in the same direction as the
	 * input array. We simply copy the pointer, and remember
	 * that it is not allocated.
	 */

	s->cell    = cv;
	s->dynamic = 0;
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
		 long int* cc,
		 void***   cv )
    {
	*cc = s->n;
	*cv = s->cell;
    }

    CSLICE
    cslice_reverse (CSLICE s)
    {
	long int i;

	if (s->n == 1) {
	    /* A single-element slice reversed is itself. */
	    return s;
	}

	if (s->dynamic) {
	    for (i=0; i < (s->n)/2; i++) {
		  void* tmp = s->cell [i];
		  s->cell [i] = s->cell [s->n - i - 1];
		  s->cell [s->n - i - 1] = tmp;
	    }
	} {
	    void** origin = s->cell;

	    s->dynamic = 1;
	    s->cell = NALLOC (s->n, void*);
	    ASSERT (s->cell, "Allocation failure");

	    for (i=0; i < s->n; i++) {
		 s->cell [i] = origin [s->n - i -1];
	     }
	}

	return s;
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
	    /* Make it dynamic, already in larger size */
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

    CSLICE
    cslice_from_list (Tcl_Interp* interp, Tcl_Obj* l)
    {
	int lc;
	Tcl_Obj** lv;

	if (Tcl_ListObjGetElements (interp, l, &lc, &lv) != TCL_OK) {
	    return 0;
	} else {
	    return cslice_create (lc, lv);
	}
    }

    Tcl_Obj*
    cslice_to_list (CSLICE s)
    {
	return Tcl_NewListObj (s->n, (Tcl_Obj**) s->cell);
    }

    long int
    cslice_size (CSLICE s)
    {
	return s->n;
    }

    void*
    cslice_at (CSLICE s, long int at)
    {
	ASSERT_BOUNDS (at, s->n);
	return s->cell [at];
    }
}

# ### ### ### ######### ######### #########
## Ready
package provide c::slice 1
