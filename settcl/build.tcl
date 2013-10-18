#!/bin/sh
# -*- tcl -*- \
exec kettle -f "$0" "${1+$@}"
# For kettle sources, documentation, etc. see
# - http://core.tcl.tk/akupries/kettle
# - http://chiselapp.com/user/andreas_kupries/repository/Kettle
kettle testsuite  ../tests/settcl
kettle benchmarks ../benchmarks/settcl
kettle tcl
