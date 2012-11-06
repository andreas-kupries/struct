#!/bin/sh
# -*- tcl -*- \
exec kettle -f "$0" "${1+$@}"
kettle testsuite  ../tests/prioqueuetcl
kettle benchmarks ../benchmarks/prioqueuetcl
kettle tcl
