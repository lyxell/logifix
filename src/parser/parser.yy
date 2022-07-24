%require "3.5.1"
%glr-parser
%language "c++"
%locations
%parse-param {souffle::SouffleProgram *program}
%param {const char* filename}
%param {std::vector<logifix::parser::token>& tokens}
%param {size_t& pos}
%expect 1089
%expect-rr 802
%define api.location.type {logifix::parser::location}
%start root
%code requires
{
#include <souffle/SouffleInterface.h>
#include <stack>
#include <unordered_set>
#include <iomanip>
#include "parser.h"
#define YYINITDEPTH  50000
#define YYMAXDEPTH   100000
}

/* undefined token */

%token
    UNDEFINED

/* operators */
%token
    ADDITION_ASSIGNMENT             "+="
    ARROW                           "->"
    BITWISE_AND_ASSIGNMENT          "&="
    BITWISE_OR_ASSIGNMENT           "|="
    DECREMENT                       "--"
    DIVISION_ASSIGNMENT             "/="
    DOUBLECOLON                     "::"
    ELLIPSIS                        "..."
    EQUALS                          "=="
    BITWISE_EXCLUSIVE_OR_ASSIGNMENT "^="
    GREATER_THAN_OR_EQUAL           ">="
    INCREMENT                       "++"
    LESS_THAN_OR_EQUAL              "<="
    LOGICAL_AND                     "&&"
    LOGICAL_OR                      "||"
    MULTIPLICATION_ASSIGNMENT       "*="
    NOT_EQUALS                      "!="
    SIGNED_RIGHT_SHIFT_ASSIGNMENT   ">>="
    SIGNED_LEFT_SHIFT_ASSIGNMENT    "<<="
    UNSIGNED_RIGHT_SHIFT_ASSIGNMENT ">>>="
    SUBTRACTION_ASSIGNMENT          "-="
    REMAINDER_ASSIGNMENT            "%="

/* literals and identifiers */
%token
    BOOLEAN_LITERAL
    FLOATING_POINT_LITERAL
    IDENTIFIER
    INTEGER_LITERAL
    NULL_LITERAL
    STRING_LITERAL
    TEXT_BLOCK
    CHARACTER_LITERAL

/**
 * var and yield are not keywords, but rather restricted identifiers (§3.8).
 * var has special meaning as the type of a local variable declaration (§14.4,
 * §14.14.1, §14.14.2, §14.20.3) and the type of a lambda formal parameter
 * (§15.27.1). yield has special meaning in a yield statement (§14.21). All
 * invocations of a method named yield must be qualified so as to be
 * distinguished from a yield statement.
 */
%token
    VAR
    YIELD

/**
 * A further ten character sequences are restricted keywords: open, module,
 * requires, transitive, exports, opens, to, uses, provides, and with. These
 * character sequences are tokenized as keywords solely where they appear as
 * terminals in the ModuleDeclaration, ModuleDirective, and RequiresModifier
 * productions (§7.7). They are tokenized as identifiers everywhere else, for
 * compatibility with programs written before the introduction of restricted
 * keywords. There is one exception: immediately to the right of the character
 * sequence requires in the ModuleDirective production, the character sequence
 * transitive is tokenized as a keyword unless it is followed by a separator,
 * in which case it is tokenized as an identifier.
 */
%token
    OPEN
    MODULE
    REQUIRES
    TRANSITIVE
    EXPORTS
    OPENS
    TO
    USES
    PROVIDES
    WITH

/**
 * keywords
 * (https://docs.oracle.com/javase/specs/jls/se15/html/jls-3.html#jls-3.9)
 */
%token
    ABSTRACT
    CONTINUE
    FOR
    NEW
    SWITCH
    ASSERT
    DEFAULT
    IF
    PACKAGE
    SYNCHRONIZED
    BOOLEAN
    DO
    GOTO
    PRIVATE
    THIS
    BREAK
    DOUBLE
    IMPLEMENTS
    PROTECTED
    THROW
    BYTE
    ELSE
    IMPORT
    PUBLIC
    THROWS
    CASE
    ENUM
    INSTANCEOF
    RETURN
    TRANSIENT
    CATCH
    EXTENDS
    INT
    SHORT
    TRY
    CHAR
    FINAL
    INTERFACE
    STATIC
    VOID
    CLASS
    FINALLY
    LONG
    STRICTFP
    VOLATILE
    CONST
    FLOAT
    NATIVE
    SUPER
    WHILE
    UNDERSCORE

%%

root: compilation_unit { ROOT($1); }
    ;

/* Productions from §3 (Lexical Structure) */

identifier: IDENTIFIER  { $$ = ID("identifier", @$); }
          | VAR         { $$ = ID("identifier", @$); }
          | YIELD       { $$ = ID("identifier", @$); }
          | OPEN        { $$ = ID("identifier", @$); }
          | MODULE      { $$ = ID("identifier", @$); }
          | REQUIRES    { $$ = ID("identifier", @$); }
          | TRANSITIVE  { $$ = ID("identifier", @$); }
          | EXPORTS     { $$ = ID("identifier", @$); }
          | OPENS       { $$ = ID("identifier", @$); }
          | TO          { $$ = ID("identifier", @$); }
          | USES        { $$ = ID("identifier", @$); }
          | PROVIDES    { $$ = ID("identifier", @$); }
          | WITH        { $$ = ID("identifier", @$); }
          ;

identifier_opt: identifier
              | %empty      { $$ = NIL; }
              ;

type_identifier: IDENTIFIER  { $$ = ID("identifier", @$); }
               | OPEN        { $$ = ID("identifier", @$); }
               | MODULE      { $$ = ID("identifier", @$); }
               | REQUIRES    { $$ = ID("identifier", @$); }
               | TRANSITIVE  { $$ = ID("identifier", @$); }
               | EXPORTS     { $$ = ID("identifier", @$); }
               | OPENS       { $$ = ID("identifier", @$); }
               | TO          { $$ = ID("identifier", @$); }
               | USES        { $$ = ID("identifier", @$); }
               | PROVIDES    { $$ = ID("identifier", @$); }
               | WITH        { $$ = ID("identifier", @$); }
               ;

literal: INTEGER_LITERAL        { $$ = ID("integer_literal", @$); }
       | FLOATING_POINT_LITERAL { $$ = ID("floating_point_literal", @$); }
       | BOOLEAN_LITERAL        { $$ = ID("boolean_literal", @$); }
       | CHARACTER_LITERAL      { $$ = ID("character_literal", @$); }
       | STRING_LITERAL         { $$ = ID("string_literal", @$); }
       | TEXT_BLOCK             { $$ = ID("text_block", @$); }
       | NULL_LITERAL           { $$ = ID("null_literal", @$); }
       ;

/* Productions from §4 (Types, Values, and Variables) */

primitive_type: annotations_opt numeric_type   { $$ = ID("primitive_type", @$);
                                                 PARENT_LIST($$, "annotations", $1);
                                                 PARENT($$, "name", $2); }
              | annotations_opt boolean_type   { $$ = ID("primitive_type", @$);
                                                 PARENT_LIST($$, "annotations", $1);
                                                 PARENT($$, "name", $2); }

              ;

boolean_type: BOOLEAN { $$ = ID("boolean_type", @$); }
            ;

numeric_type: integral_type
            | floating_point_type
            ;

integral_type: BYTE     { $$ = ID("byte_type",  @$); }
             | SHORT    { $$ = ID("short_type", @$); }
             | INT      { $$ = ID("int_type",   @$); }
             | LONG     { $$ = ID("long_type",  @$); }
             | CHAR     { $$ = ID("char_type",  @$); }
             ;

floating_point_type: FLOAT      { $$ = ID("float_type",     @$); }
                   | DOUBLE     { $$ = ID("double_type",    @$); }
                   ;

reference_type: class_type
              | array_type
              ;

class_type:                annotations type_identifier type_arguments_or_diamond_opt { $$ = ID("class_type", @$);
                                                                            PARENT($$, "parent", NIL);
                                                                            PARENT($$, "name", $type_identifier);
                                                                            PARENT_LIST($$, "annotations", $annotations);
                                                                            PARENT($$, "type_arguments", $type_arguments_or_diamond_opt); }
          |                            type_identifier type_arguments_or_diamond_opt { $$ = ID("class_type", @$);
                                                                            PARENT($$, "parent", NIL);
                                                                            PARENT($$, "name", $type_identifier);
                                                                            PARENT_LIST($$, "annotations", NIL);
                                                                            PARENT($$, "type_arguments", $type_arguments_or_diamond_opt); }
          | class_type '.' annotations type_identifier type_arguments_or_diamond_opt { $$ = ID("class_type", @$);
                                                                            PARENT($$, "parent", $1);
                                                                            PARENT($$, "name", $type_identifier);
                                                                            PARENT_LIST($$, "annotations", $annotations);
                                                                            PARENT($$, "type_arguments", $type_arguments_or_diamond_opt); }
          | class_type '.'             type_identifier type_arguments_or_diamond_opt { $$ = ID("class_type", @$);
                                                                            PARENT($$, "parent", $1);
                                                                            PARENT($$, "name", $type_identifier);
                                                                            PARENT_LIST($$, "annotations", NIL);
                                                                            PARENT($$, "type_arguments", $type_arguments_or_diamond_opt); }
          ;

array_type: primitive_type  dims { $$ = ID("array_type", @$); PARENT($$, "type", $primitive_type); PARENT_LIST($$, "dims", $dims); }
          | class_type      dims { $$ = ID("array_type", @$); PARENT($$, "type", $class_type); PARENT_LIST($$, "dims", $dims); }
          ;

dims: dim dims  { $$ = LIST($1, $2); }
    | dim       { $$ = LIST($1, NIL); }
    ;

dims_opt: dims
        | %empty { $$ = NIL; }
        ;

dim: annotations '[' ']' { $$ = ID("dim", @$); PARENT_LIST($$, "annotations", $annotations); }
   |             '[' ']' { $$ = ID("dim", @$); PARENT_LIST($$, "annotations", NIL); }
   ;

unann_dims: unann_dim unann_dims { $$ = LIST($1, $2); }
          | unann_dim            { $$ = LIST($1, NIL); }
          ;

unann_dims_opt: unann_dims
              | %empty { $$ = NIL; }
              ;

unann_dim: '[' ']'
         ;

type_parameter: type_parameter_modifiers_opt type_identifier type_bound_opt
                { $$ = ID("type_parameter", @$);
                  PARENT_LIST($$, "modifiers", $type_parameter_modifiers_opt);
                  PARENT($$, "bound", $type_bound_opt); }
              ;

type_parameter_modifier: annotation
                       ;

type_parameter_modifiers_opt: type_parameter_modifier type_parameter_modifiers_opt  { $$ = LIST($1, $2); }
                            | %empty                                                { $$ = NIL; }
                            ;

type_bound: EXTENDS class_type additional_bounds_opt { $$ = ID("type_bound", @$);
                                                       PARENT($$, "type", $class_type);
                                                       PARENT_LIST($$, "additional_bounds", $additional_bounds_opt); }
          ;

type_bound_opt: type_bound
              | %empty { $$ = NIL; }
              ;

additional_bound: '&' class_type { $$ = $2; }
                ;

additional_bounds_opt: additional_bound additional_bounds_opt   { $$ = LIST($1, $2); }
                     | %empty                                   { $$ = NIL; }
                     ;

type_arguments: '<' type_argument_list '>' { $$ = ID("type_arguments", @$);
                                             PARENT_LIST($$, "arguments", $type_argument_list); }
              ;

type_arguments_opt: type_arguments
                  | %empty { $$ = NIL; }
                  ;

type_argument_list: type_argument ',' type_argument_list    { $$ = LIST($1, $3); }
                  | type_argument                           { $$ = LIST($1, NIL); }
                  ;

type_argument: reference_type
             | wildcard
             ;

wildcard: annotations_opt '?' wildcard_bounds_opt { $$ = ID("wildcard", @$);
                                                    PARENT_LIST($$, "annotations", $annotations_opt);
                                                    PARENT($$, "wildcard_bounds_opt", $wildcard_bounds_opt); }
        ;

