#!/bin/sh
# -*- tcl -*- \
exec kettle -f "$0" "${1+$@}"
kettle testsuite  ../tests/settcl
kettle benchmarks ../benchmarks/settcl
kettle tcl
