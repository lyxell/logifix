# Creating your own analysis

## Expression

## Statement

### block

```erlang
block(id, statements)
```

* `id` Id
* `statements` [[Statement](#statement)]

### for_statement

```erlang
for_statement(id, init, condition, update, body)
```

* `id` Id
* `init` [LocalVariableDeclaration](#local_variable_declaration) | [StatementExpressionList](#statement_expression_list)
* `then` [Statement](#statement)
* `else` [Statement](#statement)

### enhanced_for_statement

```erlang
enhanced_for_statement(id, param, expression, body)
```

* `id` Id
* `param` [FormalParameter](#formal_parameter)
* `expression` [Expression](#expression)
* `body` [Statement](#statement)

### while_statement

```erlang
while_statement(id, condition, body)
```

* `id` Id
* `condition` [Expression](#expression)
* `body` [Statement](#statement)

### do_statement

```erlang
do_statement(id, condition, body)
```

* `id` Id
* `condition` [Expression](#expression)
* `body` [Statement](#statement)

### empty_statement

```erlang
empty_statement(id)
```

* `id` Id

### throw_statement

```erlang
throw_statement(id, expression)
```

* `id` Id
* `expression` [Expression](#expression)

### expression_statement

```erlang
expression_statement(id, expression)
```

* `id` Id
* `expression` [Expression](#expression)

### return_statement

```erlang
return_statement(id, expression)
```

* `id` Id
* `expression` [Expression](#expression)

### if_statement

```erlang
if_statement(id, condition, then, else)
```

* `id` Id
* `condition` [Expression](#expression)
* `then` [Statement](#statement)
* `else` [Statement](#statement)

### try_statement

```erlang
try_statement(id, body, catches, finally)
```

* `id` Id
* `body` [Block](#block)
* `catches` 
* `finally`

### try_with_resources_statement

```erlang
try_with_resources_statement(id, resources, body, catches, finally)
```

* `id` Id
* `resources` 
* `body` [Block](#block)
* `catches`
* `finally`

### local_variable_declaration_statement

```erlang
local_variable_declaration_statement(id, declaration)
```

* `id` Id
* `declaration` [LocalVariableDeclaration](#local_variable_declaration)

### throw_statement

```erlang
throw_statement(id, expression)
```

* `id` Id
* `expression` [Expression](#expressions)

## Class body declarations

### constructor_declaration

```erlang
constructor_declaration(id, modifiers, declarator, throws, body)
```

* `id`
* `modifiers` [[Modifier](#modifier)]
* `throws`
* `body`

### method_declaration

```erlang
method_declaration(id, modifiers, header, body)
```

* `id`
* `modifiers` [[Modifier](#modifier)]
* `header`
* `body`

## Misc

### local_variable_declaration

```erlang
local_variable_declaration(id, modifiers, type, declarators)
```

* `id`
* `modifiers` [[Modifier](#modifier)]
* `type`
* `declarators` [[VariableDeclarator](#variable_declarator)]

### variable_declarator

```erlang
variable_declarator(id, declarator_id, initializer)
```

* `id`
* `declarator_id`
* `initializer`

## Modifier

### final_modifier

### private_modifier

### protected_modifier

### public_modifier

### static_modifier

### transient_modifier

### volatile_modifier