wildcard_bounds: EXTENDS reference_type { $$ = ID("wildcard_bound", @$); PARENT($$, "type", $reference_type); }
               | SUPER   reference_type { $$ = ID("wildcard_bound", @$); PARENT($$, "type", $reference_type); }
               ;

wildcard_bounds_opt: wildcard_bounds
                   | %empty { $$ = NIL; }
                   ;

/* Productions from §6 (Names) */

identifiers_dot_separated: identifier '.' identifiers_dot_separated { $$ = LIST($1, $3); }
                         | identifier                               { $$ = LIST($1, NIL); }
                         ;

module_name: identifiers_dot_separated
           ;

module_names_comma_separated: module_name ',' module_names_comma_separated  { $$ = NIL; } // TODO
                            | module_name                                   { $$ = NIL; } // TODO
                            ;

package_name: identifiers_dot_separated
            ;

type_name: identifiers_dot_separated
         ;

type_names_comma_separated: type_name ',' type_names_comma_separated        { $$ = NIL; } // TODO
                          | type_name                                       { $$ = NIL; } // TODO
                          ;


expression_name: identifiers_dot_separated { $$ = ID("expression_name", @$);
                                             PARENT_LIST($$, "identifiers", $identifiers_dot_separated); }
               ;

package_or_type_name: identifiers_dot_separated
                    ;

/* Productions from §7 (Packages and Modules) */

compilation_unit: ordinary_compilation_unit
                | modular_compilation_unit
                ;

ordinary_compilation_unit: package_declaration import_declarations type_declarations
                         { $$ = ID("ordinary_compilation_unit", @$);
                           PARENT($$, "package_declaration", $package_declaration);
                           PARENT_LIST($$, "import_declarations", $import_declarations);
                           PARENT_LIST($$, "type_declarations", $type_declarations); }
                         | package_declaration import_declarations
                         { $$ = ID("ordinary_compilation_unit", @$);
                           PARENT($$, "package_declaration", $package_declaration);
                           PARENT_LIST($$, "import_declarations", $import_declarations);
                           PARENT_LIST($$, "type_declarations", NIL); }
                         | package_declaration                     type_declarations
                         { $$ = ID("ordinary_compilation_unit", @$);
                           PARENT($$, "package_declaration", $package_declaration);
                           PARENT_LIST($$, "import_declarations", NIL);
                           PARENT_LIST($$, "type_declarations", $type_declarations); }
                         | package_declaration
                         { $$ = ID("ordinary_compilation_unit", @$);
                           PARENT($$, "package_declaration", $package_declaration);
                           PARENT_LIST($$, "import_declarations", NIL);
                           PARENT_LIST($$, "type_declarations", NIL); }
                         |                     import_declarations type_declarations
                         { $$ = ID("ordinary_compilation_unit", @$);
                           PARENT($$, "package_declaration", NIL);
                           PARENT_LIST($$, "import_declarations", $import_declarations);
                           PARENT_LIST($$, "type_declarations", $type_declarations); }
                         |                     import_declarations
                         { $$ = ID("ordinary_compilation_unit", @$);
                           PARENT($$, "package_declaration", NIL);
                           PARENT_LIST($$, "import_declarations", $import_declarations);
                           PARENT_LIST($$, "type_declarations", NIL); }
                         |                                         type_declarations
                         { $$ = ID("ordinary_compilation_unit", @$);
                           PARENT($$, "package_declaration", NIL);
                           PARENT_LIST($$, "import_declarations", NIL);
                           PARENT_LIST($$, "type_declarations", $type_declarations); }
                         | %empty { $$ = NIL; }
                         ;

modular_compilation_unit: import_declarations module_declaration { $$ = ID("modular_compilation_unit", @$); PARENT_LIST($$, "import_declarations", $import_declarations); }
                        |                     module_declaration { $$ = ID("modular_compilation_unit", @$); PARENT_LIST($$, "import_declarations", NIL); }
                        ;

package_declaration: package_modifiers PACKAGE identifiers_dot_separated ';' { $$ = ID("package_declaration", @$); PARENT_LIST($$, "modifiers", $package_modifiers); }
                   |                   PACKAGE identifiers_dot_separated ';' { $$ = ID("package_declaration", @$); PARENT_LIST($$, "modifiers", NIL); }
                   ;

package_modifier: annotation
                ;

package_modifiers: package_modifier package_modifiers { $$ = LIST($1, $2); }
                 | package_modifier                   { $$ = LIST($1, NIL); }
                 ;

import_declaration: IMPORT import_specification ';' { $$ = ID("import_declaration", @$);
                                                      PARENT($$, "specification", $import_specification); }
                  | IMPORT STATIC static_import_specification ';' { $$ = ID("static_import_declaration", @$);
                                                                    PARENT($$, "specification", $static_import_specification); }
                  ;

import_declarations: import_declaration import_declarations { $$ = LIST($1, $2); }
                   | import_declaration                     { $$ = LIST($1, NIL); }
                   ;

import_specification: package_or_type_name '.' identifier           { $$ = ID("import_specification", @$);
                                                                      PARENT_LIST($$, "left_hand_side", $package_or_type_name); 
                                                                      PARENT($$, "import", $identifier); }
                    | package_or_type_name '.' on_demand_specifier  { $$ = ID("import_specification", @$);
                                                                      PARENT_LIST($$, "left_hand_side", $package_or_type_name);
                                                                      PARENT($$, "import", $on_demand_specifier); }
                    ;

static_import_specification: type_name '.' identifier           { $$ = ID("import_specification", @$);
                                                                  PARENT_LIST($$, "left_hand_side", $type_name);
                                                                  PARENT($$, "import", $identifier); }

                           | type_name '.' on_demand_specifier  { $$ = ID("import_specification", @$);
                                                                  PARENT_LIST($$, "left_hand_side", $type_name);
                                                                  PARENT($$, "import", $on_demand_specifier); }
                           ;

on_demand_specifier: '*' { $$ = ID("on_demand_specifier", @$); }
                   ;

type_declaration: class_declaration
                | interface_declaration
                | ';'                     { $$ = NIL; }
                ;

type_declarations: type_declaration type_declarations { $$ = LIST($1, $2); }
                 | type_declaration                   { $$ = LIST($1, NIL); }
                 ;

module_declaration:             OPEN MODULE identifiers_dot_separated '{' module_directives_opt '}' { $$ = ID("module_declaration", @$); }
                  |                  MODULE identifiers_dot_separated '{' module_directives_opt '}' { $$ = ID("module_declaration", @$); }
                  | annotations OPEN MODULE identifiers_dot_separated '{' module_directives_opt '}' { $$ = ID("module_declaration", @$); }
                  | annotations      MODULE identifiers_dot_separated '{' module_directives_opt '}' { $$ = ID("module_declaration", @$); }
                  ;

module_directive: REQUIRES  requires_modifiers_opt module_name              ';' { $$ = ID("module_directive", @$); }
                | EXPORTS   package_name                                    ';' { $$ = ID("module_directive", @$); }
                | EXPORTS   package_name TO module_names_comma_separated    ';' { $$ = ID("module_directive", @$); }
                | OPENS     package_name                                    ';' { $$ = ID("module_directive", @$); }
                | OPENS     package_name TO module_names_comma_separated    ';' { $$ = ID("module_directive", @$); }
                | USES      type_name                                       ';' { $$ = ID("module_directive", @$); }
                | PROVIDES  type_name WITH type_names_comma_separated       ';' { $$ = ID("module_directive", @$); }
                ;

module_directives_opt: module_directive module_directives_opt   { $$ = LIST($1, $2); }
                     | %empty                                   { $$ = NIL; }
                     ;

requires_modifier: TRANSITIVE { $$ = ID("transitive_modifier", @$); }
                 | STATIC     { $$ = ID("static_modifier", @$); }
                 ;

requires_modifiers_opt: requires_modifier requires_modifiers_opt { $$ = LIST($1, $2); }
                      | %empty                                   { $$ = NIL; }
                      ;

/* Productions from §8 (Classes) */

class_declaration: normal_class_declaration
                 | enum_declaration
                 ;

normal_class_declaration: class_modifiers CLASS type_identifier type_parameters_opt superclass_opt superinterfaces_opt class_body
                            { $$ = ID("class_declaration", @$);
                              PARENT_LIST($$, "modifiers", $class_modifiers);
                              PARENT($$, "name", $type_identifier);
                              PARENT_LIST($$, "type_parameters", $type_parameters_opt);
                              PARENT($$, "superclass", $superclass_opt);
                              PARENT_LIST($$, "superinterfaces", $superinterfaces_opt);
                              PARENT($$, "body", $class_body); }
                        |                 CLASS type_identifier type_parameters_opt superclass_opt superinterfaces_opt class_body
                            { $$ = ID("class_declaration", @$);
                              PARENT_LIST($$, "modifiers", NIL);
                              PARENT($$, "name", $type_identifier);
                              PARENT_LIST($$, "type_parameters", $type_parameters_opt);
                              PARENT($$, "superclass", $superclass_opt);
                              PARENT_LIST($$, "superinterfaces", $superinterfaces_opt);
                              PARENT($$, "body", $class_body); }
                        ;

class_modifier: annotation
              | PUBLIC      { $$ = ID("public_modifier",    @$); }
              | PROTECTED   { $$ = ID("protected_modifier", @$); }
              | PRIVATE     { $$ = ID("private_modifier",   @$); }
              | ABSTRACT    { $$ = ID("abstract_modifier",  @$); }
              | STATIC      { $$ = ID("static_modifier",    @$); }
              | FINAL       { $$ = ID("final_modifier",     @$); }
              | STRICTFP    { $$ = ID("strictfp_modifier",  @$); }
              ;

class_modifiers: class_modifier class_modifiers { $$ = LIST($1, $2); }
               | class_modifier                 { $$ = LIST($1, NIL); }
               ;

type_parameters: '<' type_parameter_list '>' { $$ = $2; }
               ;

type_parameters_opt: type_parameters
                   | %empty { $$ = NIL; }
                   ;

type_parameter_list: type_parameter ',' type_parameter_list { $$ = LIST($1, $3); }
                   | type_parameter { $$ = LIST($1, NIL); }
                   ;

superclass: EXTENDS class_type { $$ = $class_type; }
          ;

superclass_opt: superclass
              | %empty { $$ = NIL; }
              ;

superinterfaces: IMPLEMENTS interface_type_list { $$ = $interface_type_list; }
               ;

superinterfaces_opt: superinterfaces
                   | %empty { $$ = NIL; }
                   ;

interface_type_list: class_type ',' interface_type_list { $$ = LIST($1, $3); }
                   | class_type                         { $$ = LIST($1, NIL); }
                   ;

class_body: '{' class_body_declarations '}' { $$ = ID("class_body", @$); PARENT_LIST($$, "declarations", $class_body_declarations); }
          | '{' '}'                         { $$ = ID("class_body", @$); PARENT_LIST($$, "declarations", NIL); }
          ;

class_body_opt: class_body
              | %empty { $$ = NIL; }
              ;

class_body_declaration: class_member_declaration
                      | instance_initializer
                      | static_initializer
                      | constructor_declaration
                      ;

class_body_declarations: class_body_declaration class_body_declarations { $$ = LIST($1, $2); }
                       | class_body_declaration                         { $$ = LIST($1, NIL); }
                       ;

class_body_declarations_opt: class_body_declarations
                           | %empty { $$ = NIL; }
                           ;

class_member_declaration: field_declaration
                        | method_declaration
                        | class_declaration
                        | interface_declaration
                        | ';' { $$ = ID("empty_declaration", @$); }
                        ;

field_declaration:                     unann_type variable_declarator_list ';' { $$ = ID("field_declaration", @$);
                                                                                 PARENT_LIST($$, "modifiers", NIL);
                                                                                 PARENT($$, "type", $unann_type);
                                                                                 PARENT_LIST($$, "declarators", $variable_declarator_list); }

                 |  field_modifiers unann_type variable_declarator_list ';' { $$ = ID("field_declaration", @$);
                                                                              PARENT_LIST($$, "modifiers", $field_modifiers);
                                                                              PARENT($$, "type", $unann_type);
                                                                              PARENT_LIST($$, "declarators", $variable_declarator_list); }
                 ;

