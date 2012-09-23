#!/bin/sh
# -*- tcl -*- \
exec tclsh "$0" ${1+"$@"}
set me [file normalize [info script]]
set mydir  [file dirname $me]
set topdir [file dirname $mydir]
set packages {
    {struct_stack_tcl stack.tcl}
}
proc main {} {
    global argv tcl_platform tag
    set tag {}
    if {![llength $argv]} {
	if {$tcl_platform(platform) eq "windows"} {
	    set argv gui
	} else {
	    set argv help
	}
    }
    if {[catch {
	eval _$argv
    }]} usage
    exit 0
}
proc usage {{status 1}} {
    global errorInfo
    if {[info exists errorInfo] && ($errorInfo ne {}) &&
	![string match {invalid command name "_*"*} $errorInfo]
    } {
	puts stderr $::errorInfo
	exit
    }

    global argv0
    set prefix "Usage: "
    foreach c [lsort -dict [info commands _*]] {
	set c [string range $c 1 end]
	if {[catch {
	    H${c}
	} res]} {
	    puts stderr "$prefix$argv0 $c args...\n"
	} else {
	    puts stderr "$prefix$argv0 $c $res\n"
	}
	set prefix "       "
    }
    exit $status
}
proc Hrequire {} { return "\n\tReturn build requirements to run before this." }
proc _require {} { puts {} }
proc Hhelp {} { return "\n\tPrint this help" }
proc _help {} {
    usage 0
    return
}
proc Hrecipes {} { return "\n\tList all build commands, without details." }
proc _recipes {} {
    set r {}
    foreach c [info commands _*] {
	lappend r [string range $c 1 end]
    }
    puts [lsort -dict $r]
    return
}
proc Hmaindoc {} { return "?destination?\n\t(Re)Generate the embedded documentation." }
proc _maindoc {{dst {../embedded}}} {
    cd $::topdir/doc

    puts "Removing old documentation..."
    file delete -force $dst/man
    file delete -force $dst/www

    file mkdir $dst/man
    file mkdir $dst/www

    puts "Generating man pages..."
    exec 2>@ stderr >@ stdout dtplite -ext n -o $dst/man nroff .
    puts "Generating 1st html..."
    exec 2>@ stderr >@ stdout dtplite -merge -o $dst/www html .
    puts "Generating 2nd html, resolving cross-references..."
    exec 2>@ stderr >@ stdout dtplite -merge -o $dst/www html .

    cd  $dst/man
    file delete -force .idxdoc .tocdoc
    cd  ../www
    file delete -force .idxdoc .tocdoc

    return
}
proc Htextdoc {} { return "destination\n\tGenerate plain text documentation in specified directory." }
proc _textdoc {dst} {
    set destination [file normalize $dst]

    cd $::topdir/doc

    puts "Removing old text documentation at ${dst}..."
    file delete -force $destination

    file mkdir $destination

    puts "Generating pages..."
    exec 2>@ stderr >@ stdout dtplite -ext txt -o $destination text .

    cd  $destination
    file delete -force .idxdoc .tocdoc

    return
}
if 0 {proc Hfigures {} { return "\n\t(Re)Generate the figures and diagrams for the documentation." }
proc _figures {} {
    cd $::topdir/doc/stack/figures

    puts "Generating (tklib) diagrams..."
    eval [linsert [glob *.dia] 0 exec 2>@ stderr >@ stdout dia convert -t -o . png]

    return
}}
main
