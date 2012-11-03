#!/bin/sh
# -*- tcl -*- \
exec kettle -f "$0" "${1+$@}"
kettle testsuite  ../tests/queuetcl
kettle benchmarks ../benchmarks/queuetcl
kettle tcl