field_modifier: annotation
              | FINAL       { $$ = ID("final_modifier",     @$); }
              | PRIVATE     { $$ = ID("private_modifier",   @$); }
              | PROTECTED   { $$ = ID("protected_modifier", @$); }
              | PUBLIC      { $$ = ID("public_modifier",    @$); }
              | STATIC      { $$ = ID("static_modifier",    @$); }
              | TRANSIENT   { $$ = ID("transient_modifier", @$); }
              | VOLATILE    { $$ = ID("volatile_modifier",  @$); }
              ;

field_modifiers: field_modifier field_modifiers { $$ = LIST($1, $2); }
               | field_modifier { $$ = LIST($1, NIL); }
               ;

variable_declarator_list: variable_declarator ',' variable_declarator_list  { $$ = LIST($1, $3); }
                        | variable_declarator                               { $$ = LIST($1, NIL);  }
                        ;

variable_declarator: variable_declarator_id                          { $$ = ID("variable_declarator", @$);
                                                                       PARENT($$, "declarator_id", $variable_declarator_id);
                                                                       PARENT($$, "initializer", NIL); }
                   | variable_declarator_id '=' variable_initializer { $$ = ID("variable_declarator", @$);
                                                                       PARENT($$, "declarator_id", $variable_declarator_id);
                                                                       PARENT($$, "initializer", $variable_initializer); }
                   ;

variable_declarator_id: identifier      { $$ = ID("variable_declarator_id", @$);
                                          PARENT($$, "name", $identifier);
                                          PARENT_LIST($$, "dims", NIL); }
                      | identifier dims { $$ = ID("variable_declarator_id", @$);
                                          PARENT($$, "name", $identifier);
                                          PARENT_LIST($$, "dims", $dims); }
                      ;

variable_initializer: expression
                    | array_initializer
                    ;

unann_type: unann_primitive_type
          | unann_reference_type
          ;

unann_primitive_type: numeric_type { $$ = ID("primitive_type", @$);
                                     PARENT_LIST($$, "annotations", NIL);
                                     PARENT($$, "name", $1); }
                    | boolean_type { $$ = ID("primitive_type", @$);
                                     PARENT_LIST($$, "annotations", NIL);
                                     PARENT($$, "name", $1); }
                    ;

unann_reference_type: unann_class_type
                    | unann_array_type
                    ;

unann_class_type:                                      type_identifier type_arguments_opt { $$ = ID("class_type", @$);
                                                                                            PARENT($$, "name", $type_identifier);
                                                                                            PARENT($$, "type_arguments", $type_arguments_opt);
                                                                                            PARENT_LIST($$, "annotations", NIL);
                                                                                            PARENT($$, "parent", NIL); }
                | unann_class_type '.' annotations_opt type_identifier type_arguments_opt { $$ = ID("class_type", @$);
                                                                                            PARENT($$, "name", $type_identifier);
                                                                                            PARENT($$, "type_arguments", $type_arguments_opt);
                                                                                            PARENT_LIST($$, "annotations", $annotations_opt);
                                                                                            PARENT($$, "parent", $1); }
                ;

unann_array_type: unann_primitive_type dims { $$ = ID("array_type", @$);
                                              PARENT($$, "type", $unann_primitive_type);
                                              PARENT_LIST($$, "dims", $dims); }
                | unann_class_type     dims { $$ = ID("array_type", @$);
                                              PARENT($$, "type", $unann_class_type);
                                              PARENT_LIST($$, "dims", $dims); }

                ;

method_declaration: method_modifiers method_header method_body { $$ = ID("method_declaration", @$);
                                                                 PARENT_LIST($$, "modifiers", $method_modifiers);
                                                                 PARENT($$, "header", $method_header);
                                                                 PARENT($$, "body", $method_body); }
                  |                  method_header method_body { $$ = ID("method_declaration", @$);
                                                                 PARENT_LIST($$, "modifiers", NIL);
                                                                 PARENT($$, "header", $method_header);
                                                                 PARENT($$, "body", $method_body); }
                  ;

method_modifiers: method_modifier method_modifiers  { $$ = LIST($1, $2); }
                | method_modifier                   { $$ = LIST($1, NIL); }
                ;

method_modifier: annotation
               | PUBLIC       { $$ = ID("public_modifier",          @$); }
               | PROTECTED    { $$ = ID("protected_modifier",       @$); }
               | PRIVATE      { $$ = ID("private_modifier",         @$); }
               | ABSTRACT     { $$ = ID("abstract_modifier",        @$); }
               | STATIC       { $$ = ID("static_modifier",          @$); }
               | FINAL        { $$ = ID("final_modifier",           @$); }
               | SYNCHRONIZED { $$ = ID("synchronized_modifier",    @$); }
               | NATIVE       { $$ = ID("native_modifier",          @$); }
               | STRICTFP     { $$ = ID("strictfp_modifier",        @$); }
               ;

method_header:                                 result method_declarator throws_opt  { $$ = ID("method_header", @$);
                                                                                      PARENT_LIST($$, "type_parameters", NIL);
                                                                                      PARENT($$, "result", $result);
                                                                                      PARENT($$, "declarator", $method_declarator);
                                                                                      PARENT_LIST($$, "throws", $throws_opt); }
             | type_parameters annotations_opt result method_declarator throws_opt  { $$ = ID("method_header", @$);
                                                                                      PARENT_LIST($$, "type_parameters", $type_parameters);
                                                                                      PARENT($$, "result", $result);
                                                                                      PARENT($$, "declarator", $method_declarator);
                                                                                      PARENT_LIST($$, "throws", $throws_opt); }
             ;

result: unann_type
      | VOID { $$ = ID("void_type", @$); }
      ;

method_declarator: identifier '('                        formal_parameter_list_opt ')' dims_opt
                                                                 { $$ = ID("method_declarator", @$);
                                                                   PARENT($$, "name", $identifier);
                                                                   PARENT_LIST($$, "params", $formal_parameter_list_opt); }
                 | identifier '(' receiver_parameter ',' formal_parameter_list_opt ')' dims_opt
                                                                 { $$ = ID("method_declarator", @$); }
                 ;

receiver_parameter: annotations unann_type THIS                 { $$ = ID("receiver_parameter", @$); }
                  |             unann_type THIS                 { $$ = ID("receiver_parameter", @$); }
                  | annotations unann_type identifier '.' THIS  { $$ = ID("receiver_parameter", @$); }
                  |             unann_type identifier '.' THIS  { $$ = ID("receiver_parameter", @$); }
                  ;

formal_parameter_list: formal_parameter ',' formal_parameter_list   { $$ = LIST($1, $3); }
                     | formal_parameter                             { $$ = LIST($1, NIL); }
                     ;

formal_parameter_list_opt: formal_parameter_list
                         | %empty { $$ = NIL; }
                         ;

formal_parameter: variable_modifiers unann_type variable_declarator_id { $$ = ID("formal_parameter", @$);
                                                                         PARENT_LIST($$, "modifiers", $variable_modifiers);
                                                                         PARENT($$, "type", $unann_type);
                                                                         PARENT($$, "declarator_id", $variable_declarator_id); }
                |                    unann_type variable_declarator_id { $$ = ID("formal_parameter", @$);
                                                                         PARENT_LIST($$, "modifiers", NIL);
                                                                         PARENT($$, "type", $unann_type);
                                                                         PARENT($$, "declarator_id", $variable_declarator_id); }
                | variable_arity_parameter
                ;

variable_arity_parameter: variable_modifiers unann_type annotations_opt "..." identifier
                                { $$ = ID("variable_arity_parameter", @$);
                                  PARENT_LIST($$, "modifiers", $variable_modifiers);
                                  PARENT($$, "type", $unann_type);
                                  PARENT_LIST($$, "annotations", $annotations_opt);
                                  PARENT($$, "name", $identifier); }
                        |                    unann_type annotations_opt "..." identifier
                                { $$ = ID("variable_arity_parameter", @$);
                                  PARENT_LIST($$, "modifiers", NIL);
                                  PARENT($$, "type", $unann_type);
                                  PARENT_LIST($$, "annotations", $annotations_opt);
                                  PARENT($$, "name", $identifier); }

                        ;

variable_modifier: annotation
                 | FINAL { $$ = ID("final_modifier", @$); }
                 ;

variable_modifiers: variable_modifier variable_modifiers { $$ = LIST($1, $2); }
                  | variable_modifier                    { $$ = LIST($1, NIL); }
                  ;

throws: THROWS exception_type_list { $$ = $2; }
      ;

throws_opt: throws
          | %empty { $$ = NIL; }
          ;

exception_type_list: exception_type ',' exception_type_list { $$ = LIST($1, $3); }
                   | exception_type                         { $$ = LIST($1, NIL); }
                   ;

// type variable removed to handle ambiguity
exception_type: class_type
              ;

method_body: block
           | ';' { $$ = NIL; }
           ;

instance_initializer: block
                    ;

static_initializer: STATIC block { $$ = ID("static_initializer", @$); PARENT($$, "block", $block); }
                  ;

constructor_declaration: constructor_modifiers constructor_declarator throws_opt constructor_body { $$ = ID("constructor_declaration", @$);
                                                                                                    PARENT_LIST($$, "modifiers", $constructor_modifiers);
                                                                                                    PARENT($$, "declarator", $constructor_declarator);
                                                                                                    PARENT_LIST($$, "throws", $throws_opt);
                                                                                                    PARENT($$, "body", $constructor_body); }

                       |                       constructor_declarator throws_opt constructor_body { $$ = ID("constructor_declaration", @$);
                                                                                                    PARENT_LIST($$, "modifiers", NIL);
                                                                                                    PARENT($$, "declarator", $constructor_declarator);
                                                                                                    PARENT_LIST($$, "throws", $throws_opt);
                                                                                                    PARENT($$, "body", $constructor_body); }

                       ;

constructor_modifier: annotation
                    | PUBLIC      { $$ = ID("public_modifier", @$); }
                    | PROTECTED   { $$ = ID("protected_modifier", @$); }
                    | PRIVATE     { $$ = ID("private_modifier", @$); }
                    ;

constructor_modifiers: constructor_modifier constructor_modifiers   { $$ = LIST($1, $2); }
                     | constructor_modifier                         { $$ = LIST($1, NIL); }
                     ;

constructor_declarator: type_parameters_opt simple_type_name '(' receiver_parameter ',' formal_parameter_list_opt ')' { $$ = ID("constructor_declarator", @$);
                                                                                                                        PARENT_LIST($$, "params", $formal_parameter_list_opt); }
                      | type_parameters_opt simple_type_name '(' formal_parameter_list_opt ')'                        { $$ = ID("constructor_declarator", @$);
                                                                                                                        PARENT_LIST($$, "params", $formal_parameter_list_opt); }
                      ;

simple_type_name: type_identifier
                ;

constructor_body: '{' explicit_constructor_invocation_opt block_statements_opt '}' { $$ = ID("constructor_body", @$);
                                                                                     PARENT($$, "invocation", $explicit_constructor_invocation_opt);
                                                                                     PARENT_LIST($$, "statements", $block_statements_opt); }
                ;

explicit_constructor_invocation: type_arguments_opt                         THIS '(' argument_list_opt ')' ';' { $$ = ID("constructor_invocation", @$); PARENT_LIST($$, "arguments", $argument_list_opt); }
                               | type_arguments_opt                        SUPER '(' argument_list_opt ')' ';' { $$ = ID("constructor_invocation", @$); PARENT_LIST($$, "arguments", $argument_list_opt); }
                               | expression_name    '.' type_arguments_opt SUPER '(' argument_list_opt ')' ';' { $$ = ID("constructor_invocation", @$); PARENT_LIST($$, "arguments", $argument_list_opt); }
                               | primary            '.' type_arguments_opt SUPER '(' argument_list_opt ')' ';' { $$ = ID("constructor_invocation", @$); PARENT_LIST($$, "arguments", $argument_list_opt); }
                               ;

