# setc.tcl --
#
#       Implementation of set operations for Tcl (values, not objects).
#       This code based on critcl v3.1
#	API compatible with the implementation in Tcl (8.5+
#
# Copyright (c) 2012 Andreas Kupries <andreas_kupries@users.sourceforge.net>
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.

# # ## ### ##### ######## ############# #####################
## Requisites

package require Tcl 8.5
package require critcl 3.1.2

# # ## ### ##### ######## ############# #####################
## Administrivia

critcl::license {Andreas Kupries} {BSD licensed}

critcl::summary {Set operations for Tcl.}

critcl::description {
    This package implements set operations for Tcl. It uses the abstract
    data type provided by package 'c::set' to handle actual storage
    and operations. The abstract datatype is wrapped into a custom
    Tcl_ObjType.
}

critcl::subject set lot bag union intersect difference
critcl::subject {data structure} {symmetric difference} {include element}
critcl::subject structure {exclude element}
critcl::subject {abstract data structure}
critcl::subject {generic data structure}

critcl::api import c::slice 1
critcl::api import c::set   1

critcl::tsources policy.tcl

# # ## ### ##### ######## ############# #####################
## Tcl_ObjType for CSET.

critcl::csources objtype.c
critcl::cheaders objtype.h
critcl::ccode {#include <objtype.h>}

critcl::argtype CSET {
    if (setc_get (interp, @@, &@A) != TCL_OK) { return TCL_ERROR; }
}

critcl::resulttype CSET {
    Tcl_SetObjResult (interp, setc_new (rv));
    return TCL_OK;
}

# # ## ### ##### ######## ############# #####################
## API. Functional. Sideeffect-free.

critcl::cproc ::struct::set::contains {CSET s Tcl_Obj* element} boolean {
    return cset_contains (s, element);
}

critcl::ccommand ::struct::set::create {} {
    /* Syntax: create item... */

    CSET s = cset_create (setc_dup, setc_free, setc_compare, 0);
    int i;

    /* TODO: convenience function list -> set */
    for (i=1;i<objc;i++) {
	  cset_vadd (s, objv[i]);
    }

    Tcl_SetObjResult (interp, setc_new (s));
    return TCL_OK;
}

critcl::ccommand ::struct::set::difference {} {
    /* Syntax: difference A B... */

    int i;
    CSET a, b;

    if (objc < 2) {
	Tcl_WrongNumArgs (interp, 1, objv, "s ...");
	return TCL_ERROR;
    }

    for (i = 1; i < objc; i++) {
	if (setc_get (interp, objv[i], &a) != TCL_OK) {
	    return TCL_ERROR;
	}
    }

    setc_get (interp, objv [1], &a);
    a = cset_dup (a);

    for (i = 2; i < objc; i++) {
	setc_get (interp, objv [i], &b);
	cset_vdifference (a, b);
	if (cset_empty (a)) break;
    }

    Tcl_SetObjResult (interp, setc_new (a));
    return TCL_OK;
}

critcl::cproc ::struct::set::empty {CSET s} int {
    return cset_empty (s);
}

critcl::cproc ::struct::set::equal {CSET a CSET b} boolean {
    return cset_equal (a, b);
}

critcl::ccommand ::struct::set::exclude {} {
    /* Syntax: exclude A item... */

    int i;
    CSET a;

    if (objc < 2) {
	Tcl_WrongNumArgs (interp, 1, objv, "s ...");
	return TCL_ERROR;
    }

    if (setc_get (interp, objv[1], &a) != TCL_OK) {
	return TCL_ERROR;
    }

    a = cset_dup (a);
    for (i = 2; i < objc; i++) {
	cset_vsubtract (a, objv [i]);
	if (cset_empty (a)) break;
    }

    Tcl_SetObjResult (interp, setc_new (a));
    return TCL_OK;
}

critcl::ccommand ::struct::set::include {} {
    /* Syntax: include A item... */

    int i;
    CSET a;

    if (objc < 2) {
	Tcl_WrongNumArgs (interp, 1, objv, "s ...");
	return TCL_ERROR;
    }

    if (setc_get (interp, objv[1], &a) != TCL_OK) {
	return TCL_ERROR;
    }

    a = cset_dup (a);

    for (i = 2; i < objc; i++) {
	cset_vadd (a, objv [i]);
    }

    Tcl_SetObjResult (interp,setc_new (a));
    return TCL_OK;
}

critcl::ccommand ::struct::set::intersect {} {
    /* Syntax: intersect A?... */

    int i;
    CSET a, b;

    if (objc == 1) {
	return TCL_OK;
    }
    if (objc == 2) {
	Tcl_SetObjResult (interp, objv [1] );
	return TCL_OK;
    }

    for (i = 1; i < objc; i++) {
	if (setc_get (interp, objv[i], &a) != TCL_OK) {
	    return TCL_ERROR;
	}
    }

    setc_get (interp, objv [1], &a);
    a = cset_dup (a);

    for (i = 2; i < objc; i++) {
	setc_get (interp, objv [i], &b);
	cset_vintersect (a, b);
	if (cset_empty (a)) break;
    }

    Tcl_SetObjResult (interp, setc_new (a));
    return TCL_OK;
}

critcl::cproc ::struct::set::intersect3 {CSET a CSET b} Tcl_Obj* {
    CSET da = cset_difference (a, b);
    CSET i  = cset_intersect  (a, b);
    CSET db = cset_difference (b, a);
    Tcl_Obj* v [3];
    Tcl_Obj* r;

    v[0] =  setc_new (i);
    v[1] =  setc_new (da);
    v[2] =  setc_new (db);

    r = Tcl_NewListObj (3, v);
    Tcl_IncrRefCount (r);
    return r;
}

critcl::cproc ::struct::set::size {CSET s} int {
    return cset_size (s);
}

critcl::cproc ::struct::set::subset {CSET a CSET b} int {
    return cset_subset (a, b);
}

critcl::cproc ::struct::set::superset {CSET a CSET b} int {
    return cset_superset (a, b);
}

critcl::cproc ::struct::set::symdifference {CSET a CSET b} CSET {
    return cset_symdiff (a, b);
}

critcl::ccommand ::struct::set::union {} {
    /* Syntax: union A?... */

    int i;
    CSET r, b;

    if (objc == 1) {
	return TCL_OK;
    }
    if (objc == 2) {
	Tcl_SetObjResult (interp, objv [1] );
	return TCL_OK;
    }

    for (i = 1; i < objc; i++) {
	if (setc_get (interp, objv[i], &b) != TCL_OK) {
	    return TCL_ERROR;
	}
    }

    r = cset_create (setc_dup, setc_free, setc_compare, 0);
    for (i = 1; i < objc; i++) {
	setc_get (interp, objv [i], &b);
	cset_vunion (r, b);
    }

    Tcl_SetObjResult (interp, setc_new (r));
    return TCL_OK;
}

# # ## ### ##### ######## ############# #####################
## API. Imperative.

critcl::ccommand ::struct::set::add {} {
    /* Syntax: add Svar A?... */

    Tcl_Obj* varname;
    Tcl_Obj* s_boxed;
    CSET     s, b;
    int      i;

    if (objc < 2) {
	Tcl_WrongNumArgs (interp, 1, objv, "Svar ...");
	return TCL_ERROR;
    }

    if (objc == 2) {
	return TCL_OK;
    }

    for (i = 2; i < objc; i++) {
	if (setc_get (interp, objv[i], &b) != TCL_OK) {
	    return TCL_ERROR;
	}
    }

    varname = objv[1];

    s_boxed = Tcl_ObjGetVar2 (interp, varname, 0, 0);

    if (!s_boxed) {
	s       = cset_create (setc_dup, setc_free, setc_compare, 0);
	s_boxed = setc_new (s);

    } else {
	int dup = 0;
	if (Tcl_IsShared (s_boxed)) {
	    s_boxed = Tcl_DuplicateObj (s_boxed);
	    dup = 1;
	}

	if (setc_get (interp, s_boxed, &s) != TCL_OK) {
	    return TCL_ERROR;
	}

	if (dup) {
	    Tcl_InvalidateStringRep (s_boxed);
	}
    }

    for (i = 2; i < objc; i++) {
	setc_get (interp, objv[i], &b);
	cset_vunion (s, b);
    }

    if (!Tcl_ObjSetVar2 (interp, varname, 0, s_boxed, TCL_LEAVE_ERR_MSG)) {
	return TCL_ERROR;
    }

    return TCL_OK;
}

critcl::ccommand ::struct::set::set {} {
    /* Syntax: add Svar item?... */

    Tcl_Obj* varname;
    Tcl_Obj* s_boxed;
    CSET     s;
    int      i, added;

    if (objc < 2) {
	Tcl_WrongNumArgs (interp, 1, objv, "Svar ...");
	return TCL_ERROR;
    }

    if (objc == 2) {
	Tcl_SetObjResult (interp, Tcl_NewIntObj (0));
	return TCL_OK;
    }

    varname = objv[1];

    s_boxed = Tcl_ObjGetVar2 (interp, varname, 0, 0);

    if (!s_boxed) {
	s       = cset_create (setc_dup, setc_free, setc_compare, 0);
	s_boxed = setc_new (s);

    } else {
	int dup = 0;
	if (Tcl_IsShared (s_boxed)) {
	    s_boxed = Tcl_DuplicateObj (s_boxed);
	    dup = 1;
	}

	if (setc_get (interp, s_boxed, &s) != TCL_OK) {
	    return TCL_ERROR;
	}

	if (dup) {
	    Tcl_InvalidateStringRep (s_boxed);
	}
    }

    for (i = 2, added = 0; i < objc; i++) {
	added += cset_vadd (s, objv [i]);
    }

    if (!Tcl_ObjSetVar2 (interp, varname, 0, s_boxed, TCL_LEAVE_ERR_MSG)) {
	return TCL_ERROR;
    }

    Tcl_SetObjResult (interp, Tcl_NewIntObj (added));
    return TCL_OK;
}

critcl::ccommand ::struct::set::subtract {} {
    /* Syntax: subtract Svar A?... */

    Tcl_Obj* varname;
    Tcl_Obj* s_boxed;
    CSET     s, b;
    int      i, dup;

    if (objc < 2) {
	Tcl_WrongNumArgs (interp, 1, objv, "Svar ...");
	return TCL_ERROR;
    }

    if (objc == 2) {
	return TCL_OK;
    }

    varname = objv[1];

    s_boxed = Tcl_ObjGetVar2 (interp, varname, 0, TCL_LEAVE_ERR_MSG);
    if (!s_boxed) {
	return TCL_ERROR;
    }

    for (i = 2; i < objc; i++) {
	if (setc_get (interp, objv[i], &b) != TCL_OK) {
	    return TCL_ERROR;
	}
    }

    dup = 0;
    if (Tcl_IsShared (s_boxed)) {
	s_boxed = Tcl_DuplicateObj (s_boxed);
	dup = 1;
    }

    if (setc_get (interp, s_boxed, &s) != TCL_OK) {
	return TCL_ERROR;
    }

    for (i = 2; i < objc; i++) {
	setc_get (interp, objv[i], &b);
	cset_vdifference (s, b);
	if (cset_empty (s)) break;
    }

    if (!Tcl_ObjSetVar2 (interp, varname, 0, s_boxed, TCL_LEAVE_ERR_MSG)) {
	return TCL_ERROR;
    }

    if (dup) {
	Tcl_InvalidateStringRep (s_boxed);
    }
    return TCL_OK;
}

critcl::ccommand ::struct::set::unset {} {
    /* Syntax: unset Svar item?... */

    Tcl_Obj* varname;
    Tcl_Obj* s_boxed;
    CSET     s;
    int      i, removed, dup;

    if (objc < 2) {
	Tcl_WrongNumArgs (interp, 1, objv, "Svar ...");
	return TCL_ERROR;
    }

    if (objc == 2) {
	Tcl_SetObjResult (interp, Tcl_NewIntObj (0));
	return TCL_OK;
    }

    varname = objv[1];
    s_boxed = Tcl_ObjGetVar2 (interp, varname, 0, TCL_LEAVE_ERR_MSG);
    if (!s_boxed) {
	return TCL_ERROR;
    }

    dup = 0;
    if (Tcl_IsShared (s_boxed)) {
	s_boxed = Tcl_DuplicateObj (s_boxed);
	dup = 1;
    }

    if (setc_get (interp, s_boxed, &s) != TCL_OK) {
	return TCL_ERROR;
    }

    if (dup) {
	Tcl_InvalidateStringRep (s_boxed);
    }

    for (i = 2, removed = 0; i < objc; i++) {
	removed += cset_vsubtract (s, objv [i]);
    }

    if (!Tcl_ObjSetVar2 (interp, varname, 0, s_boxed, TCL_LEAVE_ERR_MSG)) {
	return TCL_ERROR;
    }

    Tcl_SetObjResult (interp, Tcl_NewIntObj (removed));
    return TCL_OK;
}

# # ## ### ##### ######## ############# #####################
## Ready

package provide struct::set 3
return

# # ## ### ##### ######## ############# #####################
