[keywords {ordered list} {queue, prioritized} {priority queue}]

[comment {= = == === ===== ======== ============= =====================}]
[section {Class API}]

The [namespace ::struct] namespace contains a single command with
multiple methods for the creation of priority queue objects. These are:

[list_begin definitions]
[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct prioqueue}] [method new] [opt [arg options]...]]

This class method creates a new priority queue instance with an
associated global Tcl command and returns the fully qualified name of
this command. The name is auto-generated.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct prioqueue}] [method create] [arg name] [opt [arg options]...]]

This class method creates a new priority queue instance with an
associated global Tcl command and returns the fully qualified name of
this command. The [arg name] is chosen by the user and provided as the
argument of the method.

[list_end]

In both cases options can be used to specify the key type and ordering
of the instance. The key type options are:

[list_begin options]
[opt_def -integer]
The key is an integer number. This is the default type if it was not
specified at instance creation. The default ordering for this type is
[option -decreasing].

[opt_def -real]
The key is a floating point number. The default ordering for this type
is [option -decreasing].

[opt_def -ascii]
The key is a plain string. The default ordering for this type is
[option -increasing].

[opt_def -dict]
[opt_def -dictionary]
The key is a Tcl dictionary. The default ordering for this type is
[option -increasing].
[list_end]


The key ordering options are:

[list_begin options]
[opt_def -increasing]
The data in the queue is sorted in ascending order.
[opt_def -decreasing]
The data in the queue is sorted in descending order.
[list_end]

The default ordering is type specific, see above.

[comment {= = == === ===== ======== ============= =====================}]
[section {Instance API}]

All instances of queues created through the [sectref {Class API}]
support the following API:


[list_begin definitions]

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg prioqueueObj] [method clear]]

This method removes all elements from the priority queue.

[para] The method returns the empty string as its result.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg prioqueueObj] [method destroy]]

This method destroys the priority queue instance and the associated
Tcl command, releasing any associated resurces.

[para] The method returns the empty string as its result.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg prioqueueObj] [method peek] [opt [arg count]]]

This method returns the first [arg count] items found in the priority
queue, without removing them from the queue. The associated priorites
are [emph not] returned. The count defaults to [const 1].

[para] If more than one element is requested the result will be a list
of elements, with the front element of the queue as the first element
of the result. If only one element is requested the result is that
element.

[para] An error will be thrown if the queue does not hold enough
elements to satisfy the request, i.e. less than [arg count] elements.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg prioqueueObj] [method peekpriority] [opt [arg count]]]

This method behaves like method [method peek], except that it returns
the priorities instead of the elements themselves.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg prioqueueObj] [method pop] [opt [arg count]]]

This method behaves like method [method peek], except that it removes
the returned element(s) as well.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg prioqueueObj] [method put] [arg {item prio}] [opt [arg {item prio ...}]]]

This method adds the [arg item] or items specified to the priority
queue.

The [arg prio] value associated with each item must be a valid
priority key for this type of queue
(See the [sectref {Class API}]).

An error will be thrown if this is not the case, and nothing will be
added, even if other key/item combinations are valid.

[para] Items are inserted at their priority ranking. Items with equal
priority are added in the order they were added.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg prioqueueObj] [method remove] [arg element]]

This method removes the selected [arg element] from the priority queue.

[para] The method returns the empty string as its result.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [arg prioqueueObj] [method size]]

This method returns the number of elements stored in the priority
queue.

[list_end]

[comment {= = == === ===== ======== ============= =====================}]
[include prioqueue_changes.inc]
[include license.inc]
[include feedback.inc]
