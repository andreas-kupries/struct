# queuec.tcl --
#
#       Implementation of a queue data structure for Tcl.
#       This code based on critcl v3.1
#	API compatible with the implementation in Tcl (8.5+OO)
#
# Copyright (c) 2012 Andreas Kupries <andreas_kupries@users.sourceforge.net>
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.

# # ## ### ##### ######## ############# #####################
## Requisites

package require Tcl 8.5
package require critcl 3.1.1

critcl::buildrequirement {
    package require critcl::class 1.0.3 ; # DSL, easy spec of Tcl class/object commands.
}

# # ## ### ##### ######## ############# #####################
## Administrivia

critcl::license {Andreas Kupries} {BSD licensed}

critcl::summary {Queue objects for Tcl.}

critcl::description {
    This package implements queue objects for Tcl. It uses the abstract
    data type provided by package 'c::queue' to handle actual storage
    and operations.
}

critcl::subject {double-sided queue}
critcl::subject {double-sided fifo}
critcl::subject queue fifo stack lifo
critcl::subject {data structure}
critcl::subject structure
critcl::subject {abstract data structure}
critcl::subject {generic data structure}

critcl::api import c::slice 1
critcl::api import c::queue 1

critcl::tsources policy.tcl

# # ## ### ##### ######## ############# #####################
## Implementation

critcl::argtype queuesize {
    if (Tcl_GetIntFromObj (interp, @@, &@A) != TCL_OK) {
	return TCL_ERROR;
    }
    if (@A < 0) {
	Tcl_AppendResult (interp, "invalid size ",
			  Tcl_GetString (@@),
			  NULL);
	return TCL_ERROR;
    }
} int int

critcl::argtype queueindex {
    if (Tcl_GetIntFromObj (interp, @@, &@A) != TCL_OK) {
	return TCL_ERROR;
    }
    if ((@A < 0) || (cqueue_size ((CQUEUE) cd) <= @A)) {
	Tcl_AppendResult (interp, "invalid index ",
			  Tcl_GetString (@@),
			  NULL);
	return TCL_ERROR;
    }
} int int

critcl::argtype where {
    const char* where = { "head", "tail", NULL }; 
    if (Tcl_GetIndexFromObj (interp, @@, where, "location", TCL_EXACT, &@A) != TCL_OK) {
	return TCL_ERROR;
    }
} int int

# Custom definition.
critcl::resulttype sTcl_Obj* {
    if (rv == NULL) { return TCL_ERROR; }
    Tcl_SetObjResult(interp, rv);
    /* No refcount adjustment */
    return TCL_OK;
} Tcl_Obj*

# ### ### ### ######### ######### #########
## Ready

