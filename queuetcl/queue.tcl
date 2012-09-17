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

    variable mymiddle myat mytail myhead

    # # ## ### ##### ######## ############# #####################
    ## Lifecycle management.

    constructor {} {
	set myat     0
	set mymiddle {}
	set mytail {}
	set myhead  {}
	return
    }

    # # ## ### ##### ######## ############# #####################
    ## Accessors

    method size {} {
	return [expr { [llength $mymiddle] + [llength $mytail] + [llength $myhead] - $myat }]
    }

    method get {at n} {
	my CheckIndex $at
	# check n, > 0, at+n < queue_size
	# xxx todo get





	if {$n == 1} {
	    # Handle this as a special case
	    # Single item gets are not listified
	    set result [lindex $result 0]
	}
	return $result
    }

    method head {{n 1}} {
	my CheckCount $n

	# Incremental accumulation of the result from the head,
	# middle, and tail buffers (in this order). Because of the
	# check above we know that they contain enough elements to
	# satisfy the request, even if each buffer alone does not have
	# enough.

	set missing $n
	if {[llength $myhead]} {
	    # Pull from the unget buffer first.
	    set k [llength $myhead]
	    if {$missing >= $k} {
		# We need equal or more than is in this buffer. We take everything.
		lappend result {*}[lreverse $myhead]
		incr missing -$k
	    } else {
		# This buffer contains more than the whole request.
		incr missing -1
		lappend result {*}[lreverse [lrange $myhead end-$missing end]]
		set missing 0
	    }
	}

	if {$missing} {
	    # Try the regular return buffer next, if needed.
	    set k [expr {[llength $mymiddle] - $myat}]
	    if {$missing >= $k} {
		# We need equal or more than is found in this
		# buffer. We take everything.
		lappend result {*}[lrange $mymiddle $myat end]
		incr missing -$k
	    } else {
		# This buffer contains more than the whole request.
		set  stop $myat
		incr stop $missing
		incr stop -1
		lappend result {*}[lrange $mymiddle $myat $stop]
		set missing 0
	    }
	}

	if {$missing} {
	    # At last check the append buffer, if needed
	    set k [llength $mytail]
	    if {$missing >= $k} {
		# We need equal or more than is found in this
		# buffer. We take everything.
		lappend result {*}$mytail
		incr missing -$k
	    } else {
		# This buffer contains more than the whole request.
		incr missing -1
		lappend result {*}[lrange $mytail 0 $missing]
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

    method tail {{n 1}} {
	my CheckCount $n

	# Incremental accumulation of the result from the head,
	# middle, and tail buffers (in reverse order). Because of the
	# check above we know that they contain enough elements to
	# satisfy the request, even if each buffer alone does not have
	# enough.

	# XXX todo tail
    }

    # # ## ### ##### ######## ############# #####################
    ## Manipulators

    method clear {} {
	set myat     0
	set mymiddle {}
	set mytail   {}
	set myhead   {}
	return
    }

    method append {item args} {
	lappend mytail $item {*}$args
	return
    }

    method prepend {item args} {
	lappend myhead $item {*}$args
	return
    }

    method pop {where {n 1}} {
	switch -exact -- $where {
	    head {
		set result [my head $n]
		my DropHead $n
	    }
	    tail {
		set result [my tail $n]
		my DropTail $n
	    }
	}
	return $result
    }

    # # ## ### ##### ######## ############# #####################
    ## Internals. Drop elements from the head, or tail of the queue.

    method DropHead {n} {
	# XXX todo drop head
    }

    method DropTail {n} {
	# XXX todo drop tail
    }

    # # ## ### ##### ######## ############# #####################
    ## Internals - Helper to manage refcounts.

    method K {x y} { set x }

    # size:  integer,    >= 0
    # count: integer,    >= 0, < cqueue_size(s)
    # index: list index, >= 0, < cqueue_size(s)

    method CheckSize {n} {
	if {![string is int -strict $n] || ($n < 0)} {
	    return -code error "expected non-negative integer but got \"$n\""
	}
    }

    method CheckCount {n} {
	if {![string is int -strict $n] || ($n < 1)} {
	    return -code error "expected positive integer but got \"$n\""
	}
	if {[my size] < $n} {
	    return -code error "not enough elements"
	}
    }

    method CheckIndex {iv} {
	upvar 1 $iv index

	if {$index eq "end"} {
	    set index 0
	    return
	}

	if {[string is int -strict $index]} {
	    my CheckIndexRange index
	    return
	}

	if {[regexp {^end-(.+)$} $index -> a]} {
	    # end-x        ==> x
	    # end-0 == end ==> 0 == bottom
	    if {![string is int -strict $a]} { my ReportBadIndex $index }
	    set index $a
	    my CheckIndexRange index
	    return
	}

	if {[regexp {^([^-]+)-([^-]+)$} $index -> a b]} {
	    # N-M
	    if {![string is int -strict $a]} { my ReportBadIndex $index }
	    if {![string is int -strict $b]} { my ReportBadIndex $index }

	    set index [expr {$a - $b}]
	    my CheckIndexRange index
	    return
	}

	if {[regexp {^([^-]+)[+]([^-]+)$} $index -> a b]} {
	    # N+M
	    if {![string is int -strict $a]} { my ReportBadIndex $index }
	    if {![string is int -strict $b]} { my ReportBadIndex $index }

	    set index [expr {$a + $b}]
	    my CheckIndexRange index
	    return
	}

	my ReportBadIndex $index
    }

    method CheckIndexRange {iv} {
	upvar 1 $iv index
	set sz [my size]
	if {($index < 0) || ($sz <= $index)} {
	    return -code error "invalid index \"$index\""
	}
	return
    }

    method ReportBadIndex {index} {
	return -code error "bad index \"$index\": must be integer?\[+-]integer? or end?\[+-]integer?"
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
