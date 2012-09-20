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
package require critcl 3.1.2

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

critcl::api import c::index 1
critcl::api import c::slice 1
critcl::api import c::queue 1

critcl::tsources policy.tcl

# # ## ### ##### ######## ############# #####################
## Implementation

# qgetsize:   integer,    > 0
# queuesize:  integer,    >= 0
# queueindex: list index, >= 0, <  cqueue_size(s)
# queuecount: integer,    >  0, <= cqueue_size(s) [checked in proc bodies!]

critcl::argtype qgetsize {
    if ((Tcl_GetIntFromObj (interp, @@, &@A) != TCL_OK) || (@A < 1)) {
	Tcl_ResetResult  (interp);
	Tcl_AppendResult (interp, "expected positive integer but got \"",
			  Tcl_GetString (@@), "\"", NULL);
	return TCL_ERROR;
    }
} int int

critcl::argtype queuesize {
    if ((Tcl_GetIntFromObj (interp, @@, &@A) != TCL_OK) || (@A < 0)) {
	Tcl_ResetResult  (interp);
	Tcl_AppendResult (interp, "expected non-negative integer but got \"",
			  Tcl_GetString (@@), "\"", NULL);
	return TCL_ERROR;
    }
} int int

critcl::argtype queueindex {
    int n = cqueue_size ((CQUEUE) cd);
    if (cindex_get (interp, @@, n-1, &@A) != TCL_OK) {
	return TCL_ERROR;
    }
    if ((@A < 0) || (n <= @A)) {
	char buf [20];
	sprintf (buf, "%d", @A);
	Tcl_AppendResult (interp, "invalid index \"",
			  buf, "\"", NULL);
	return TCL_ERROR;
    }
} int int

critcl::argtype queuecount {
    if ((Tcl_GetIntFromObj(interp, @@, &@A) != TCL_OK) ||
	(@A < 1)) {
	Tcl_ResetResult  (interp);
	Tcl_AppendResult (interp, "expected positive integer but got \"",
			  Tcl_GetString (@@), "\"", NULL);
	return TCL_ERROR;
    }
    /* Check of size overrun is done in using proc bodies!
     * Limitation of cproc default handling.
     */
} int int

critcl::argtype where {
    const char* where[] = { "head", "tail", NULL }; 
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
	/* Common code for head, tail, and pop */

	static Tcl_Obj*
	StructQueueC_Head (CQUEUE instance, int n) {
	    if (n == 1) {
		return cqueue_first (instance);
	    } else {
		CSLICE   s      = cqueue_head (instance, n);
		Tcl_Obj* result = cslice_to_list (s);
		cslice_destroy (s);
		return result;
	    }
	}

	static Tcl_Obj*
	StructQueueC_Tail (CQUEUE instance, int n) {
	    if (n == 1) {
		return cqueue_last (instance);
	    } else {
		CSLICE   s      = cqueue_tail (instance, n);
		Tcl_Obj* result = cslice_to_list (s);
		cslice_destroy (s);
		return result;
	    }
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

    method head proc {queuecount {n 1}} ok {
	if (n > cqueue_size (instance)) {
	    Tcl_AppendResult (interp, "not enough elements", NULL);
	    return TCL_ERROR;
	}

	Tcl_SetObjResult (interp, StructQueueC_Head (instance, n));
	return TCL_OK;
    }

    method tail proc {queuecount {n 1}} ok {
	if (n > cqueue_size (instance)) {
	    Tcl_AppendResult (interp, "not enough elements", NULL);
	    return TCL_ERROR;
	}

	Tcl_SetObjResult (interp, StructQueueC_Tail (instance, n));
	return TCL_OK;
    }

    method get proc {queueindex at qgetsize n} sTcl_Obj* {
	if ((at+n) > cqueue_size (instance)) {
	    Tcl_AppendResult (interp, "not enough elements", NULL);
	    return 0;
	} else {
	    CSLICE s        = cqueue_get (instance, at, n);
	    Tcl_Obj* result = cslice_to_list (s);
	    cslice_destroy (s);
	    return result;
	}
    }

    # # ## ### ##### ######## ############# #####################

    method clear proc {} void {
	cqueue_clear (instance);
    }

    method append command {item ?item ...?} {
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

	sl = cslice_create (objc-2, (void**) objv+2);
	cqueue_append_slice (instance, sl);

	for (i = 2; i < objc; i++) {
	    Tcl_IncrRefCount (objv[i]);
	}

	cslice_destroy (sl);
	return TCL_OK;
    }

    method prepend command {item ?item ...?} {
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

	sl = cslice_create (objc-2, (void**) objv+2);
	cqueue_prepend_slice (instance, sl);

	for (i = 2; i < objc; i++) {
	    Tcl_IncrRefCount (objv[i]);
	}

	cslice_destroy (sl);
	return TCL_OK;
    }

    method pop proc {where istail queuecount {n 1}} sTcl_Obj* {
	Tcl_Obj* result;

	if (n > cqueue_size (instance)) {
	    Tcl_AppendResult (interp, "not enough elements", NULL);
	    return TCL_ERROR;
	}

	if (istail) {
	    result = StructQueueC_Tail (instance, n);
	    cqueue_remove_tail (instance, n);
	} else {
	    result = StructQueueC_Head (instance, n);
	    cqueue_remove_head (instance, n);
	}
	return result;
    }

    # # ## ### ##### ######## ############# #####################
}

# # ## ### ##### ######## ############# #####################
## Ready
package provide struct::queue 2
