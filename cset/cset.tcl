# cset.tcl --
#
#	Low-level set data structure.
#	With some wrapping usable as a Tcl_Obj* value.
#
# Copyright (c) 2012 Andreas Kupries <andreas_kupries@users.sourceforge.net>

# # ## ### ##### ######## ############# #####################
## Requirements

package require Tcl 8.5
package require critcl 3 ;# stubs management

# # ## ### ##### ######## ############# #####################
## Administrivia

critcl::license {Andreas Kupries} BSD

critcl::summary {A C-level abstract datatype for sets}

critcl::description {
    This package implements an abstract data type for sets,
    at the C-level.  No Tcl-binding is provided. See package
    'struct::set' for that.
}

critcl::subject set lot bag union intersect difference
critcl::subject {symmetric difference} {include element}
critcl::subject {data structure} {exclude element}
critcl::subject structure
critcl::subject {abstract data structure}
critcl::subject {generic data structure}

critcl::api import c::slice 1

# # ## ### ##### ######## ############# #####################
## Configuration

critcl::api header cset.h
critcl::cheaders   csetInt.h jsw/tree.h

# # ## ### ##### ######## ############# #####################
## Exported API

# # ## ### ##### ######## ############# #####################
## Lifecycle management. Object creation and destruction.

critcl::api function CSET cset_create  {
    CSET_CELL_DUP dup
    CSET_CELL_REL release
    CSET_CELL_CMP compare
    {void const*} clientdata
}

critcl::api function void cset_destroy {CSET q}

# # ## ### ##### ######## ############# #####################
## Accessors, and operations.

critcl::api function CSLICE     cset_contents   {{const CSET} s}
critcl::api function int        cset_contains   {{const CSET} s {void const*} item}
critcl::api function CSET       cset_dup        {{const CSET} s}
critcl::api function CSET       cset_difference {{const CSET} a {const CSET} b}
critcl::api function int        cset_empty      {{const CSET} s}
critcl::api function int        cset_equal      {{const CSET} a {const CSET} b}
critcl::api function CSET       cset_intersect  {{const CSET} a {const CSET} b}
critcl::api function CSET       cset_symdiff    {{const CSET} a {const CSET} b}
critcl::api function int        cset_subset     {{const CSET} a {const CSET} b}
critcl::api function int        cset_superset   {{const CSET} a {const CSET} b}
critcl::api function {long int} cset_size       {{const CSET} s}
critcl::api function CSET       cset_union      {{const CSET} a {const CSET} b}

# # ## ### ##### ######## ############# #####################
## Modifiers. Operations which modify the 1st argument.

critcl::api function void cset_vdifference {CSET a {const CSET} b}
critcl::api function void cset_vintersect  {CSET a {const CSET} b}
critcl::api function void cset_vsymdiff    {CSET a {const CSET} b}
critcl::api function void cset_vunion      {CSET a {const CSET} b}

# # ## ### ##### ######## ############# #####################
## Modifiers. Operations which modify the 1st argument.
## 2nd argument is element, not set.

critcl::api function void cset_vadd      {CSET s {void const*} item}
critcl::api function void cset_vsubtract {CSET s {void const*} item}

# # ## ### ##### ######## ############# #####################
## User data management. Set and retrieve.

critcl::api function void       cset_clientdata_set {CSET s {void const*} clientdata}
critcl::api function void*      cset_clientdata_get {{const CSET} s}

# # ## ### ##### ######## ############# #####################
## Implementation.

critcl::csources cset.c
critcl::csources jsw/tree.c
critcl::ccode {} ; # Fake the 'nothing to build detector'

# # ## ### ##### ######## ############# #####################
## Ready
package provide c::set 1
