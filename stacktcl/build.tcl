#!/bin/sh
# -*- tcl -*- \
exec kettle -f "$0" "${1+$@}"
kettle testsuite  ../tests/stacktcl
kettle benchmarks ../benchmarks/stacktcl
kettle tcl
