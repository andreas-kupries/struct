[keywords fifo queue]

[comment {= = == === ===== ======== ============= =====================}]
[section {Class API}]

The [namespace ::struct] namespace contains a single command with
multiple methods for the creation of queue objects. These are:

[list_begin definitions]
[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct queue}] [method new]]

This class method creates a new queue instance with an associated
global Tcl command and returns the fully qualified name of this
command. The name is auto-generated.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct queue}] [method create] [arg name]]

This class method creates a new queue instance with an associated
global Tcl command and returns the fully qualified name of this
command. The [arg name] is chosen by the user and provided as the
argument of the method.

[list_end]

[comment {= = == === ===== ======== ============= =====================}]
[section {Instance API}]

All instances of queues created through the [sectref {Class API}]
support the following API:

[list_begin definitions]

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg queueObj] [cmd destroy]]

This method destroys the stack instance and the associated Tcl
command, releasing any associated resurces.

[para] The method returns the empty string as its result.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg queueObj] [cmd size]]

This method returns the number of elements stored in the queue.


[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg queueObj] [cmd peek] [opt "[arg count]"]]

This method returns the front [arg count] elements of the queue, without
removing them from the queue. The count defaults to [const 1].

[para] If more than one element is requested the result will be a list
of elements, with the front element of the queue as the first
element of the result. If only one element is requested the result is
that element.

[para] An error will be thrown if the queue does not hold enough
elements to satisfy the request, i.e. less than [arg count] elements.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg queueObj] [cmd get] [opt "[arg count]"]]

This method returns the front [arg count] elements of the queue, and
removes them from the queue. The count defaults to [const 1].

[para] If more than one element is requested the result will be a list
of elements, with the front element of the queue as the first
element of the result. If only one element is requested the result is
that element.

[para] An error will be thrown if the queue does not hold enough
elements to satisfy the request, i.e. less than [arg count] elements.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg queueObj] [cmd clear]]

This method removes all elements from the queue.

[para] The method returns the empty string as its result.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg queueObj] [cmd put] [arg item] [opt [arg ...]]]

This method adds the specified [arg item] or items at the end of the queue.
If more than one [arg item] is given, they will be added in the order
they are listed.

[para] The method returns the empty string as its result.

[para] The pair of methods [method put] and [method get] manage a queue.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg queueObj] [cmd unget] [arg item] [opt [arg ...]]]

This method is a variant of [method put] above, except that it adds the
elements at the front of the queue.

[para] The pair of methods [method unget] and [method get] manage a stack.

[list_end]

[comment {= = == === ===== ======== ============= =====================}]
[include license.inc]
[include feedback.inc]
