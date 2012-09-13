#!/bin/sh
# -*- tcl -*- \
exec tclsh "$0" ${1+"$@"}
set me [file normalize [info script]]
set mydir  [file dirname $me]
set topdir [file dirname $mydir]
set packages {
    cstack
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
proc tag {t} {
    global tag
    set tag $t
    return
}
proc myexit {} {
    tag ok
    puts DONE
    return
}
proc log {args} {
    global tag
    set newline 1
    if {[lindex $args 0] eq "-nonewline"} {
	set newline 0
	set args [lrange $args 1 end]
    }
    if {[llength $args] == 2} {
	lassign $args chan text
	if {$chan ni {stdout stderr}} {
	    ::_puts {*}[lrange [info level 0] 1 end]
	    return
	}
    } else {
	set text [lindex $args 0]
	set chan stdout
    }
    # chan <=> tag, if not overriden
    if {[string match {Files left*} $text]} {
	set tag warn
	set text \n$text
    }
    if {$tag eq {}} { set tag $chan }
    #::_puts $tag/$text

    .t insert end-1c $text $tag
    set tag {}
    if {$newline} { 
	.t insert end-1c \n
    }

    update
    return
}
proc +x {path} {
    catch { file attributes $path -permissions u+x }
    return
}
proc grep {file pattern} {
    set lines [split [read [set chan [open $file r]]] \n]
    close $chan
    return [lsearch -all -inline -glob $lines $pattern]
}
proc version {file} {
    set provisions [grep $file {*package provide*}]
    #puts /$provisions/
    return [lindex $provisions 0 3]
}
proc Hrequire {} { return "\n\tReturn build requirements to run before this." }
proc _require {} { puts { cslice } }
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
proc Hinstall {} { return "?destination?\n\tInstall all packages.\n\tdestination = path of package directory, default \[info library\]." }
proc _install {{ldir {}} {config {}}} {
    global packages
    if {[llength [info level 0]] < 2} {
	set ldir [info library]
	set idir [file dirname [file dirname $ldir]]/include
    } else {
	set idir [file dirname $ldir]/include
    }

    # Create directories, might not exist.
    file mkdir $idir
    file mkdir $ldir

    foreach p $packages {
	puts ""

	set src     $::mydir/$p.tcl
	set version [version $src]

	file delete -force             [pwd]/BUILD.$p

	if {$config ne {}} {
	    RunCritcl -target $config -cache [pwd]/BUILD.$p -libdir $ldir -includedir $idir -pkg $src
	} else {
	    RunCritcl -cache [pwd]/BUILD.$p -libdir $ldir -includedir $idir -pkg $src
	}

	if {![file exists $ldir/$p]} {
	    set ::NOTE {warn {DONE, with FAILURES}}
	    break
	}

	file delete -force $ldir/$p$version
	file rename        $ldir/$p $ldir/$p$version

	puts -nonewline "Installed package:     "
	tag ok
	puts $ldir/$p$version
    }
    return
}
proc Hdebug {} { return "?destination?\n\tInstall all packages, build for debugging.\n\tdestination = path of package directory, default \[info library\]." }
proc _debug {{ldir {}} {config {}}} {
    global packages
    if {[llength [info level 0]] < 2} {
	set ldir [info library]
	set idir [file dirname [file dirname $ldir]]/include
    } else {
	set idir [file dirname $ldir]/include
    }

    # Create directories, might not exist.
    file mkdir $idir
    file mkdir $ldir

    foreach p $packages {
	puts ""

	set src     $::mydir/$p.tcl
	set version [version $src]

	file delete -force             [pwd]/BUILD.$p

	if {$config ne {}} {
	    RunCritcl -keep -debug all -target $config -cache [pwd]/BUILD.$p -libdir $ldir -includedir $idir -pkg $src
	} else {
	    RunCritcl -keep -debug all -cache [pwd]/BUILD.$p -libdir $ldir -includedir $idir -pkg $src
	}

	if {![file exists $ldir/$p]} {
	    set ::NOTE {warn {DONE, with FAILURES}}
	    break
	}

	file delete -force $ldir/$p$version
	file rename        $ldir/$p $ldir/$p$version

	puts -nonewline "Installed package:     "
	tag ok
	puts $ldir/$p$version
    }
    return
}
proc Hgui {} { return "\n\tInstall all packages, and application.\n\tDone from a small GUI." }
proc _gui {} {
    global INSTALLPATH
    package require Tk
    package require widget::scrolledwindow

    wm protocol . WM_DELETE_WINDOW ::_exit

    label  .l -text {Install Path: }
    entry  .e -textvariable ::INSTALLPATH
    button .i -command Install -text Install

    widget::scrolledwindow .st -borderwidth 1 -relief sunken
    text   .t
    .st setwidget .t

    .t tag configure stdout -font {Helvetica 8}
    .t tag configure stderr -background red    -font {Helvetica 12}
    .t tag configure ok     -background green  -font {Helvetica 8}
    .t tag configure warn   -background yellow -font {Helvetica 12}

    grid .l  -row 0 -column 0 -sticky new
    grid .e  -row 0 -column 1 -sticky new
    grid .i  -row 0 -column 2 -sticky new
    grid .st -row 1 -column 0 -sticky swen -columnspan 2

    grid rowconfigure . 0 -weight 0
    grid rowconfigure . 1 -weight 1

    grid columnconfigure . 0 -weight 0
    grid columnconfigure . 1 -weight 1
    grid columnconfigure . 2 -weight 0

    set INSTALLPATH [info library]

    # Redirect all output into our log window, and disable uncontrolled exit.
    rename ::puts ::_puts
    rename ::log ::puts
    rename ::exit   ::_exit
    rename ::myexit ::exit

    # And start to interact with the user.
    vwait forever
    return
}
proc Install {} {
    global INSTALLPATH NOTE
    .i configure -state disabled

    set NOTE {ok DONE}
    set fail [catch {
	_install $INSTALLPATH

	puts ""
	tag  [lindex $NOTE 0]
	puts [lindex $NOTE 1]
    } e o]

    .i configure -state normal
    .i configure -command ::_exit -text Exit -bg green

    if {$fail} {
	# rethrow
	return {*}$o $e
    }
    return
}
proc Hwrap4tea {} { return "?destination?\n\tGenerate a source package with TEA-based build system wrapped around critcl.\n\tdestination = path of source package directory, default is sub-directory 'tea' of the CWD." }
proc _wrap4tea {{dst {}}} {
    global packages

    if {[llength [info level 0]] < 2} {
	set dst [file join [pwd] tea]
    }

    file mkdir $dst

    package require critcl::app

    foreach p $packages {
	set src     $::mydir/$p.tcl
	set version [version $src]

	file delete -force             [pwd]/BUILD
	critcl::app::main [list -cache [pwd]/BUILD -libdir $dst -tea $src]
	file delete -force         $dst/$p$version
	file rename        $dst/$p $dst/$p$version

	puts "Wrapped package:     $dst/$p$version"
    }
    return
}
proc Hdrop {} { return "?destination?\n\tRemove packages.\n\tdestination = path of package directory, default \[info library\]." }
proc _drop {{dst {}}} {
    global packages

    if {[llength [info level 0]] < 2} {
	set dstl [info library]
    } else {
	set dstl $dst
    }

    foreach item $packages {
	# Package: /name/

	if {[llength $item] == 2} {
	    lassign $item vfile name
	} else {
	    lassign $item name
	    set vfile ${name}.tcl
	}

	if {$vfile ne {}} {
	    set version  [version $::mydir/$vfile]
	} else {
	    set version {}
	}

	file delete -force $dstl/$name$version
	puts "Removed package:     $dstl/$name$version"
    }
}
proc Htest {} { return "\n\tRun the package testsuites." }
proc _test {{config {}}} {
    puts {No tests available}
}
proc Hdoc {} { return "?destination?\n\t(Re)Generate the embedded documentation." }
proc _doc {{dst {../../embedded/cstack}}} {
    cd $::topdir/doc/cstack

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

    cd $::topdir/doc/stack

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

proc RunCritcl {args} {
    #puts [info level 0]
    if {![catch {
	package require critcl::app 3.1
    }]} {
	#puts "......... [package ifneeded critcl::app [package present critcl::app]]"
	critcl::app::main $args
	return
    } else {
	foreach cmd {
	    critcl3 critcl3.kit critcl3.tcl critcl3.exe
	    critcl critcl.kit critcl.tcl critcl.exe
	} {
	    # Locate the candidate.
	    set cmd [auto_execok $cmd]

	    # Ignore applications which were not found.
	    if {![llength $cmd]} continue

	    # Proper native path needed, especially on windows. On
	    # windows this also works (best) with a starpack for
	    # critcl, instead of a starkit.

	    set cmd [file nativename [lindex [auto_execok $cmd] 0]]

	    # Ignore applications which are too old to support
	    # -v|--version, or are too old as per their returned
	    # version.
	    if {[catch {
		set v [eval [list exec $cmd --version]]
	    }] || ([package vcompare $v 3.1] < 0)} continue

	    # Perform the requested action.
	    set cmd [list exec 2>@ stderr >@ stdout $cmd {*}$args]
	    #puts "......... $cmd"
	    eval $cmd
	    return
	}
    }

    puts "Unable to find a usable critcl 3.1 application (package). Stop."
    ::exit 1
}
main
