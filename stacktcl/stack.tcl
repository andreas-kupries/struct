## stack.tcl --
# # ## ### ##### ######## ############# #####################
#
#	Stack implementation for Tcl 8.5+TclOO
#
# Copyright (c) 2010-2012 Andreas Kupries

# # ## ### ##### ######## ############# #####################

package require Tcl 8.5
package require TclOO

# # ## ### ##### ######## ############# #####################

oo::class create ::struct::stack {
    # # ## ### ##### ######## ############# #####################
    ## Instance state.

    # - The list of elements.
    #   Grows to the right.
    #   The topmost element is stored last.

    variable mystack

    # # ## ### ##### ######## ############# #####################
    ## Lifecycle management.

    constructor {} {
	set mystack {}
	return
    }

    # # ## ### ##### ######## ############# #####################
    ## Accessors

    method size {} {
	return [llength $mystack]
    }

    method top {} {
	my CheckEmpty
	return [lindex $mystack end]
    }

    method bottom {} {
	my CheckEmpty
	return [lindex $mystack 0]
    }

    method at {at} {
	my CheckIndex $at
	return [lindex $mystack end-$at]
    }

    method atr {at} {
	my CheckIndex $at
	return [lindex $mystack $at]
    }

    method get {} {
	return [lreverse $mystack]
    }

    method getr {} {
	return $mystack
    }

    method peek {{n 1}} {
	my CheckCount $n

	if {$n == 1} {
	    # Handle this as a special case
	    # Single item peeks are not listified
	    return [lindex $mystack end]
	}

	# Otherwise, return a list of items
	incr n -1
	return [lreverse [lrange $mystack end-$n end]]
    }

    method peekr {{n 1}} {
	my CheckCount $n

	if {$n == 1} {
	    # Handle this as a special case.
	    # Single item peeks are not listified
	    return [lindex $mystack end]
	}

	# Otherwise, return a list of items, in reversed order.
	incr n -1
	return [lrange $mystack end-$n end]
    }

    # # ## ### ##### ######## ############# #####################
    ## Manipulators

    method pop {{n 1}} {
	my CheckCount $n

	if {$n == 1} {
	    # Handle this as a special case.
	    # Single item pops are not listified
	    set item [lindex $mystack end]
	    set size [llength $mystack]
	    if {$n == $size} {
		set mystack {}
	    } else {
		set mystack [lreplace [my K $mystack [unset mystack]] end end]
	    }
	    return $item
	}

	set size [llength $mystack]

	# Otherwise, return a list of items, and remove the items from the
	# stack.
	if {$n == $size} {
	    set result  [lreverse [my K $mystack [unset mystack]]]
	    set mystack {}
	} else {
	    incr n -1
	    set result  [lreverse [lrange $mystack end-$n end]]
	    set mystack [lreplace [my K $mystack [unset mystack]] end-$n end]
	}
	return $result
    }

    method clear {} {
	set mystack {}
	return
    }

    method push {item args} {
	lappend mystack $item {*}$args
	return
    }

    method rotate {n steps} {
	set len [llength $mystack]
	if {$n > $len} {
	    return -code error "insufficient items on stack to perform request"
	}

	# Rotation algorithm:
	# do
	#   Find the insertion point in the stack
	#   Move the end item to the insertion point
	# repeat $steps times

	set start [expr {$len - $n}]
	set steps [expr {$steps % $n}]

	if {$steps == 0} return

	for {set i 0} {$i < $steps} {incr i} {
	    set item [lindex $mystack end]
	    set mystack [linsert \
			     [lreplace \
				  [my K $mystack [unset mystack]] \
				  end end] $start $item]
	}
	return
    }

    method trim {n} {
	if { ![string is integer -strict $n]} {
	    return -code error "expected integer but got \"$n\""
	} elseif { $n < 0 } {
	    return -code error "invalid size $n"
	} elseif { $n >= [llength $mystack] } {
	    # Stack is smaller than requested, do nothing.
	    return {}
	}

	# n < [llength $mystack]
	# pop '[llength $mystack]' - n elements.

	if {!$n} {
	    set result [lreverse [my K $mystack [unset mystack]]]
	    set mystack {}
	} else {
	    set result  [lreverse [lrange $mystack $n end]]
	    set mystack [lreplace [my K $mystack [unset mystack]] $n end]
	}

	return $result
    }

    method trim* {n} {
	if { ![string is integer -strict $n]} {
	    return -code error "expected integer but got \"$n\""
	} elseif { $n < 0 } {
	    return -code error "invalid size $n"
	}

	if { $n >= [llength $mystack] } {
	    # Stack is smaller than requested, do nothing.
	    return
	}

	# n < [llength $mystack]
	# pop '[llength $mystack]' - n elements.

	# No results, compared to trim. 

	if {!$n} {
	    set mystack {}
	} else {
	    set mystack [lreplace [my K $mystack [unset mystack]] $n end]
	}

	return
    }

    # # ## ### ##### ######## ############# #####################
    ## Internals - Helper to manage refcounts.

    method K {x y} { set x }

    method CheckEmpty {} {
	upvar 1 mystack mystack
	if {![llength $mystack]} {
	    return -code error "insufficient items on stack to fulfill request"
	}
    }

    method CheckIndex {i} {
	upvar 1 mystack mystack
	if {![string is int -strict $i]} {
	    return -code error "expected integer but got \"$i\""
	}
	if {($i < 0) || ([llength $mystack] <= $i)} {
	    return -code error "invalid index $i"
	}
    }

    method CheckCount {n} {
	upvar 1 mystack mystack
	if {![string is int -strict $n]} {
	    return -code error "expected integer but got \"$n\""
	}
	if {$n < 1} {
	    return -code error "invalid item count $n"
	}
	if {[llength $mystack] < $n} {
	    return -code error "insufficient items on stack to fulfill request"
	}
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
    namespace export stack
    namespace ensemble create
}

# # ## ### ##### ######## ############# #####################
## Ready

package provide struct::stack 2
return

# # ## ### ##### ######## ############# #####################
