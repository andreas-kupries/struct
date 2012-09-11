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
command. The name is auto-generated and of the form
[const ::struct::stack][var N] where [var N] is a random integer
number.

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
[call [arg stackObj] [method destroy]]

This method destroys the stack instance and the associated Tcl
command, releasing any associated resurces.

[para] The method returns the empty as its result.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method size]]

This method returns the number of elements stored in the stack.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method get]]

This method returns all elements stored in the stack as a list,
without removing them from the stack. The topmost element is returned
as the first element of the list, while the bottom-most element is the
last.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method getr]]

This method is a variant of method [method get] which returns the
elements in reversed order. This means that the topmost element is
returned as the last element of the list, while the bottom-most
element is the first.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method peek] [opt [arg count]]]

This method returns the top [arg count] elements of the stack, without
removing them from the stack. The count defaults to [const 1].

[para] If more than one element is requested the result will be a list
of elements, with the topmost element of the stack as the first
element of the result. If only one element is requested the result is
that element.

[para] An error will be thrown if the stack does not hold enough
elements to satisfy the request, i.e. less than [arg count] elements.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method peekr] [opt [arg count]]]

This method is a variant of method [method peek] which returns the
requested items in reversed order. This means that the topmost element
is returned as the last element of the list.

[para] For [arg count] == [const 1] there is no difference, of course.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method pop] [opt [arg count]]]

This method returns the top [arg count] items of the stack, and
removes them from the stack. The count defaults to [const 1].

[para] If more than one element is requested the result will be a list
of elements, with the topmost element of the stack as the first
element of the result. If only one element is requested the result is
that element.

[para] An error will be thrown if the stack does not hold enough
elements to satisfy the request, i.e. less than [arg count] elements.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method clear]]

This method removes all elements from the stack.

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

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg stackObj] [method push] [arg item] [opt [arg item...]]]

This method pushes the specified [arg item] or items onto the stack.
If more than one [arg item] is given, they will be pushed in the order
they are listed.

[list_end]

[include license.inc]
[include feedback.inc]
