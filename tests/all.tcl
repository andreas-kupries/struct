# -*- tcl -*-
# Run all .test files (i.e. the testsuites) in the current directory.

foreach t [lsort -dict [glob *.test]] {
    source $t
}
