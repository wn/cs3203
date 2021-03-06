#include "Lexer.h"
#include "catch.hpp"

#include <sstream>
#include <string>
#include <vector>

std::string prettyTypeStr(const std::vector<backend::lexer::Token>& tokens) {
    std::string result;
    for (const auto& token : tokens) {
        // Only add a space between tokens and not after last token.
        if (&token == &tokens.back()) {
            result += backend::lexer::prettyPrintType(token.type);
        } else {
            result += backend::lexer::prettyPrintType(token.type) + " ";
        }
    }
    return result;
}

TEST_CASE("Test valid integer") {
    std::stringstream query = std::stringstream("00");
    REQUIRE_THROWS(backend::lexer::tokenizeWithWhitespace(query));

    std::stringstream query2 = std::stringstream("001");
    REQUIRE_THROWS(backend::lexer::tokenizeWithWhitespace(query2));

    std::stringstream query3 = std::stringstream("0");
    REQUIRE_NOTHROW(backend::lexer::tokenizeWithWhitespace(query3));

    std::stringstream query4 = std::stringstream("10");
    REQUIRE_NOTHROW(backend::lexer::tokenizeWithWhitespace(query4));

    std::stringstream query5 = std::stringstream("1");
    REQUIRE_NOTHROW(backend::lexer::tokenizeWithWhitespace(query5));

    std::stringstream query6 = std::stringstream("11");
    REQUIRE_NOTHROW(backend::lexer::tokenizeWithWhitespace(query6));
}

TEST_CASE("Empty tokens test") {
    std::stringstream query = std::stringstream("");
    std::string expected = "";
    std::vector<backend::lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(query);
    REQUIRE(prettyTypeStr(lexerTokens) == expected);
}

TEST_CASE("Multiple consecutive spaces are recognized as one WHITESPACE") {
    std::stringstream query = std::stringstream("stmt stmt;read read;assign\n"
                                                "      assign; select\n"
                                                "stmt such that follows\n"
                                                "(\n"
                                                "stmt\n"
                                                "        ,\n"
                                                "_         )\n"
                                                ";");
    std::string expected =
    "NAME WHITESPACE NAME SEMICOLON NAME WHITESPACE NAME SEMICOLON NAME WHITESPACE NAME SEMICOLON "
    "WHITESPACE NAME WHITESPACE NAME WHITESPACE NAME WHITESPACE NAME WHITESPACE NAME WHITESPACE "
    "LPAREN WHITESPACE NAME WHITESPACE "
    "COMMA WHITESPACE UNDERSCORE WHITESPACE RPAREN WHITESPACE SEMICOLON";
    std::vector<backend::lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(query);
    REQUIRE(prettyTypeStr(lexerTokens) == expected);
}

TEST_CASE("Multiple consecutive newlines, space and tabs are recognized as one WHITESPACE") {
    std::stringstream query = std::stringstream("select\n\n" // 2 newlines
                                                "stmt  " // 2 spaces
                                                "such\t\t" // 2 tabs
                                                "that " // 1 space
                                                "follows\n" // 1 newline
                                                "* (\nstmt\n  " // 1 newline, 2 space
                                                ",\r\n\r\n" // 2 CR-CF s
                                                "_\r\r" // 2 CRs
                                                ");");
    std::string expected =
    // select stmt such that
    "NAME WHITESPACE NAME WHITESPACE NAME WHITESPACE NAME WHITESPACE "
    // follows *
    "NAME WHITESPACE MULT WHITESPACE "
    // ( stmt
    "LPAREN WHITESPACE NAME WHITESPACE "
    // , _ )
    "COMMA WHITESPACE UNDERSCORE WHITESPACE RPAREN "
    // ;
    "SEMICOLON";
    std::vector<backend::lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(query);
    REQUIRE(prettyTypeStr(lexerTokens) == expected);
}

