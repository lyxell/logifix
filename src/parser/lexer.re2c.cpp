#include "parser.h"

#include <iostream>
#include <string>
#include <vector>

namespace logifix::parser {

std::optional<token_collection> lex(const std::string& content) {
    token_collection tokens;
    const uint8_t* YYCURSOR = reinterpret_cast<const uint8_t*>(content.c_str());
    const uint8_t* YYLIMIT = reinterpret_cast<const uint8_t*>(content.c_str()) + content.size();
    const uint8_t* YYMARKER = nullptr;
    while (true) {
        const uint8_t* YYSTART = YYCURSOR;
        /*!re2c
        re2c:define:YYCTYPE = uint8_t;
        re2c:yyfill:enable = 0;
        re2c:eof = 0;
        re2c:flags:utf-8  = 1;

        // INTEGER LITERALS
        Underscores = "_"+;
        OctalDigit     = [0-7];
        OctalDigitOrUnderscore = OctalDigit | "_";
        OctalDigitsAndUnderscores = OctalDigitOrUnderscore+;
        OctalDigits    = OctalDigit
                       | OctalDigit OctalDigitsAndUnderscores? OctalDigit;
        OctalNumeral   = "0" OctalDigits
                       | "0" Underscores OctalDigits;
        BinaryDigit    = [01];
        BinaryDigitOrUnderscore = BinaryDigit | "_";
        BinaryDigitsAndUnderscores = BinaryDigitOrUnderscore+;
        BinaryDigits   = BinaryDigit
                       | BinaryDigit BinaryDigitsAndUnderscores? BinaryDigit;
        BinaryNumeral  = "0" [bB] BinaryDigits;
        HexDigit       = [0-9a-fA-F];
        HexDigitOrUnderscore = HexDigit | "_";
        HexDigitsAndUnderscores = HexDigitOrUnderscore+;
        HexDigits      = HexDigit
                       | HexDigit HexDigitsAndUnderscores? HexDigit;
        HexNumeral     = "0" [xX] HexDigits;
        NonZeroDigit   = [1-9];
        Digit          = "0"
                       | NonZeroDigit;
        DigitOrUnderscore = Digit | "_";
        DigitsAndUnderscores = DigitOrUnderscore+;
        Digits         = Digit
                       | Digit DigitsAndUnderscores? Digit;
        DecimalNumeral = "0"
                       | NonZeroDigit Digits?
                       | NonZeroDigit Underscores Digits;
        IntegerTypeSuffix = [lL];
        BinaryIntegerLiteral = BinaryNumeral IntegerTypeSuffix?;
        OctalIntegerLiteral = OctalNumeral IntegerTypeSuffix?;
        HexIntegerLiteral = HexNumeral IntegerTypeSuffix?;
        DecimalIntegerLiteral = DecimalNumeral IntegerTypeSuffix?;
        IntegerLiteral = DecimalIntegerLiteral
                       | HexIntegerLiteral
                       | OctalIntegerLiteral
                       | BinaryIntegerLiteral;

        // FLOATING POINT LITERALS
        Sign = [+-];
        SignedInteger = Digits | Sign Digits;
        HexSignificand = HexNumeral "."
                       | "0" [xX] HexDigits? "." HexDigits;
        BinaryExponentIndicator = [pP];
        BinaryExponent = BinaryExponentIndicator SignedInteger;
        FloatTypeSuffix = [fFdD];
        HexadecimalFloatingPointLiteral =
            HexSignificand BinaryExponent FloatTypeSuffix?;
        ExponentIndicator = [eE];
        ExponentPart = ExponentIndicator SignedInteger;
        DecimalFloatingPointLiteral =
              Digits "." Digits? ExponentPart? FloatTypeSuffix?
            | "." Digits ExponentPart? FloatTypeSuffix?
            | Digits ExponentPart FloatTypeSuffix?
            | Digits ExponentPart? FloatTypeSuffix;
        FloatingPointLiteral = DecimalFloatingPointLiteral
                             | HexadecimalFloatingPointLiteral;

        // ESCAPE SEQUENCES
        OctalEscape = "\\" OctalDigit
                    | "\\" OctalDigit OctalDigit
                    | "\\" [0-3] OctalDigit OctalDigit;
        EscapeSequence = "\\" "b"
                       | "\\" "s"
                       | "\\" "t"
                       | "\\" "n"
                       | "\\" "f"
                       | "\\" "r"
                       | "\\" "\""
                       | "\\" "'"
                       | "\\" "\\"
                       | OctalEscape;
        UnicodeEscape = "\\" ("u"+) HexDigit HexDigit HexDigit HexDigit;

        // NULL LITERAL
        NullLiteral = "null";

        // BOOLEAN LITERALS
        BooleanLiteral = "true" | "false";

        // CHARACTER LITERALS
        CharacterLiteral = ['] [^\r\n\x00'\\] ['] | ['] UnicodeEscape ['] | ['] EscapeSequence ['];

        // STRING LITERALS
        // The spec does not allow for UnicodeEscapes in strings, but OpenJDK does
        StringLiteral = ["] ([^\r\n\x00"\\] | UnicodeEscape | EscapeSequence)* ["];

        // TEXT BLOCKS
        LineTerminator = "\r" | "\n" | "\r\n";
        TextBlockWhiteSpace = " " | "\t" | "\f";
        TextBlockCharacter = [^\r\n\x00\\] | EscapeSequence | LineTerminator;
        TextBlock = ["] ["] ["] TextBlockWhiteSpace* LineTerminator TextBlockCharacter* ["] ["] ["];

        // COMMENTS (https://stackoverflow.com/a/36328890)
        SingleLineComment = "//" [^\x00\n]* [\n\x00];
        MultiLineComment  = "/" "*" [^\x00*]* "*"+ ([^\x00/*][^\x00*]* "*"+)* "/";

        // IDENTIFIERS
        Identifier = [a-zA-Z_$][a-zA-Z_$0-9]*;

        SingleLineComment {
            tokens.emplace_back(token_type::single_line_comment, std::string(YYSTART, YYCURSOR));
            continue;
        }

        MultiLineComment {
            tokens.emplace_back(token_type::multi_line_comment, std::string(YYSTART, YYCURSOR));
            continue;
        }

        TextBlock {
            tokens.emplace_back(token_type::text_block, std::string(YYSTART, YYCURSOR));
            continue;
        }

        [ \t\v\n\r] {
            tokens.emplace_back(token_type::whitespace, std::string(YYSTART, YYCURSOR));
            continue;
        }

        CharacterLiteral {
            tokens.emplace_back(token_type::character_literal, std::string(YYSTART, YYCURSOR));
            continue;
        }

        StringLiteral {
            tokens.emplace_back(token_type::string_literal, std::string(YYSTART, YYCURSOR));
            continue;
        }

        "var"        | "yield"       | "open"         | "module"    | "requires"       |
        "transitive" | "exports"     | "opens"        | "to"        | "uses"           |
        "provides"   | "with"
        {
            tokens.emplace_back(token_type::restricted, std::string(YYSTART, YYCURSOR));
            continue;
        }

        // https://docs.oracle.com/javase/specs/jls/se15/html/jls-3.html#jls-3.9
        "abstract"   | "continue"    | "for"          | "new"       |  "switch"        |
        "assert"     | "default"     | "if"           | "package"   |  "synchronized"  |
        "boolean"    | "do"          | "goto"         | "private"   |  "this"          |
        "break"      | "double"      | "implements"   | "protected" |  "throw"         |
        "byte"       | "else"        | "import"       | "public"    |  "throws"        |
        "case"       | "enum"        | "instanceof"   | "return"    |  "transient"     |
        "catch"      | "extends"     | "int"          | "short"     |  "try"           |
        "char"       | "final"       | "interface"    | "static"    |  "void"          |
        "class"      | "finally"     | "long"         | "strictfp"  |  "volatile"      |
        "const"      | "float"       | "native"       | "super"     |  "while"         |
        "_"
        {
            tokens.emplace_back(token_type::keyword, std::string(YYSTART, YYCURSOR));
            continue;
        }

        // https://docs.oracle.com/javase/specs/jls/se15/html/jls-3.html#jls-3.11
        "(" | ")" | "{" | "}" | "[" | "]" | ";" | ","   | "." | "..." | "@" | "::"
        {
            tokens.emplace_back(token_type::sep, std::string(YYSTART, YYCURSOR));
            continue;
        }

        // https://docs.oracle.com/javase/specs/jls/se15/html/jls-3.html#jls-3.12
        // the shift operators are omitted since they conflict with type variables
        "="  | ">"  | "<"  | "!"  | "~"  | "?"  | ":"  | "->" |
        "==" | ">=" | "<=" | "!=" | "&&" | "||" | "++" | "--" |
        "+"  | "-"  | "*"  | "/"  | "&"  | "|"  | "^"  | "%"  |
        "+=" | "-=" | "*=" | "/=" | "&=" | "|=" | "^=" | "%=" | "<<=" | ">>=" | ">>>="
        {
            tokens.emplace_back(token_type::op, std::string(YYSTART, YYCURSOR));
            continue;
        }
        IntegerLiteral {
            tokens.emplace_back(token_type::integer_literal, std::string(YYSTART, YYCURSOR));
            continue;
        }
        FloatingPointLiteral {
            tokens.emplace_back(token_type::floating_point_literal, std::string(YYSTART, YYCURSOR));
            continue;
        }
        BooleanLiteral {
            tokens.emplace_back(token_type::boolean_literal, std::string(YYSTART, YYCURSOR));
            continue;
        }
        NullLiteral {
            tokens.emplace_back(token_type::null_literal, std::string(YYSTART, YYCURSOR));
            continue;
        }
        Identifier {
            tokens.emplace_back(token_type::identifier, std::string(YYSTART, YYCURSOR));
            continue;
        }
        * {
            return {};
        }
        $ {
            tokens.emplace_back(token_type::eof, std::string());
            break;
        }
        */
    }
    return tokens;
}

} // namespace logifix::parser
