# cqueue.tcl --
#
#	Low-level queue (FIFO) data structure.
#	With some wrapping usable as a Tcl-level queue object.
#
#	The implemented queue is double sided, i.e. data can be
#	added and removed at both ends. This makes it usable as
#	stack (LIFO) as well.
#
# Copyright (c) 2012 Andreas Kupries <andreas_kupries@users.sourceforge.net>

# # ## ### ##### ######## ############# #####################
## Requirements

package require Tcl 8.5
package require critcl 3 ;# stubs management

# # ## ### ##### ######## ############# #####################
## Administrivia

critcl::license {Andreas Kupries} BSD

critcl::summary {A C-level abstract datatype for queues}

critcl::description {
    This package implements an abstract data type for queues (FIFOs),
    at the C-level.  No Tcl-binding is provided. See package
    'struct::queue' for that. The implemented queue is double sided,
    i.e. data can be added and removed at both ends. This makes it
    usable as stack (LIFO) as well.
}

critcl::subject queue fifo {double-sided queue} {double-ended queue}
critcl::subject stack lifo
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

# # ## ### ##### ######## ############# #####################
## Lifecycle management. Object creation and destruction.

critcl::api function CQUEUE     cqueue_create  {CQUEUE_CELL_FREE freeCell void* clientdata}
critcl::api function void       cqueue_destroy {CQUEUE q}

# # ## ### ##### ######## ############# #####################
## Accessors.
# Queue size. First element. Last element.
# Slice of front elements. Slice of end elements.
# Arbitrary slice of elements.

critcl::api function {long int} cqueue_size  {CQUEUE q}
critcl::api function void*      cqueue_first {CQUEUE q}
critcl::api function void*      cqueue_last  {CQUEUE q}
critcl::api function CSLICE     cqueue_head  {CQUEUE q               {long int} n}
critcl::api function CSLICE     cqueue_tail  {CQUEUE q               {long int} n}
critcl::api function CSLICE     cqueue_get   {CQUEUE q {long int} at {long int} n}

# # ## ### ##### ######## ############# #####################
## Modifiers.
# - Add element at queue end, front.
# - Remove elements at queue end, front.
# - Drop elements at queue end, front (remove, no cell release).
# - Move contents of a source queue over into a destination.
##
# Notes:
# Add    - Item allocation is responsibility of caller. Queue takes ownership of the item.
# Remove - Queue frees allocated items.
# Drop   - Like remove, but doesn't free, assumes that caller
#          is taking ownership of the cell data.

critcl::api function void cqueue_append        {CQUEUE q void* item}
critcl::api function void cqueue_prepend       {CQUEUE q void* item}
critcl::api function void cqueue_append_slice  {CQUEUE q CSLICE s}
critcl::api function void cqueue_prepend_slice {CQUEUE q CSLICE s}
critcl::api function void cqueue_remove_head   {CQUEUE q {long int} n}
critcl::api function void cqueue_remove_tail   {CQUEUE q {long int} n}
critcl::api function void cqueue_clear         {CQUEUE q}
critcl::api function void cqueue_drop_head     {CQUEUE q {long int} n}
critcl::api function void cqueue_drop_tail     {CQUEUE q {long int} n}
critcl::api function void cqueue_drop_all      {CQUEUE q}
critcl::api function void cqueue_move          {CQUEUE q CQUEUE src {long int} n}
critcl::api function void cqueue_move_all      {CQUEUE q CQUEUE src}

# # ## ### ##### ######## ############# #####################
## User data management. Set and retrieve.

critcl::api function void       cqueue_clientdata_set {CQUEUE s void* clientdata}
critcl::api function void*      cqueue_clientdata_get {CQUEUE s}

# # ## ### ##### ######## ############# #####################
## Implementation.

critcl::csources cqueue.c
critcl::ccode {} ; # Fake the 'nothing to build detector'

# # ## ### ##### ######## ############# #####################
## Ready
package provide c::queue 1
