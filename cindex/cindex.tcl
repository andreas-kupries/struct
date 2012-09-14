# cindex.tcl --
#
#	Replication of Tcl's internal TclGetForIndex() so that
#	packages can use the "end", "end-N", and "N+M" forms of
#	list indices.
#
# Copyright (c) 2012 Tcl Core.

# # ## ### ##### ######## ############# #####################
## Requirements

package require Tcl 8.5
package require critcl 3 ;# stubs management

# # ## ### ##### ######## ############# #####################
## Administrivia

critcl::license {Tcl Core} BSD

critcl::summary {Access to an TclGetForIndex() equivalent}

critcl::description {
    This package replicates Tcl's internal TclGetForIndex()
    so that packages can use the "end", "end-N", and "N+M"
    forms of list indices.
}

critcl::subject TclGetForIndex {lindex indexing} {lrange indexing}

# # ## ### ##### ######## ############# #####################
## Configuration

critcl::api header cindex.h
critcl::cheaders   cindexInt.h

# # ## ### ##### ######## ############# #####################
## Exported API

critcl::api function int cindex_get {
    Tcl_Interp* interp
    Tcl_Obj*    obj
    int         endValue
    int*        indexPtr
}

# TclParseNumber

# # ## ### ##### ######## ############# #####################
## Implementation.

critcl::csources cindex.c
critcl::ccode {} ; # Fake the 'nothing to build detector'

# # ## ### ##### ######## ############# #####################
## Ready
package provide c::index 1