TEST_CASE("Lexer captures line numbers") {
    std::stringstream query = std::stringstream("apple\n" // line ends with newline
                                                "ball \t\t\t \r\n" // line ends with CR-CF
                                                " \r" // CR does not end a line on it's own
                                                "cat \n"
                                                "\n" // blank line
                                                "dog");
    std::vector<backend::lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(query);

    std::string expected = "NAME WHITESPACE NAME WHITESPACE NAME WHITESPACE NAME";
    REQUIRE(prettyTypeStr(lexerTokens) == expected);

    REQUIRE(lexerTokens[0].nameValue == "apple");
    REQUIRE(lexerTokens[0].line == 1);

    // \n causes newline
    REQUIRE(lexerTokens[2].nameValue == "ball");
    REQUIRE(lexerTokens[2].line == 2);

    // \r\n causes one newline, and \r does not cause another newline.0
    REQUIRE(lexerTokens[4].nameValue == "cat");
    REQUIRE(lexerTokens[4].line == 3);

    // \\n\n causes 2 newlines
    REQUIRE(lexerTokens[6].nameValue == "dog");
    REQUIRE(lexerTokens[6].line == 5);
}

TEST_CASE("Queries with no clauses") {
    std::stringstream query1 = std::stringstream("variable v; Select v");
    std::string expected1 = "NAME WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME";
    std::vector<backend::lexer::Token> lexerTokens1 = backend::lexer::tokenizeWithWhitespace(query1);
    REQUIRE(prettyTypeStr(lexerTokens1) == expected1);

    std::stringstream query2 = std::stringstream("stmt s, s1; Select s");
    std::string expected2 =
    "NAME WHITESPACE NAME COMMA WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME";
    std::vector<backend::lexer::Token> lexerTokens2 = backend::lexer::tokenizeWithWhitespace(query2);
    REQUIRE(prettyTypeStr(lexerTokens2) == expected2);

    std::stringstream query3 = std::stringstream("assign a, asd; Select asd");
    std::string expected3 =
    "NAME WHITESPACE NAME COMMA WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME";
    std::vector<backend::lexer::Token> lexerTokens3 = backend::lexer::tokenizeWithWhitespace(query3);
    REQUIRE(prettyTypeStr(lexerTokens3) == expected3);

    std::stringstream query4 = std::stringstream("procedure foo; Select foo");
    std::string expected4 = "NAME WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME";
    std::vector<backend::lexer::Token> lexerTokens4 = backend::lexer::tokenizeWithWhitespace(query4);
    REQUIRE(prettyTypeStr(lexerTokens4) == expected4);
}

TEST_CASE("Queries with synonyms matching design entities") {
    std::stringstream query1 = std::stringstream("stmt stmt; Select stmt");
    std::string expected1 = "NAME WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME";

    std::vector<backend::lexer::Token> lexerTokens1 = backend::lexer::tokenizeWithWhitespace(query1);

    REQUIRE(prettyTypeStr(lexerTokens1) == expected1);

    std::stringstream query2 = std::stringstream("read read; Select read");
    std::string expected2 = "NAME WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME";

    std::vector<backend::lexer::Token> lexerTokens2 = backend::lexer::tokenizeWithWhitespace(query2);

    REQUIRE(prettyTypeStr(lexerTokens2) == expected2);
}

TEST_CASE("Queries with synonyms that are named as a Select token") {
    std::stringstream query = std::stringstream("variable Select; select Select");
    std::string expected = "NAME WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME";

    std::vector<backend::lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(query);

    REQUIRE(prettyTypeStr(lexerTokens) == expected);
}

TEST_CASE("Queries with such that tokens mixed in between") {
    std::stringstream query =
    std::stringstream("assign that; variable such; Select such such that Uses(that, such)");
    std::string expected = "NAME WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME "
                           "SEMICOLON WHITESPACE NAME WHITESPACE NAME WHITESPACE NAME WHITESPACE "
                           "NAME WHITESPACE NAME LPAREN NAME COMMA WHITESPACE NAME RPAREN";
    std::vector<backend::lexer::Token> lexerTokens = backend::lexer::tokenizeWithWhitespace(query);
    REQUIRE(prettyTypeStr(lexerTokens) == expected);
}

