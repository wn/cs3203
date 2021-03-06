#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>

namespace backend {
namespace lexer {
enum TokenType {
    // Characters
    LBRACE,
    RBRACE,
    LPAREN,
    RPAREN,
    SEMICOLON,
    COMMA,
    UNDERSCORE,
    DOUBLE_QUOTE,
    SINGLE_EQ, // To prevent ambiguity
    NOT,
    ANDAND,
    OROR,
    EQEQ,
    NEQ,
    GT,
    GTE,
    LT,
    LTE,
    PLUS,
    MINUS,
    MULT,
    DIV,
    MOD,
    PERIOD,
    HASH,

    // Words, which have value in them.
    NAME,
    INTEGER,

    // Only used in QPL
    WHITESPACE,
};

std::string prettyPrintType(TokenType t);

struct Token {
    // Required
    TokenType type;
    int line;
    int linePosition;

    // Use only for NAME and INTEGER
    std::string nameValue;
    std::string integerValue;

    explicit Token(TokenType t) : type(t), line(), linePosition(), nameValue(), integerValue(){};
};

std::vector<Token> tokenize(std::istream& stream);

std::vector<Token> tokenizeWithWhitespace(std::istream& stream);
} // namespace lexer
} // namespace backend
