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
## API

# ::struct::set::empty --
#
#       Determines emptiness of the set
#
# Parameters:
#       set	-- The set to check for emptiness.
#
# Results:
#       A boolean value. True indicates that the set is empty.
#
# Side effects:
#       None.
#
# Notes:

proc ::struct::set::empty {set} {
    return [expr {![llength $set]}]
}

# ::struct::set::size --
#
#	Computes the cardinality of the set.
#
# Parameters:
#	set	-- The set to inspect.
#
# Results:
#       An integer greater than or equal to zero.
#
# Side effects:
#       None.

proc ::struct::set::size {set} {
    return [llength [lsort -unique $set]]
}

# ::struct::set::contains --
#
#	Determines if the item is in the set.
#
# Parameters:
#	set	-- The set to inspect.
#	item	-- The element to look for.
#
# Results:
#	A boolean value. True indicates that the element is present.
#
# Side effects:
#       None.

proc ::struct::set::contains {set item} {
    return [expr {$item in $set}]
}

# ::struct::set::union --
#
#	Computes the union of the arguments.
#
# Parameters:
#	args	-- List of sets to unify.
#
# Results:
#	The union of the arguments.
#
# Side effects:
#       None.

proc ::struct::set::union {args} {
    ::set result {}
    foreach set $args {
	lappend result {*}$args
    }
    return [lsort -unique $result]
}

# ::struct::set::intersect --
#
#	Computes the intersection of the arguments.
#
# Parameters:
#	args	-- List of sets to intersect.
#
# Results:
#	The intersection of the arguments
#
# Side effects:
#       None.

proc ::struct::set::intersect {args} {
    switch -exact -- [llength $args] {
	0 {return {}}
	1 {return [lindex $args 0]}
    }

    foreach set [lassign $args res] {
	::set res [Intersect $res $set]
	# Quick bail out when we reach empty
	if {![llength $res]} { return {} }
    }
    return $res
}

# ::struct::set::difference --
#
#	Compute difference of two sets.
#
# Parameters:
#	A, B	-- Sets to compute the difference for.
#
# Results:
#	A - B
#
# Side effects:
#       None.

proc ::struct::set::difference {A B} {
    if {![llength $A]} {return {}}
    if {![llength $B]} {return $A}

    ::set A [lsort -unique $A]
    ::set B [lsort -unique $B]

    ::set res {}
    foreach x $A {
	if {$x in $B} continue
	lappend res $x
    }
    return $res
}

# ::struct::set::symdiff --
#
#	Compute symmetric difference of two sets.
#
# Parameters:
#	A, B	-- The sets to compute the s.difference for.
#
# Results:
#	The symmetric difference of the two input sets.
#
# Side effects:
#       None.

proc ::struct::set::symdiff {A B} {
    # symdiff == (A-B) + (B-A) == (A+B)-(A*B)
    if {![llength $A]} {return $B}
    if {![llength $B]} {return $A}
    return [union \
	    [difference $A $B] \
	    [difference $B $A]]
}

# ::struct::set::intersect3 --
#
#	Return intersection and differences for two sets.
#
# Parameters:
#	A, B	-- The sets to inspect.
#
# Results:
#	List containing A*B, A-B, and B-A
#
# Side effects:
#       None.

proc ::struct::set::intersect3 {A B} {
    return [list \
	    [intersect  $A $B] \
	    [difference $A $B] \
	    [difference $B $A]]
}

# ::struct::set::equal --
#
#	Compares two sets for equality.
#
# Parameters:
#	a	First set to compare.
#	b	Second set to compare.
#
# Results:
#	A boolean. True if the lists are equal.
#
# Side effects:
#       None.

proc ::struct::set::equal {A B} {
    ::set A [lsort -unique $A]
    ::set B [lsort -unique $B]

    # Equal if of same cardinality and difference is empty.

    if {[llength $A] != [llength $B]} {
	return 0
    }
    return [expr {![llength [difference $A $B]]}]
}


# ::struct::set::include --
#
#	Add an element to a set.
#
# Parameters:
#	Avar	-- Reference to the set variable to extend.
#	element	-- The item to add to the set.
#
# Results:
#	None.
#
# Side effects:
#       The set in the variable referenced by Avar is extended
#	by the element (if the element was not already present).

proc ::struct::set::include {Avar element} {
    # Avar = Avar + {element}
    upvar 1 $Avar A
    if {[info exists A] && ($element in $A)} return
    lappend A $element
    return
}

# ::struct::set::exclude --
#
#	Remove an element from a set.
#
# Parameters:
#	Avar	-- Reference to the set variable to shrink.
#	element	-- The item to remove from the set.
#
# Results:
#	None.
#
# Side effects:
#       The set in the variable referenced by Avar is shrunk,
#	the element remove (if the element was actually present).

proc ::struct::set::exclude {Avar element} {
    # Avar = Avar - {element}
    upvar 1 $Avar A
    if {![info exists A]} {return -code error "can't read \"$Avar\": no such variable"}
    while {[::set pos [lsearch -exact $A $element]] >= 0} {
	::set A [lreplace [K $A [unset A]] $pos $pos]
    }
    return
}

# ::struct::set::add --
#
#	Add a set to a set. Similar to 'union', but the first argument
#	is a variable.
#
# Parameters:
#	Avar	-- Reference to the set variable to extend.
#	B	-- The set to add to the set in Avar.
#
# Results:
#	None.
#
# Side effects:
#       The set in the variable referenced by Avar is extended
#	by all the elements in B.

proc ::struct::set::add {Avar B} {
    # Avar = Avar + B
    upvar 1 $Avar A
    if {![info exists A]} {set A {}}
    ::set A [union [K $A [unset A]] $B]
    return
}

# ::struct::set::subtract --
#
#	Remove a set from a set. Similar to 'difference', but the first argument
#	is a variable.
#
# Parameters:
#	Avar	-- Reference to the set variable to shrink.
#	B	-- The set to remove from the set in Avar.
#
# Results:
#	None.
#
# Side effects:
#       The set in the variable referenced by Avar is shrunk,
#	all elements of B are removed.

proc ::struct::set::subtract {Avar B} {
    # Avar = Avar - B
    upvar 1 $Avar A
    if {![info exists A]} {return -code error "can't read \"$Avar\": no such variable"}
    ::set A [difference [K $A [unset A]] $B]
    return
}

# ::struct::set::subset --
#
#	A predicate checking if the first set is a subset
#	or equal to the second set.
#
# Parameters:
#	A	-- The possible subset.
#	B	-- The set to compare to.
#
# Results:
#	A boolean value, true if A is subset of or equal to B
#
# Side effects:
#       None.

proc ::struct::set::subset {A B} {
    # A subset|== B <=> (A == A*B)
    return [equal $A [intersect $A $B]]
}

proc ::struct::set::superset {A B} {
    # A superset|== B <=> (B == A*B)
    return [equal $B [intersect $A $B]]
}

# # ## ### ##### ######## ############# #####################
## Helper commands.

proc ::struct::set::K {x y} {::set x}

proc ::struct::set::Intersect {A B} {
    # Result is nothing if either input is nothing.
    if {![llength $A]} {return {}}
    if {![llength $B]} {return {}}

    ::set A [lsort -unique $A]
    ::set B [lsort -unique $B]

    # Search the smaller set.
    if {[llength $B] > [llength $A]} {
	::set res $A
	::set A $B
	::set B $res
    }

    ::set res {}
    foreach x $B {
	if {$x ni $A} continue
	lappend res $x
    }
    return $res
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
