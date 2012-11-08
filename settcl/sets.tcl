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

proc ::struct::set::contains {S element} {
    return [expr {$element in $S}]
}

proc ::struct::set::create {args} {
    return [lsort -unique $args]
}

proc ::struct::set::difference {S args} {
    # Fast bailout
    if {![llength $S]} { return $S }

    ::set S [lsort -unique $S]
    foreach B $args {
	::set S [Difference [K $S [::unset S]] $B]
	# Fast bailout
	if {![llength $S]} { return $S }
    }

    return $S
}

proc ::struct::set::empty {S} {
    return [expr {![llength $S]}]
}

proc ::struct::set::equal {A B} {
    # Prevent duplicates from affecting the comparison.
    ::set A [lsort -unique $A]
    ::set B [lsort -unique $B]

    # Equal if of the same cardinality and string identical for
    # canonical order.

    return [expr {([llength $A] == [llength $B]) && ([lsort $A] eq [lsort $B])}]
}

proc ::struct::set::exclude {S args} {
    difference $S $args
}

proc ::struct::set::include {S args} {
    union $S $args
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

proc ::struct::set::intersect3 {A B} {
    return [list \
		[intersect  $A $B] \
		[difference $A $B] \
		[difference $B $A]]
}

proc ::struct::set::size {S} {
    return [llength [lsort -unique $S]]
}

proc ::struct::set::subset {A B} {
    # A subset|== B <=> (A == A*B)
    return [equal $A [intersect $A $B]]
}

proc ::struct::set::superset {A B} {
    # A superset|== B <=> (B == A*B)
    return [equal $B [intersect $A $B]]
}

proc ::struct::set::symdifference {A B} {
    # symdiff == (A-B) + (B-A) == (A+B)-(A*B)
    if {![llength $A]} {return $B}
    if {![llength $B]} {return $A}
    return [union [difference $A $B] [difference $B $A]]
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
    if {![info exists S]} { ::set S {} }
    ::set S [union [K $S [::unset S]] {*}$args]
    return
}

proc ::struct::set::set {Svar element} {
    # Svar += element
    upvar 1 $Svar S
    if {![info exists S]} {
	::set S {}
    } else {
	::set S [lsort -unique $S]
    }
    if {$element in $S} { return 0 }
    lappend S $element
    return 1
}

proc ::struct::set::subtract {Svar args} {
    upvar 1 $Svar S
    if {![info exists S]} {
	return -code error -errorcode {STRUCT SET UNDEFINED VARIABLE} \
	    "can't read \"$Svar\": no such variable"
    }
    ::set S [difference [K $S [::unset S]] {*}$args]
    return
}

proc ::struct::set::unset {Svar element} {
    # Svar -= element
    upvar 1 $Svar S
    if {![info exists S]} {
	return -code error -errorcode {STRUCT SET UNDEFINED VARIABLE} \
	    "can't read \"$Svar\": no such variable"
    }
    ::set S [lsort -unique $S]
    if {$element ni $S} { return 0 }
    ::set pos [lsearch -exact $S $element]
    ::set S [lreplace [K $S [::unset S]] $pos $pos]
    return 1
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