TEST_CASE("Queries with one such that clause and a relationship") {
    std::stringstream query1 = std::stringstream("while w; Select w such that Parent*(w, 7)");
    std::string expected1 =
    "NAME WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME WHITESPACE NAME WHITESPACE "
    "NAME WHITESPACE NAME MULT LPAREN NAME COMMA WHITESPACE INTEGER RPAREN";

    std::vector<backend::lexer::Token> lexerTokens1 = backend::lexer::tokenizeWithWhitespace(query1);

    REQUIRE(prettyTypeStr(lexerTokens1) == expected1);

    std::stringstream query2 = std::stringstream("if ifs; Select ifs such that Follows*(5,ifs)");
    std::string expected2 =
    "NAME WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME WHITESPACE NAME WHITESPACE "
    "NAME WHITESPACE NAME MULT LPAREN INTEGER COMMA NAME RPAREN";

    std::vector<backend::lexer::Token> lexerTokens2 = backend::lexer::tokenizeWithWhitespace(query2);

    REQUIRE(prettyTypeStr(lexerTokens2) == expected2);
}

TEST_CASE("Queries with just pattern clauses") {
    std::stringstream query1 =
    std::stringstream("variable v; assign a; Select a pattern a(_, _\"v\"_)");
    std::string expected1 =
    "NAME WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME SEMICOLON WHITESPACE NAME "
    "WHITESPACE NAME WHITESPACE NAME WHITESPACE NAME LPAREN UNDERSCORE COMMA WHITESPACE UNDERSCORE "
    "DOUBLE_QUOTE NAME DOUBLE_QUOTE UNDERSCORE RPAREN";
    std::vector<backend::lexer::Token> lexerTokens1 = backend::lexer::tokenizeWithWhitespace(query1);
    REQUIRE(prettyTypeStr(lexerTokens1) == expected1);

    std::stringstream query2 =
    std::stringstream("variable v, x, y; assign a; Select a pattern a(_, _\"v+x*y\"_)");
    std::string expected2 =
    "NAME WHITESPACE NAME COMMA WHITESPACE NAME COMMA WHITESPACE NAME SEMICOLON WHITESPACE NAME "
    "WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME WHITESPACE NAME WHITESPACE NAME "
    "LPAREN UNDERSCORE COMMA WHITESPACE UNDERSCORE DOUBLE_QUOTE NAME PLUS NAME MULT NAME "
    "DOUBLE_QUOTE UNDERSCORE RPAREN";

    std::vector<backend::lexer::Token> lexerTokens2 = backend::lexer::tokenizeWithWhitespace(query2);
    REQUIRE(prettyTypeStr(lexerTokens2) == expected2);
}

TEST_CASE("Queries with a design entity reference in a Uses/Modifies relationship") {
    std::stringstream query1 = std::stringstream("stmt s; Select s such that Uses(3, \"count\")");
    std::string expected1 =
    "NAME WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME WHITESPACE NAME WHITESPACE "
    "NAME WHITESPACE NAME LPAREN INTEGER COMMA WHITESPACE DOUBLE_QUOTE NAME DOUBLE_QUOTE RPAREN";

    std::vector<backend::lexer::Token> lexerTokens1 = backend::lexer::tokenizeWithWhitespace(query1);

    REQUIRE(prettyTypeStr(lexerTokens1) == expected1);

    std::stringstream query2 =
    std::stringstream("stmt s; Select s such that Modifies(\"procedure1\", \"x\")");
    std::string expected2 =
    "NAME WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME WHITESPACE NAME WHITESPACE "
    "NAME WHITESPACE NAME LPAREN DOUBLE_QUOTE NAME DOUBLE_QUOTE COMMA WHITESPACE DOUBLE_QUOTE NAME "
    "DOUBLE_QUOTE RPAREN";
    std::vector<backend::lexer::Token> lexerTokens2 = backend::lexer::tokenizeWithWhitespace(query2);

    REQUIRE(prettyTypeStr(lexerTokens2) == expected2);
}

