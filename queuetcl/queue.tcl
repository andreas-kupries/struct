## queue.tcl --
# # ## ### ##### ######## ############# #####################
#
#	Queue implementation for Tcl 8.5+TclOO
#
# Copyright (c) 2010-2012 Andreas Kupries

# # ## ### ##### ######## ############# #####################

package require Tcl 8.5
package require TclOO

# # ## ### ##### ######## ############# #####################

oo::class create ::struct::queue {
    # # ## ### ##### ######## ############# #####################
    ## Instance state.

    # - The list of elements from which elements are returned
    # - And the index of the next element to return, in that list.
    # - The list of elements new elements are added to.
    # - The list of elements which are put back.

    variable myreturn myat myappend myunget

    # # ## ### ##### ######## ############# #####################
    ## Lifecycle management.

    constructor {} {
	set myat     0
	set myreturn {}
	set myappend {}
	set myunget  {}
	return
    }

    # # ## ### ##### ######## ############# #####################
    ## Accessors

    method size {} {
	return [expr { [llength $myreturn] + [llength $myappend] + [llength $myunget] - $myat }]
    }

    method get {{n 1}} {
	if {$n < 1} {
	    return -code error "invalid item count $n"
	}
	set len [my size]
	if {$n > $len} {
	    return -code error "insufficient items in queue to fulfill request"
	}

	# Incremental accumulation of the result from the unget,
	# return, and append buffers. Because of the check above we
	# know that their total contains enough elements, even if each
	# buffer alone does not have enough.

	set missing $n
	if {[llength $myunget]} {
	    # Pull from the unget buffer first.
	    set k [llength $myunget]
	    if {$missing >= $k} {
		# We need equal or more than is in this buffer. We take everything.
		lappend result {*}[lreverse $myunget]
		incr missing -$k
		set myunget {}
	    } else {
		# This buffer contains more than the whole request.
		incr missing -1
		lappend result {*}[lreverse [lrange $myunget end-$missing end]]
		incr missing
		set myunget [lrange $myunget 0 end-$missing]
		set missing 0
	    }
	}

	# We try the regular buffer twice. The second time through we
	# are actually looking at the data from the append buffer
	# which got shifted in in branch (*).
	foreach _ {0 1} {
	    if {$missing} {
		# Try the regular return buffer next, if needed.
		set k [expr {[llength $myreturn] - $myat}]
		if {$missing >= $k} {
		    # We need equal or more than is found in this
		    # buffer. We take everything. And shift (*).
		    lappend result {*}[lrange $myreturn $myat end]
		    incr missing -$k
		    my Shift
		} else {
		    # This buffer contains more than the whole request.
		    set  stop $myat
		    incr stop $missing
		    incr stop -1
		    lappend result {*}[lrange $myreturn $myat $stop]
		    incr stop
		    set myat $stop
		    set missing 0
		    break
		}
	    }
	}

	if {$missing} {
	    return -code error "Queue size information inconsistency"
	}

	if {$n == 1} {
	    # Handle this as a special case
	    # Single item gets are not listified
	    set result [lindex $result 0]
	}
	return $result
    }


    method peek {{n 1}} {
	# Peeking is slightly different from 'get' because the buffers
	# are only queried, and not changed.

	if {$n < 1} {
	    return -code error "invalid item count $n"
	}
	set len [my size]
	if {$n > $len} {
	    return -code error "insufficient items in queue to fulfill request"
	}

	# Incremental accumulation of the result from the unget,
	# return, and append buffers. Because of the check above we
	# know that their total contains enough elements, even if each
	# buffer alone does not have enough.

	set missing $n
	if {[llength $myunget]} {
	    # Pull from the unget buffer first.
	    set k [llength $myunget]
	    if {$missing >= $k} {
		# We need equal or more than is in this buffer. We take everything.
		lappend result {*}[lreverse $myunget]
		incr missing -$k
	    } else {
		# This buffer contains more than the whole request.
		incr missing -1
		lappend result {*}[lreverse [lrange $myunget end-$missing end]]
		set missing 0
	    }
	}

	if {$missing} {
	    # Try the regular return buffer next, if needed.
	    set k [expr {[llength $myreturn] - $myat}]
	    if {$missing >= $k} {
		# We need equal or more than is found in this
		# buffer. We take everything.
		lappend result {*}[lrange $myreturn $myat end]
		incr missing -$k
	    } else {
		# This buffer contains more than the whole request.
		set  stop $myat
		incr stop $missing
		incr stop -1
		lappend result {*}[lrange $myreturn $myat $stop]
		set missing 0
	    }
	}

	if {$missing} {
	    # At last check the append buffer, if needed
	    set k [llength $myappend]
	    if {$missing >= $k} {
		# We need equal or more than is found in this
		# buffer. We take everything.
		lappend result {*}$myappend
		incr missing -$k
	    } else {
		# This buffer contains more than the whole request.
		incr missing -1
		lappend result {*}[lrange $myappend 0 $missing]
		set missing 0
	    }
	}

	if {$missing} {
	    return -code error "Queue size information inconsistency"
	}

	if {$n == 1} {
	    # Handle this as a special case
	    # Single item gets are not listified
	    set result [lindex $result 0]
	}
	return $result
    }

    # # ## ### ##### ######## ############# #####################
    ## Manipulators

    method clear {} {
	set myat     0
	set myreturn {}
	set myappend {}
	set myunget  {}
	return
    }

    method put {item args} {
	lappend myappend $item {*}$args
	return
    }

    method unget {item args} {
	lappend myunget $item {*}$args
	return
    }

    # # ## ### ##### ######## ############# #####################

    method Shift {} {
	set myat 0
	set myreturn $myappend
	set myappend {}
	return
    }

    # # ## ### ##### ######## ############# #####################
}

# # ## ### ##### ######## ############# #####################
## Tcl level policy settings.
#
## Make the main class command available as method under the
## larger namespace.

package require Tcl 8.5

namespace eval ::struct {
    namespace export queue
    namespace ensemble create
}

# # ## ### ##### ######## ############# #####################
## Ready

package provide struct::queue 2
return

# # ## ### ##### ######## ############# #####################
