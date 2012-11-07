#!/bin/sh
# -*- tcl -*- \
exec kettle -f "$0" "${1+$@}"
kettle depends-on ../cslice ../cstack
kettle critcl3
