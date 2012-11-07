
[subsection {Changes for version 2}]

Version 2 introduces incompatible changes to the API of queue objects,
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
	struct::queue FOO       ; # or
	set foo [struct::queue]
}]
	whereas the new is
[example {
	struct queue create FOO ; # or
	set foo [struct queue new]
}]

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[enum] 	The methods [method peek], [method put], [method unget], and
	[method get] have been renamed to [method head],
	[method append], [method prepend], and [method {pop head}],
	respectively.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[enum] 	The methods [method all], [method at], [method tail], and
	[method {pop tail}] are new.

[list_end]
