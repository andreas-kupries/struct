#!/bin/sh
# -*- tcl -*- \
exec kettle -f "$0" "${1+$@}"
kettle depends-on ../cslice ../cset
kettle testsuite  ../tests/setc
kettle benchmarks ../benchmarks/setc
kettle critcl3
