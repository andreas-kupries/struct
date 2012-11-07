## prioqueue.tcl --
# # ## ### ##### ######## ############# #####################
#
#	Priority queue implementation for Tcl 8.5+TclOO
#
# Copyright (c) 2002,2003 Michael Schlenker
# Copyright (c) 2008 Alejandro Paz <vidriloco@gmail.com>
# Copyright (c) 2010-2012 Andreas Kupries

# # ## ### ##### ######## ############# #####################

package require Tcl 8.5
package require TclOO

# # ## ### ##### ######## ############# #####################

oo::class create ::struct::prioqueue {
    # # ## ### ##### ######## ############# #####################
    ## Instance state.

    variable mytype myorder

    # # ## ### ##### ######## ############# #####################
    ## Lifecycle management.

    constructor {args} {
	# args = options...
	set ordermap {
	}

	set mytype -integer
	set myorder {}
	foreach option $args {
	    switch -exact $option {
		-integer    -
		-real       -
		-ascii      -
		-dict       -
		-dictionary { set mytype $option }
		-increasing -
		-decreasing { set myorder $option }
		default {
		    return -code error \
			"Unknown option \"$option\", expected one of ..."
		}
	    }
	}

	if {$myorder eq {}} {
	    set myorder [dict get {
		-integer    -decreasing
		-real       -decreasing
		-ascii      -increasing
		-dict       -increasing
		-dictionary -increasing
	    } $mytype]
	}



	return
    }

    # # ## ### ##### ######## ############# #####################
    ## Accessors

    method size {} {
    }

    method peek {{n 1}} {
	my CheckCount $n
    }

    method peekpriority {{n 1}} {
	my CheckCount $n
    }

    # # ## ### ##### ######## ############# #####################
    ## Manipulators

    method clear {} {
	return
    }

    method put {item prio args} {
	return
    }

    method pop {{n 1}} {
    }

    # # ## ### ##### ######## ############# #####################
    ## Internals. Drop elements from the head, or tail of the prioqueue.

    method DropHead {n} {
	set have [llength $myhead]
	if {$n >= $have} {
	    # Drop everything, and continue
	    incr n -$have
	    set myhead {}
	    if {!$n} return
	} else {
	    # Drop partial, and stop.
	    set myhead [lrange [my K $myhead [unset myhead]] 0 end-$n]
	    return
	}

	set  have [llength $mymiddle]
	incr have -$myat

	while {1} {
	    if {$n >= $have} {
		# Drop everything, and continue
		incr n -$have
		set mymiddle {}
		set myat 0
		if {!$n} return

		# Shift tail to middle, and retry.
		set mymiddle $mytail
		set mytail {}
		set  have [llength $mymiddle]
		continue
	    } else {
		# Drop partial, and stop.
		incr myat $n
		return
	    }
	}

	return -code error "Bad drop head"
    }

    # # ## ### ##### ######## ############# #####################
    ## Internals - Helper to manage refcounts.

    method K {x y} { set x }

    # size:  integer,    >= 0
    # count: integer,    >= 0, < cprioqueue_size(s)
    # index: list index, >= 0, < cprioqueue_size(s)

    method CheckSize {n} {
	if {![string is int -strict $n] || ($n < 0)} {
	    return -code error "expected non-negative integer but got \"$n\""
	}
    }

    method CheckGetSize {n} {
	if {![string is int -strict $n] || ($n < 1)} {
	    return -code error "expected positive integer but got \"$n\""
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

    method DumpState {{prefix {}}} {
	puts stderr ${prefix}[self]=@$myat|([lreverse $myhead])_($mymiddle)_($mytail)
	return
    }
    #export DumpState

    # # ## ### ##### ######## ############# #####################
}

# # ## ### ##### ######## ############# #####################
## Tcl level policy settings.
#
## Make the main class command available as method under the
## larger namespace.

package require Tcl 8.5

namespace eval ::struct {
    namespace export prioqueue
    namespace ensemble create
}

# # ## ### ##### ######## ############# #####################
## Ready

package provide struct::prioqueue 2
return

# # ## ### ##### ######## ############# #####################