explicit_constructor_invocation_opt: explicit_constructor_invocation
                                   | %empty { $$ = NIL; }
                                   ;

enum_declaration: class_modifiers ENUM type_identifier superinterfaces_opt enum_body { $$ = ID("enum_declaration", @$);
                                                                                       PARENT_LIST($$, "modifiers", $class_modifiers);
                                                                                       PARENT($$, "name", $type_identifier);
                                                                                       PARENT_LIST($$, "superinterfaces", $superinterfaces_opt);
                                                                                       PARENT($$, "body", $enum_body); }
                                                                                       
                |                 ENUM type_identifier superinterfaces_opt enum_body { $$ = ID("enum_declaration", @$);
                                                                                       PARENT_LIST($$, "modifiers", NIL);
                                                                                       PARENT($$, "name", $type_identifier);
                                                                                       PARENT_LIST($$, "superinterfaces", $superinterfaces_opt);
                                                                                       PARENT($$, "body", $enum_body); }
                ;

enum_body: '{' enum_constant_list_opt ',' enum_body_declarations_opt '}' { $$ = ID("enum_body", @$);
                                                                           PARENT_LIST($$, "constants", $enum_constant_list_opt);
                                                                           PARENT_LIST($$, "declarations", $enum_body_declarations_opt); }
         | '{' enum_constant_list_opt     enum_body_declarations_opt '}' { $$ = ID("enum_body", @$);
                                                                           PARENT_LIST($$, "constants", $enum_constant_list_opt);
                                                                           PARENT_LIST($$, "declarations", $enum_body_declarations_opt); }
         ;

enum_constant_list: enum_constant ',' enum_constant_list    { $$ = LIST($1, $3); }
                  | enum_constant                           { $$ = LIST($1, NIL); }
                  ;

enum_constant_list_opt: enum_constant_list
                      | %empty { $$ = NIL; }
                      ;

enum_constant: enum_constant_modifiers_opt identifier '(' argument_list_opt ')' class_body_opt
                { $$ = ID("enum_constant", @$);
                  PARENT_LIST($$, "modifiers", $enum_constant_modifiers_opt);
                  PARENT($$, "name", $identifier);
                  PARENT_LIST($$, "argument_list", $argument_list_opt);
                  PARENT($$, "body", $class_body_opt); }
             | enum_constant_modifiers_opt identifier                           class_body_opt
                { $$ = ID("enum_constant", @$);
                  PARENT_LIST($$, "modifiers", $enum_constant_modifiers_opt);
                  PARENT($$, "name", $identifier);
                  PARENT_LIST($$, "argument_list", NIL);
                  PARENT($$, "body", $class_body_opt); }
             ;

enum_constant_modifier: annotation
                      ;

enum_constant_modifiers_opt: enum_constant_modifier enum_constant_modifiers_opt { $$ = LIST($1, $2); }
                           | %empty                                             { $$ = NIL; }
                           ;

enum_body_declarations: ';' class_body_declarations_opt { $$ = $2; }
                      ;

enum_body_declarations_opt: enum_body_declarations
                          | %empty { $$ = NIL; }
                          ;

/* Productions from §9 (Interfaces) */

interface_declaration: normal_interface_declaration
                     | annotation_type_declaration
                     ;

normal_interface_declaration: interface_modifiers INTERFACE type_identifier type_parameters_opt extends_interfaces_opt interface_body
                              { $$ = ID("interface_declaration", @$);
                                PARENT_LIST($$, "modifiers", $interface_modifiers);
                                PARENT($$, "name", $type_identifier);
                                PARENT_LIST($$, "type_parameters", $type_parameters_opt);
                                PARENT_LIST($$, "extends", $extends_interfaces_opt);
                                PARENT_LIST($$, "body", $interface_body); }
                            |                     INTERFACE type_identifier type_parameters_opt extends_interfaces_opt interface_body
                              { $$ = ID("interface_declaration", @$);
                                PARENT_LIST($$, "modifiers", NIL);
                                PARENT($$, "name", $type_identifier);
                                PARENT_LIST($$, "type_parameters", $type_parameters_opt);
                                PARENT_LIST($$, "extends", $extends_interfaces_opt);
                                PARENT_LIST($$, "body", $interface_body); }
                            ;

interface_modifier: annotation
                  | PUBLIC      { $$ = ID("public_modifier", @$); }
                  | PROTECTED   { $$ = ID("protected_modifier", @$); }
                  | PRIVATE     { $$ = ID("private_modifier", @$); }
                  | ABSTRACT    { $$ = ID("abstract_modifier", @$); }
                  | STATIC      { $$ = ID("static_modifier", @$); }
                  | STRICTFP    { $$ = ID("strictfp_modifier", @$); }
                  ;

interface_modifiers: interface_modifier interface_modifiers { $$ = LIST($1, $2); }
                   | interface_modifier                     { $$ = LIST($1, NIL); }
                   ;

extends_interfaces: EXTENDS interface_type_list { $$ = $2; }
                 ;

extends_interfaces_opt: extends_interfaces
                      | %empty { $$ = NIL; }
                      ;

interface_body: '{' interface_member_declarations_opt '}' { $$ = $2; }
              ;

interface_member_declaration: constant_declaration
                            | interface_method_declaration
                            | class_declaration
                            | interface_declaration
                            | ';' { $$ = NIL; }
                            ;

interface_member_declarations_opt: interface_member_declaration interface_member_declarations_opt   { $$ = LIST($1, $2); }
                                 | %empty                                                           { $$ = NIL; }
                                 ;

constant_declaration: constant_modifiers unann_type variable_declarator_list ';'
                        { $$ = ID("constant_declaration", @$);
                          PARENT_LIST($$, "modifiers", $constant_modifiers);
                          PARENT($$, "type", $unann_type);
                          PARENT_LIST($$, "declarators", $variable_declarator_list); }
                    |                    unann_type variable_declarator_list ';'
                        { $$ = ID("constant_declaration", @$);
                          PARENT_LIST($$, "modifiers", NIL);
                          PARENT($$, "type", $unann_type);
                          PARENT_LIST($$, "declarators", $variable_declarator_list); }
                    ;

constant_modifier: annotation
                 | PUBLIC      { $$ = ID("public_modifier", @$); }
                 | STATIC      { $$ = ID("static_modifier", @$); }
                 | FINAL       { $$ = ID("final_modifier", @$); }
                 ;

constant_modifiers: constant_modifier constant_modifiers    { $$ = LIST($1, $2); }
                  | constant_modifier                       { $$ = LIST($1, NIL); }
                  ;

interface_method_declaration: interface_method_modifiers method_header method_body { $$ = ID("method_declaration", @$);
                                                                                     PARENT_LIST($$, "modifiers", $interface_method_modifiers);
                                                                                     PARENT($$, "header", $method_header);
                                                                                     PARENT($$, "body", $method_body); }
                            |                            method_header method_body { $$ = ID("method_declaration", @$);
                                                                                     PARENT_LIST($$, "modifiers", NIL);
                                                                                     PARENT($$, "header", $method_header);
                                                                                     PARENT($$, "body", $method_body); }
                            ;

interface_method_modifier: annotation
                         | PUBLIC      { $$ = ID("public_modifier", @$); }
                         | PRIVATE     { $$ = ID("private_modifier", @$); }
                         | ABSTRACT    { $$ = ID("abstract_modifier", @$); }
                         | DEFAULT     { $$ = ID("default_modifier", @$); }
                         | STATIC      { $$ = ID("static_modifier", @$); }
                         | STRICTFP    { $$ = ID("strictfp_modifier", @$); }
                         ;

interface_method_modifiers: interface_method_modifier interface_method_modifiers { $$ = LIST($1, $2); }
                          | interface_method_modifier                            { $$ = LIST($1, NIL); }
                          ;

annotation_type_declaration: interface_modifiers '@' INTERFACE type_identifier annotation_type_body
                                    { $$ = ID("annotation_type_declaration", @$);
                                      PARENT_LIST($$, "modifiers", $interface_modifiers);
                                      PARENT($$, "name", $type_identifier);
                                      PARENT_LIST($$, "body", $annotation_type_body); }
                           |                     '@' INTERFACE type_identifier annotation_type_body
                                    { $$ = ID("annotation_type_declaration", @$);
                                      PARENT_LIST($$, "modifiers", NIL);
                                      PARENT($$, "name", $type_identifier);
                                      PARENT_LIST($$, "body", $annotation_type_body); }
                           ;

annotation_type_body: '{' annotation_type_member_declarations_opt '}' { $$ = $2; }
                    ;

annotation_type_member_declaration: annotation_type_element_declaration
                                  | constant_declaration
                                  | class_declaration
                                  | interface_declaration
                                  | ';' { $$ = NIL; }
                                  ;

annotation_type_member_declarations_opt: annotation_type_member_declaration annotation_type_member_declarations_opt
                                            { $$ = LIST($1, $2); }
                                       | %empty 
                                            { $$ = NIL; }
                                       ;

annotation_type_element_declaration: annotation_type_element_modifiers unann_type identifier '(' ')' dims_opt default_value_opt ';'
                                            { $$ = ID("annotation_type_element_declaration", @$);
                                              PARENT_LIST($$, "modifiers", $annotation_type_element_modifiers);
                                              PARENT($$, "type", $unann_type);
                                              PARENT($$, "name", $identifier);
                                              PARENT_LIST($$, "dims", $dims_opt);
                                              PARENT($$, "default_value", $default_value_opt); }
                                   |                                   unann_type identifier '(' ')' dims_opt default_value_opt ';'
                                            { $$ = ID("annotation_type_element_declaration", @$);
                                              PARENT($$, "modifiers", NIL);
                                              PARENT($$, "type", $unann_type);
                                              PARENT($$, "name", $identifier);
                                              PARENT_LIST($$, "dims", $dims_opt);
                                              PARENT($$, "default_value", $default_value_opt); }
                                   ;

annotation_type_element_modifier: annotation
                                | PUBLIC      { $$ = ID("public_modifier", @$); }
                                | ABSTRACT    { $$ = ID("abstract_modifier", @$); }
                                ;

annotation_type_element_modifiers: annotation_type_element_modifier annotation_type_element_modifiers   { $$ = LIST($1, $2); }
                                 | annotation_type_element_modifier                                     { $$ = LIST($1, NIL); }

default_value: DEFAULT element_value { $$ = $2; }
             ;

default_value_opt: default_value
                 | %empty { $$ = NIL; }
                 ;

annotation: normal_annotation
          | marker_annotation
          | single_element_annotation
          ;

annotations_opt: annotation annotations_opt { $$ = LIST($1, $2); }
               | %empty                     { $$ = NIL; }
               ;

annotations: annotation annotations { $$ = LIST($1, $2); }
           | annotation             { $$ = LIST($1, NIL); }
           ;

normal_annotation: '@' type_name '(' element_value_pair_list_opt ')'
                            { $$ = ID("annotation", @$);
                               PARENT_LIST($$, "type", $type_name);
                               PARENT_LIST($$, "values", $element_value_pair_list_opt); }
                 ;

element_value_pair_list: element_value_pair ',' element_value_pair_list { $$ = LIST($1, $3); }
                       | element_value_pair                             { $$ = LIST($1, NIL); }
                       ;

element_value_pair_list_opt: element_value_pair_list
                           | %empty { $$ = NIL; }
                           ;

element_value_pair: identifier '=' element_value { $$ = ID("element_value_pair", @$);
                                                   PARENT($$, "key", $identifier);
                                                   PARENT($$, "element_value", $element_value); }
                  ;

element_value: conditional_expression
             | element_value_array_initializer
             | annotation
             ;

element_value_array_initializer: '{' element_value_list_opt ',' '}'
                                        { $$ = ID("element_value_array_initializer", @$);
                                          PARENT_LIST($$, "element_value_list_opt", $element_value_list_opt); }
                               | '{' element_value_list_opt '}'
                                        { $$ = ID("element_value_array_initializer", @$);
                                          PARENT_LIST($$, "element_value_list_opt", $element_value_list_opt); }

                               ;

