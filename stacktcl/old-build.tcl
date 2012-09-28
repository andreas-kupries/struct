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
    catch { file attributes $path -permissions ugo+x }
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
proc Hinstall {} { return "?destination?\n\tInstall Tcl package.\n\tdestination = path of package directory, default \[info library\]." }
proc _install-tcl {{ldir {}}} { eval [lreplace [info level 0] 0 0 _install] }
proc Hinstall {} { return "?destination?\n\tInstall all packages.\n\tdestination = path of package directory, default \[info library\]." }
proc _install {{ldir {}}} {
    global packages
    if {[llength [info level 0]] < 2} {
	set ldir [info library]
    }

    # Create directories, might not exist.
    file mkdir $ldir

    foreach p $packages {
	lassign $p pdir vfile

	puts ""

	set src     $::mydir/$vfile
	set version [version $src]

	file mkdir $ldir/$pdir

	file copy -force $::mydir/$vfile       $ldir/$pdir
	file copy -force $::mydir/pkgIndex.tcl $ldir/$pdir
	file delete -force             $ldir/$pdir$version
	file rename        $ldir/$pdir $ldir/$pdir$version

	puts -nonewline "Installed package:     "
	tag ok
	puts $ldir/$pdir$version
    }
    return
}
proc Hdebug {} { return "?destination?\n\tInstall all packages, build for debugging.\n\tdestination = path of package directory, default \[info library\]." }
proc _debug {{ldir {}}} {
    if {[llength [info level 0]] < 2} {
	set ldir [info library]
    }
    _install $ldir
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
    puts "Not applicable"
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
	lassign $item pdir vfile

	set version  [version $::mydir/$vfile]

	file delete -force $dstl/$pdir$version
	puts "Removed package:     $dstl/$pdir$version"
    }
}

proc Htest {} { return "\n\tRun the package testsuites." }
proc _test {{config {}}} {
    # Build and install in a transient location for the testing, if necessary.

    # Then run the tests...
    set log [open LOG.stacktcl w]

    cd $::topdir/tests/stacktcl

    # options for tcltest. (l => line information for failed tests).
    # Note: See tcllib's sak.tcl for a more mature and featureful system of
    # running a testsuite and postprocessing results.

    package require struct::matrix

    struct::matrix M
    M add columns 5
    M add row {File Total Passed Skipped Failed}

    set ctotal   0
    set cpassed  0
    set cskipped 0
    set cfailed  0

    set pipe [open "|[info nameofexecutable] ../include/all.tcl -verbose bpstenl |& cat"]

    while {![eof $pipe]} {
	if {[gets $pipe line] < 0} continue

	puts $log $line ; # Full log.

	if {[string match "++++*" $line] ||
	    [string match "----*start" $line]} {
	    # Flash report of activity...
	    puts -nonewline "\r$line                                  "
	    flush stdout
	    continue
	}

	# Failed tests are reported immediately, in full.
	if {[string match {*error: test failed*} $line]} {
	    # shorten the shown path for the test file.
	    set r [lassign [split $line :] path]
	    set line [join [linsert $r 0 [file tail $path]] :]
	    set line [string map {{error: test } {}} $line]
	    puts \r$line\t\t
	    flush stdout
	    continue
	}

	# Collect the statistics (per .test file).
	if {![string match *Total* $line]} continue
	lassign $line file _ total _ passed _ skipped _ failed
	if {$failed}  { set failed  " $failed"  } ; # indent, stand out.
	if {$skipped} { set skipped " $skipped" } ; # indent, stand out.
	M add row [list $file $total $passed $skipped $failed]

	incr ctotal   $total
	incr cpassed  $passed
	incr cskipped $skipped
	incr cfailed  $failed
    }


    M add row {File Total Passed Skipped Failed}
    M add row [list {} $ctotal $cpassed $cskipped $cfailed]

    puts "\n"
    puts [M format 2string]
    puts ""

    return
}
proc Hdoc {} { return "?destination?\n\t(Re)Generate the embedded documentation." }
proc _doc {{dst {../embedded}}} {
    cd $::topdir/doc

    puts "Removing old documentation..."
    file delete -force $dst/man/files/stacktcl
    file delete -force $dst/www/doc/files/stacktcl

    file mkdir $dst/man/files/stacktcl
    file mkdir $dst/www/doc/files/stacktcl

    puts "Generating man pages..."
    exec 2>@ stderr >@ stdout dtplite -ext n -o $dst/man nroff stacktcl
    puts "Generating 1st html..."
    exec 2>@ stderr >@ stdout dtplite -merge -o $dst/www html stacktcl
    puts "Generating 2nd html, resolving cross-references..."
    exec 2>@ stderr >@ stdout dtplite -merge -o $dst/www html stacktcl

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
main
