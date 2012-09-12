#!/usr/bin/env tclsh
# -*- tcl -*-

package require Tcl 8.5

# Run all build.tcl applications found in sub-directories, passing any
# arguments.
#
# For the moment we use dict sorting and assume that this is good
# enough to handle the package build dependencies. If it isn't we have
# to go for a more explict approach.

apply {{self} {
    global argv
    set selfdir [file dirname $self]
    set sep _______________________________________________
    set noe [info nameofexecutable]

    puts ""
    foreach child [lsort -dict [glob -directory $selfdir */$self]] {
	puts "[file join {*}[lrange [file split $child] end-1 end]] $sep"

	catch {
	    exec 2>@ stderr >@ stdout $noe $child {*}$argv
	}

	puts ""
	puts ""
    }
}} [file tail [info script]]
exit
