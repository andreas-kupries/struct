# stackc.tcl --
#
#       Implementation of a stack data structure for Tcl.
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
package require critcl 3.1

critcl::buildrequirement {
    package require critcl::class ; # DSL, easy spec of Tcl class/object commands.
}

# # ## ### ##### ######## ############# #####################
## Administrivia

critcl::license {Andreas Kupries} {BSD licensed}

critcl::summary {Stack objects for Tcl.}

critcl::description {
    This package implements stack objects for Tcl. It uses the
    abstract data type provided by package 'cstack' to handle actual
    storage and operations.
}

critcl::subject stack
critcl::subject {data structure}
critcl::subject structure
critcl::subject {abstract data structure}
critcl::subject {generic data structure}

critcl::api import c::slice 1
critcl::api import c::stack 1

# # ## ### ##### ######## ############# #####################
## Implementation

critcl::argtype stacksize {
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

critcl::class::define ::struct::stack {
    # # ## ### ##### ######## ############# #####################

    include c_slice/c_sliceDecls.h ; # API of the generic CSLICE used by CSTACK.
    include c_stack/c_stackDecls.h ; # API of the generic CSTACK we are binding to.
    type    CSTACK

    support {
	/* * ** *** ***** ******** ************* ********************* */
	/* Cell liefcycle, release */

	static void
	StructStackC_FreeCell (void* cell) {
	    Tcl_DecrRefCount ((Tcl_Obj*) cell);
	}

	/* * ** *** ***** ******** ************* ********************* */
	/* Common code for peek, peekr, and pop */

	static int
	StructStackC_GetN (CSTACK instance, Tcl_Interp* interp,
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

	    if (*n > cstack_size (instance)) {
		Tcl_AppendResult (interp,
			  "insufficient items on stack to fulfill request",
			  NULL);
		return TCL_ERROR;
	    }

	    return TCL_OK;
	}

	static Tcl_Obj*
	StructStackC_Elements (CSTACK instance, int n, CSLICE_DIRECTION dir) {
	    CSLICE s = cstack_get (instance, n, dir);
	    void** cells;
	    long int ln;
	    Tcl_Obj* result;

	    cslice_get (s, &cells, &ln);
	    if (n == 1) {
		result = (Tcl_Obj*) cells [0];
	    } else {
		result = Tcl_NewListObj (ln, (Tcl_Obj**) cells);
	    }
	    cslice_delete (s);
	    return result;
	}
    }

    # # ## ### ##### ######## ############# #####################
    ## Lifecycle management.

    constructor {
	instance = cstack_create (StructStackC_FreeCell, 0);
    } {
	/* Set back reference from CSTACK instance to instance command */
	cstack_clientdata_set (instance, (ClientData) cmd);
    }

    destructor {
	/* Release the whole stack. */
	cstack_destroy (instance);
    }

    method destroy proc {} void {
	Tcl_DeleteCommandFromToken(interp, (Tcl_Command) cstack_clientdata_get (instance));
    }

    # # ## ### ##### ######## ############# #####################

    method size proc {} int {
	return cstack_size (instance);
    }

    method get proc {} Tcl_Obj* {
	Tcl_Obj* result;
	int      n = cstack_size (instance);

	if (!n) {
	    return Tcl_NewListObj (0,NULL);
	} else {
	    return StructStackC_Elements (instance, n, cslice_normal);
	}
    }

    method getr proc {} Tcl_Obj* {
	Tcl_Obj* result;
	int n = cstack_size (instance);

	if (!n) {
	    return Tcl_NewListObj (0,NULL);
	} else {
	    return StructStackC_Elements (instance, n, cslice_revers);
	}
    }

    method peek command {?n?} {
	int n = 1;

	if (StructStackC_GetN (instance, interp, objc, objv, &n) != TCL_OK) {
	    return TCL_ERROR;
	}

	Tcl_SetObjResult (interp, StructStackC_Elements (instance, n, cslice_normal));
	return TCL_OK;
    }

    method peekr command {?n?} {
	int n = 1;

	if (StructStackC_GetN (instance, interp, objc, objv, &n) != TCL_OK) {
	    return TCL_ERROR;
	}

	Tcl_SetObjResult (interp, StructStackC_Elements (instance, n, cslice_revers));
	return TCL_OK;
    }

    method pop command {?n?} {
	int n = 1;

	if (StructStackC_GetN (instance, interp, objc, objv, &n) != TCL_OK) {
	    return TCL_ERROR;
	}

	Tcl_SetObjResult (interp, StructStackC_Elements (instance, n, cslice_normal));
	cstack_pop (instance, n);
	return TCL_OK;
    }

    # # ## ### ##### ######## ############# #####################

    method clear proc {} void {
	cstack_pop (instance, cstack_size (instance));
    }

    method push command {item ?item...?} {
	int i;

	if (objc < 3) {
	    Tcl_WrongNumArgs (interp, 2, objv, "item ?item ...?");
	    return TCL_ERROR;
	}

	for (i = 2; i < objc; i++) {
	    cstack_push (instance, objv[i]);
	    Tcl_IncrRefCount (objv[i]);
	}

	return TCL_OK;
    }

    method rotate proc {int n int steps} ok {
	if (n > cstack_size (instance)) {
	    Tcl_AppendResult (interp,
		      "insufficient items on stack to perform request",
		      NULL);
	    return TCL_ERROR;
	}

	cstack_rol (instance, n, steps);
	return TCL_OK;
    }

    method trim proc {stacksize n} Tcl_Obj* {
	int len = cstack_size (instance);

	if (n < len) {
	    Tcl_Obj* result = StructStackC_Elements (instance, len-n, cslice_normal);
	    cstack_trim (instance, n);
	    return result;
	}

	return Tcl_NewListObj (0,NULL);
    }

    method trim* proc {stacksize n} void {
	if (n < cstack_size (instance)) {
	    cstack_trim (instance, n);
	}
    }

    # # ## ### ##### ######## ############# #####################
}

# # ## ### ##### ######## ############# #####################
## Ready
package provide struct::stack 2
