#!/bin/sh
# -*- tcl -*- \
exec kettle -f "$0" "${1+$@}"
kettle doc-destination ../embedded
kettle doc             ../doc
