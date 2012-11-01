#!/bin/sh
# -*- tcl -*- \
exec kettle -f "$0" "${1+$@}"
kettle depends-on ../cslice
kettle critcl3
