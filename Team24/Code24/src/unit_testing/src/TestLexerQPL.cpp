#include "Lexer.h"
#include "catch.hpp"

#include <sstream>
#include <string>
#include <vector>

// TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/131)
// Testing the values of NAME tokens. e.g.
// map<int, string> indexToExpectedName = {{0, "select"}, ...};
// for (auto& p : indexToExpectedName) {
//   REQUIRE(lexerTokens2[p.first].name == p.second);
// }

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

TEST_CASE("Empty tokens test") {
    std::stringstream query = std::stringstream("");
    std::string expected = "";
    std::vector<backend::lexer::Token> lexerTokens = backend::lexer::tokenize(query);
    REQUIRE(prettyTypeStr(lexerTokens) == expected);
}

TEST_CASE("Multiple whitespace tokens") {
    std::stringstream query = std::stringstream("stmt stmt;read read;assign\n"
                                                "      assign; select\n"
                                                "stmt such that follows\n"
                                                "(\n"
                                                "stmt\n"
                                                "        ,\n"
                                                "_         )\n"
                                                ";");
    std::string expected = "NAME NAME SEMICOLON NAME NAME SEMICOLON NAME NAME SEMICOLON NAME NAME"
                           " NAME NAME NAME LPAREN NAME COMMA UNDERSCORE RPAREN SEMICOLON";
    std::vector<backend::lexer::Token> lexerTokens = backend::lexer::tokenize(query);
    REQUIRE(prettyTypeStr(lexerTokens) == expected);
}

TEST_CASE("Queries with no clauses") {
    std::stringstream query1 = std::stringstream("variable v; Select v");
    std::string expected1 = "NAME NAME SEMICOLON NAME NAME";
    std::vector<backend::lexer::Token> lexerTokens1 = backend::lexer::tokenize(query1);
    REQUIRE(prettyTypeStr(lexerTokens1) == expected1);

    std::stringstream query2 = std::stringstream("stmt s, s1; Select s");
    std::string expected2 = "NAME NAME COMMA NAME SEMICOLON NAME NAME";
    std::vector<backend::lexer::Token> lexerTokens2 = backend::lexer::tokenize(query2);
    REQUIRE(prettyTypeStr(lexerTokens2) == expected2);

    std::stringstream query3 = std::stringstream("assign a, asd; Select asd");
    std::string expected3 = "NAME NAME COMMA NAME SEMICOLON NAME NAME";
    std::vector<backend::lexer::Token> lexerTokens3 = backend::lexer::tokenize(query3);
    REQUIRE(prettyTypeStr(lexerTokens3) == expected3);

    std::stringstream query4 = std::stringstream("procedure foo; Select foo");
    std::string expected4 = "NAME NAME SEMICOLON NAME NAME";
    std::vector<backend::lexer::Token> lexerTokens4 = backend::lexer::tokenize(query4);
    REQUIRE(prettyTypeStr(lexerTokens4) == expected4);
}

TEST_CASE("Queries with synonyms matching design entities") {
    std::stringstream query1 = std::stringstream("stmt stmt; Select stmt");
    std::string expected1 = "NAME NAME SEMICOLON NAME NAME";
    std::vector<backend::lexer::Token> lexerTokens1 = backend::lexer::tokenize(query1);
    REQUIRE(prettyTypeStr(lexerTokens1) == expected1);

    std::stringstream query2 = std::stringstream("read read; Select read");
    std::string expected2 = "NAME NAME SEMICOLON NAME NAME";
    std::vector<backend::lexer::Token> lexerTokens2 = backend::lexer::tokenize(query2);
    REQUIRE(prettyTypeStr(lexerTokens2) == expected2);
}

TEST_CASE("Queries with synonyms that are named as a Select token") {
    std::stringstream query = std::stringstream("variable Select; select Select");
    std::string expected = "NAME NAME SEMICOLON NAME NAME";
    std::vector<backend::lexer::Token> lexerTokens = backend::lexer::tokenize(query);
    REQUIRE(prettyTypeStr(lexerTokens) == expected);
}

TEST_CASE("Queries with such that tokens mixed in between") {
    std::stringstream query =
    std::stringstream("assign that; variable such; Select such such that Uses(that, such)");
    std::string expected = "NAME NAME SEMICOLON NAME NAME SEMICOLON NAME NAME NAME NAME NAME "
                           "LPAREN NAME COMMA NAME RPAREN";
    std::vector<backend::lexer::Token> lexerTokens = backend::lexer::tokenize(query);
    REQUIRE(prettyTypeStr(lexerTokens) == expected);
}

TEST_CASE("Queries with one such that clause and a relationship") {
    std::stringstream query1 = std::stringstream("while w; Select w such that Parent*(w, 7)");
    std::string expected1 =
    "NAME NAME SEMICOLON NAME NAME NAME NAME NAME MULT LPAREN NAME COMMA INTEGER RPAREN";
    std::vector<backend::lexer::Token> lexerTokens1 = backend::lexer::tokenize(query1);
    REQUIRE(prettyTypeStr(lexerTokens1) == expected1);

    std::stringstream query2 = std::stringstream("if ifs; Select ifs such that Follows*(5,ifs)");
    std::string expected2 =
    "NAME NAME SEMICOLON NAME NAME NAME NAME NAME MULT LPAREN INTEGER COMMA NAME RPAREN";
    std::vector<backend::lexer::Token> lexerTokens2 = backend::lexer::tokenize(query2);
    REQUIRE(prettyTypeStr(lexerTokens2) == expected2);
}

