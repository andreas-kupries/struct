#!/bin/sh
# -*- tcl -*- \
exec kettle -f "$0" "${1+$@}"
kettle depends-on ../cindex ../cslice ../cstack
kettle testsuite  ../tests/stackc
kettle benchmarks ../benchmarks/stackc
kettle critcl3
