#!/usr/bin/env tclsh
# -*- tcl -*-

package require Tcl 8.5

# Run all build.tcl applications found in sub-directories, passing any
# arguments.

apply {{self} {
    global argv
    set selfdir [file dirname $self]
    set sep _______________________________________________
    set noe [info nameofexecutable]

    puts ""
    foreach child [glob -directory $selfdir */$self] {
	puts "[file join {*}[lrange [file split $child] end-1 end]] $sep"

	exec 2>@ stderr >@ stdout $noe $child {*}$argv

	puts ""
	puts ""
    }
}} [file tail [info script]]
exit
