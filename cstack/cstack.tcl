# cstack.tcl --
#
#	Low-level stack data structure.
#	With some wrapping usable as a Tcl-level stack object.
#
# Copyright (c) 2008-2012 Andreas Kupries <andreas_kupries@users.sourceforge.net>

# # ## ### ##### ######## ############# #####################
## Requirements

package require Tcl 8.5
package require critcl 3 ;# stubs management

# # ## ### ##### ######## ############# #####################
## Administrivia

critcl::license {Andreas Kupries} BSD

critcl::summary {A C-level abstract datatype for stacks}

critcl::description {
    This package implements an abstract data type for stacks, at the C-level.
    No Tcl-binding is provided. See package 'struct::stack' for that.
}

critcl::subject stack
critcl::subject lifo
critcl::subject {data structure}
critcl::subject structure
critcl::subject {abstract data structure}
critcl::subject {generic data structure}

# # ## ### ##### ######## ############# #####################
## Configuration

critcl::api import c::slice 1
critcl::api header cstack.h
critcl::cheaders   cstackInt.h

# # ## ### ##### ######## ############# #####################
## Exported API

# Lifecycle management. Object creation and destruction

critcl::api function CSTACK     cstack_create  {CSTACK_CELL_FREE freeCell void* clientdata}
critcl::api function void       cstack_destroy {CSTACK s}

# Accessors. Stack size, topmost element, indexed element, slice of elements.

critcl::api function {long int} cstack_size {CSTACK s}
critcl::api function void*      cstack_top  {CSTACK s}
critcl::api function void*      cstack_at   {CSTACK s {long int} i}
critcl::api function void*      cstack_atr  {CSTACK s {long int} i}
critcl::api function CSLICE     cstack_get  {CSTACK s {long int} at {long int} n CSLICE_DIRECTION dir}
critcl::api function CSLICE     cstack_getr {CSTACK s {long int} at {long int} n CSLICE_DIRECTION dir}

# Modifiers.
#  - push -- Item allocation is responsibility of caller.
#            Stack takes ownership of the item.
#  - pop  -- Stack frees allocated item.
#  - trim -- Ditto
#  - top  -- Provides top item, no transfer of ownership.
#  - del  -- Releases stack, cell array, and items, if any.
#  - drop -- Like pop, but doesn't free, assumes that caller
#            is taking ownership of the pointer.

critcl::api function void       cstack_push {CSTACK s void*      item}
critcl::api function void       cstack_pop  {CSTACK s {long int} n}
critcl::api function void       cstack_trim {CSTACK s {long int} n}
critcl::api function void       cstack_drop {CSTACK s {long int} n}
critcl::api function void       cstack_rol  {CSTACK s {long int} n {long int} step}
critcl::api function void       cstack_move {CSTACK s CSTACK src}

# User data management. Set and retrieve.

critcl::api function void       cstack_clientdata_set {CSTACK s void* clientdata}
critcl::api function void*      cstack_clientdata_get {CSTACK s}

# # ## ### ##### ######## ############# #####################
## Implementation.

critcl::csources cstack.c
critcl::ccode {} ; # Fake the 'nothing to build detector'

# # ## ### ##### ######## ############# #####################
## Ready
package provide c::stack 1
