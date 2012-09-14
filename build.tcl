#!/usr/bin/env tclsh
# -*- tcl -*-

package require Tcl 8.5

set sep _______________________________________________
set noe [info nameofexecutable]

proc get {name args} {
    global noe
    if {![catch {
	exec 2>@ stderr $noe $name/build.tcl {*}$args
    } result]} {
	return $result
    }
    return {}
}

proc run {name args} {
    global noe sep
    puts "$name $sep $args"
    if {[catch {
	exec 2>@ stderr >@ stdout $noe $name/build.tcl {*}$args
    } msg]} { puts $msg }
    puts ""
    puts ""
    return
}

proc do {name} {
    global argv ok
    # Ignore if already done.
    if {[dict exists $ok $name]} return
    # Handle requirements first.
    foreach r [get $name require] { do $r }
    # Then ourselves.
    run $name {*}$argv
    dict set ok $name .
}

# Run all build.tcl applications found in sub-directories, passing any
# arguments.
#
# The children are run in arbitrary order (as returned by the OS).
# Each child C is asked for its requirements, which are then done
# before C, recursively (implicit topological sorting). Requirement
# loops are currently not detected and will cause infinite recursion
# until the stack limit.

apply {{self} {
    global ok argv
    set selfdir [file dirname $self]
    set cmd     [lindex $argv 0]


    set ok {}
    puts ""
    foreach child [lsort -dict [glob -directory $selfdir */$self]] {
	set name [lindex [file split $child] end-1]
	# ignore the children which do not support the specified command.
	if {$cmd ni [get $name recipes]} {
	    puts "$name: Not applicable"
	    continue
	}
	do $name
    }
}} [file tail [info script]]
exit
