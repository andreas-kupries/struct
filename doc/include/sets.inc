[keywords set union intersection difference {symmetric difference}]
[keywords emptiness membership cardinality]
[keywords inclusion exclusion]

The [namespace ::struct] namespace contains a single ensemble command
[cmd set] with multiple methods for the manipulation of set values.
These methods are:

[list_begin definitions]

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method add] [arg svar] [opt [arg set]...]]

This method manipulates the variable [arg svar], adding all elements
of the [arg set]s to the set stored in this variable. If no sets are
specified the variable is not changed. This has precendence over the
next statement.

[para] If the variable named by [arg svar] does not exist it will be
created.

[para] The result of the method is the empty string.

[para] See also the related methods [method include], [method set],
and [method union].

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method contains] [arg set] [arg element]]

This method returns a boolean value indicating if the [arg set] value
contains the named [arg element] ([const true]), or not
([const false]).

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method create] [opt [arg element]...]]

This method returns a set containing the specified [arg element]s.
Duplicates in the argumnts are removed.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method difference] [arg S] [opt [arg Si]...]]

This method computes the set difference of set value [arg S] and the
set values [arg S1], [arg S2], ..., i.e.
"([arg S] - [arg S1]) - [arg S2] ..." and returns it as the result of
the method.

[para] If no sets [arg Si] are specified then [arg S] is returned.

[para] See also the related methods [method exclude],
[method subtract], and [method unset].

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method empty] [arg set]]

This method returns a boolean value indicating if the [arg set] value
is an empty set ([const true]), or not ([const false]).

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method equal] [arg set1] [arg set2]]

This method returns a boolean value indicating if the two set values
contain the same set ([const true]) or not ([const false]).

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method exclude] [arg set] [opt [arg element]...]]

This method removes the specified [arg elements] from the [arg set] value
and returns the reduced set as the result of the method.

[para] If no elements are specified then [arg set] is returned.

[para] See also the related methods [method difference],
[method subtract], and [method unset].

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method include] [arg set] [opt [arg element]...]]

This method adds the specified [arg elements] to the [arg set] value
and returns the extended set as the result of the method.

[para] If no elements are specified then [arg set] is returned.

[para] See also the related methods [method add], [method union], and
[method set].

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method intersect] [opt [arg set]...]]

This method computes the intersection of all the specified [arg set]
values and returns it as the result of the method.

An empty set is returned if no [arg set]s are specified.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method intersect3] [arg set1] [arg set2]]

This method is a combination of the methods [method intersect] and
[method difference].

Its result is a three-element list containing "[arg set1]*[arg set2]",
"[arg set1]-[arg set2]", and "[arg set2]-[arg set1]", in this
order. In other words, the intersection of the two arguments, and
their differences.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method set] [arg svar] [opt [arg element]...]]

This method manipulates the variable [arg svar], adding the specified
[arg element]s to the set stored in this variable. If no elements are
specified the variable is not changed. This has precendence over the
next statement.

[para] If the variable named by [arg svar] does not exist it will be
created.

[para] The result of the method is the number of elements actually
added to the set in [arg svar].

[para] See also the related methods [method add], [method include],
and [method union].

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method size] [arg set]]

This method returns the cardinalty of the [arg set] value, i.e. the
number of elements contained in the set. This is an integer number
greater than or equal to zero.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method subset] [arg A] [arg B]]

This method returns a boolean value indicating if the set value
[arg A] is a subset of the set value [arg B] ([const true]), or not
([const false]).

[para] [emph Note] that a set A equal to B is a subset too.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method subtract] [arg svar] [opt [arg set]...]]

This method manipulates the variable [arg svar], removing all elements
of the [arg set]s from the set stored in this variable. If no sets are
specified the variable is not changed. This has precendence over the
next statement.

[para] If the variable named by [arg svar] does not exist and error
will be thrown.

[para] The result of the method is the empty string.

[para] See also the related methods [method difference],
[method exclude], and [method unset].

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method superset] [arg A] [arg B]]

This method returns a boolean value indicating if the set value
[arg A] is a superset of the set value [arg B] ([const true]), or not
([const false]).

[para] [emph Note] that a set A equal to B is a superset too.

[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method symdifference] [arg A] [arg B]]

This method computes the symmetric difference of the sets [arg A] and
[arg B], i.e. ("([arg A] - [arg B]) + ([arg B] - [arg A])") and
returns it as the result of the command.


[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method union] [opt [arg set]...]]

This method computes the union of [arg set] values i.e.
"([arg S1] + [arg S2]) + [arg S3] ..." and returns it as the result of
the method.

[para] An empty set is returned if no [arg set]s are specified.

[para] See also the related methods [method add], [method include],
and [method set].


[comment {- - -- --- ----- -------- ------------- ---------------------}]
[call [cmd {::struct set}] [method unset] [arg svar] [opt [arg element]...]]

This method manipulates the variable [arg svar], removing the specified
[arg element]s from the set stored in this variable. If no elements are
specified the variable is not changed. This has precendence over the
next statement.

[para] If the variable named by [arg svar] does not exist an error
will be thrown.

[para] The result of the method is the number of elements actually
removed from to the set in [arg svar].

[para] See also the related methods [method difference],
[method exclude], and [method subtract].

[list_end]

Two groups of 4 methods deserve some additional attention to explain
the similarities and differences between them. These are:

[list_begin enumerated]
[enum] [method add], [method include], [method set], and [method union]
[enum] [method difference], [method exclude], [method subtract], and [method unset]
[list_end]

Each can be organized into a 2x2 table with one axis showing the type
of the first argument (set value, or set variable), and the types of
the other arguments (elements, or set values). See below.

[example {
    Value      Variable
    -----      --------
    union      add      | Sets
    include    set      | Elements

    Value      Variable
    -----      --------
    difference subtract | Sets
    exclude    unset    | Elements
}]

[comment { [section REFERENCES] }]
[comment {= = == === ===== ======== ============= =====================}]
[include stack_changes.inc]
[include license.inc]
[include feedback.inc]
