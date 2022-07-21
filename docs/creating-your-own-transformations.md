# Creating your own transformations

Logifix uses Datalog to write analyses and transformations.

The source code of all transformations can be found
[here](/src/rules).

## A simple transformation

We will write a small transformation that simplifies Java
expressions like `str.substring(13, str.length())` to
`str.substring(13)` when `str` has type `java.lang.String`.

We will test our transformation on the following file:

```java
/* file: Test.java */

class Test {

    class Inner {
        Inner substring(int start, int end) {
            return this;
        }
        int length() {
            return 0;
        }
    }

    void testInner(Inner inner) {
        System.out.println(inner.substring(3, inner.length()));
    }

    void testString(String str) {
        System.out.println(str.substring(3, str.length()));
    }

    void testStringNoMatch(String str) {
        System.out.println(str.substring(2));
    }

}

```

## Getting started

### Step 1

We start by creating a new folder to hold the files related to
our new transformation.

```bash
mkdir src/rules/my_custom_rule
echo '{"description": "My custom rule"}' > src/rules/my_custom_rule/data.json
touch src/rules/my_custom_rule/implementation.dl
```

### Step 2

We must also edit `src/program.dl` and comment out the includes
to all other rules, especially rule `simplify_calls_to_string_substring` since
this rule already implements the functionality from this tutorial.

### Step 3

We then open `src/rules/my_custom_rule/implementation.dl` and we start
by specifying a name for this transformation and by choosing a
variable name for the AST node that we will be replacing. 
We also specify that the node we will be replacing is a method
invocation to a method called `substring`:

```prolog
/* file: src/rules/my_custom_rule/implementation.dl */

replace_node_with_string("my_custom_rule", node, "") :-
    
    /* Match a method invocation */
    method_invocation(node, _, "substring", _).
```
Note that using underscores in place of variable names in a relation will match any value.
The relations `replace_node_with_string` and `method_invocation` are
from the Logifix API, they have the following signatures:

```prolog
replace_node_with_string(rule_name: symbol, node: id, replacement: symbol)
```

> The attribute `rule_name` is a string that identifies this
> transformation, it is used to refer to the transformation in
> the command line interface.
> 
> The attribute `node` is the ID of the AST node that will be
> replaced.
> 
> The attribute `replacement` is a string that contains the code
> fragment that will replace the AST node.


```prolog
method_invocation(id: id, object: id, method: symbol, arguments: id_list)
```

> The attribute `id` is the ID of the AST node
> that represents the method invocation that is matched.
> 
> The attribute `object` is the ID of the AST node that
> is the object of the method invocation. In the case of
> an invocation to `str.substring()`, `object` would represent
> `str`.
> 
> The attribute `method` is a string that matches the method
> name of the method invocation.
> 
> The attribute `arguments` is a list of IDs that represents
> the AST nodes of the arguments to the method invocation.

### Step 4

If we were to save, recompile Logifix,
and run our transformation by invoking `logifix --accept-all --patch Test.java`
we should now get the following result:

```diff
diff --git a/Test.java b/Test.java
@@ -10,15 +10,15 @@
     }

     void testInner(Inner inner) {
-        System.out.println(inner.substring(3, inner.length()));
+        System.out.println();
     }

     void testString(String str) {
-        System.out.println(str.substring(3, str.length()));
+        System.out.println();
     }

     void testStringNoMatch(String str) {
-        System.out.println(str.substring(2));
+        System.out.println();
     }

 }
```

## Specifying the arguments of the method invocation

We proceed by specifying the arguments of the method invocation.

```prolog
/* file: src/rules/my_custom_rule/implementation.dl */

replace_node_with_string("my_custom_rule", node, "") :-

    /* Match a method invocation of `substring` with two arguments */
    method_invocation(node, _, "substring", [arg1, [arg2, nil]]).
```

If we were to save and run our transformation now we would replace all method invocations
of `substring` that has exactly two parameters. We should get the following result:

```diff
diff --git a/Test.java b/Test.java
@@ -10,11 +10,11 @@
     }

     void testInner(Inner inner) {
-        System.out.println(inner.substring(3, inner.length()));
+        System.out.println();
     }

     void testString(String str) {
-        System.out.println(str.substring(3, str.length()));
+        System.out.println();
     }

     void testStringNoMatch(String str) {
```

## Limiting by type

We proceed by limiting the matches to nodes where the object has the correct type.

```prolog
/* file: src/rules/my_custom_rule/implementation.dl */

replace_node_with_string("my_custom_rule", node, "") :-

    /* Match a method invocation of `substring` with two arguments */
    method_invocation(node, substring_object, "substring", [arg1, [arg2, nil]]),
    
    /* Limit matches to nodes where `substring_object` has type `java.lang.String` */
    has_type(substring_object, ["java.lang", "String", nil]).
```

## Limiting by argument

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

We should now get the following result:

```diff
diff --git a/Test.java b/Test.java
@@ -14,7 +14,7 @@
     }

     void testString(String str) {
-        System.out.println(str.substring(3, str.length()));
+        System.out.println();
     }

     void testStringNoMatch(String str) {
```

## Comparing objects

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

## Building the replacement string