element_value_list: element_value ',' element_value_list { $$ = LIST($1, $3); }
                  | element_value                        { $$ = LIST($1, NIL); }
                  ;

element_value_list_opt: element_value_list
                      | %empty              { $$ = NIL; }

marker_annotation: '@' type_name { $$ = ID("annotation", @$);
                                   PARENT_LIST($$, "type", $type_name); }
                 ;

single_element_annotation: '@' type_name '(' element_value ')' { $$ = ID("annotation", @$);
                                                                  PARENT_LIST($$, "type", $type_name);
                                                                  PARENT($$, "value", $element_value); }
                         ;

/* Productions from §10 (Arrays) */

array_initializer: '{' variable_initializer_list ',' '}' { $$ = ID("array_initializer", @$); PARENT_LIST($$, "initializers", $variable_initializer_list); }
                 | '{' variable_initializer_list '}'     { $$ = ID("array_initializer", @$); PARENT_LIST($$, "initializers", $variable_initializer_list); }
                 | '{'  '}'                              { $$ = ID("array_initializer", @$); PARENT_LIST($$, "initializers", NIL); }
                 | '{' ',' '}'                           { $$ = ID("array_initializer", @$); PARENT_LIST($$, "initializers", NIL); }
                 ;

variable_initializer_list: variable_initializer ',' variable_initializer_list   { $$ = LIST($1, $3); }
                         | variable_initializer                                 { $$ = LIST($1, NIL); }
                         ;

/* Productions from §14 (Blocks and statements) */

block: '{' block_statements '}' { $$ = ID("block", @$);
                                  PARENT_LIST($$, "statements", $block_statements); }
     | '{' '}'                  { $$ = ID("block", @$);
                                  PARENT_LIST($$, "statements", NIL); }
     ;

block_statements: block_statement block_statements  { $$ = LIST($1, $2); }
                | block_statement                   { $$ = LIST($1, NIL); }
                ;

block_statements_opt: block_statements 
                    | %empty { $$ = NIL; }
                    ;

block_statement: local_variable_declaration_statement
               | class_declaration
               | statement
               ;

local_variable_declaration_statement: local_variable_declaration ';' { $$ = ID("local_variable_declaration_statement", @$);
                                                                       PARENT($$, "declaration", $local_variable_declaration); }
                                    ;

local_variable_declaration: variable_modifiers local_variable_type variable_declarator_list { $$ = ID("local_variable_declaration", @$);
                                                                                              PARENT_LIST($$, "modifiers", $variable_modifiers);
                                                                                              PARENT($$, "type", $local_variable_type);
                                                                                              PARENT_LIST($$, "declarators", $variable_declarator_list); }
                        
                          |                    local_variable_type variable_declarator_list { $$ = ID("local_variable_declaration", @$);
                                                                                              PARENT_LIST($$, "modifiers", NIL);
                                                                                              PARENT($$, "type", $local_variable_type);
                                                                                              PARENT_LIST($$, "declarators", $variable_declarator_list); }
                          ;

local_variable_type: unann_type
                   | VAR { $$ = ID("var_type", @$); }
                   ;

statement: statement_without_trailing_substatement
         | labeled_statement
         | if_then_statement
         | if_then_else_statement
         | while_statement
         | for_statement
         ;

statement_no_short_if: statement_without_trailing_substatement
                     | labeled_statement_no_short_if
                     | if_then_else_statement_no_short_if
                     | while_statement_no_short_if
                     | for_statement_no_short_if
                     ;

statement_without_trailing_substatement: block
                                       | empty_statement
                                       | expression_statement
                                       | assert_statement
                                       | switch_statement
                                       | do_statement
                                       | break_statement
                                       | continue_statement
                                       | return_statement
                                       | synchronized_statement
                                       | throw_statement
                                       | try_statement
                                       | yield_statement
                                       ;

empty_statement: ';' { $$ = ID("empty_statement", @$); }
               ;

labeled_statement: identifier ':' statement { $$ = ID("labeled_statement", @$);
                                              PARENT($$, "label", $identifier);
                                              PARENT($$, "statement", $statement); }
                 ;

labeled_statement_no_short_if: identifier ':' statement_no_short_if { $$ = ID("labeled_statement", @$);
                                                                      PARENT($$, "label", $identifier);
                                                                      PARENT($$, "statement", $statement_no_short_if); }
                             ;

expression_statement: statement_expression ';' { $$ = ID("expression_statement", @$);
                                                 PARENT($$, "expression", $statement_expression); }

statement_expression: assignment
                    | pre_increment_expression
                    | pre_decrement_expression
                    | post_increment_expression
                    | post_decrement_expression
                    | method_invocation
                    | class_instance_creation_expression
                    ;

if_then_statement: IF '(' expression ')' statement { $$ = ID("if_statement", @$);
                                                     PARENT($$, "condition", $expression);
                                                     PARENT($$, "then", $statement);
                                                     PARENT($$, "else", NIL); }
                 ;

if_then_else_statement: IF '(' expression ')' statement_no_short_if ELSE statement { $$ = ID("if_statement", @$);
                                                                                     PARENT($$, "condition", $expression);
                                                                                     PARENT($$, "then", $statement_no_short_if);
                                                                                     PARENT($$, "else", $statement); }
                      ;

if_then_else_statement_no_short_if: IF '(' expression ')' statement_no_short_if ELSE statement_no_short_if { $$ = ID("if_statement", @$); }
                                  ;

assert_statement: ASSERT                expression ';' { $$ = ID("assert_statement", @$);
                                                         PARENT($$, "condition", $2); }
                | ASSERT expression ':' expression ';' { $$ = ID("assert_statement", @$);
                                                         PARENT($$, "condition", $2);
                                                         PARENT($$, "message", $3); }
                ;

switch_statement: SWITCH '(' expression ')' switch_block { $$ = ID("switch_statement", @$);
                                                           PARENT($$, "expression", $expression);
                                                           PARENT($$, "block", $switch_block); }
                ;

switch_block: '{' switch_rules '}' { $$ = ID("switch_block", @$); PARENT($$, "rules", $switch_rules); }
            | '{' switch_block_statement_groups_opt switch_labels_with_colon_opt '}'
                    { $$ = ID("switch_block", @$);
                      PARENT_LIST($$, "statement_groups", $switch_block_statement_groups_opt);
                      PARENT_LIST($$, "labels", $switch_labels_with_colon_opt); }
            ;

switch_rule: switch_label "->" expression ';'
                { $$ = ID("switch_rule", @$);
                  PARENT($$, "label", $switch_label);
                  PARENT($$, "body", $expression); }
           | switch_label "->" block
                { $$ = ID("switch_rule", @$);
                  PARENT($$, "label", $switch_label);
                  PARENT($$, "body", $block); }
           | switch_label "->" throw_statement
                { $$ = ID("switch_rule", @$);
                  PARENT($$, "label", $switch_label);
                  PARENT($$, "body", $throw_statement); }
           ;

switch_rules: switch_rule switch_rules  { $$ = LIST($1, $2); }
            | switch_rule               { $$ = LIST($1, NIL); }
            ;

switch_block_statement_group: switch_label ':' switch_labels_with_colon_opt block_statements
                            { $$ = ID("statement_group", @$);
                              PARENT($$, "label", $switch_label);
                              PARENT_LIST($$, "labels", $switch_labels_with_colon_opt);
                              PARENT_LIST($$, "statements", $block_statements); }
                            ;

switch_block_statement_groups_opt: switch_block_statement_group switch_block_statement_groups_opt   { $$ = LIST($1, $2); }
                                 | %empty                                                           { $$ = NIL; }
                                 ;

switch_label: CASE case_constants { $$ = ID("switch_case", @$);
                                    PARENT_LIST($$, "expressions", $case_constants); }
            | DEFAULT             { $$ = ID("switch_default", @$); }
            ;

switch_labels_with_colon_opt: switch_label ':' switch_labels_with_colon_opt { $$ = LIST($1, $3); }
                            | %empty                                        { $$ = NIL; }
                            ;

case_constant: conditional_expression
             ;

case_constants: case_constant ',' case_constants { $$ = LIST($1, $3); }
              | case_constant { $$ = LIST($1, NIL); }
              ;

while_statement: WHILE '(' expression ')' statement { $$ = ID("while_statement", @$);
                                                      PARENT($$, "condition", $expression);
                                                      PARENT($$, "body", $statement); }
               ;

while_statement_no_short_if: WHILE '(' expression ')' statement_no_short_if { $$ = ID("while_statement", @$);
                                                                              PARENT($$, "condition", $expression);
                                                                              PARENT($$, "body", $statement_no_short_if); }
                           ;

do_statement: DO statement WHILE '(' expression ')' ';' { $$ = ID("do_statement", @$);
                                                          PARENT($$, "condition", $expression);
                                                          PARENT($$, "body", $statement); }
            ;

for_statement: basic_for_statement
             | enhanced_for_statement
             ;

for_statement_no_short_if: basic_for_statement_no_short_if
                         | enhanced_for_statement_no_short_if
                         ;

basic_for_statement: FOR '(' for_init_opt ';' expression_opt ';' for_update_opt ')' statement { $$ = ID("for_statement", @$);
                  PARENT($$, "init", $for_init_opt);
                  PARENT($$, "condition", $expression_opt);
                  PARENT_LIST($$, "update", $for_update_opt);
                  PARENT($$, "body", $statement); }
                   ;

basic_for_statement_no_short_if: FOR '(' for_init_opt ';' expression_opt ';' for_update_opt ')' statement_no_short_if { $$ = ID("for_statement", @$);
                                        PARENT($$, "init", $for_init_opt);
                                        PARENT($$, "condition", $expression_opt);
                                        PARENT_LIST($$, "update", $for_update_opt);
                                        PARENT($$, "body", $statement_no_short_if); }
                               ;

for_init: statement_expression_list
        | local_variable_declaration
        ;

for_init_opt: for_init
            | %empty { $$ = NIL; }

for_update: statement_expression_list
          ;

for_update_opt: for_update
              | %empty { $$ = NIL; }
              ;

statement_expression_list: statement_expression ',' statement_expression_list { $$ = LIST($1, $3); }
                         | statement_expression                               { $$ = LIST($1, NIL); }
                         ;

enhanced_for_statement: FOR '(' enhanced_for_formal_parameter ':' expression ')' statement { $$ = ID("enhanced_for_statement", @$);
                                                                                             PARENT($$, "param", $enhanced_for_formal_parameter);
                                                                                             PARENT($$, "expression", $expression);
                                                                                             PARENT($$, "body", $statement); }
                      ;

enhanced_for_formal_parameter: variable_modifiers local_variable_type variable_declarator_id { $$ = ID("formal_parameter", @$);
                                                                                               PARENT_LIST($$, "modifiers", $variable_modifiers);
                                                                                               PARENT($$, "type", $local_variable_type);
                                                                                               PARENT($$, "declarator_id", $variable_declarator_id); }
                             |                    local_variable_type variable_declarator_id { $$ = ID("formal_parameter", @$);
                                                                                               PARENT_LIST($$, "modifiers", NIL);
                                                                                               PARENT($$, "type", $local_variable_type);
                                                                                               PARENT($$, "declarator_id", $variable_declarator_id); }
                             ;

enhanced_for_statement_no_short_if: FOR '(' enhanced_for_formal_parameter  ':' expression ')' statement_no_short_if
                                                { $$ = ID("enhanced_for_statement", @$);
                                                  PARENT($$, "param", $enhanced_for_formal_parameter);
                                                  PARENT($$, "expression", $expression);
                                                  PARENT($$, "body", $statement_no_short_if); }

                                  ;

break_statement: BREAK identifier_opt ';' { $$ = ID("break_statement", @$); }
               ;

yield_statement: YIELD expression ';' { $$ = ID("yield_statement", @$); }
               ;

continue_statement: CONTINUE identifier_opt ';' { $$ = ID("continue_statement", @$); }
                  ;

