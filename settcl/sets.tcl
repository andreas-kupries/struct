 ## sets.tcl --
# # ## ### ##### ######## ############# #####################
#
#	Set implementation for Tcl 8.5+
#	Contrary to stacks, queues, etc. sets are _values_.
#
# Copyright (c) 2004-2012 Andreas Kupries

# # ## ### ##### ######## ############# #####################

package require Tcl 8.5

namespace eval ::struct::set {
    namespace export {[a-z]*}
    namespace ensemble create
}

# # ## ### ##### ######## ############# #####################
## API. Functional. Sideeffect-free.

proc ::struct::set::contains {s element} {
    return [expr {$element in $s}]
}

proc ::struct::set::create {args} {
    return [lsort -unique $args]
}

proc ::struct::set::difference {s args} {
    # Fast bailout
    if {![llength $s]} { return $s }

    ::set s [lsort -unique $s]
    foreach B $args {
	::set s [Difference [K $s [::unset s]] $B]
	# Fast bailout
	if {![llength $s]} { return $s }
    }

    return $s
}

proc ::struct::set::empty {s} {
    return [expr {![llength $s]}]
}

proc ::struct::set::equal {a b} {
    # Prevent duplicates from affecting the comparison.
    ::set A [lsort -unique $a]
    ::set B [lsort -unique $b]

    # Equal if of the same cardinality and string identical for
    # canonical order.

    return [expr {([llength $A] == [llength $B]) && ([lsort $A] eq [lsort $B])}]
}

proc ::struct::set::exclude {s args} {
    difference $s $args
}

proc ::struct::set::include {s args} {
    union $s $args
}

proc ::struct::set::intersect {args} {
    # Fast bailout for no or single argument.
    switch -exact -- [llength $args] {
	0 {return {}}
	1 {return [lindex $args 0]}
    }

    ::set remainder [lassign $args Z]

    # Quick bail out on reaching an empty result
    if {![llength $Z]} { return {} }

    ::set Z [lsort -unique $Z]

    foreach B $remainder {
	::set Z [Intersect $Z $B]
	# Quick bail out on reaching an empty result
	if {![llength $Z]} { return {} }
    }

    return $Z
}

proc ::struct::set::intersect3 {a b} {
    return [list \
		[intersect  $a $b] \
		[difference $a $b] \
		[difference $b $a]]
}

proc ::struct::set::size {s} {
    return [llength [lsort -unique $s]]
}

proc ::struct::set::subset {a b} {
    # A subset|== B <=> (A == A*B)
    return [equal $a [intersect $a $b]]
}

proc ::struct::set::superset {a b} {
    # A superset|== B <=> (B == A*B)
    return [equal $b [intersect $a $b]]
}

proc ::struct::set::symdifference {a b} {
    # symdiff == (A-B) + (B-A) == (A+B)-(A*B)
    if {![llength $a]} {return $b}
    if {![llength $b]} {return $a}
    return [union [difference $a $b] [difference $b $a]]
}

proc ::struct::set::union {args} {
    ::set Z {}
    foreach B $args {
	lappend Z {*}$B
    }
    return [lsort -unique $Z]
}

# # ## ### ##### ######## ############# #####################
## API. Imperative.

proc ::struct::set::add {Svar args} {
    upvar 1 $Svar S
    if {![llength $args]} return
    if {![info exists S]} { ::set S {} }
    ::set S [union [K $S [::unset S]] {*}$args]
    return
}

proc ::struct::set::set {Svar args} {
    # Svar += element
    upvar 1 $Svar S
    if {![llength $args]} { return 0 }
    if {![info exists S]} {
	::set S {}
    } else {
	::set S [lsort -unique $S]
    }
    ::set added 0
    foreach element $args {
	if {$element in $S} continue
	lappend S $element
	incr added
    }
    return $added
}

proc ::struct::set::subtract {Svar args} {
    upvar 1 $Svar S
    if {![llength $args]} return
    if {![info exists S]} {
	return -code error -errorcode {STRUCT SET UNDEFINED VARIABLE} \
	    "can't read \"$Svar\": no such variable"
    }
    ::set S [difference [K $S [::unset S]] {*}$args]
    return
}

proc ::struct::set::unset {Svar args} {
    # Svar -= element
    upvar 1 $Svar S
    if {![llength $args]} { return 0 }
    if {![info exists S]} {
	return -code error -errorcode {STRUCT SET UNDEFINED VARIABLE} \
	    "can't read \"$Svar\": no such variable"
    }
    ::set S [lsort -unique $S]
    ::set removed 0
    foreach element $args {
	if {$element ni $S} continue
	::set pos [lsearch -exact $S $element]
	::set S [lreplace [K $S [::unset S]] $pos $pos]
	incr removed
    }
    return $removed
}

# # ## ### ##### ######## ############# #####################
## Helper commands.

proc ::struct::set::K {x y} {::set x}

proc ::struct::set::Difference {A B} {
    # A is the result accumulator. We know that it is free of
    # duplicates, and not empty.

    # Fast bailout for various special circumstances
    if {![llength $B]} {return $A}
    ::set B [lsort -unique $B]

    ::set Z {}
    foreach x $A {
	if {$x in $B} continue
	lappend Z $x
    }
    return $Z
}

proc ::struct::set::Intersect {A B} {
    # A is the result accumulator. We know that it does not contain
    # duplicates, nor is it empty.

    if {![llength $B]} {return {}}
    ::set B [lsort -unique $B]

    # Search the smaller set, iterate the larger.
    if {[llength $B] > [llength $A]} {
	::set tmp $A
	::set A $B
	::set B $tmp
	::unset tmp
    }

    ::set Z {}
    foreach x $B {
	if {$x ni $A} continue
	lappend Z $x
    }

    # No duplicates, as subset of the deduplicated B (or the already
    # duplicate-free A).
    return $Z
}

# # ## ### ##### ######## ############# #####################
## Tcl level policy settings.
#
## Make the main command available as method under the larger
## namespace.

namespace eval ::struct {
    namespace export set
    namespace ensemble create
}

# # ## ### ##### ######## ############# #####################
## Ready

package provide struct::set 3
return

# # ## ### ##### ######## ############# #####################
