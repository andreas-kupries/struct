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
package require critcl 3.1.2

critcl::buildrequirement {
    package require critcl::class 1.0.3 ; # DSL, easy spec of Tcl class/object commands.
}

# # ## ### ##### ######## ############# #####################
## Administrivia

critcl::license {Andreas Kupries} {BSD licensed}

critcl::summary {Stack objects for Tcl.}

critcl::description {
    This package implements stack objects for Tcl. It uses the abstract
    data type provided by package 'c::stack' to handle actual storage
    and operations.
}

critcl::subject stack lifo
critcl::subject {data structure}
critcl::subject structure
critcl::subject {abstract data structure}
critcl::subject {generic data structure}

critcl::api import c::index 1
critcl::api import c::slice 1
critcl::api import c::stack 1

critcl::tsources policy.tcl

# # ## ### ##### ######## ############# #####################
## Implementation - Class Helpers - Custom argument/result processing.

# stacksize:  integer,    >= 0
# stackindex: list index, >= 0, <  cstack_size (s)
# stackcount: integer,    >  0, <= cstack_size (s)

critcl::argtype stacksize {
    if ((Tcl_GetIntFromObj (interp, @@, &@A) != TCL_OK) || (@A < 0)) {
	Tcl_ResetResult  (interp);
	Tcl_AppendResult (interp, "expected non-negative integer but got \"",
			  Tcl_GetString (@@), "\"", NULL);
	return TCL_ERROR;
    }
} int int

critcl::argtype stackindex {
    int n = cstack_size ((CSTACK) cd);
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

critcl::argtype stackcountfull {
    if ((Tcl_GetIntFromObj(interp, @@, &@A) != TCL_OK) ||
	(@A < 1)) {
	Tcl_ResetResult  (interp);
	Tcl_AppendResult (interp, "expected positive integer but got \"",
			  Tcl_GetString (@@), "\"", NULL);
	return TCL_ERROR;
    }
    if (@A > cstack_size ((CSTACK) cd)) {
	Tcl_AppendResult (interp, "not enough elements", NULL);
	return TCL_ERROR;
    }
} int int

critcl::argtype stackcount {
    if ((Tcl_GetIntFromObj(interp, @@, &@A) != TCL_OK) ||
	(@A < 1)) {
	Tcl_ResetResult  (interp);
	Tcl_AppendResult (interp, "expected positive integer but got \"",
			  Tcl_GetString (@@), "\"", NULL);
	return TCL_ERROR;
    }
    /* Check for size overrun is done in the using proc body.
     * This here cannot check the default value.
     */
} int int

# Custom definition.
critcl::resulttype sTcl_Obj* {
    if (rv == NULL) { return TCL_ERROR; }
    Tcl_SetObjResult(interp, rv);
    /* No refcount adjustment */
    return TCL_OK;
} Tcl_Obj*

# # ## ### ##### ######## ############# #####################
## Implementation - Class

critcl::class::define ::struct::stack {
    # # ## ### ##### ######## ############# #####################

    include c_slice/c_sliceDecls.h ; # API of the generic CSLICE used by CSTACK.
    include c_stack/c_stackDecls.h ; # API of the generic CSTACK we are binding to.
    type    CSTACK

    support {
	/* * ** *** ***** ******** ************* ********************* */
	/* Cell lifecycle, release */

	static void
	StructStackC_FreeCell (void* cell) {
	    Tcl_DecrRefCount ((Tcl_Obj*) cell);
	}

	/* * ** *** ***** ******** ************* ********************* */
	/* Common code for top, bottom, and pop */

	static Tcl_Obj*
	StructStackC_Top (CSTACK instance, int n) {
	    if (n == 1) {
		return cstack_top (instance);
	    } else {
		Tcl_Obj* result;
		CSLICE s = cstack_get (instance, n-1, n);
		s = cslice_reverse (s);
		result = cslice_to_list (s);
		cslice_destroy (s);
		return result;
	    }
	}

	static Tcl_Obj*
	StructStackC_Bottom (CSTACK instance, int n) {
	    if (n == 1) {
		return cstack_bottom (instance);
	    } else {
		Tcl_Obj* result;
		CSLICE s = cstack_get (instance, cstack_size(instance)-1, n);
		s = cslice_reverse (s);
		result = cslice_to_list (s);
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

    method at proc {stackindex at} sTcl_Obj* {
	return cstack_at (instance, at);
    }

    method get proc {} sTcl_Obj* {
	int n = cstack_size (instance);
	if (!n) {
	    return Tcl_NewListObj (0,NULL);
	} else {
	    Tcl_Obj* result;
	    CSLICE s = cstack_get (instance, n-1, n);
	    s = cslice_reverse (s);
	    result = cslice_to_list (s);
	    cslice_destroy (s);
	    return result;
	}
    }

    method top proc {stackcount {n 1}} ok {
	if (n > cstack_size (instance)) {
	    Tcl_AppendResult (interp, "not enough elements", NULL);
	    return TCL_ERROR;
	}

	Tcl_SetObjResult (interp, StructStackC_Top (instance, n));
	return TCL_OK;
    }

    method bottom proc {stackcount {n 1}} ok {
	if (n > cstack_size (instance)) {
	    Tcl_AppendResult (interp, "not enough elements", NULL);
	    return TCL_ERROR;
	}

	Tcl_SetObjResult (interp, StructStackC_Bottom (instance, n));
	return TCL_OK;
    }

    method pop proc {stackcount {n 1}} ok {
	if (n > cstack_size (instance)) {
	    Tcl_AppendResult (interp, "not enough elements", NULL);
	    return TCL_ERROR;
	}

	Tcl_SetObjResult (interp, StructStackC_Top (instance, n));
	cstack_pop (instance, n);
	return TCL_OK;
    }

    # # ## ### ##### ######## ############# #####################

    method clear proc {} void {
	cstack_clear (instance);
    }

    method push command {item ?item ...?} {
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

    method rotate proc {stackcountfull n int steps} ok {
	cstack_rol (instance, n, steps);
	return TCL_OK;
    }

    method trim proc {stacksize n} sTcl_Obj* {
	int len = cstack_size (instance);
	if (n >= len) {
	    return Tcl_NewListObj (0,NULL);
	} else {
	    Tcl_Obj* result;
	    CSLICE s = cstack_get (instance, len-n-1, len-n);
	    s = cslice_reverse (s);
	    result = cslice_to_list (s);
	    cslice_destroy (s);
	    cstack_trim (instance, n);
	    return result;
	}
    }

    method trim* proc {stacksize n} void {
	if (n >= cstack_size (instance)) return ;
	cstack_trim (instance, n);
    }

    # # ## ### ##### ######## ############# #####################
}

# # ## ### ##### ######## ############# #####################
## Ready
package provide struct::stack 2