return_statement: RETURN expression_opt ';' { $$ = ID("return_statement", @$);
                                              PARENT($$, "expression", $expression_opt); }
                ;

throw_statement: THROW expression ';' { $$ = ID("throw_statement", @$); PARENT($$, "expression", $expression); }
               ;

synchronized_statement: SYNCHRONIZED '(' expression ')' block { $$ = ID("synchronized_statement", @$);
                                                                PARENT($$, "expression", $expression);
                                                                PARENT($$, "block", $block); }
                      ;

try_statement: TRY block catches                { $$ = ID("try_statement", @$);
                                                  PARENT($$, "body", $block);
                                                  PARENT_LIST($$, "catches", $catches);
                                                  PARENT($$, "finally", NIL); }
             | TRY block catches_opt finally    { $$ = ID("try_statement", @$);
                                                  PARENT($$, "body", $block);
                                                  PARENT_LIST($$, "catches", $catches_opt);
                                                  PARENT($$, "finally", $finally); }
             | try_with_resources_statement
             ;

catches: catch_clause catches { $$ = LIST($1, $2); }
       | catch_clause         { $$ = LIST($1, NIL); }
       ;

catches_opt: catches
           | %empty { $$ = NIL; }

catch_clause: CATCH '(' catch_formal_parameter ')' block { $$ = ID("catch_block", @$);
                                                           PARENT($$, "parameter", $catch_formal_parameter);
                                                           PARENT($$, "block", $block); }
            ;

catch_formal_parameter: variable_modifiers catch_type variable_declarator_id { $$ = ID("catch_formal_parameter", @$);
                                                                               PARENT_LIST($$, "modifiers", $variable_modifiers);
                                                                               PARENT_LIST($$, "types", $catch_type);
                                                                               PARENT($$, "declarator_id", $variable_declarator_id); }
                      |                    catch_type variable_declarator_id { $$ = ID("catch_formal_parameter", @$);
                                                                               PARENT_LIST($$, "modifiers", NIL);
                                                                               PARENT_LIST($$, "types", $catch_type);
                                                                               PARENT($$, "declarator_id", $variable_declarator_id); }
                      ;

catch_type: unann_class_type catch_type_tail { $$ = LIST($1, $2); }
          ;

catch_type_tail: '|' class_type catch_type_tail { $$ = LIST($2, $3); }
               | %empty                         { $$ = NIL; }
               ;

finally: FINALLY block { $$ = ID("finally_block", @$); PARENT($$, "block", $block); }
       ;

finally_opt: finally
           | %empty { $$ = NIL; }
           ;

try_with_resources_statement: TRY resource_specification block catches_opt finally_opt { $$ = ID("try_with_resources_statement", @$);
                                                                                         PARENT_LIST($$, "resources", $resource_specification);
                                                                                         PARENT($$, "body", $block);
                                                                                         PARENT_LIST($$, "catches", $catches_opt);
                                                                                         PARENT($$, "finally", $finally_opt); }
                            ;

resource_specification: '(' resource_list ';' ')'   { $$ = $2; }
                      | '(' resource_list ')'       { $$ = $2; }
                      ;

resource_list: resource ';' resource_list   { $$ = LIST($1, $3); }
             | resource                     { $$ = LIST($1, NIL); }
             ;

resource: variable_modifiers local_variable_type identifier '=' expression { $$ = ID("resource", @$);
                                                                             PARENT_LIST($$, "modifiers", $variable_modifiers);
                                                                             PARENT($$, "type", $local_variable_type);
                                                                             PARENT($$, "name", $identifier);
                                                                             PARENT($$, "initializer", $expression); }
        |                    local_variable_type identifier '=' expression { $$ = ID("resource", @$);
                                                                             PARENT_LIST($$, "modifiers", NIL);
                                                                             PARENT($$, "type", $local_variable_type);
                                                                             PARENT($$, "name", $identifier);
                                                                             PARENT($$, "initializer", $expression); }
        | variable_access
        ;

variable_access: expression_name
               | field_access
               ;

/* Productions from §15 (Expressions) */

/**
 * The precedence here is only needed because we are using primary instead of primary_no_new_array in array_access
 * (an extension added to conform with OpenJDK)
 */
primary: primary_no_new_array           %dprec 2
       | array_creation_expression      %dprec 1
       ;

primary_no_new_array: literal
                    | class_literal
                    | THIS                  { $$ = ID("this_expression", @$); }
                    | type_name '.' THIS    { $$ = ID("this_expression", @$); } // TODO
                    | '(' expression ')'    { $$ = COPY_ID($2, @$); }
                    | class_instance_creation_expression
                    | field_access
                    | array_access
                    | method_invocation
                    | method_reference
                    ;

class_literal: type_name        unann_dims_opt '.' CLASS { $$ = ID("class_literal", @$); PARENT_LIST($$, "type", $type_name); }
             | numeric_type     unann_dims_opt '.' CLASS { $$ = ID("class_literal", @$); PARENT($$, "type", $numeric_type); }
             | BOOLEAN          unann_dims_opt '.' CLASS { $$ = ID("class_literal", @$); }
             | VOID                            '.' CLASS { $$ = ID("class_literal", @$); }
             ;

class_instance_creation_expression: NEW type_arguments_opt class_or_interface_type_to_instantiate '(' argument_list_opt ')' class_body_opt
                                        { $$ = ID("class_instance_creation_expression", @$);
                                          PARENT($$, "qualifier", NIL);
                                          PARENT($$, "type_arguments", $type_arguments_opt);
                                          PARENT($$, "type", $class_or_interface_type_to_instantiate);
                                          PARENT_LIST($$, "arguments", $argument_list_opt);
                                          PARENT($$, "body", $class_body_opt); }
                                  | expression_name '.' NEW type_arguments_opt class_or_interface_type_to_instantiate '(' argument_list_opt ')' class_body_opt
                                        { $$ = ID("class_instance_creation_expression", @$);
                                          PARENT($$, "qualifier", $expression_name);
                                          PARENT($$, "type_arguments", $type_arguments_opt);
                                          PARENT($$, "type", $class_or_interface_type_to_instantiate);
                                          PARENT_LIST($$, "arguments", $argument_list_opt);
                                          PARENT($$, "body", $class_body_opt); }
                                  | primary         '.' NEW type_arguments_opt class_or_interface_type_to_instantiate '(' argument_list_opt ')' class_body_opt
                                        { $$ = ID("class_instance_creation_expression", @$);
                                          PARENT($$, "qualifier", $primary);
                                          PARENT($$, "type_arguments", $type_arguments_opt);
                                          PARENT($$, "type", $class_or_interface_type_to_instantiate);
                                          PARENT_LIST($$, "arguments", $argument_list_opt);
                                          PARENT($$, "body", $class_body_opt); }
                                  ;

class_or_interface_type_to_instantiate: class_type;

/*
class_or_interface_type_to_instantiate: annotated_identifiers type_arguments_or_diamond_opt { $$ = ID("class_type", @$);
                                                                                              PARENT_LIST($$, "name", $annotated_identifiers);
                                                                                              PARENT($$, "type_arguments", $type_arguments_or_diamond_opt);
                                                                                              PARENT_LIST($$, "annotations", NIL); }
                         ;

annotated_identifier: identifier             { $$ = ID("annotated_identifier", @$); }
                    | annotations identifier { $$ = ID("annotated_identifier", @$); }
                    ;

annotated_identifiers: annotated_identifier '.' annotated_identifiers   { $$ = LIST($1, $3); }
                     | annotated_identifier                             { $$ = LIST($1, NIL); }
                     ;
*/
type_arguments_or_diamond: type_arguments
                         | '<' '>' { $$ = ID("type_arguments", @$); PARENT_LIST($$, "arguments", NIL); }
                         ;

type_arguments_or_diamond_opt: type_arguments_or_diamond
                             | %empty { $$ = NIL; }
                             ;

field_access: field_access_subject '.' identifier { $$ = ID("field_access", @$);
                                                    PARENT($$, "subject", $field_access_subject);
                                                    PARENT($$, "field", $identifier); }
            ;

field_access_subject: primary 
                    | SUPER                 { $$ = ID("super", @$); }
                    | type_name '.' SUPER   { $$ = ID("super", @$); } // TODO
                    ;

/**
 * While the spec states that the left hand side uses the production
 * primary_no_new_array we allow primary to conform with OpenJDK
 */
array_access: expression_name       '[' expression ']' { $$ = ID("array_access", @$); PARENT($$, "subject", $expression_name); PARENT($$, "index", $expression); }
            | primary               '[' expression ']' { $$ = ID("array_access", @$); PARENT($$, "subject", $primary); PARENT($$, "index", $expression); }
            ;

/* The rule using type_name as subject is omitted to handle an ambiguity */
method_invocation:                                             identifier '(' argument_list_opt ')'
                    { $$ = ID("method_invocation", @$);
                      PARENT($$, "subject", NIL);
                      PARENT($$, "type_arguments", NIL);
                      PARENT($$, "method", $identifier);
                      PARENT_LIST($$, "arguments", $argument_list_opt); }
                 | expression_name      '.' type_arguments_opt identifier '(' argument_list_opt ')'
                    { $$ = ID("method_invocation", @$);
                      PARENT($$, "subject", $expression_name);
                      PARENT($$, "type_arguments", $type_arguments_opt);
                      PARENT($$, "method", $identifier);
                      PARENT_LIST($$, "arguments", $argument_list_opt); }
                 | primary              '.' type_arguments_opt identifier '(' argument_list_opt ')'
                    { $$ = ID("method_invocation", @$);
                      PARENT($$, "subject", $primary);
                      PARENT($$, "type_arguments", $type_arguments_opt);
                      PARENT($$, "method", $identifier);
                      PARENT_LIST($$, "arguments", $argument_list_opt); }
                 | SUPER                '.' type_arguments_opt identifier '(' argument_list_opt ')'
                    { $$ = ID("method_invocation", @$);
                      // TODO: subject
                      PARENT($$, "type_arguments", $type_arguments_opt);
                      PARENT($$, "method", $identifier);
                      PARENT_LIST($$, "arguments", $argument_list_opt); }
                 | type_name '.' SUPER  '.' type_arguments_opt identifier '(' argument_list_opt ')'
                    { $$ = ID("method_invocation", @$);
                      // TODO: subject
                      PARENT_LIST($$, "placeholder", $type_name);
                      PARENT($$, "type_arguments", $type_arguments_opt);
                      PARENT($$, "method", $identifier);
                      PARENT_LIST($$, "arguments", $argument_list_opt); }
                 ;

argument_list: expression ',' argument_list { $$ = LIST($1, $3); }
             | expression                   { $$ = LIST($1, NIL); }
             ;

argument_list_opt: argument_list
                 | %empty { $$ = NIL; }
                 ;

method_reference: expression_name       "::" type_arguments_opt identifier %dprec 2 { $$ = ID("method_reference", @$);
                                                                                      PARENT($$, "subject", $expression_name);
                                                                                      PARENT($$, "type_arguments", $type_arguments_opt);
                                                                                      PARENT($$, "method", $identifier); }
                | primary               "::" type_arguments_opt identifier
                    { $$ = ID("method_reference", @$); PARENT($$, "subject", $primary); }
                | reference_type        "::" type_arguments_opt identifier %dprec 1
                    { $$ = ID("method_reference", @$); PARENT($$, "subject", $reference_type); }
                | SUPER                 "::" type_arguments_opt identifier { $$ = ID("method_reference", @$); }
                | type_name '.' SUPER   "::" type_arguments_opt identifier { $$ = ID("method_reference", @$); }
                | class_type            "::" type_arguments_opt NEW
                    { $$ = ID("method_reference", @$); PARENT($$, "subject", $class_type); }
                | array_type            "::" NEW                           { $$ = ID("method_reference", @$); }
                    { $$ = ID("method_reference", @$); PARENT($$, "subject", $array_type); }
                ;

