#!/bin/sh
# -*- tcl -*- \
exec kettle -f "$0" "${1+$@}"
kettle depends-on ../cindex ../cslice ../cqueue
kettle testsuite ../tests/queuec
kettle critcl3