TEST_CASE("Queries with underscore in either arguments in a Follows/Parent relationship") {
    std::stringstream query1 = std::stringstream("assign a; Select a such that Parent*(_, a)");
    std::string expected1 =
    "NAME WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME WHITESPACE NAME WHITESPACE "
    "NAME WHITESPACE NAME MULT LPAREN UNDERSCORE COMMA WHITESPACE NAME RPAREN";
    std::vector<backend::lexer::Token> lexerTokens1 = backend::lexer::tokenizeWithWhitespace(query1);
    REQUIRE(prettyTypeStr(lexerTokens1) == expected1);

    std::stringstream query2 = std::stringstream("stmt s; Select s such that Follows(s, _)");
    std::string expected2 =
    "NAME WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME WHITESPACE NAME WHITESPACE "
    "NAME WHITESPACE NAME LPAREN NAME COMMA WHITESPACE UNDERSCORE RPAREN";
    std::vector<backend::lexer::Token> lexerTokens2 = backend::lexer::tokenizeWithWhitespace(query2);
    REQUIRE(prettyTypeStr(lexerTokens2) == expected2);
}

TEST_CASE("Queries with underscore in either arguments in a Uses/Modifies relationship") {
    // Semantically invalid to have an underscore in the first argument, however, it is
    // syntactically correct. Semantic validation will be handled in the query preprocessor.
    std::stringstream query1 = std::stringstream("variable v; Select v such that Modifies(6, _)");
    std::string expected1 =
    "NAME WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME WHITESPACE NAME WHITESPACE "
    "NAME WHITESPACE NAME LPAREN INTEGER COMMA WHITESPACE UNDERSCORE RPAREN";
    std::vector<backend::lexer::Token> lexerTokens1 = backend::lexer::tokenizeWithWhitespace(query1);
    REQUIRE(prettyTypeStr(lexerTokens1) == expected1);

    std::stringstream query2 = std::stringstream("variable v; Select v such that Uses(_, v)");
    std::string expected2 =
    "NAME WHITESPACE NAME SEMICOLON WHITESPACE NAME WHITESPACE NAME WHITESPACE NAME WHITESPACE "
    "NAME WHITESPACE NAME LPAREN UNDERSCORE COMMA WHITESPACE NAME RPAREN";
    std::vector<backend::lexer::Token> lexerTokens2 = backend::lexer::tokenizeWithWhitespace(query2);
    REQUIRE(prettyTypeStr(lexerTokens2) == expected2);
}

TEST_CASE("Names and Integers can be immediately followed by other tokens") {
    std::stringstream query1 = std::stringstream("(_1+1_)");
    std::string expected1 = "LPAREN UNDERSCORE INTEGER PLUS INTEGER UNDERSCORE RPAREN";
    std::vector<backend::lexer::Token> lexerTokens1 = backend::lexer::tokenizeWithWhitespace(query1);
    REQUIRE(prettyTypeStr(lexerTokens1) == expected1);
}

TEST_CASE("Test PERIOD") {
    std::stringstream queryString = std::stringstream(". .. .");
    std::stringstream queryStringCopy = std::stringstream(". .. .");

    std::string expectedTokensWithWhitespace = "PERIOD WHITESPACE PERIOD PERIOD WHITESPACE PERIOD";
    std::string expectedTokensWithoutWhitespace = "PERIOD PERIOD PERIOD PERIOD";

    std::vector<backend::lexer::Token> tokensWithWhitespace =
    backend::lexer::tokenizeWithWhitespace(queryString);
    std::vector<backend::lexer::Token> tokensWithoutWhitespace = backend::lexer::tokenize(queryStringCopy);


    REQUIRE(expectedTokensWithWhitespace == prettyTypeStr(tokensWithWhitespace));
    REQUIRE(expectedTokensWithoutWhitespace == prettyTypeStr(tokensWithoutWhitespace));
}

TEST_CASE("Test HASH") {
    std::stringstream queryString = std::stringstream("# ## #");
    std::stringstream queryStringCopy = std::stringstream("# ## #");

    std::string expectedTokensWithWhitespace = "HASH WHITESPACE HASH HASH WHITESPACE HASH";
    std::string expectedTokensWithoutWhitespace = "HASH HASH HASH HASH";

    std::vector<backend::lexer::Token> tokensWithWhitespace =
    backend::lexer::tokenizeWithWhitespace(queryString);
    std::vector<backend::lexer::Token> tokensWithoutWhitespace = backend::lexer::tokenize(queryStringCopy);


    REQUIRE(expectedTokensWithWhitespace == prettyTypeStr(tokensWithWhitespace));
    REQUIRE(expectedTokensWithoutWhitespace == prettyTypeStr(tokensWithoutWhitespace));
}
