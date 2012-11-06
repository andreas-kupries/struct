
[subsection {Changes for version 2}]

Version 2 introduces incompatible changes to the API of priority queue objects,
therefore the change to the major version number.

[para] The changes in detail are:

[list_begin enumerated]

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[enum] 	Version 2 requires Tcl 8.5 or higher. Tcl 8.4 or less is not
	supported anymore.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[enum]	Instance creation syntax has changed.
	For comparison, the old syntax is
[example {
	struct::prioqueue FOO       ; # or
	set foo [struct::prioqueue]
}]
	whereas the new is
[example {
	struct prioqueue create FOO ; # or
	set foo [struct prioqueue new]
}]

Similarly, the options specifying key type and order are now found
[emph after] the instance name, not before it.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[enum] 	The method [method get] has been renamed to [method pop].

[list_end]
