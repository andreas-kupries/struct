# cqueue.tcl --
#
#	Low-level queue data structure.
#	With some wrapping usable as a Tcl-level queue object.
#
# Copyright (c) 2008-2012 Andreas Kupries <andreas_kupries@users.sourceforge.net>

# # ## ### ##### ######## ############# #####################
## Requirements

package require Tcl 8.5
package require critcl 3 ;# stubs management

# # ## ### ##### ######## ############# #####################
## Administrivia

critcl::license {Andreas Kupries} BSD

critcl::summary {A C-level abstract datatype for queues}

critcl::description {
    This package implements an abstract data type for queues, at the C-level.
    No Tcl-binding is provided. See package 'struct::queue' for that.
}

critcl::subject queue
critcl::subject fifo
critcl::subject {data structure}
critcl::subject structure
critcl::subject {abstract data structure}
critcl::subject {generic data structure}

# # ## ### ##### ######## ############# #####################
## Configuration

critcl::api import c::slice 1
critcl::api import c::stack 1

critcl::api header cqueue.h
critcl::cheaders   cqueueInt.h

# # ## ### ##### ######## ############# #####################
## Exported API

# Lifecycle management. Object creation and destruction

critcl::api function CQUEUE     cqueue_create  {CQUEUE_CELL_FREE freeCell void* clientdata}
critcl::api function void       cqueue_destroy {CQUEUE s}

# Accessors. Queue size, front element, slice of front elements.

critcl::api function {long int} cqueue_size {CQUEUE s}
critcl::api function void*      cqueue_head {CQUEUE s}
critcl::api function void*      cqueue_tail {CQUEUE s}
critcl::api function CSLICE     cqueue_get  {CQUEUE s {long int} n CSLICE_DIRECTION dir}

# Modifiers.
#  - put     -- Item allocation is responsibility of caller.
#               Queue takes ownership of the item.
#  - unget   -- Item allocation is responsibility of caller.
#                Queue takes ownership of the item.
#  - remove  -- Queue frees allocated item.
#  - trim    -- Ditto
#  - drop    -- Like remove, but doesn't free, assumes that caller
#               is taking ownership of the pointer.

critcl::api function void       cqueue_put    {CQUEUE s void* item}
critcl::api function void       cqueue_unget  {CQUEUE s void* item}
critcl::api function void       cqueue_remove {CQUEUE s {long int} n}
critcl::api function void       cqueue_trim   {CQUEUE s {long int} n}
critcl::api function void       cqueue_drop   {CQUEUE s {long int} n}
critcl::api function void       cqueue_move   {CQUEUE s CQUEUE src}

# User data management. Set and retrieve.

critcl::api function void       cqueue_clientdata_set {CQUEUE s void* clientdata}
critcl::api function void*      cqueue_clientdata_get {CQUEUE s}

# # ## ### ##### ######## ############# #####################
## Implementation.

critcl::csources cqueue.c
critcl::ccode {} ; # Fake the 'nothing to build detector'

# # ## ### ##### ######## ############# #####################
## Ready
package provide c::queue 1
