[keywords stack lifo]

[comment {= = == === ===== ======== ============= =====================}]
[section {Class API}]

The [namespace ::struct] namespace contains a single command with
multiple methods for the creation of stack objects. These are:

[list_begin definitions]
[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct stack}] [method new]]

This class method creates a new stack instance with an associated
global Tcl command and returns the fully qualified name of this
command. The name is auto-generated.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct stack}] [method create] [arg name]]

This class method creates a new stack instance with an associated
global Tcl command and returns the fully qualified name of this
command. The [arg name] is chosen by the user and provided as the
argument of the method.

[list_end]

[comment {= = == === ===== ======== ============= =====================}]
[section {Instance API}]

All instances of stacks created through the [sectref {Class API}]
support the following API:

[list_begin definitions]

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method at] [arg index]]

This method returns the requested element stored without removing it
from the stack. The topmost element of the stack is addressed by index
[const 0], whereas the bottommost element can be addressed as either
[var size]-1, or [const end]. This addressing order matches the order
used by method [method get] to return the stack.

[para] An error is thrown if the specified index is not within the
boundaries of the stack.

[para] Note that the method accepts for [arg index] all syntactical
forms supported by the Tcl builtin [cmd lindex].

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method bottom] [opt [arg count]]]

This method returns the bottom [arg count] elements of the stack,
without removing them from the stack. The count defaults to [const 1].

[para] If only one element is requested the result is that element. If
more than one element is requested the result will be a list of
elements, with the bottommost element of the stack returned as the
last element of the result. This matches the ordering of method
[method get].

[para] An error will be thrown if the stack does not hold enough
elements to satisfy the request, i.e. less than [arg count] elements.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method clear]]

This method removes all elements from the stack.

[para] The method returns the empty string as its result.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method destroy]]

This method destroys the stack instance and the associated Tcl
command, releasing any associated resurces.

[para] The method returns the empty string as its result.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method get]]

This method returns all elements stored in the stack as a list,
without removing them from the stack. The topmost element of the stack
is returned as the first element of the list, while the bottom-most
element of the stack is matched to the last element of the list.

[para] In other words, writing the list in left-to-right order the top
of the stack will be at the left, and the bottom at the right.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method pop] [opt [arg count]]]

This method returns the top [arg count] items of the stack, and
removes them from the stack. The count defaults to [const 1].

[para] If only one element is requested the result is that element. If
more than one element is requested the result will be a list of
elements, with the topmost element of the stack as the first element
of the result. This matches the ordering of method [method get].

[para] An error will be thrown if the stack does not hold enough
elements to satisfy the request, i.e. less than [arg count] elements.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method push] [arg item] [opt [arg item...]]]

This method pushes the specified [arg item] or items onto the stack.
If more than one [arg item] is given, they will be pushed in the order
they are listed.

[para] The method returns the empty string as its result.


[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method rotate] [arg count] [arg steps]]

This method rotates the top [arg count] elements of the stack by the
specified number of [arg steps].

[para] Rotating 1 element (i.e. "[arg count] == 1") is a no-op.
Similarly "[arg steps] == 0 mod [arg n]" is a no-op as well.

[para] Laying out the contents of the stack as returned by method
[method get], i.e. top at the left, a positive number of steps rotates
the elements left, whereas a negative [arg steps] rotates to the
right.

[para] The result of the method is the empty string.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method size]]

This method returns the number of elements stored in the stack.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method top] [opt [arg count]]]

This method returns the top [arg count] elements of the stack, without
removing them from the stack. The count defaults to [const 1].

[para] If only one element is requested the result is that element. If
more than one element is requested the result will be a list of
elements, with the topmost element of the stack returned as the first
element of the result. This matches the ordering of method [method get].

[para] An error will be thrown if the stack does not hold enough
elements to satisfy the request, i.e. less than [arg count] elements.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method trim] [opt [arg newsize]]]

This method shrinks the stack to contain at most [arg newsize]
elements and returns a list containing the elements which were
removed. Nothing is done if the stack is already at the specified
size, or smaller. In that case the result is the empty list.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method trim*] [opt [arg newsize]]]

This method is a variant of method [method trim] which does not return
the removed elements, and only performs any necessary shrinking.

[list_end]

[comment {= = == === ===== ======== ============= =====================}]
[include license.inc]
[include feedback.inc]