TEST_CASE("Queries with just pattern clauses") {
    std::stringstream query1 =
    std::stringstream("variable v; assign a; Select a pattern a(_, _\"v\"_)");
    std::string expected1 = "NAME NAME SEMICOLON NAME NAME SEMICOLON NAME NAME NAME NAME "
                            "LPAREN UNDERSCORE COMMA UNDERSCORE DOUBLE_QUOTE NAME DOUBLE_QUOTE "
                            "UNDERSCORE RPAREN";
    std::vector<backend::lexer::Token> lexerTokens1 = backend::lexer::tokenize(query1);
    REQUIRE(prettyTypeStr(lexerTokens1) == expected1);

    std::stringstream query2 =
    std::stringstream("variable v, x, y; assign a; Select a pattern a(_, _\"v+x*y\"_)");
    std::string expected2 = "NAME NAME COMMA NAME COMMA NAME SEMICOLON NAME NAME SEMICOLON NAME "
                            "NAME NAME NAME LPAREN UNDERSCORE COMMA UNDERSCORE DOUBLE_QUOTE NAME "
                            "PLUS NAME MULT NAME DOUBLE_QUOTE UNDERSCORE RPAREN";
    std::vector<backend::lexer::Token> lexerTokens2 = backend::lexer::tokenize(query2);
    REQUIRE(prettyTypeStr(lexerTokens2) == expected2);
}

TEST_CASE("Queries with a design entity reference in a Uses/Modifies relationship") {
    std::stringstream query1 = std::stringstream("stmt s; Select s such that Uses(3, \"count\")");
    std::string expected1 = "NAME NAME SEMICOLON NAME NAME NAME NAME NAME LPAREN INTEGER COMMA "
                            "DOUBLE_QUOTE NAME DOUBLE_QUOTE RPAREN";
    std::vector<backend::lexer::Token> lexerTokens1 = backend::lexer::tokenize(query1);
    REQUIRE(prettyTypeStr(lexerTokens1) == expected1);

    std::stringstream query2 =
    std::stringstream("stmt s; Select s such that Modifies(\"procedure1\", \"x\")");
    std::string expected2 = "NAME NAME SEMICOLON NAME NAME NAME NAME NAME LPAREN DOUBLE_QUOTE "
                            "NAME DOUBLE_QUOTE COMMA DOUBLE_QUOTE NAME DOUBLE_QUOTE RPAREN";
    std::vector<backend::lexer::Token> lexerTokens2 = backend::lexer::tokenize(query2);
    REQUIRE(prettyTypeStr(lexerTokens2) == expected2);
}

TEST_CASE("Queries with underscore in either arguments in a Follows/Parent relationship") {
    std::stringstream query1 = std::stringstream("assign a; Select a such that Parent*(_, a)");
    std::string expected1 =
    "NAME NAME SEMICOLON NAME NAME NAME NAME NAME MULT LPAREN UNDERSCORE COMMA NAME RPAREN";
    std::vector<backend::lexer::Token> lexerTokens1 = backend::lexer::tokenize(query1);
    REQUIRE(prettyTypeStr(lexerTokens1) == expected1);

    std::stringstream query2 = std::stringstream("stmt s; Select s such that Follows(s, _)");
    std::string expected2 =
    "NAME NAME SEMICOLON NAME NAME NAME NAME NAME LPAREN NAME COMMA UNDERSCORE RPAREN";
    std::vector<backend::lexer::Token> lexerTokens2 = backend::lexer::tokenize(query2);
    REQUIRE(prettyTypeStr(lexerTokens2) == expected2);
}

TEST_CASE("Queries with underscore in either arguments in a Uses/Modifies relationship") {
    // Semantically invalid to have an underscore in the first argument, however, it is
    // syntactically correct. Semantic validation will be handled in the query preprocessor.
    std::stringstream query1 = std::stringstream("variable v; Select v such that Modifies(6, _)");
    std::string expected1 =
    "NAME NAME SEMICOLON NAME NAME NAME NAME NAME LPAREN INTEGER COMMA UNDERSCORE RPAREN";
    std::vector<backend::lexer::Token> lexerTokens1 = backend::lexer::tokenize(query1);
    REQUIRE(prettyTypeStr(lexerTokens1) == expected1);

    std::stringstream query2 = std::stringstream("variable v; Select v such that Uses(_, v)");
    std::string expected2 =
    "NAME NAME SEMICOLON NAME NAME NAME NAME NAME LPAREN UNDERSCORE COMMA NAME RPAREN";
    std::vector<backend::lexer::Token> lexerTokens2 = backend::lexer::tokenize(query2);
    REQUIRE(prettyTypeStr(lexerTokens2) == expected2);
}