critcl::class::define ::struct::queue {
    # # ## ### ##### ######## ############# #####################

    include c_slice/c_sliceDecls.h ; # API of the generic CSLICE used by CQUEUE.
    include c_queue/c_queueDecls.h ; # API of the generic CQUEUE we are binding to.
    type    CQUEUE

    support {
	/* * ** *** ***** ******** ************* ********************* */
	/* Cell lifecycle, release */

	static void
	StructQueueC_FreeCell (void* cell) {
	    Tcl_DecrRefCount ((Tcl_Obj*) cell);
	}

	/* * ** *** ***** ******** ************* ********************* */
	/* Common code for peek, peekr, and pop */

	static int
	StructQueueC_GetN (CQUEUE instance, Tcl_Interp* interp,
			   int objc, Tcl_Obj*const* objv, int* n) {

	    if ((objc != 2) && (objc != 3)) {
		Tcl_WrongNumArgs (interp, 2, objv, "?n?");
		return TCL_ERROR;
	    }

	    if (objc == 3) {
		if (Tcl_GetIntFromObj(interp, objv[2], n) != TCL_OK) {
		    return TCL_ERROR;
		} else if (*n < 1) {
		    Tcl_AppendResult (interp, "invalid item count ",
				      Tcl_GetString (objv[2]),
				      NULL);
		    return TCL_ERROR;
		}
	    }

	    if (*n > cqueue_size (instance)) {
		Tcl_AppendResult (interp,
			  "insufficient items on queue to fulfill request",
			  NULL);
		return TCL_ERROR;
	    }

	    return TCL_OK;
	}

	static Tcl_Obj*
	StructQueueC_Elements (CQUEUE instance, int n, int reverse) {
	    CSLICE s = cqueue_get (instance, n-1, n);
	    void** cells;
	    long int ln;
	    Tcl_Obj* result;

	    if (reverse) s = cslice_reverse (s);

	    cslice_get (s, &ln, &cells);
	    if (n == 1) {
		result = (Tcl_Obj*) cells [0];
	    } else {
		result = Tcl_NewListObj (ln, (Tcl_Obj**) cells);
	    }
	    cslice_destroy (s);
	    return result;
	}
    }

    # # ## ### ##### ######## ############# #####################
    ## Lifecycle management.

    constructor {
        if (objc > 0) {
	    Tcl_WrongNumArgs (interp, objcskip, objv-objcskip, NULL);
	    goto error;
        }

	instance = cqueue_create (StructQueueC_FreeCell, 0);
    } {
	/* Set back reference from CQUEUE instance to instance command */
	cqueue_clientdata_set (instance, (ClientData) cmd);
    }

    destructor {
	/* Release the whole queue. */
	cqueue_destroy (instance);
    }

    method destroy proc {} void {
	Tcl_DeleteCommandFromToken(interp, (Tcl_Command) cqueue_clientdata_get (instance));
    }

    # # ## ### ##### ######## ############# #####################

    method size proc {} int {
	return cqueue_size (instance);
    }

    method first proc {} sTcl_Obj* {
	// xxx todo size check
	return cqueue_first (instance);
    }

    method last proc {} sTcl_Obj* {
	// xxx todo size check
	return cqueue_last (instance);
    }

    method head proc {queueindex n} sTcl_Obj* {
    }

    method tail proc {queueindex n} sTcl_Obj* {
    }

    method get proc {queueindex at int n} sTcl_Obj* {
    }

    # # ## ### ##### ######## ############# #####################

    method append command {item ?item ...?} {
	// xxx todo - we can use slices...

	int i;
	CSLICE sl;

	if (objc < 3) {
	    Tcl_WrongNumArgs (interp, 2, objv, "item ...");
	    return TCL_ERROR;
	}

	/* Even with a slice to bulk-push
	 * we still need a loop to fix the
	 * ref counts proper.
	 */

	sl = cslice_create (objc-2, objv+2);
	cstack_push_slice (instance, sl);

	for (i = 2; i < objc; i++) {
	    Tcl_IncrRefCount (objv[i]);
	}

	cslice_destroy (sl);
	return TCL_OK;

cqueue_
    }

    method prepend command {item ?item ...?} {
	// xxx todo - we can use slices...


	int i;
	CSLICE sl;

	if (objc < 3) {
	    Tcl_WrongNumArgs (interp, 2, objv, "item ...");
	    return TCL_ERROR;
	}

	/* Even with a slice to bulk-push
	 * we still need a loop to fix the
	 * ref counts proper.
	 */

	sl = cslice_create (objc-2, objv+2);
	cstack_push_slice (instance, sl);

	for (i = 2; i < objc; i++) {
	    Tcl_IncrRefCount (objv[i]);
	}

	cslice_destroy (sl);
	return TCL_OK;

    }

    # stack: pop ... no result! bad!
    method remove proc {where istail queueindex n} void {
	if (istail) {
	    cqueue_remove_tail (instance, n);
	} else {
	    cqueue_remove_head (instance, n);
	}
    }

    method clear proc {} void {
	cqueue_clear (instance);
    }

    # # ## ### ##### ######## ############# #####################
}

# # ## ### ##### ######## ############# #####################
## Ready
package provide struct::queue 2
