
[subsection {Changes for version 2}]

Version 2 introduces incompatible changes to the API of stack objects,
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
	struct::stack FOO       ; # or
	set foo [struct::stack]
}]
	whereas the new is
[example {
	struct stack create FOO ; # or
	set foo [struct stack new]
}]

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[enum]	Method [method getr] has been dropped. Use
[example {
	lreverse [FOO get]	
}]
	instead, assuming that [cmd FOO] is a stack instance.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[enum] 	The methods [method peek] and [method peekr] have been renamed
	to [method top] and [method bottom], respectively.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[enum] 	The method [method at] is new.

[list_end]