array_creation_expression: NEW primitive_type           dim_exprs dims_opt      { $$ = ID("array_creation_expression", @$);
                                                                                  PARENT($$, "type", $primitive_type);
                                                                                  PARENT_LIST($$, "dim_exprs", $dim_exprs);
                                                                                  PARENT_LIST($$, "dims", $dims_opt);
                                                                                  PARENT($$, "initializer", NIL); }
                         | NEW class_type               dim_exprs dims_opt      { $$ = ID("array_creation_expression", @$);
                                                                                  PARENT($$, "type", $class_type);
                                                                                  PARENT_LIST($$, "dim_exprs", $dim_exprs);
                                                                                  PARENT_LIST($$, "dims", $dims_opt);
                                                                                  PARENT($$, "initializer", NIL); }
                         | NEW primitive_type           dims array_initializer  { $$ = ID("array_creation_expression", @$);
                                                                                  PARENT_LIST($$, "dim_exprs", NIL);
                                                                                  PARENT_LIST($$, "dims", $dims);
                                                                                  PARENT($$, "initializer", $array_initializer); }
                         | NEW class_type               dims array_initializer  { $$ = ID("array_creation_expression", @$);
                                                                                  PARENT($$, "type", $class_type);
                                                                                  PARENT_LIST($$, "dim_exprs", NIL);
                                                                                  PARENT_LIST($$, "dims", $dims);
                                                                                  PARENT($$, "initializer", $array_initializer); }
                         ;

dim_exprs: dim_expr dim_exprs   { $$ = LIST($1, $2); }
         | dim_expr             { $$ = LIST($1, NIL); }
         ;

dim_expr: annotations '[' expression ']' { $$ = ID("dim_expression", @$); PARENT_LIST($$, "annotations", $annotations); PARENT($$, "expression", $expression); }
        | '[' expression ']'             { $$ = ID("dim_expression", @$); PARENT_LIST($$, "annotations", NIL); PARENT($$, "expression", $expression); }
        ;
        
expression: lambda_expression
          | assignment_expression
          ;

expression_opt: expression
              | %empty { $$ = NIL; }
              ;

lambda_expression: lambda_parameters "->" lambda_body { $$ = ID("lambda_expression", @$);
                                                        PARENT($$, "params", $lambda_parameters);
                                                        PARENT($$, "body", $lambda_body); }
                 ;

lambda_parameters: '(' ')'                              { $$ = ID("lambda_params", @$); }
                 | '(' lambda_parameter_list ')'        { $$ = ID("lambda_params", @$);
                                                          PARENT_LIST($$, "params", $lambda_parameter_list); }
                 | lambda_parameter_single_identifier   { $$ = ID("lambda_params", @$);
                                                          PARENT_LIST($$, "params", LIST($1, NIL)); }
                 ;

lambda_parameter_list: lambda_parameter_list_first_option
                     | lambda_parameter_list_second_option
                     ;

lambda_parameter_list_first_option: lambda_parameter ',' lambda_parameter_list_first_option { $$ = LIST($1, $3); }
                                  | lambda_parameter                                        { $$ = LIST($1, NIL); }
                                  ;

lambda_parameter_list_second_option: lambda_parameter_single_identifier ',' lambda_parameter_list_second_option { $$ = LIST($1, $3); }
                                   | lambda_parameter_single_identifier                                         { $$ = LIST($1, NIL); }
                                   ;

lambda_parameter: variable_modifiers lambda_parameter_type variable_declarator_id { $$ = ID("formal_parameter", @$);
                                                                                        PARENT_LIST($$, "modifiers", $variable_modifiers);
                                                                                        PARENT($$, "type", $lambda_parameter_type);
                                                                                        PARENT($$, "declarator_id", $variable_declarator_id); }

                |                    lambda_parameter_type variable_declarator_id { $$ = ID("formal_parameter", @$);
                                                                                    PARENT_LIST($$, "modifiers", NIL);
                                                                                    PARENT($$, "type", $lambda_parameter_type);
                                                                                    PARENT($$, "declarator_id", $variable_declarator_id); }
                | variable_arity_parameter 
                ;

/* In reality this should only match identifiers and not variable_declarator_ids */
lambda_parameter_single_identifier: variable_declarator_id { $$ = ID("formal_parameter", @$);
                                                 PARENT_LIST($$, "modifiers", NIL);
                                                 PARENT($$, "type", NIL);
                                                 PARENT($$, "declarator_id", $variable_declarator_id); }

lambda_parameter_type: unann_type
                     | VAR { $$ = ID("var_type", @$); }
                     ;

lambda_body: expression
           | block
           ;

assignment_expression: conditional_expression
                     | assignment
                     ;

assignment: left_hand_side assignment_operator expression { $$ = ID("assignment_expression", @$);
                                                            PARENT($$, "left", $left_hand_side);
                                                            PARENT($$, "right", $expression); }
          ;

assignment_operator: '='
                   | "*="
                   | "/="
                   | "%="
                   | "+="
                   | "-="
                   | "<<="
                   | ">>="
                   | ">>>="
                   | "&="
                   | "^="
                   | "|="
                   ;

/* Allow left_hand_side to be parenthesized to conform with OpenJDK */
left_hand_side: expression_name
              | field_access
              | array_access
              | '(' left_hand_side ')' { $$ = $2; }
              ;

conditional_expression: conditional_or_expression
                      | conditional_or_expression '?' expression ':' conditional_expression { $$ = ID("conditional_expression", @$);
                                                                                                 PARENT($$, "condition", $1);
                                                                                                 PARENT($$, "then", $3);
                                                                                                 PARENT($$, "else", $5); }
                      | conditional_or_expression '?' expression ':' lambda_expression { $$ = ID("conditional_expression", @$); }
                      ;

conditional_or_expression: conditional_and_expression %dprec 2
                         | conditional_or_expression "||" conditional_and_expression %dprec 1 { INFIX($$, "conditional_or_expression", @$, $1, $3); }
                         ;

conditional_and_expression: inclusive_or_expression %dprec 2
                          | conditional_and_expression "&&" inclusive_or_expression %dprec 1 { INFIX($$, "conditional_and_expression", @$, $1, $3); }
                          ;

inclusive_or_expression: exclusive_or_expression %dprec 2
                       | inclusive_or_expression '|' exclusive_or_expression %dprec 1 { INFIX($$, "inclusive_or_expression", @$, $1, $3); }
                       ;

exclusive_or_expression: and_expression %dprec 2
                       | exclusive_or_expression '^' and_expression %dprec 1 { INFIX($$, "exclusive_or_expression", @$, $1, $3); }
                       ;

and_expression: equality_expression %dprec 2
              | and_expression '&' equality_expression %dprec 1 { INFIX($$, "and_expression", @$, $1, $3); }
              ;

equality_expression: relational_expression %dprec 2
                   | equality_expression "==" relational_expression %dprec 1 { INFIX($$, "equals_expression", @$, $1, $3); }
                   | equality_expression "!=" relational_expression %dprec 1 { INFIX($$, "not_equals_expression", @$, $1, $3); }
                   ;

relational_expression: shift_expression %dprec 2
                     | relational_expression '<' shift_expression      %dprec 1 { INFIX($$, "less_than_expression", @$, $1, $3); }
                     | relational_expression '>' shift_expression      %dprec 1 { INFIX($$, "greater_than_expression", @$, $1, $3); }
                     | relational_expression "<=" shift_expression     %dprec 1 { INFIX($$, "less_than_or_equals_expression", @$, $1, $3); }
                     | relational_expression ">=" shift_expression     %dprec 1 { INFIX($$, "greater_than_or_equals_expression", @$, $1, $3); }
                     | relational_expression INSTANCEOF reference_type %dprec 1 { INFIX($$, "instanceof_expression", @$, $1, $3); }
                     ;

shift_expression: additive_expression %dprec 2
                | shift_expression '<' '<' additive_expression %dprec 1 { INFIX($$, "signed_left_shift_expression", @$, $1, $additive_expression); }
                | shift_expression '>' '>' additive_expression %dprec 1 { INFIX($$, "signed_right_shift_expression", @$, $1, $additive_expression); }
                | shift_expression '>' '>' '>' additive_expression %dprec 1 { INFIX($$, "unsigned_right_shift_expression", @$, $1, $additive_expression); }
                ;

additive_expression: multiplicative_expression %dprec 2
                   | additive_expression '+' multiplicative_expression %dprec 1 { INFIX($$, "addition_expression", @$, $1, $3); }
                   | additive_expression '-' multiplicative_expression %dprec 1 { INFIX($$, "subtraction_expression", @$, $1, $3); }
                   ;

multiplicative_expression: unary_expression %dprec 2
                         | multiplicative_expression '*' unary_expression %dprec 1 { INFIX($$, "multiplication_expression", @$, $1, $3); }
                         | multiplicative_expression '/' unary_expression %dprec 1 { INFIX($$, "division_expression", @$, $1, $3); }
                         | multiplicative_expression '%' unary_expression %dprec 1 { INFIX($$, "remainder_expression", @$, $1, $3); }
                         ;

unary_expression: pre_increment_expression
                | pre_decrement_expression
                | '+' unary_expression { $$ = ID("unary_plus_expression", @$); PARENT($$, "expression", $2); }
                | '-' unary_expression { $$ = ID("unary_minus_expression", @$); PARENT($$, "expression", $2); }
                | unary_expression_not_plus_minus
                ;

pre_increment_expression: "++" unary_expression { $$ = ID("prefix_increment_expression", @$); PARENT($$, "expression", $unary_expression); }
                        ;

pre_decrement_expression: "--" unary_expression { $$ = ID("prefix_decrement_expression", @$); PARENT($$, "expression", $unary_expression); }
                        ;

unary_expression_not_plus_minus: postfix_expression
                               | '~' unary_expression { $$ = ID("bitwise_complement_expression", @$); PARENT($$, "expression", $unary_expression); }
                               | '!' unary_expression { $$ = ID("logical_complement_expression", @$); PARENT($$, "expression", $unary_expression); }
                               | cast_expression
                               | switch_expression
                               ;

postfix_expression: primary
                  | expression_name
                  | post_increment_expression
                  | post_decrement_expression
                  ;

post_increment_expression: postfix_expression "++" { $$ = ID("postfix_increment_expression", @$); PARENT($$, "expression", $postfix_expression); }
                         ;

post_decrement_expression: postfix_expression "--" { $$ = ID("postfix_decrement_expression", @$); PARENT($$, "expression", $postfix_expression); }
                         ;

cast_expression: '(' primitive_type                         ')' unary_expression                { $$ = ID("cast_expression", @$);
                                                                                                  PARENT($$, "type", $primitive_type); 
                                                                                                  PARENT($$, "value", $unary_expression); }
               | '(' reference_type additional_bounds_opt   ')' unary_expression_not_plus_minus { $$ = ID("cast_expression", @$);
                                                                                                  PARENT($$, "type", $reference_type); 
                                                                                                  PARENT_LIST($$, "bounds", $additional_bounds_opt);
                                                                                                  PARENT($$, "value", $unary_expression_not_plus_minus); }
               | '(' reference_type additional_bounds_opt   ')' lambda_expression               { $$ = ID("cast_expression", @$);
                                                                                                  PARENT($$, "type", $reference_type); 
                                                                                                  PARENT_LIST($$, "bounds", $additional_bounds_opt);
                                                                                                  PARENT($$, "value", $lambda_expression); }
               ;

switch_expression: SWITCH '(' expression ')' switch_block
                 ;

