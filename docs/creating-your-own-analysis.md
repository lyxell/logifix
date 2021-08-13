## Creating your own analysis

### Expression

### Statement

#### for_statement

```erlang
for_statement(id, init, condition, update, body)
```

* `id` Id
* `init` [LocalVariableDeclaration](#local_variable_declaration) | [StatementExpressionList](#statement_expression_list)
* `then` [Statement](#statement)
* `else` [Statement](#statement)

#### enhanced_for_statement

```
enhanced_for_statement(id, param, expression, body)
```

* `id` Id
* `param` [FormalParameter](#formal_parameter)
* `expression` [Expression](#expression)
* `body` [Statement](#statement)


#### if_statement

```erlang
if_statement(id, condition, then, else)
```

* `id` Id
* `condition` [Expression](#expression)
* `then` [Statement](#statement)
* `else` [Statement](#statement)

#### return_statement

```erlang
return_statement(id, expression)
```

* `id` Id
* `expression` [Expression](#expressions)

#### throw_statement

```erlang
throw_statement(id, expression)
```

* `id` Id
* `expression` [Expression](#expressions)
