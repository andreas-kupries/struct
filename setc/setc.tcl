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

critcl::api import c::set 1

critcl::tsources policy.tcl

## TODO: Tcl_ObjType for CSET, then result and argument types for cproc's

# # ## ### ##### ######## ############# #####################
## API. Functional. Sideeffect-free.

critcl::cproc ::struct::set::contains {CSET S Tcl_Obj* element} boolean {
}

critcl::ccommand ::struct::set::create {} {
    CSET s = cset_create (SetFree, SetEqual, 0);
    int i;
    /* convenience function list -> set */
    for (i=2;i<objc;i++) {
      cset_vadd (s, objv[i]);
    }
    return s;
}

critcl::ccommand ::struct::set::difference {} {
}

critcl::cproc ::struct::set::empty {CSET s} int {
    return cset_empty (s);
}

critcl::cproc ::struct::set::equal {CSET a CSET b} boolean {
    return cset_equal (a, b);
}

critcl::ccommand ::struct::set::exclude {} {
}

critcl::ccommand ::struct::set::include {} {
}

critcl::ccommand ::struct::set::intersect {} {
}

critcl::cproc ::struct::set::intersect3 {CSET a CSET b} Tcl_Obj* {
    CSET da = cset_difference (a, b);
    CSET i  = cset_intersect  (a, b);
    CSET db = cset_difference (b, a);

    // list, ... 3
    return ... ;
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
}

# # ## ### ##### ######## ############# #####################
## API. Imperative.

critcl::ccommand ::struct::set::add {} {
}

critcl::ccommand ::struct::set::set {} {
}

critcl::ccommand ::struct::set::subtract {} {
}

critcl::ccommand ::struct::set::unset {} {
}

# # ## ### ##### ######## ############# #####################
## Ready

package provide struct::set 3
return

# # ## ### ##### ######## ############# #####################
