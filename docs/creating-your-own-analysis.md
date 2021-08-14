# Creating your own transformations

Logifix uses Datalog to write analyses and transformations.

The source code of all transformations can be found [here](/src/rules).

## A simple transformation

We will write a small transformation that simplifies expressions like `str.substring(13, str.length())`
to `str.substring(13)` when `str` has type `java.lang.String`.

We will use the relation `replace_node_with_string` from the Logifix API.
It has the following signature:

```prolog
replace_node_with_string(rule_name: symbol, node: id, replacement: symbol)
```

> The attribute `rule_name` is a string that identifies this transformation, it is used to refer
> to the transformation in the command line interface.
> 
> The attribute `node` is the ID of the AST node
> that will be replaced.
> 
> The attribute `replacement` is a string that contains the code fragment that will replace the AST node.

### Getting started

We start by specifying a name for this transformation and by choosing a variable name for the 
AST node that we will be replacing. We will leave the replacement string empty for now.

We also specify that the node we will be replacing is a method invocation.

```prolog
/* file: src/rules/my_custom_rule/implementation.dl */

replace_node_with_string("my_custom_rule", node, "") :-
    
    /* Match a method invocation */
    method_invocation(node, _, _, _).
```

Underscores match any value. If we were to save and run our transformation now we would replace all method invocations
with an empty string.

> The relation `method_invocation` is from the Logifix API.
> It has the following signature:
> 
> ```prolog
> method_invocation(id: id, object: id, method: symbol, arguments: id_list)
> ```

### Specifying the attributes of the method invocation

We proceed by specifying the attributes of the method invocation.

```prolog
/* file: src/rules/my_custom_rule/implementation.dl */

replace_node_with_string("my_custom_rule", node, "") :-

    /* Match a method invocation of `substring` with two arguments */
    method_invocation(node, substring_object, "substring", [arg1, [arg2, nil]]).
```

If we were to save and run our transformation now we would replace all method invocations
of `substring` that has exactly two parameters.

### Limiting by type

We proceed by limiting the matches to nodes where the object has the correct type.

```prolog
/* file: src/rules/my_custom_rule/implementation.dl */

replace_node_with_string("my_custom_rule", node, "") :-

    /* Match a method invocation of `substring` with two arguments */
    method_invocation(node, substring_object, "substring", [arg1, [arg2, nil]]),
    
    /* Limit matches to nodes where `substring_object` has type `java.lang.String` */
    has_type(substring_object, ["java.lang", "String", nil]).
```

### Limiting by argument

We continue by limiting the matches to nodes where the second argument to `substring`
is a method invocation of `length`.

```prolog
/* file: src/rules/my_custom_rule/implementation.dl */

replace_node_with_string("my_custom_rule", node, "") :-

    /* Match a method invocation of `substring` with two arguments */
    method_invocation(node, substring_object, "substring", [arg1, [arg2, nil]]),
    
    /* Limit matches to nodes where `substring_object` has type `java.lang.String` */
    has_type(substring_object, ["java.lang", "String", nil]),
    
    /* Limit matches to nodes where `arg2` is a method invocation of `length` with no arguments */
    method_invocation(arg2, length_object, "length", nil).
```

### Comparing objects

We must also make sure that `length_object` and `substring_object` refer to the same object.
We do this by checking that they are declared at the same location.

```prolog
/* file: src/rules/my_custom_rule/implementation.dl */

replace_node_with_string("my_custom_rule", node, "") :-

    /* Match a method invocation of `substring` with two arguments */
    method_invocation(node, substring_object, "substring", [arg1, [arg2, nil]]),
    
    /* Limit matches to nodes where `substring_object` has type `java.lang.String` */
    has_type(substring_object, ["java.lang", "String", nil]),
    
    /* Limit matches to nodes where `arg2` is a method invocation of `length` with no arguments */
    method_invocation(arg2, length_object, "length", nil),
    
    /* Limit matches to nodes where `substring_object` and `length_object` are declared at the same location */
    point_of_declaration(substring_object, decl),
    point_of_declaration(length_object, decl).
```

We are now ready to build the string that we will use to replace the AST node.

### Building the replacement string
