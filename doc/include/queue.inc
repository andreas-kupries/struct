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
[call [arg queueObj] [method all]]

This method returns all elements stored in the queue as a list,
without removing them from the queue. The first element of the queue
is returned as the first element of the list, while the last element
of the queue is matched to the last element of the list.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg queueObj] [method append] [arg item] [opt [arg ...]]]

This method adds the specified [arg item] or items at the end of the queue.
If more than one [arg item] is given, they will be added in the order
they are listed.

[para] The method returns the empty string as its result.

[para] The pair of methods [method put] and [method get] manage a queue.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg queueObj] [method at] [arg index] [opt "[arg count]"]]

This method returns the segment of [arg count] elements starting at
the [arg index] of the queue, without removing them from the queue.
The count defaults to [const 1].

[para] If more than one element is requested the result will be a list
of elements, with the front element of the queue as the first
element of the result. If only one element is requested the result is
that element.

[para] An error will be thrown if the queue does not hold enough
elements to satisfy the request, i.e. [arg at]+[arg count] reaching
beyond the end of the queue.

[para] The first element of the queue is at index [const 0].

[para] Note that the method accepts for [arg index] all syntactical
forms supported by the Tcl builtin [cmd lindex].

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg queueObj] [method clear]]

This method removes all elements from the queue.

[para] The method returns the empty string as its result.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg queueObj] [method destroy]]

This method destroys the queue instance and the associated Tcl
command, releasing any associated resurces.

[para] The method returns the empty string as its result.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg queueObj] [method head] [opt "[arg count]"]]

This method returns the first [arg count] elements of the queue,
without removing them from the queue. The count defaults to [const 1].

[para] If more than one element is requested the result will be a list
of elements, with the front element of the queue as the first
element of the result. If only one element is requested the result is
that element.

[para] An error will be thrown if the queue does not hold enough
elements to satisfy the request, i.e. less than [arg count] elements.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg queueObj] [method {pop head}] [opt [arg n]]]

This method behaves like method [method head], except that it removes
the returned element(s) as well.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg queueObj] [method {pop tail}] [opt [arg n]]]

This method behaves like method [method tail], except that it removes
the returned element(s) as well.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg queueObj] [method prepend] [arg item] [opt [arg ...]]]

This method behaves like method [method append], except that it adds
the elements at the [emph beginning] of the queue.

[para] The pair of methods [method prepend] and [method {pop head}]
manage a stack.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg queueObj] [method size]]

This method returns the number of elements stored in the queue.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg queueObj] [method tail] [opt "[arg count]"]]

This method returns the last [arg count] elements of the queue,
without removing them from the queue. The count defaults to [const 1].

[para] If more than one element is requested the result will be a list
of elements, with the front element of the queue as the first
element of the result. If only one element is requested the result is
that element.

[para] An error will be thrown if the queue does not hold enough
elements to satisfy the request, i.e. less than [arg count] elements.

[list_end]

[comment {= = == === ===== ======== ============= =====================}]
[include queue_changes.inc]
[include license.inc]
[include feedback.inc]
