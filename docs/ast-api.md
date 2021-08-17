# AST API Reference
##  Top-level declarations
### import_specification

```erlang
import_specification(id: id, package: symbol, import: symbol)
```

 * `id`             Id
 * `package`        symbol
 
### class_declaration

```erlang
class_declaration(id: id, modifiers: id_list, name: id, superclass: id, superinterfaces: id_list, body: id)
```

 * `id`                 Id
 * `modifiers`          [Modifier]
 * `name`               Identifier
 * `superclass`         ClassType
 * `superinterfaces`    [ClassType]
 * `body`               ClassBody
 
### class_body

```erlang
class_body(id: id, declarations: id_list)
```

 * `id`                 Id
 * `declarations`       Declaration
 
##  Blocks and statements
### block

```erlang
block(id: id, statements: id_list)
```

 * `id`             Id
 * `statements`     Statement
 
### local_variable_declaration_statement

```erlang
local_variable_declaration_statement(id: id, declaration: id)
```

 * `id`             Id
 * `declaration`    LocalVariableDeclaration
 
### for_statement

```erlang
for_statement(id: id, init: id, condition: id, update: id, body: id)
```

 * `id`         Id
 * `init`       LocalVariableDeclaration | StatementExpressionList
 * `condition`  Expression
 * `update`     StatementExpressionList
 * `body`       Statement
 
### enhanced_for_statement

```erlang
enhanced_for_statement(id: id, param: id, expression: id, body: id)
```

 * `id`         Id
 * `param`      FormalParameter
 * `expression` Expression
 * `body`       Statement
 
### while_statement

```erlang
while_statement(id: id, condition: id, body: id)
```

 * `id`         Id
 * `condition`  Expression
 * `body`       Statement
 
### do_statement

```erlang
do_statement(id: id, condition: id, body: id)
```

 * `id`         Id
 * `condition`  Expression
 * `body`       Statement
 
### empty_statement

```erlang
empty_statement(id: id)
```

 * `id`         Id
 
### throw_statement

```erlang
throw_statement(id: id, expr: id)
```

 * `id`         Id
 * `expr`       Expression
 
### expression_statement

```erlang
expression_statement(id: id, expr: id)
```

 * `id`         Id
 * `expr`       Expression
 
### return_statement

```erlang
return_statement(id: id, expr: id)
```

 * `id`         Id
 * `expr`       Expression
 
### if_statement

```erlang
if_statement(id: id, condition: id, then: id, else: id)
```

 * `id`         Id
 * `condition`  Expression
 * `then`       Statement
 * `else`       Statement
 
### try_statement

```erlang
try_statement(id: id, body: id, catches: id_list, finally: id)
```

 * `id`         Id
 * `body`       Block
 * `catches`    [CatchBlock]
 * `finally`    FinallyBlock
 
### try_with_resources_statement

```erlang
try_with_resources_statement(id: id, resources: id_list, body: id, catches: id_list, finally: id)
```

 * `id`         Id
 * `resources`  [Resource]
 * `body`       Block
 * `finally`    FinallyBlock
 
### finally_block

```erlang
finally_block(id: id, block: id)
```

 * `id`         Id
 * `block`      Block
 
##  Expressions
### assignment_expression

```erlang
assignment_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### conditional_or_expression

```erlang
conditional_or_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### conditional_and_expression

```erlang
conditional_and_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### inclusive_or_expression

```erlang
inclusive_or_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### exclusive_or_expression

```erlang
exclusive_or_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### equals_expression

```erlang
equals_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### not_equals_expression

```erlang
not_equals_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### less_than_expression

```erlang
less_than_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### greater_than_expression

```erlang
greater_than_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### less_than_or_equals_expression

```erlang
less_than_or_equals_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### greater_than_or_equals_expression

```erlang
greater_than_or_equals_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### instanceof_expression

```erlang
instanceof_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### signed_left_shift_expression

```erlang
signed_left_shift_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### signed_right_shift_expression

```erlang
signed_right_shift_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### unsigned_right_shift_expression

```erlang
unsigned_right_shift_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### addition_expression

```erlang
addition_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### subtraction_expression

```erlang
subtraction_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### multiplication_expression

```erlang
multiplication_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### division_expression

```erlang
division_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### remainder_expression

```erlang
remainder_expression(id: id, left: id, right: id)
```

 * `id`     Id
 * `left`   Expression
 * `right`  Expression
 
### logical_complement_expression

```erlang
logical_complement_expression(id: id, expression: id)
```

 * `id`         Id
 * `expression` Expression
 
### cast_expression

```erlang
cast_expression(id: id, type: id, value: id)
```

 * `id`         Id
 * `type`       Type
 * `value`      Expression
 
### conditional_expression

```erlang
conditional_expression(id: id, cond: id, then: id, else: id)
```

 * `id`         Id
 * `condition`  Expression
 * `then`       Expression
 * `else`       Expression
 
### class_instance_creation_expression

```erlang
class_instance_creation_expression(id: id, qualifier: id, type_arguments: id, type: id, arguments: id_list, body: id)
```

 * `id`             Id
 * `qualifier`      Expression
 * `type_arguments` TypeArguments
 * `type`           Type
 * `arguments`      [Expression]
 * `body`           ClassBody
 
### lambda_expression

```erlang
lambda_expression(id: id, params: id, body: id)
```

 * `id`             Id
 * `params`         LambdaParams
 * `body`           Block | Expression
 
### lambda_params

```erlang
lambda_params(id: id, params: id_list)
```

 * `id`             Id
 * `params`         [FormalParameter]
 
### method_invocation

```erlang
method_invocation(id: id, subject: id, method: symbol, arguments: id_list)
```

 * `id`             Id
 * `subject`        Expression
 * `method`         symbol
 * `arguments`      [Expression]
 
### method_reference

```erlang
method_reference(id: id, subject: id, type_arguments: id, method: symbol)
```

 * `id`             Id
 * `subject`        Expression
 * `type_arguments` TypeArguments
 * `method`         symbol
 
### field_access

```erlang
field_access(id: id, subject: id, field: id)
```

 * `id`             Id
 * `subject`        Expression
 * `field`          Identifier
 
##  Types
### primitive_type

```erlang
primitive_type(id: id, annotations: id_list, name: id)
```

 * `id`             Id
 * `annotations`    [Annotation]
 * `name`           symbol
 
### class_type

```erlang
class_type(id: id, parent: id, name: symbol, type_arguments: id, annotations: id_list)
```

 * `id`             Id
 * `parent`         ClassType
 * `name`           symbol
 * `type_arguments` TypeArguments
 * `annotations`    [Annotation]
 
### type_arguments

```erlang
type_arguments(id: id, arguments: id_list)
```

 * `id`             Id
 * `arguments`      [ClassType]
 