%code
{

#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).filename = YYRHSLOC(Rhs, 1).filename;               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).filename = YYRHSLOC(Rhs, 0).filename;               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)

#define NIL 0
#define ROOT(id) insert_root(program, id)
#define ID(name, loc) create_id(program, name, loc)
#define COPY_ID(child, loc) copy_id(program, child, loc)
#define LIST(head, tail) create_id_list(program, head, tail)
#define PARENT(parent, name, child) insert_parent_of(program, parent, name, child)
#define PARENT_LIST(parent, name, children) insert_parent_of_list(program, parent, name, children)
#define INFIX(parent, name, loc, left, right) do { parent = ID(name, loc); PARENT(parent, "left", left); PARENT(parent, "right", right); } while (0)

int create_id_list(souffle::SouffleProgram* program, int head, int tail) {
    std::array<souffle::RamDomain, 2> arr = {head, tail};
    int result = program->getRecordTable().pack(arr.data(), 2);
    return result;
}

int create_id(souffle::SouffleProgram* program, const char* type, const logifix::parser::location& loc) {
    assert(type != nullptr);
    assert(loc.filename != nullptr);
    std::array<souffle::RamDomain, 6> arr = {
        program->getSymbolTable().encode(type),
        program->getSymbolTable().encode(loc.filename),
        souffle::RamDomain(loc.begin),
        souffle::RamDomain(loc.end),
        souffle::RamDomain(loc.begin),
        souffle::RamDomain(loc.end)
    };
    return program->getRecordTable().pack(arr.data(), arr.size());
}

int copy_id(souffle::SouffleProgram* program, int child, const logifix::parser::location& loc) {
    auto* ptr = program->getRecordTable().unpack(child, 6);
    std::array<souffle::RamDomain, 6> arr = {
        ptr[0],
        ptr[1],
        ptr[2],
        ptr[3],
        souffle::RamDomain(loc.begin),
        souffle::RamDomain(loc.end)
    };
    auto new_id = program->getRecordTable().pack(arr.data(), arr.size());
    auto* parent_of = program->getRelation("parent_of");
    std::vector<std::tuple<int,std::string,int>> parent_of_tuples;
    for (souffle::tuple& output : *parent_of) {
        int x;
        std::string symbol;
        int y;
        output >> x >> symbol >> y;
        if (x == child) {
            parent_of_tuples.emplace_back(new_id, symbol, y);
        }
    }
    for (auto [x, symbol, y] : parent_of_tuples) {
        parent_of->insert(souffle::tuple(parent_of, {x, program->getSymbolTable().encode(symbol), y}));
    }
    auto* parent_of_list = program->getRelation("parent_of_list");
    std::vector<std::tuple<int,std::string,int>> parent_of_list_tuples;
    for (souffle::tuple& output : *parent_of_list) {
        int x;
        std::string symbol;
        int y;
        output >> x >> symbol >> y;
        if (x == child) {
            parent_of_list_tuples.emplace_back(new_id, symbol, y);
        }
    }
    for (auto [x, symbol, y] : parent_of_list_tuples) {
        parent_of_list->insert(souffle::tuple(parent_of_list, {x, program->getSymbolTable().encode(symbol), y}));
    }
    return new_id;
}

void insert_root(souffle::SouffleProgram* program, int id) {
    auto* relation = program->getRelation("root");
    assert(relation != nullptr);
    relation->insert(souffle::tuple(relation, {id}));
}

void insert_parent_of(souffle::SouffleProgram* program, int parent, const char* name, int child) {
    auto* relation = program->getRelation("parent_of");
    assert(relation != nullptr);
    relation->insert(souffle::tuple(relation, {parent, program->getSymbolTable().encode(name), child}));
}

void insert_parent_of_list(souffle::SouffleProgram* program, int parent, const char* name, int children) {
    auto* relation = program->getRelation("parent_of_list");
    assert(relation != nullptr);
    relation->insert(souffle::tuple(relation, {parent, program->getSymbolTable().encode(name), children}));
}

/* Build this table with /\([A-Z]\+\)/{"\L\1\e", yy::parser::token::\1}, in vim */
std::unordered_map<std::string, int> keywords = {
    {"abstract", yy::parser::token::ABSTRACT},
    {"continue", yy::parser::token::CONTINUE},
    {"for", yy::parser::token::FOR},
    {"new", yy::parser::token::NEW},
    {"switch", yy::parser::token::SWITCH},
    {"assert", yy::parser::token::ASSERT},
    {"default", yy::parser::token::DEFAULT},
    {"if", yy::parser::token::IF},
    {"package", yy::parser::token::PACKAGE},
    {"synchronized", yy::parser::token::SYNCHRONIZED},
    {"boolean", yy::parser::token::BOOLEAN},
    {"do", yy::parser::token::DO},
    {"goto", yy::parser::token::GOTO},
    {"private", yy::parser::token::PRIVATE},
    {"this", yy::parser::token::THIS},
    {"break", yy::parser::token::BREAK},
    {"double", yy::parser::token::DOUBLE},
    {"implements", yy::parser::token::IMPLEMENTS},
    {"protected", yy::parser::token::PROTECTED},
    {"throw", yy::parser::token::THROW},
    {"byte", yy::parser::token::BYTE},
    {"else", yy::parser::token::ELSE},
    {"import", yy::parser::token::IMPORT},
    {"public", yy::parser::token::PUBLIC},
    {"throws", yy::parser::token::THROWS},
    {"case", yy::parser::token::CASE},
    {"enum", yy::parser::token::ENUM},
    {"instanceof", yy::parser::token::INSTANCEOF},
    {"return", yy::parser::token::RETURN},
    {"transient", yy::parser::token::TRANSIENT},
    {"catch", yy::parser::token::CATCH},
    {"extends", yy::parser::token::EXTENDS},
    {"int", yy::parser::token::INT},
    {"short", yy::parser::token::SHORT},
    {"try", yy::parser::token::TRY},
    {"char", yy::parser::token::CHAR},
    {"final", yy::parser::token::FINAL},
    {"interface", yy::parser::token::INTERFACE},
    {"static", yy::parser::token::STATIC},
    {"void", yy::parser::token::VOID},
    {"class", yy::parser::token::CLASS},
    {"finally", yy::parser::token::FINALLY},
    {"long", yy::parser::token::LONG},
    {"strictfp", yy::parser::token::STRICTFP},
    {"volatile", yy::parser::token::VOLATILE},
    {"const", yy::parser::token::CONST},
    {"float", yy::parser::token::FLOAT},
    {"native", yy::parser::token::NATIVE},
    {"super", yy::parser::token::SUPER},
    {"while", yy::parser::token::WHILE},
    {"_", yy::parser::token::UNDERSCORE},
};

std::unordered_map<std::string, int> restricted = {
    {"var", yy::parser::token::VAR},
    {"yield", yy::parser::token::YIELD},
    {"open", yy::parser::token::OPEN},
    {"module", yy::parser::token::MODULE},
    {"requires", yy::parser::token::REQUIRES},
    {"transitive", yy::parser::token::TRANSITIVE},
    {"exports", yy::parser::token::EXPORTS},
    {"opens", yy::parser::token::OPENS},
    {"to", yy::parser::token::TO},
    {"uses", yy::parser::token::USES},
    {"provides", yy::parser::token::PROVIDES},
    {"with", yy::parser::token::WITH}
};

std::unordered_map<std::string, int> operators = {
    {"!=", yy::parser::token::NOT_EQUALS},
    {"%=", yy::parser::token::REMAINDER_ASSIGNMENT},
    {"&&", yy::parser::token::LOGICAL_AND},
    {"&=", yy::parser::token::BITWISE_AND_ASSIGNMENT},
    {"*=", yy::parser::token::MULTIPLICATION_ASSIGNMENT},
    {"++", yy::parser::token::INCREMENT},
    {"+=", yy::parser::token::ADDITION_ASSIGNMENT},
    {"--", yy::parser::token::DECREMENT},
    {"-=", yy::parser::token::SUBTRACTION_ASSIGNMENT},
    {"->", yy::parser::token::ARROW},
    {"/=", yy::parser::token::DIVISION_ASSIGNMENT},
    {"<<=", yy::parser::token::SIGNED_LEFT_SHIFT_ASSIGNMENT},
    {"<=", yy::parser::token::LESS_THAN_OR_EQUAL},
    {"==", yy::parser::token::EQUALS},
    {">=", yy::parser::token::GREATER_THAN_OR_EQUAL},
    {">>=", yy::parser::token::SIGNED_RIGHT_SHIFT_ASSIGNMENT},
    {">>>=", yy::parser::token::UNSIGNED_RIGHT_SHIFT_ASSIGNMENT},
    {"^=", yy::parser::token::BITWISE_EXCLUSIVE_OR_ASSIGNMENT},
    {"|=", yy::parser::token::BITWISE_OR_ASSIGNMENT},
    {"||", yy::parser::token::LOGICAL_OR},
};

int yylex(int* yylval, logifix::parser::location* yylloc, const char* filename, std::vector<logifix::parser::token>& tokens, size_t& pos) {
    assert(filename != nullptr);

    /* Skip non-semantic tokens */
    std::unordered_set skip = {
        logifix::parser::token_type::whitespace,
        logifix::parser::token_type::single_line_comment,
        logifix::parser::token_type::multi_line_comment
    };
    while (tokens.size() && skip.find(std::get<0>(tokens.back())) != skip.end()) {
       pos += std::get<1>(tokens.back()).size();
       tokens.pop_back();
    }

    if (tokens.size() == 0) {
        return yy::parser::token::UNDEFINED;
    }
    auto [type, content] = tokens.back();
    tokens.pop_back();
    auto start = pos;
    auto end = pos + content.size();
    pos = end;
    yylloc->filename = filename;
    yylloc->begin = start;
    yylloc->end = end;
    if (type == logifix::parser::token_type::identifier) {
        return yy::parser::token::IDENTIFIER;
    }
    if (type == logifix::parser::token_type::restricted) {
        auto it = restricted.find(std::string(content));
        if (it == restricted.end()) {
            return yy::parser::token::UNDEFINED;
        }
        return it->second;
    }
    if (type == logifix::parser::token_type::keyword) {
        auto it = keywords.find(std::string(content));
        if (it == keywords.end()) {
            return yy::parser::token::UNDEFINED;
        }
        return it->second;
    }
    if (type == logifix::parser::token_type::character_literal) {
        return yy::parser::token::CHARACTER_LITERAL;
    }
    if (type == logifix::parser::token_type::integer_literal) {
        return yy::parser::token::INTEGER_LITERAL;
    }
    if (type == logifix::parser::token_type::floating_point_literal) {
        return yy::parser::token::FLOATING_POINT_LITERAL;
    }
    if (type == logifix::parser::token_type::boolean_literal) {
        return yy::parser::token::BOOLEAN_LITERAL;
    }
    if (type == logifix::parser::token_type::null_literal) {
        return yy::parser::token::NULL_LITERAL;
    }
    if (type == logifix::parser::token_type::text_block) {
        return yy::parser::token::TEXT_BLOCK;
    }
    if (type == logifix::parser::token_type::string_literal) {
        return yy::parser::token::STRING_LITERAL;
    }
    if (type == logifix::parser::token_type::sep) {
        if (content.size() == 1) return content[0];
        if (content == "...") return yy::parser::token::ELLIPSIS;
        if (content == "::")  return yy::parser::token::DOUBLECOLON;
        return yy::parser::token::UNDEFINED;
    }
    if (type == logifix::parser::token_type::op) {
        if (content.size() == 1) return content[0];
        auto it = operators.find(std::string(content));
        if (it == operators.end()) {
            return yy::parser::token::UNDEFINED;
        }
        return it->second;
    }
    if (type == logifix::parser::token_type::eof) {
        return EOF;
    }
    return yy::parser::token::UNDEFINED;
}

};

%%
namespace yy
{

void parser::error(const logifix::parser::location& loc, const std::string& msg) {
#if 0
    assert(loc.filename != nullptr);
    std::cerr << msg << " in " << loc << std::endl;
#endif
}

}

namespace logifix::parser {

int parse(souffle::SouffleProgram* program, const char* filename, const char* content) {
    assert(filename != nullptr);
    assert(content != nullptr);

    auto tokens = lex(content);
    if (!tokens) {
        return 1;
    }
#if 0
    std::cerr << "Tokens" << std::endl;
    std::cerr << "---------------" << std::endl;
    size_t s = 0;
    for (auto [t, c] : *tokens) {
        std::cerr << std::setw(10) << c << std::setw(10) << s << " " << s+c.size() << std::endl;
        s += c.size();
    }
    std::cerr << "===============" << std::endl;
#endif
    std::reverse(tokens->begin(), tokens->end());
    assert(program != nullptr);
    size_t pos = 0;
    yy::parser parser(program, filename, *tokens, pos);
    return parser();
}

}
