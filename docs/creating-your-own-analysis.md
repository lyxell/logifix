# Creating your own analysis

Logifix uses Datalog to write analyses. The source code of all
analyses can be found [here](/src/rules).

## Example

We will write a small analysis that simplifies an expression like
`str.substring(13, str.length())` into `str.substring(13)` when
`str` has type `String`.

```prolog
rewrite_node("simplify_calls_to_string_substring", id, cat(subject_str, ".substring(", arg1_str, ")")) :-

    /* match a method invocation to substring with two arguments */
    method_invocation(id, substring_subject, "substring", [arg1, [arg2, nil]]),

    /* assert that substring_subject has type String */
    has_string_type(substring_subject),
    
    /* the second argument is a method invocation to length with no arguments */
    method_invocation(arg2, length_subject, "length", nil),

    /* The substring_subject and the length subject refer to the same object */
    point_of_declaration(substring_subject, decl),
    point_of_declaration(length_subject, decl),
    
    string_representation(subject, substring_subject_str),
    string_representation(arg1, arg1_str).
```
