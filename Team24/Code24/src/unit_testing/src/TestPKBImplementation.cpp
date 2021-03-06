#include "Logger.h"
#include "PKB.h"
#include "PKBImplementation.h"
#include "TestParserHelpers.h"
#include "catch.hpp"

#include <algorithm>
#include <unordered_map>

namespace backend {
namespace testpkb {

const char STRUCTURED_STATEMENT[] = "procedure MySpecialProc {"

                                    "while (y == 3) {"
                                    "gucci = 1;"
                                    "}"

                                    "if (!(armani == gucci)) then {"
                                    "armani = 7;"
                                    "} else {"
                                    "apple = 1;"
                                    "}"

                                    "some_var = 23 + another_var;"
                                    "}";
template <typename T> std::vector<T> GetSortedVector(std::vector<T> v) {
    sort(v.begin(), v.end());
    return v;
}

TEST_CASE("Test getDirectFollow") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);


    STATEMENT_NUMBER_SET expected = { 3 };
    REQUIRE(pkb.getDirectFollow(1) == expected);

    STATEMENT_NUMBER_SET expected2 = {};
    REQUIRE(pkb.getDirectFollow(2) == expected2);

    STATEMENT_NUMBER_SET expected3 = { 6 };
    REQUIRE(pkb.getDirectFollow(3) == expected3);

    STATEMENT_NUMBER_SET expected4 = {};
    REQUIRE(pkb.getDirectFollow(4) == expected4);

    STATEMENT_NUMBER_SET expected5 = {};
    REQUIRE(pkb.getDirectFollow(5) == expected5);

    STATEMENT_NUMBER_SET expected6 = {};
    REQUIRE(pkb.getDirectFollow(6) == expected6);
}

TEST_CASE("Test getDirectFollowedBy") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected = {};
    REQUIRE(pkb.getDirectFollowedBy(1) == expected);

    STATEMENT_NUMBER_SET expected2 = {};
    REQUIRE(pkb.getDirectFollowedBy(2) == expected2);

    STATEMENT_NUMBER_SET expected3 = { 1 };
    REQUIRE(pkb.getDirectFollowedBy(3) == expected3);

    STATEMENT_NUMBER_SET expected4 = {};
    REQUIRE(pkb.getDirectFollowedBy(4) == expected4);

    STATEMENT_NUMBER_SET expected5 = {};
    REQUIRE(pkb.getDirectFollowedBy(5) == expected5);

    STATEMENT_NUMBER_SET expected6 = { 3 };
    REQUIRE(pkb.getDirectFollowedBy(6) == expected6);
}

TEST_CASE("Test getStatementsFollowedBy") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected = {};
    REQUIRE(pkb.getStatementsFollowedBy(1) == expected);

    STATEMENT_NUMBER_SET expected2 = {};
    REQUIRE(pkb.getStatementsFollowedBy(2) == expected2);

    STATEMENT_NUMBER_SET expected3 = { 1 };
    REQUIRE(pkb.getStatementsFollowedBy(3) == expected3);

    STATEMENT_NUMBER_SET expected4 = {};
    REQUIRE(pkb.getStatementsFollowedBy(4) == expected4);

    STATEMENT_NUMBER_SET expected5 = {};
    REQUIRE(pkb.getStatementsFollowedBy(5) == expected5);

    STATEMENT_NUMBER_SET expected6 = { 3, 1 };
    REQUIRE(pkb.getStatementsFollowedBy(6) == expected6);
}

TEST_CASE("Test getStatementsThatFollows") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected = { 3, 6 };
    REQUIRE(pkb.getStatementsThatFollows(1) == expected);

    STATEMENT_NUMBER_SET expected2 = {};
    REQUIRE(pkb.getStatementsThatFollows(2) == expected2);

    STATEMENT_NUMBER_SET expected3 = { 6 };
    REQUIRE(pkb.getStatementsThatFollows(3) == expected3);

    STATEMENT_NUMBER_SET expected4 = {};
    REQUIRE(pkb.getStatementsThatFollows(4) == expected4);

    STATEMENT_NUMBER_SET expected5 = {};
    REQUIRE(pkb.getStatementsThatFollows(5) == expected5);

    STATEMENT_NUMBER_SET expected6 = {};
    REQUIRE(pkb.getStatementsThatFollows(6) == expected6);
}

TEST_CASE("Test getAllStatementsThatFollows") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected = { 3, 6 };
    STATEMENT_NUMBER_SET actual = pkb.getAllStatementsThatFollows();
    REQUIRE(actual == expected);
}

TEST_CASE("Test getAllStatementsThatAreFollowed") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected = { 1, 3 };
    STATEMENT_NUMBER_SET actual = pkb.getAllStatementsThatAreFollowed();
    REQUIRE(actual == expected);
}

TEST_CASE("Test getAncestors") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected = {};
    REQUIRE(pkb.getAncestors(1) == expected);

    STATEMENT_NUMBER_SET expected2 = { 1 };
    REQUIRE(pkb.getAncestors(2) == expected2);

    STATEMENT_NUMBER_SET expected3 = {};
    REQUIRE(pkb.getAncestors(3) == expected3);

    STATEMENT_NUMBER_SET expected4 = { 3 };
    REQUIRE(pkb.getAncestors(4) == expected4);

    STATEMENT_NUMBER_SET expected5 = { 3 };
    REQUIRE(pkb.getAncestors(5) == expected5);

    STATEMENT_NUMBER_SET expected6 = {};
    REQUIRE(pkb.getAncestors(6) == expected6);
}

TEST_CASE("Test getDescendants") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected = { 2 };
    REQUIRE(pkb.getDescendants(1) == expected);

    STATEMENT_NUMBER_SET expected2 = {};
    REQUIRE(pkb.getDescendants(2) == expected2);

    STATEMENT_NUMBER_SET expected3 = { 4, 5 };
    STATEMENT_NUMBER_SET actual3 = pkb.getDescendants(3);
    REQUIRE(actual3 == expected3);

    STATEMENT_NUMBER_SET expected4 = {};
    REQUIRE(pkb.getDescendants(4) == expected4);

    STATEMENT_NUMBER_SET expected5 = {};
    REQUIRE(pkb.getDescendants(5) == expected5);

    STATEMENT_NUMBER_SET expected6 = {};
    REQUIRE(pkb.getDescendants(6) == expected6);
}

TEST_CASE("Test getParent") {
    const char STRUCTURED_STATEMENT[] = "procedure MySpecialProc {"
                                        "while (y == 3) {"
                                        "a=3;"
                                        "while (x == 3) {"
                                        "x = 1;"
                                        "}"
                                        "b=4;"
                                        "}"
                                        "}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected = {};
    REQUIRE(pkb.getParent(1) == expected);

    STATEMENT_NUMBER_SET expected2 = { 1 };
    REQUIRE(pkb.getParent(2) == expected2);

    STATEMENT_NUMBER_SET expected3 = { 1 };
    REQUIRE(pkb.getParent(3) == expected3);

    STATEMENT_NUMBER_SET expected4 = { 3 };
    REQUIRE(pkb.getParent(4) == expected4);

    STATEMENT_NUMBER_SET expected5 = { 1 };
    REQUIRE(pkb.getParent(5) == expected5);
}

TEST_CASE("Test getChildren") {
    const char STRUCTURED_STATEMENT[] = "procedure MySpecialProc {"
                                        "while (y == 3) {"
                                        "a=3;"
                                        "while (x == 3) {"
                                        "x = 1;"
                                        "}"
                                        "b=4;"
                                        "}"
                                        "}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected = { 2, 3, 5 };
    REQUIRE(pkb.getChildren(1) == expected);

    STATEMENT_NUMBER_SET expected2 = {};
    REQUIRE(pkb.getChildren(2) == expected2);

    STATEMENT_NUMBER_SET expected3 = { 4 };
    REQUIRE(pkb.getChildren(3) == expected3);

    STATEMENT_NUMBER_SET expected4 = {};
    REQUIRE(pkb.getChildren(4) == expected4);

    STATEMENT_NUMBER_SET expected5 = {};
    REQUIRE(pkb.getChildren(5) == expected5);
}

TEST_CASE("Test getStatementsThatHaveAncestors") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    STATEMENT_NUMBER_SET expected = { 2, 4, 5 };
    STATEMENT_NUMBER_SET actual = pkb.getStatementsThatHaveAncestors();

    REQUIRE(expected == actual);
}

TEST_CASE("Test getStatementsThatHaveDescendants") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    STATEMENT_NUMBER_SET expected = { 1, 3 };
    STATEMENT_NUMBER_SET actual = pkb.getStatementsThatHaveDescendants();

    REQUIRE(expected == actual);
}

// We use cars and modify jewellery
const char USES_AND_MODIFIES_PROGRAM[] = "procedure main {"
                                         "while (2 == nike) {" // 1
                                         "gucci = adidas;" // 2
                                         "}"
                                         "call callee1;" // 3
                                         "call callee2;" // 4
                                         "}\n"
                                         "procedure callee1 {"
                                         "if (3 == 3) then {" // 5
                                         "pasha = yeezy;" //  6
                                         "} else {"
                                         "print newBalance;" // 7
                                         "}"
                                         "}"
                                         "procedure callee2 {"
                                         "read armani;" // 8
                                         "}";

TEST_CASE("Test getStatementsThatUse") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<VARIABLE_NAME, STATEMENT_NUMBER_SET> testCases = {
        { "nike", { 1 } }, { "adidas", { 1, 2 } }, { "yeezy", { 3, 5, 6 } }, { "newBalance", { 3, 5, 7 } }
    };
    for (auto& p : testCases) {
        VARIABLE_NAME input = p.first;
        STATEMENT_NUMBER_SET expected = p.second;
        STATEMENT_NUMBER_SET actual = pkb.getStatementsThatUse(input);
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getStatementsThatUseSomeVariable") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    STATEMENT_NUMBER_SET expected = { 1, 2, 3, 5, 6, 7 };
    STATEMENT_NUMBER_SET actual = pkb.getStatementsThatUseSomeVariable();
    REQUIRE(actual == expected);
}

TEST_CASE("Test getProceduresThatUse") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<VARIABLE_NAME, PROCEDURE_NAME_LIST> testCases = {
        { "nike", { "main" } },
        { "adidas", { "main" } },
        { "yeezy", { "callee1", "main" } },
        { "newBalance", { "callee1", "main" } },
        { "gucci", {} },
        { "pasha", {} },
        { "armani", {} },
        { "nonexistentvariable", {} },
    };
    for (auto& p : testCases) {
        VARIABLE_NAME input = p.first;
        PROCEDURE_NAME_LIST expected = GetSortedVector(p.second);
        PROCEDURE_NAME_LIST actual = GetSortedVector(pkb.getProceduresThatUse(input));
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getProceduresThatUseSomeVariable") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    PROCEDURE_NAME_LIST expected = { "callee1", "main" };
    PROCEDURE_NAME_LIST actual = GetSortedVector(pkb.getProceduresThatUseSomeVariable());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getVariablesUsedIn Statement") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<STATEMENT_NUMBER, VARIABLE_NAME_LIST> testCases = {
        { 1, { "adidas", "nike" } },      { 2, { "adidas" } },
        { 3, { "newBalance", "yeezy" } }, { 4, {} },
        { 5, { "newBalance", "yeezy" } }, { 6, { "yeezy" } },
        { 7, { "newBalance" } },          { 8, {} }
    };
    for (auto& p : testCases) {
        STATEMENT_NUMBER input = p.first;
        VARIABLE_NAME_LIST expected = p.second;
        VARIABLE_NAME_LIST actual = GetSortedVector(pkb.getVariablesUsedIn(input));
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getVariablesUsedBySomeProcedure") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    VARIABLE_NAME_LIST expected = { "adidas", "newBalance", "nike", "yeezy" };
    VARIABLE_NAME_LIST actual = GetSortedVector(pkb.getVariablesUsedBySomeProcedure());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getVariablesUsedIn Procedure") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<PROCEDURE_NAME, VARIABLE_NAME_LIST> testCases = {
        { "main", { "adidas", "newBalance", "nike", "yeezy" } },
        { "callee1", { "newBalance", "yeezy" } },
        { "callee2", {} }
    };
    for (auto& p : testCases) {
        PROCEDURE_NAME input = p.first;
        VARIABLE_NAME_LIST expected = p.second;
        VARIABLE_NAME_LIST actual = GetSortedVector(pkb.getVariablesUsedIn(input));
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getVariablesUsedBySomeStatement") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    VARIABLE_NAME_LIST expected = { "adidas", "newBalance", "nike", "yeezy" };
    VARIABLE_NAME_LIST actual = GetSortedVector(pkb.getVariablesUsedBySomeStatement());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getStatementsThatModify") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<VARIABLE_NAME, STATEMENT_NUMBER_SET> testCases = {
        { "nike", {} },         { "adidas", {} },
        { "yeezy", {} },        { "newBalance", {} },
        { "gucci", { 1, 2 } },  { "pasha", { 3, 5, 6 } },
        { "armani", { 4, 8 } }, { "nonexistentvariable", {} },
    };
    for (auto& p : testCases) {
        VARIABLE_NAME input = p.first;
        STATEMENT_NUMBER_SET expected = p.second;
        STATEMENT_NUMBER_SET actual = pkb.getStatementsThatModify(input);
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getStatementsThatModifySomeVariable") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    STATEMENT_NUMBER_SET expected = { 1, 2, 3, 4, 5, 6, 8 };
    STATEMENT_NUMBER_SET actual = pkb.getStatementsThatModifySomeVariable();
    REQUIRE(actual == expected);
}

TEST_CASE("Test getProceduresThatModify") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<VARIABLE_NAME, PROCEDURE_NAME_LIST> testCases = {
        { "nike", {} },
        { "adidas", {} },
        { "yeezy", {} },
        { "newBalance", {} },
        { "gucci", { "main" } },
        { "pasha", { "callee1", "main" } },
        { "armani", { "callee2", "main" } },
        { "nonexistentvariable", {} },
    };
    for (auto& p : testCases) {
        VARIABLE_NAME input = p.first;
        PROCEDURE_NAME_LIST expected = p.second;
        PROCEDURE_NAME_LIST actual = GetSortedVector(pkb.getProceduresThatModify(input));
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getProceduresThatModifySomeVariable") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    PROCEDURE_NAME_LIST expected = { "callee1", "callee2", "main" };
    PROCEDURE_NAME_LIST actual = GetSortedVector(pkb.getProceduresThatModifySomeVariable());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getVariablesModifiedBy Statement") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<STATEMENT_NUMBER, VARIABLE_NAME_LIST> testCases = {
        { 1, { "gucci" } }, { 2, { "gucci" } }, { 3, { "pasha" } }, { 4, { "armani" } },
        { 5, { "pasha" } }, { 6, { "pasha" } }, { 7, {} },          { 8, { "armani" } },
    };
    for (auto& p : testCases) {
        STATEMENT_NUMBER input = p.first;
        VARIABLE_NAME_LIST expected = p.second;
        VARIABLE_NAME_LIST actual = GetSortedVector(pkb.getVariablesModifiedBy(input));
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getVariablesModifiedBySomeProcedure") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    VARIABLE_NAME_LIST expected = { "armani", "gucci", "pasha" };
    VARIABLE_NAME_LIST actual = GetSortedVector(pkb.getVariablesModifiedBySomeProcedure());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getVariablesModifiedBy Procedure") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    std::unordered_map<PROCEDURE_NAME, VARIABLE_NAME_LIST> testCases = {
        { "main", { "armani", "gucci", "pasha" } }, { "callee1", { "pasha" } }, { "callee2", { "armani" } }
    };
    for (auto& p : testCases) {
        PROCEDURE_NAME input = p.first;
        VARIABLE_NAME_LIST expected = p.second;
        VARIABLE_NAME_LIST actual = GetSortedVector(pkb.getVariablesModifiedBy(input));
        REQUIRE(actual == expected);
    }
}

TEST_CASE("Test getVariablesModifiedBySomeStatement") {
    Parser parser = testhelpers::GenerateParserFromTokens(USES_AND_MODIFIES_PROGRAM);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    VARIABLE_NAME_LIST expected = { "armani", "gucci", "pasha" };
    VARIABLE_NAME_LIST actual = GetSortedVector(pkb.getVariablesModifiedBySomeStatement());
    REQUIRE(actual == expected);
}

TEST_CASE("Test getAllAssignmentStatementsThatMatch") {
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET actual1 = pkb.getAllAssignmentStatementsThatMatch("_", "23+another_var", true);
    STATEMENT_NUMBER_SET expected1 = { 6 };
    REQUIRE(actual1 == expected1);

    STATEMENT_NUMBER_SET actual2 = pkb.getAllAssignmentStatementsThatMatch("_", "23+another_var", false);
    STATEMENT_NUMBER_SET expected2 = { 6 };
    REQUIRE(actual2 == expected2);

    STATEMENT_NUMBER_SET actual3 = pkb.getAllAssignmentStatementsThatMatch("_", "another_var", true);
    STATEMENT_NUMBER_SET expected3 = { 6 };
    REQUIRE(actual3 == expected3);

    STATEMENT_NUMBER_SET actual4 = pkb.getAllAssignmentStatementsThatMatch("_", "another_var", false);
    STATEMENT_NUMBER_SET expected4 = {};
    REQUIRE(actual4 == expected4);

    STATEMENT_NUMBER_SET actual5 = pkb.getAllAssignmentStatementsThatMatch("some_var", "another_var", false);
    STATEMENT_NUMBER_SET expected5 = {};
    REQUIRE(actual5 == expected5);

    STATEMENT_NUMBER_SET actual6 = pkb.getAllAssignmentStatementsThatMatch("some_var", "another_var", false);
    STATEMENT_NUMBER_SET expected6 = {};
    REQUIRE(actual6 == expected6);

    STATEMENT_NUMBER_SET actual7 =
    pkb.getAllAssignmentStatementsThatMatch("some_var", "23+another_var", false);
    STATEMENT_NUMBER_SET expected7 = { 6 };
    REQUIRE(actual7 == expected7);

    STATEMENT_NUMBER_SET actual8 = pkb.getAllAssignmentStatementsThatMatch("some_var", "23", true);
    STATEMENT_NUMBER_SET expected8 = { 6 };
    REQUIRE(actual8 == expected8);
}

TEST_CASE("Test getAllAssignmentStatementsThatMatch multiple assign") {
    const char STRUCTURED_STATEMENT[] = "procedure MySpecialProc {"
                                        "x = p + q * r;"
                                        "x = y + q * r;"
                                        "z = y + q * r;"
                                        "w = p + q + r;"
                                        "}";

    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET actual1 = pkb.getAllAssignmentStatementsThatMatch("_", "q*r", true);
    STATEMENT_NUMBER_SET expected1 = { 1, 2, 3 };
    REQUIRE(actual1 == expected1);

    STATEMENT_NUMBER_SET actual2 = pkb.getAllAssignmentStatementsThatMatch("x", "q*r", true);
    STATEMENT_NUMBER_SET expected2 = { 1, 2 };
    REQUIRE(actual2 == expected2);

    STATEMENT_NUMBER_SET actual3 = pkb.getAllAssignmentStatementsThatMatch("_", "y+q", true);
    STATEMENT_NUMBER_SET expected3 = {};
    REQUIRE(actual3 == expected3);

    STATEMENT_NUMBER_SET actual4 = pkb.getAllAssignmentStatementsThatMatch("x", "y+q*r", false);
    STATEMENT_NUMBER_SET expected4 = { 2 };
    REQUIRE(actual4 == expected4);

    STATEMENT_NUMBER_SET actual5 = pkb.getAllAssignmentStatementsThatMatch("z", "y+q*r", false);
    STATEMENT_NUMBER_SET expected5 = { 3 };
    REQUIRE(actual5 == expected5);

    STATEMENT_NUMBER_SET actual6 = pkb.getAllAssignmentStatementsThatMatch("z", "y+q*r", true);
    STATEMENT_NUMBER_SET expected6 = { 3 };
    REQUIRE(actual6 == expected6);

    STATEMENT_NUMBER_SET actual7 = pkb.getAllAssignmentStatementsThatMatch("z", "y", true);
    STATEMENT_NUMBER_SET expected7 = { 3 };
    REQUIRE(actual7 == expected7);

    STATEMENT_NUMBER_SET actual8 = pkb.getAllAssignmentStatementsThatMatch("_", "y", true);
    STATEMENT_NUMBER_SET expected8 = { 2, 3 };
    REQUIRE(actual8 == expected8);

    STATEMENT_NUMBER_SET actual9 = pkb.getAllAssignmentStatementsThatMatch("x", "q*r", false);
    STATEMENT_NUMBER_SET expected9 = {};
    REQUIRE(actual9 == expected9);

    STATEMENT_NUMBER_SET actual10 =
    pkb.getAllAssignmentStatementsThatMatch("x", "impossible_string!@#$%^&*(){}|\"[0987654321", false);
    STATEMENT_NUMBER_SET expected10 = {};
    REQUIRE(actual10 == expected10);

    STATEMENT_NUMBER_SET actual11 = pkb.getAllAssignmentStatementsThatMatch("x", "1+1", false);
    STATEMENT_NUMBER_SET expected11 = {};
    REQUIRE(actual11 == expected11);

    // These are implicit "Modifies" queries since patterns aren't specified.
    STATEMENT_NUMBER_SET actual12 = pkb.getAllAssignmentStatementsThatMatch("z", "", true);
    STATEMENT_NUMBER_SET expected12 = { 3 };
    REQUIRE(actual12 == expected12);

    STATEMENT_NUMBER_SET actual13 = pkb.getAllAssignmentStatementsThatMatch("x", "", true);
    STATEMENT_NUMBER_SET expected13 = { 1, 2 };
    REQUIRE(actual13 == expected13);

    // Left-assoc
    STATEMENT_NUMBER_SET actual14 = pkb.getAllAssignmentStatementsThatMatch("_", "((p+q)+r)", false);
    STATEMENT_NUMBER_SET expected14 = { 4 };
    REQUIRE(actual14 == expected14);

    STATEMENT_NUMBER_SET actual15 = pkb.getAllAssignmentStatementsThatMatch("x", "                 ", false);
    STATEMENT_NUMBER_SET expected15 = {};
    REQUIRE(actual15 == expected15);

    STATEMENT_NUMBER_SET actual16 = pkb.getAllAssignmentStatementsThatMatch("z", "y+(q*r)", true);
    STATEMENT_NUMBER_SET expected16 = { 3 };
    REQUIRE(actual16 == expected16);

    STATEMENT_NUMBER_SET actual17 = pkb.getAllAssignmentStatementsThatMatch("z", "(y+q)*r", true);
    STATEMENT_NUMBER_SET expected17 = {};
    REQUIRE(actual17 == expected17);

    STATEMENT_NUMBER_SET actual18 = pkb.getAllAssignmentStatementsThatMatch("_", "", true);
    STATEMENT_NUMBER_SET expected18 = { 1, 2, 3, 4 };
    REQUIRE(actual18 == expected18);
}

TEST_CASE("Test invalid getAllAssignmentStatementsThatMatch patterns") {
    const char STRUCTURED_STATEMENT[] = "procedure MySpecialProc {"
                                        "x =  q * r + 123 + 1+123;"
                                        "}";

    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET actual1 = pkb.getAllAssignmentStatementsThatMatch("_", "()", true);
    STATEMENT_NUMBER_SET expected1 = {};
    REQUIRE(actual1 == expected1);

    STATEMENT_NUMBER_SET actual1_f = pkb.getAllAssignmentStatementsThatMatch("_", "()", false);
    STATEMENT_NUMBER_SET expected1_f = {};
    REQUIRE(actual1_f == expected1_f);


    STATEMENT_NUMBER_SET actual2 = pkb.getAllAssignmentStatementsThatMatch("_", "()q*r()", true);
    STATEMENT_NUMBER_SET expected2 = {};
    REQUIRE(actual2 == expected2);

    STATEMENT_NUMBER_SET actual2_f = pkb.getAllAssignmentStatementsThatMatch("_", "()q*r()", false);
    STATEMENT_NUMBER_SET expected2_f = {};
    REQUIRE(actual2_f == expected2_f);

    STATEMENT_NUMBER_SET actual3 = pkb.getAllAssignmentStatementsThatMatch("_", "12 3", true);
    STATEMENT_NUMBER_SET expected3 = {};
    REQUIRE(actual3 == expected3);

    STATEMENT_NUMBER_SET actual3_f = pkb.getAllAssignmentStatementsThatMatch("_", "12 3", false);
    STATEMENT_NUMBER_SET expected3_f = {};
    REQUIRE(actual3_f == expected3_f);

    STATEMENT_NUMBER_SET actual4 = pkb.getAllAssignmentStatementsThatMatch("_", "q*r+12\n3", true);
    STATEMENT_NUMBER_SET expected4 = {};
    REQUIRE(actual4 == expected4);

    STATEMENT_NUMBER_SET actual4_f = pkb.getAllAssignmentStatementsThatMatch("_", "q*r+12\n3", false);
    STATEMENT_NUMBER_SET expected4_f = {};
    REQUIRE(actual4_f == expected4_f);

    STATEMENT_NUMBER_SET actual5 = pkb.getAllAssignmentStatementsThatMatch("_", "q\n*r+123", true);
    STATEMENT_NUMBER_SET expected5 = { 1 };
    REQUIRE(actual5 == expected5);

    STATEMENT_NUMBER_SET actual5_f = pkb.getAllAssignmentStatementsThatMatch("_", "q\n*r+123", false);
    STATEMENT_NUMBER_SET expected5_f = {};
    REQUIRE(actual5_f == expected5_f);

    STATEMENT_NUMBER_SET actual6 = pkb.getAllAssignmentStatementsThatMatch("_", "q*r+\n123", true);
    STATEMENT_NUMBER_SET expected6 = { 1 };
    REQUIRE(actual6 == expected6);

    STATEMENT_NUMBER_SET actual6_f = pkb.getAllAssignmentStatementsThatMatch("_", "q*r+\n123", false);
    STATEMENT_NUMBER_SET expected6_f = {};
    REQUIRE(actual6_f == expected6_f);

    STATEMENT_NUMBER_SET actual7 = pkb.getAllAssignmentStatementsThatMatch("_", "    ", true);
    STATEMENT_NUMBER_SET expected7 = { 1 };
    REQUIRE(actual7 == expected7);

    STATEMENT_NUMBER_SET actual7_f = pkb.getAllAssignmentStatementsThatMatch("_", "    ", false);
    STATEMENT_NUMBER_SET expected7_f = {};
    REQUIRE(actual7_f == expected7_f);

    STATEMENT_NUMBER_SET actual8 = pkb.getAllAssignmentStatementsThatMatch("_", "1+12 3", true);
    STATEMENT_NUMBER_SET expected8 = {};
    REQUIRE(actual8 == expected8);

    STATEMENT_NUMBER_SET actual8_f = pkb.getAllAssignmentStatementsThatMatch("_", "1+12 3", false);
    STATEMENT_NUMBER_SET expected8_f = {};
    REQUIRE(actual8_f == expected8_f);
}

TEST_CASE("Test isEntity") {
    const char STRUCTURED_STATEMENT[] = "procedure aoeu {"

                                        "while (y == 3) {"
                                        "gucci = 1;"
                                        "}"

                                        "if (!(armani == gucci)) then {"
                                        "read x;"
                                        "call y;"
                                        "} else {"
                                        "print z;"
                                        "}"
                                        "}"

                                        "procedure y {y = 1+1;}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    REQUIRE(pkb.isWhile(1));
    REQUIRE(pkb.isAssign(2));
    REQUIRE(pkb.isIfElse(3));
    REQUIRE(pkb.isRead(4));
    REQUIRE(pkb.isCall(5));
    REQUIRE(pkb.isPrint(6));

    REQUIRE_FALSE(pkb.isWhile(2));
    REQUIRE_FALSE(pkb.isWhile(3));
    REQUIRE_FALSE(pkb.isWhile(4));
    REQUIRE_FALSE(pkb.isWhile(5));
    REQUIRE_FALSE(pkb.isWhile(6));

    REQUIRE_FALSE(pkb.isAssign(1));
    REQUIRE_FALSE(pkb.isAssign(3));
    REQUIRE_FALSE(pkb.isAssign(4));
    REQUIRE_FALSE(pkb.isAssign(5));
    REQUIRE_FALSE(pkb.isAssign(6));

    REQUIRE_FALSE(pkb.isIfElse(1));
    REQUIRE_FALSE(pkb.isIfElse(2));
    REQUIRE_FALSE(pkb.isIfElse(4));
    REQUIRE_FALSE(pkb.isIfElse(5));
    REQUIRE_FALSE(pkb.isIfElse(6));

    REQUIRE_FALSE(pkb.isRead(1));
    REQUIRE_FALSE(pkb.isRead(2));
    REQUIRE_FALSE(pkb.isRead(3));
    REQUIRE_FALSE(pkb.isRead(5));
    REQUIRE_FALSE(pkb.isRead(6));

    REQUIRE_FALSE(pkb.isCall(1));
    REQUIRE_FALSE(pkb.isCall(2));
    REQUIRE_FALSE(pkb.isCall(3));
    REQUIRE_FALSE(pkb.isCall(4));
    REQUIRE_FALSE(pkb.isCall(6));

    REQUIRE_FALSE(pkb.isPrint(1));
    REQUIRE_FALSE(pkb.isPrint(2));
    REQUIRE_FALSE(pkb.isPrint(3));
    REQUIRE_FALSE(pkb.isPrint(4));
    REQUIRE_FALSE(pkb.isPrint(5));
}

TEST_CASE("Test getAllEntity") {
    const char STRUCTURED_STATEMENT[] = "procedure aoeu {"

                                        "while (y == 3) {"
                                        "gucci = 1;"
                                        "}"

                                        "if (!(armani == gucci)) then {"
                                        "read x;"
                                        "call y;"
                                        "} else {"
                                        "print z;"
                                        "}"
                                        "}"

                                        "procedure y {y = 1+1;}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    VARIABLE_NAME_LIST expectedVariables = GetSortedVector<std::string>({ "armani", "gucci", "x", "y", "z" });
    VARIABLE_NAME_LIST actualVariables = GetSortedVector(pkb.getAllVariables());
    REQUIRE(actualVariables == expectedVariables);

    VARIABLE_NAME_LIST expectedProcedures = { "aoeu", "y" };
    VARIABLE_NAME_LIST actualProcedures = GetSortedVector(pkb.getAllProcedures());
    REQUIRE(actualProcedures == expectedProcedures);

    std::unordered_set<std::string> expectedConstants = { "3", "1" };
    std::unordered_set<std::string> actualConstants = pkb.getAllConstants();
    REQUIRE(actualConstants == expectedConstants);
}

TEST_CASE("Test getProcedureThatCalls") {
    const char STRUCTURED_STATEMENT[] = "procedure a {"
                                        "call b;"
                                        "call d;"
                                        "}"

                                        "procedure b {"
                                        "call c;"
                                        "call d;"
                                        "}"

                                        "procedure c {y = 1+1;}"
                                        "procedure d {y = 1+1;}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::unordered_set<std::string> actualA = pkb.getProcedureThatCalls("a", false);
    std::unordered_set<std::string> expectedA = {};
    REQUIRE(actualA == expectedA);

    std::unordered_set<std::string> actualB = pkb.getProcedureThatCalls("b", false);
    std::unordered_set<std::string> expectedB = { "a" };
    REQUIRE(actualB == expectedB);

    std::unordered_set<std::string> actualC = pkb.getProcedureThatCalls("c", false);
    std::unordered_set<std::string> expectedC = { "b" };
    REQUIRE(actualC == expectedC);

    std::unordered_set<std::string> actualD = pkb.getProcedureThatCalls("d", false);
    std::unordered_set<std::string> expectedD = { "a", "b" };
    REQUIRE(actualD == expectedD);

    std::unordered_set<std::string> actualA_transitive = pkb.getProcedureThatCalls("a", true);
    std::unordered_set<std::string> expectedA_transitive = {};
    REQUIRE(actualA_transitive == expectedA_transitive);

    std::unordered_set<std::string> actualB_transitive = pkb.getProcedureThatCalls("b", true);
    std::unordered_set<std::string> expectedB_transitive = { "a" };
    REQUIRE(actualB_transitive == expectedB_transitive);

    std::unordered_set<std::string> actualC_transitive = pkb.getProcedureThatCalls("c", true);
    std::unordered_set<std::string> expectedC_transitive = { "a", "b" };
    REQUIRE(actualC_transitive == expectedC_transitive);

    std::unordered_set<std::string> actualD_transitive = pkb.getProcedureThatCalls("d", true);
    std::unordered_set<std::string> expectedD_transitive = { "a", "b" };
    REQUIRE(actualD_transitive == expectedD_transitive);

    std::unordered_set<std::string> actualE_transitive = pkb.getProcedureThatCalls("e", true);
    std::unordered_set<std::string> expectedE_transitive = {};
    REQUIRE(actualE_transitive == expectedE_transitive);
}

TEST_CASE("Test getProceduresCalledBy") {
    const char STRUCTURED_STATEMENT[] = "procedure a {"
                                        "call b;"
                                        "call d;"
                                        "}"

                                        "procedure b {"
                                        "call c;"
                                        "call d;"
                                        "}"

                                        "procedure c {y = 1+1;}"
                                        "procedure d {y = 1+1;}";
    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    std::unordered_set<std::string> actualA = pkb.getProceduresCalledBy("a", false);
    std::unordered_set<std::string> expectedA = { "b", "d" };
    REQUIRE(actualA == expectedA);

    std::unordered_set<std::string> actualB = pkb.getProceduresCalledBy("b", false);
    std::unordered_set<std::string> expectedB = { "c", "d" };
    REQUIRE(actualB == expectedB);

    std::unordered_set<std::string> actualC = pkb.getProceduresCalledBy("c", false);
    std::unordered_set<std::string> expectedC = {};
    REQUIRE(actualC == expectedC);

    std::unordered_set<std::string> actualD = pkb.getProceduresCalledBy("d", false);
    std::unordered_set<std::string> expectedD = {};
    REQUIRE(actualD == expectedD);

    std::unordered_set<std::string> actualE = pkb.getProceduresCalledBy("e", false);
    std::unordered_set<std::string> expectedE = {};
    REQUIRE(actualE == expectedE);

    std::unordered_set<std::string> actualA_transitive = pkb.getProceduresCalledBy("a", true);
    std::unordered_set<std::string> expectedA_transitive = { "b", "c", "d" };
    REQUIRE(actualA_transitive == expectedA_transitive);

    std::unordered_set<std::string> actualB_transitive = pkb.getProceduresCalledBy("b", true);
    std::unordered_set<std::string> expectedB_transitive = { "c", "d" };
    REQUIRE(actualB_transitive == expectedB_transitive);

    std::unordered_set<std::string> actualC_transitive = pkb.getProceduresCalledBy("c", true);
    std::unordered_set<std::string> expectedC_transitive = {};
    REQUIRE(actualC_transitive == expectedC_transitive);

    std::unordered_set<std::string> actualD_transitive = pkb.getProceduresCalledBy("d", true);
    std::unordered_set<std::string> expectedD_transitive = {};
    REQUIRE(actualD_transitive == expectedD_transitive);

    std::unordered_set<std::string> actualE_transitive = pkb.getProceduresCalledBy("e", true);
    std::unordered_set<std::string> expectedE_transitive = {};
    REQUIRE(actualE_transitive == expectedE_transitive);
}

TEST_CASE("Test getNextStatementOf") {
    const char STRUCTURED_STATEMENT[] = "procedure a {         "
                                        "  while (1 == 1) {    " // 1
                                        "    while (1==1) {    " // 2
                                        "      a = 1;          " // 3
                                        "      while (1 == 1) {" // 4
                                        "        a = 1;        " // 5
                                        "        a = 2;        " // 6
                                        "      }"
                                        "    }"
                                        "    a = 1;            " // 7
                                        "  }"
                                        "  call b;             " // 8
                                        "}"
                                        ""
                                        "procedure b {         "
                                        "  a = 1;              " // 9
                                        "  b = 2;              " // 10
                                        "}";

    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    // TEST TRANSITIVE
    STATEMENT_NUMBER_SET actual_transitive_1 = pkb.getNextStatementOf(1, true);
    STATEMENT_NUMBER_SET expected_transitive_1 = { 1, 2, 3, 4, 5, 6, 7, 8 };
    REQUIRE(actual_transitive_1 == expected_transitive_1);

    STATEMENT_NUMBER_SET actual_transitive_2 = pkb.getNextStatementOf(2, true);
    STATEMENT_NUMBER_SET expected_transitive_2 = { 1, 2, 3, 4, 5, 6, 7, 8 };
    REQUIRE(actual_transitive_2 == expected_transitive_2);

    STATEMENT_NUMBER_SET actual_transitive_3 = pkb.getNextStatementOf(3, true);
    STATEMENT_NUMBER_SET expected_transitive_3 = { 1, 2, 3, 4, 5, 6, 7, 8 };
    REQUIRE(actual_transitive_3 == expected_transitive_3);

    STATEMENT_NUMBER_SET actual_transitive_4 = pkb.getNextStatementOf(4, true);
    STATEMENT_NUMBER_SET expected_transitive_4 = { 1, 2, 3, 4, 5, 6, 7, 8 };
    REQUIRE(actual_transitive_4 == expected_transitive_4);

    STATEMENT_NUMBER_SET actual_transitive_5 = pkb.getNextStatementOf(5, true);
    STATEMENT_NUMBER_SET expected_transitive_5 = { 1, 2, 3, 4, 5, 6, 7, 8 };
    REQUIRE(actual_transitive_5 == expected_transitive_5);

    STATEMENT_NUMBER_SET actual_transitive_6 = pkb.getNextStatementOf(6, true);
    STATEMENT_NUMBER_SET expected_transitive_6 = { 1, 2, 3, 4, 5, 6, 7, 8 };
    REQUIRE(actual_transitive_6 == expected_transitive_6);

    STATEMENT_NUMBER_SET actual_transitive_7 = pkb.getNextStatementOf(7, true);
    STATEMENT_NUMBER_SET expected_transitive_7 = { 1, 2, 3, 4, 5, 6, 7, 8 };
    REQUIRE(actual_transitive_7 == expected_transitive_7);

    STATEMENT_NUMBER_SET actual_transitive_8 = pkb.getNextStatementOf(8, true);
    STATEMENT_NUMBER_SET expected_transitive_8 = {};
    REQUIRE(actual_transitive_8 == expected_transitive_8);

    STATEMENT_NUMBER_SET actual_transitive_9 = pkb.getNextStatementOf(9, true);
    STATEMENT_NUMBER_SET expected_transitive_9 = { 10 };
    REQUIRE(actual_transitive_9 == expected_transitive_9);

    STATEMENT_NUMBER_SET actual_transitive_10 = pkb.getNextStatementOf(10, true);
    STATEMENT_NUMBER_SET expected_transitive_10 = {};
    REQUIRE(actual_transitive_10 == expected_transitive_10);

    // TEST NON-TRANSITIVE
    STATEMENT_NUMBER_SET actual_non_transitive_1 = pkb.getNextStatementOf(1, false);
    STATEMENT_NUMBER_SET expected_non_transitive_1 = { 2, 8 };
    REQUIRE(actual_non_transitive_1 == expected_non_transitive_1);

    STATEMENT_NUMBER_SET actual_non_transitive_2 = pkb.getNextStatementOf(2, false);
    STATEMENT_NUMBER_SET expected_non_transitive_2 = { 3, 7 };
    REQUIRE(actual_non_transitive_2 == expected_non_transitive_2);

    STATEMENT_NUMBER_SET actual_non_transitive_3 = pkb.getNextStatementOf(3, false);
    STATEMENT_NUMBER_SET expected_non_transitive_3 = { 4 };
    REQUIRE(actual_non_transitive_3 == expected_non_transitive_3);

    STATEMENT_NUMBER_SET actual_non_transitive_4 = pkb.getNextStatementOf(4, false);
    STATEMENT_NUMBER_SET expected_non_transitive_4 = { 2, 5 };
    REQUIRE(actual_non_transitive_4 == expected_non_transitive_4);

    STATEMENT_NUMBER_SET actual_non_transitive_5 = pkb.getNextStatementOf(5, false);
    STATEMENT_NUMBER_SET expected_non_transitive_5 = { 6 };
    REQUIRE(actual_non_transitive_5 == expected_non_transitive_5);

    STATEMENT_NUMBER_SET actual_non_transitive_6 = pkb.getNextStatementOf(6, false);
    STATEMENT_NUMBER_SET expected_non_transitive_6 = { 4 };
    REQUIRE(actual_non_transitive_6 == expected_non_transitive_6);

    STATEMENT_NUMBER_SET actual_non_transitive_7 = pkb.getNextStatementOf(7, false);
    STATEMENT_NUMBER_SET expected_non_transitive_7 = { 1 };
    REQUIRE(actual_non_transitive_7 == expected_non_transitive_7);

    STATEMENT_NUMBER_SET actual_non_transitive_8 = pkb.getNextStatementOf(8, false);
    STATEMENT_NUMBER_SET expected_non_transitive_8 = {};
    REQUIRE(actual_non_transitive_8 == expected_non_transitive_8);

    STATEMENT_NUMBER_SET actual_non_transitive_9 = pkb.getNextStatementOf(9, false);
    STATEMENT_NUMBER_SET expected_non_transitive_9 = { 10 };
    REQUIRE(actual_non_transitive_9 == expected_non_transitive_9);

    STATEMENT_NUMBER_SET actual_non_transitive_10 = pkb.getNextStatementOf(10, false);
    STATEMENT_NUMBER_SET expected_non_transitive_10 = {};
    REQUIRE(actual_non_transitive_10 == expected_non_transitive_10);
}

TEST_CASE("Test getPreviousStatementOf") {
    const char STRUCTURED_STATEMENT[] = "procedure a {         "
                                        "  while (1 == 1) {    " // 1
                                        "    while (1==1) {    " // 2
                                        "      a = 1;          " // 3
                                        "      while (1 == 1) {" // 4
                                        "        a = 1;        " // 5
                                        "        a = 2;        " // 6
                                        "      }"
                                        "    }"
                                        "    a = 1;            " // 7
                                        "  }"
                                        "  call b;             " // 8
                                        "}"
                                        ""
                                        "procedure b {         "
                                        "  a = 1;              " // 9
                                        "  b = 2;              " // 10
                                        "}";

    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    // TEST TRANSITIVE
    STATEMENT_NUMBER_SET actual_transitive_1 = pkb.getPreviousStatementOf(1, true);
    STATEMENT_NUMBER_SET expected_transitive_1 = { 1, 2, 3, 4, 5, 6, 7 };
    REQUIRE(actual_transitive_1 == expected_transitive_1);

    STATEMENT_NUMBER_SET actual_transitive_2 = pkb.getPreviousStatementOf(2, true);
    STATEMENT_NUMBER_SET expected_transitive_2 = { 1, 2, 3, 4, 5, 6, 7 };
    REQUIRE(actual_transitive_2 == expected_transitive_2);

    STATEMENT_NUMBER_SET actual_transitive_3 = pkb.getPreviousStatementOf(3, true);
    STATEMENT_NUMBER_SET expected_transitive_3 = { 1, 2, 3, 4, 5, 6, 7 };
    REQUIRE(actual_transitive_3 == expected_transitive_3);

    STATEMENT_NUMBER_SET actual_transitive_4 = pkb.getPreviousStatementOf(4, true);
    STATEMENT_NUMBER_SET expected_transitive_4 = { 1, 2, 3, 4, 5, 6, 7 };
    REQUIRE(actual_transitive_4 == expected_transitive_4);

    STATEMENT_NUMBER_SET actual_transitive_5 = pkb.getPreviousStatementOf(5, true);
    STATEMENT_NUMBER_SET expected_transitive_5 = { 1, 2, 3, 4, 5, 6, 7 };
    REQUIRE(actual_transitive_5 == expected_transitive_5);

    STATEMENT_NUMBER_SET actual_transitive_6 = pkb.getPreviousStatementOf(6, true);
    STATEMENT_NUMBER_SET expected_transitive_6 = { 1, 2, 3, 4, 5, 6, 7 };
    REQUIRE(actual_transitive_6 == expected_transitive_6);

    STATEMENT_NUMBER_SET actual_transitive_7 = pkb.getPreviousStatementOf(7, true);
    STATEMENT_NUMBER_SET expected_transitive_7 = { 1, 2, 3, 4, 5, 6, 7 };
    REQUIRE(actual_transitive_7 == expected_transitive_7);

    STATEMENT_NUMBER_SET actual_transitive_8 = pkb.getPreviousStatementOf(8, true);
    STATEMENT_NUMBER_SET expected_transitive_8 = { 1, 2, 3, 4, 5, 6, 7 };
    REQUIRE(actual_transitive_8 == expected_transitive_8);

    STATEMENT_NUMBER_SET actual_transitive_9 = pkb.getPreviousStatementOf(9, true);
    STATEMENT_NUMBER_SET expected_transitive_9 = {};
    REQUIRE(actual_transitive_9 == expected_transitive_9);

    STATEMENT_NUMBER_SET actual_transitive_10 = pkb.getPreviousStatementOf(10, true);
    STATEMENT_NUMBER_SET expected_transitive_10 = { 9 };
    REQUIRE(actual_transitive_10 == expected_transitive_10);

    // TEST NON-TRANSITIVE
    STATEMENT_NUMBER_SET actual_non_transitive_1 = pkb.getPreviousStatementOf(1, false);
    STATEMENT_NUMBER_SET expected_non_transitive_1 = { 7 };
    REQUIRE(actual_non_transitive_1 == expected_non_transitive_1);

    STATEMENT_NUMBER_SET actual_non_transitive_2 = pkb.getPreviousStatementOf(2, false);
    STATEMENT_NUMBER_SET expected_non_transitive_2 = { 1, 4 };
    REQUIRE(actual_non_transitive_2 == expected_non_transitive_2);

    STATEMENT_NUMBER_SET actual_non_transitive_3 = pkb.getPreviousStatementOf(3, false);
    STATEMENT_NUMBER_SET expected_non_transitive_3 = { 2 };
    REQUIRE(actual_non_transitive_3 == expected_non_transitive_3);

    STATEMENT_NUMBER_SET actual_non_transitive_4 = pkb.getPreviousStatementOf(4, false);
    STATEMENT_NUMBER_SET expected_non_transitive_4 = { 3, 6 };
    REQUIRE(actual_non_transitive_4 == expected_non_transitive_4);

    STATEMENT_NUMBER_SET actual_non_transitive_5 = pkb.getPreviousStatementOf(5, false);
    STATEMENT_NUMBER_SET expected_non_transitive_5 = { 4 };
    REQUIRE(actual_non_transitive_5 == expected_non_transitive_5);

    STATEMENT_NUMBER_SET actual_non_transitive_6 = pkb.getPreviousStatementOf(6, false);
    STATEMENT_NUMBER_SET expected_non_transitive_6 = { 5 };
    REQUIRE(actual_non_transitive_6 == expected_non_transitive_6);

    STATEMENT_NUMBER_SET actual_non_transitive_7 = pkb.getPreviousStatementOf(7, false);
    STATEMENT_NUMBER_SET expected_non_transitive_7 = { 2 };
    REQUIRE(actual_non_transitive_7 == expected_non_transitive_7);

    STATEMENT_NUMBER_SET actual_non_transitive_8 = pkb.getPreviousStatementOf(8, false);
    STATEMENT_NUMBER_SET expected_non_transitive_8 = { 1 };
    REQUIRE(actual_non_transitive_8 == expected_non_transitive_8);

    STATEMENT_NUMBER_SET actual_non_transitive_9 = pkb.getPreviousStatementOf(9, false);
    STATEMENT_NUMBER_SET expected_non_transitive_9 = {};
    REQUIRE(actual_non_transitive_9 == expected_non_transitive_9);

    STATEMENT_NUMBER_SET actual_non_transitive_10 = pkb.getPreviousStatementOf(10, false);
    STATEMENT_NUMBER_SET expected_non_transitive_10 = { 9 };
    REQUIRE(actual_non_transitive_10 == expected_non_transitive_10);
}

TEST_CASE("getCallStatementsWithProcedureName") {
    const char program[] = "procedure a {         "
                           "  call b;             " // 1
                           "  call c;             " // 2
                           "}"
                           ""
                           "procedure b {         "
                           "  call c;             " // 3
                           "}"
                           ""
                           "procedure c {"
                           "  x = 1;              " // 4
                           "}";
    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected = { 1 };
    REQUIRE(pkb.getCallStatementsWithProcedureName("b") == expected);

    expected = { 2, 3 };
    REQUIRE(pkb.getCallStatementsWithProcedureName("c") == expected);

    expected = {};
    REQUIRE(pkb.getCallStatementsWithProcedureName("nonexistentproc") == expected);
}

TEST_CASE("getAllProceduresThatCallSomeProcedure") {
    const char program[] = "procedure a {         "
                           "  call b;             "
                           "  call c;             "
                           "}"
                           ""
                           "procedure b {         "
                           "  call c;             "
                           "}"
                           ""
                           "procedure c {         "
                           "  x = 1;              "
                           "}"
                           "procedure d {"
                           "  y = 2;"
                           "}";
    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    PROCEDURE_NAME_SET expected = { "a", "b" };
    REQUIRE(pkb.getAllProceduresThatCallSomeProcedure() == expected);
}

TEST_CASE("getAllCalledProcedures") {
    const char program[] = "procedure a {         "
                           "  call b;             "
                           "  call c;             "
                           "}"
                           ""
                           "procedure b {         "
                           "  call c;             "
                           "}"
                           ""
                           "procedure c {         "
                           "  x = 1;              "
                           "}"
                           "procedure d {"
                           "  y = 2;"
                           "}";
    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    PROCEDURE_NAME_SET expected = { "b", "c" };
    REQUIRE(pkb.getAllCalledProcedures() == expected);
}

TEST_CASE("getReadStatementsWithVariableName") {
    const char program[] = "procedure a {         "
                           "  read a;             " // 1
                           "  call b;             " // 2
                           "}"
                           ""
                           "procedure b {         "
                           "  print c;             " // 3
                           "}"
                           ""
                           "procedure c {"
                           "  read b;              " // 4
                           "  read c;              " // 5
                           "}";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected = { 1 };
    REQUIRE(pkb.getReadStatementsWithVariableName("a") == expected);

    expected = { 4 };
    REQUIRE(pkb.getReadStatementsWithVariableName("b") == expected);

    expected = { 5 };
    REQUIRE(pkb.getReadStatementsWithVariableName("c") == expected);

    expected = {};
    REQUIRE(pkb.getReadStatementsWithVariableName("nonexistentvariable") == expected);
}

TEST_CASE("getPrintStatementsWithVariableName") {
    const char program[] = "procedure a {         "
                           "  print a;             " // 1
                           "  call b;             " // 2
                           "}"
                           ""
                           "procedure b {         "
                           "  read c;             " // 3
                           "}"
                           ""
                           "procedure c {"
                           "  print b;              " // 4
                           "  print c;              " // 5
                           "}";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected = { 1 };
    REQUIRE(pkb.getPrintStatementsWithVariableName("a") == expected);

    expected = { 4 };
    REQUIRE(pkb.getPrintStatementsWithVariableName("b") == expected);

    expected = { 5 };
    REQUIRE(pkb.getPrintStatementsWithVariableName("c") == expected);

    expected = {};
    REQUIRE(pkb.getPrintStatementsWithVariableName("nonexistentvariable") == expected);
}


TEST_CASE("getProcedureNameFromCallStatement") {
    const char program[] = "procedure a {         "
                           "  call c;             " // 1
                           "  call b;             " // 2
                           "}"
                           ""
                           "procedure b {         "
                           "  call c;             " // 3
                           "}"
                           ""
                           "procedure c {"
                           "  x = 1;              " // 4
                           "}";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    REQUIRE(pkb.getProcedureNameFromCallStatement(1) == "c");
    REQUIRE(pkb.getProcedureNameFromCallStatement(2) == "b");
    REQUIRE(pkb.getProcedureNameFromCallStatement(3) == "c");
    REQUIRE(pkb.getProcedureNameFromCallStatement(4) == "");
}
TEST_CASE("getVariableNameFromReadStatement") {
    const char program[] = "procedure a {         "
                           "  read i;             " // 1
                           "  call need;          " // 2
                           "}"
                           ""
                           "procedure need {      "
                           "  read a;             " // 3
                           "}"
                           ""
                           "procedure c {"
                           "  read job;           " // 4
                           "  print badly;        " // 5
                           "}";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    REQUIRE(pkb.getVariableNameFromReadStatement(1) == "i");
    REQUIRE(pkb.getVariableNameFromReadStatement(2) == "");
    REQUIRE(pkb.getVariableNameFromReadStatement(3) == "a");
    REQUIRE(pkb.getVariableNameFromReadStatement(4) == "job");
    REQUIRE(pkb.getVariableNameFromReadStatement(5) == "");
}

TEST_CASE("getVariableNameFromPrintStatement") {
    const char program[] = "procedure a {           "
                           "  print i;              " // 1
                           "  call need;            " // 2
                           "}"
                           ""
                           "procedure need {        "
                           "  print a;              " // 3
                           "}"
                           ""
                           "procedure c {"
                           "  print job;           " // 4
                           "  read badly;          " // 5
                           "}";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    REQUIRE(pkb.getVariableNameFromPrintStatement(1) == "i");
    REQUIRE(pkb.getVariableNameFromPrintStatement(2) == "");
    REQUIRE(pkb.getVariableNameFromPrintStatement(3) == "a");
    REQUIRE(pkb.getVariableNameFromPrintStatement(4) == "job");
    REQUIRE(pkb.getVariableNameFromPrintStatement(5) == "");
}

TEST_CASE("Test getAllWhileStatementsThatMatch") {
    const char program[] = "procedure a {                   "
                           "  while (a + b == c) {          " // 1
                           "    y = 1;                      " // 2
                           "  }                             "
                           "  if (a + b == d + g) then {    " // 3
                           "    while (a + d == e) {        " // 4
                           "      y = 1;                    " // 5
                           "    }                           "
                           "  } else {                      "
                           "    y = 2;                      " // 6
                           "  }                             "
                           "  while (a + b < f) {           " // 7
                           "    if (a + b == d * h) then {  " // 8
                           "      y = 1;                    " // 9
                           "    } else {                    "
                           "      y = 2;                    " // 10
                           "    }                           "
                           "  }                             "
                           "  while (1==1) {                " // 11
                           "    y = 1;                      " // 12
                           "  }                             "
                           "}";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET actual1 = pkb.getAllWhileStatementsThatMatch("a", "", true);
    STATEMENT_NUMBER_SET expected1 = { 1, 4, 7 };
    REQUIRE(actual1 == expected1);

    STATEMENT_NUMBER_SET actual2 = pkb.getAllWhileStatementsThatMatch("b", "", true);
    STATEMENT_NUMBER_SET expected2 = { 1, 7 };
    REQUIRE(actual2 == expected2);

    STATEMENT_NUMBER_SET actual3 = pkb.getAllWhileStatementsThatMatch("c", "", true);
    STATEMENT_NUMBER_SET expected3 = { 1 };
    REQUIRE(actual3 == expected3);

    STATEMENT_NUMBER_SET actual4 = pkb.getAllWhileStatementsThatMatch("d", "", true);
    STATEMENT_NUMBER_SET expected4 = { 4 };
    REQUIRE(actual4 == expected4);

    STATEMENT_NUMBER_SET actual5 = pkb.getAllWhileStatementsThatMatch("e", "", true);
    STATEMENT_NUMBER_SET expected5 = { 4 };
    REQUIRE(actual5 == expected5);

    STATEMENT_NUMBER_SET actual6 = pkb.getAllWhileStatementsThatMatch("f", "", true);
    STATEMENT_NUMBER_SET expected6 = { 7 };
    REQUIRE(actual6 == expected6);

    STATEMENT_NUMBER_SET actual7 = pkb.getAllWhileStatementsThatMatch("g", "", true);
    STATEMENT_NUMBER_SET expected7 = {};
    REQUIRE(actual7 == expected7);

    STATEMENT_NUMBER_SET actual8 = pkb.getAllWhileStatementsThatMatch("h", "", true);
    STATEMENT_NUMBER_SET expected8 = {};
    REQUIRE(actual8 == expected8);

    STATEMENT_NUMBER_SET actual9 = pkb.getAllWhileStatementsThatMatch("_", "", true);
    STATEMENT_NUMBER_SET expected9 = { 1, 4, 7 };
    REQUIRE(actual9 == expected9);
}

TEST_CASE("Test getAllIfElseStatementsThatMatch") {
    const char program[] = "procedure a {                   "
                           "  while (a + b == c) {          " // 1
                           "    y = 1;                      " // 2
                           "  }                             "
                           "  if (a + b == d + g) then {    " // 3
                           "    while (a + d == e) {        " // 4
                           "      y = 1;                    " // 5
                           "    }                           "
                           "  } else {                      "
                           "    y = 2;                      " // 6
                           "}                               "
                           "  while (a + b < f) {           " // 7
                           "    if (a + b == h) then {  " // 8
                           "      y = 1;                    " // 9
                           "    } else {                    "
                           "      y = 2;                    " // 10
                           "    }                           "
                           "    if (1 == 1) then {          " // 11
                           "      y = 1;                    " // 12
                           "    } else {                    "
                           "      y = 2;                    " // 13
                           "    }                           "
                           "  }                             "
                           "}";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET actual1 = pkb.getAllIfElseStatementsThatMatch("a", "", true, "", true);
    STATEMENT_NUMBER_SET expected1 = { 3, 8 };
    REQUIRE(actual1 == expected1);

    STATEMENT_NUMBER_SET actual2 = pkb.getAllIfElseStatementsThatMatch("b", "", true, "", true);
    STATEMENT_NUMBER_SET expected2 = { 3, 8 };
    REQUIRE(actual2 == expected2);

    STATEMENT_NUMBER_SET actual3 = pkb.getAllIfElseStatementsThatMatch("c", "", true, "", true);
    STATEMENT_NUMBER_SET expected3 = {};
    REQUIRE(actual3 == expected3);

    STATEMENT_NUMBER_SET actual4 = pkb.getAllIfElseStatementsThatMatch("d", "", true, "", true);
    STATEMENT_NUMBER_SET expected4 = { 3 };
    REQUIRE(actual4 == expected4);

    STATEMENT_NUMBER_SET actual5 = pkb.getAllIfElseStatementsThatMatch("e", "", true, "", true);
    STATEMENT_NUMBER_SET expected5 = {};
    REQUIRE(actual5 == expected5);

    STATEMENT_NUMBER_SET actual6 = pkb.getAllIfElseStatementsThatMatch("f", "", true, "", true);
    STATEMENT_NUMBER_SET expected6 = {};
    REQUIRE(actual6 == expected6);

    STATEMENT_NUMBER_SET actual7 = pkb.getAllIfElseStatementsThatMatch("g", "", true, "", true);
    STATEMENT_NUMBER_SET expected7 = { 3 };
    REQUIRE(actual7 == expected7);

    STATEMENT_NUMBER_SET actual8 = pkb.getAllIfElseStatementsThatMatch("h", "", true, "", true);
    STATEMENT_NUMBER_SET expected8 = { 8 };
    REQUIRE(actual8 == expected8);

    STATEMENT_NUMBER_SET actual9 = pkb.getAllIfElseStatementsThatMatch("_", "", true, "", true);
    STATEMENT_NUMBER_SET expected9 = { 3, 8 };
    REQUIRE(actual9 == expected9);
}

TEST_CASE("Test getAllStatementsWithNext") {
    const char program[] = "procedure a {                   "
                           "  while (a + b == c) {          " // 1
                           "    y = 1;                      " // 2
                           "  }                             "
                           "  if (a + b == d + g) then {    " // 3
                           "    while (a + d == e) {        " // 4
                           "      y = 1;                    " // 5
                           "    }                           "
                           "  } else {                      "
                           "    y = 2;                      " // 6
                           "}                               "
                           "  if (a + b == h) then {        " // 7
                           "    y = 1;                      " // 8
                           "  } else {                      "
                           "    y = 2;                      " // 9
                           "  }                             "
                           "}"
                           "procedure b {"
                           "  a = 2;                        " // 10
                           "  b = 2;                        " // 11
                           "}";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET actual = pkb.getAllStatementsWithNext();
    STATEMENT_NUMBER_SET expected = { 1, 2, 3, 4, 5, 6, 7, 10 };
    REQUIRE(actual == expected);
}

TEST_CASE("Test getAllStatementsWithPrev") {
    const char program[] = "procedure a {                   "
                           "  a = 1;                        " // 1
                           "  while (a + b == c) {          " // 2
                           "    y = 1;                      " // 3
                           "  }                             "
                           "  if (a + b == d + g) then {    " // 4
                           "    while (a + d == e) {        " // 5
                           "      y = 1;                    " // 6
                           "    }                           "
                           "  } else {                      "
                           "    y = 2;                      " // 7
                           "}                               "
                           "  if (a + b == h) then {        " // 8
                           "    y = 1;                      " // 9
                           "  } else {                      "
                           "    y = 2;                      " // 10
                           "  }                             "
                           "}"
                           "procedure b {"
                           "  a = 2;                        " // 11
                           "  b = 2;                        " // 12
                           "}";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET actual = pkb.getAllStatementsWithPrev();
    STATEMENT_NUMBER_SET expected = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 12 };
    REQUIRE(actual == expected);
}

TEST_CASE("Test getStatementsAffectedBy/getStatementsAffectedBy") {
    const char program[] = "procedure Proc { "
                           "while (i < 10) {" // 1
                           "  x = y;" // 2
                           "  y = x;" // 3
                           "  if (unrealted == 1) then {" // 4
                           "    y = y;" // 5
                           "  } else {"
                           "    unrelated = 0;" // 6
                           "  }"
                           "  i = i + 1;" // 7
                           "}"
                           "unrelated = x+y+i;" //  8
                           "}";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    // Non existent
    PROGRAM_LINE_SET expected = {};
    REQUIRE(pkb.getStatementsAffectedBy(1, false) == expected);

    // Non-transitive Affects
    expected = { 3, 8 };
    REQUIRE(pkb.getStatementsAffectedBy(2, false) == expected);
    expected = { 5, 2, 8 };
    REQUIRE(pkb.getStatementsAffectedBy(3, false) == expected);
    expected = { 2, 8 };
    REQUIRE(pkb.getStatementsAffectedBy(5, false) == expected);
    expected = { 7, 8 };
    REQUIRE(pkb.getStatementsAffectedBy(7, false) == expected);
    expected = {};
    REQUIRE(pkb.getStatementsAffectedBy(8, false) == expected);

    // Non-transitive Affected (reflection of the above tests)
    expected = { 3, 5 };
    REQUIRE(pkb.getStatementsThatAffect(2, false) == expected);
    expected = { 2 };
    REQUIRE(pkb.getStatementsThatAffect(3, false) == expected);
    expected = { 3 };
    REQUIRE(pkb.getStatementsThatAffect(5, false) == expected);
    expected = { 7 };
    REQUIRE(pkb.getStatementsThatAffect(7, false) == expected);
    expected = { 2, 3, 5, 7 };
    REQUIRE(pkb.getStatementsThatAffect(8, false) == expected);

    // Transitive
    expected = { 2, 3, 5, 8 };
    REQUIRE(pkb.getStatementsAffectedBy(2, true) == expected);
    REQUIRE(pkb.getStatementsAffectedBy(3, true) == expected);
    REQUIRE(pkb.getStatementsAffectedBy(5, true) == expected);
    expected = { 7, 8 };
    REQUIRE(pkb.getStatementsAffectedBy(7, true) == expected);

    // Transitive Affected (reflection of the above tests)
    expected = { 2, 3, 5 };
    REQUIRE(pkb.getStatementsThatAffect(2, true) == expected);
    REQUIRE(pkb.getStatementsThatAffect(3, true) == expected);
    REQUIRE(pkb.getStatementsThatAffect(5, true) == expected);
    expected = { 2, 3, 5, 7 };
    REQUIRE(pkb.getStatementsThatAffect(8, true) == expected);
    expected = { 7 };
    REQUIRE(pkb.getStatementsThatAffect(7, true) == expected);
    expected = { 2, 3, 5, 7 };
    REQUIRE(pkb.getStatementsThatAffect(8, true) == expected);
}

TEST_CASE("Test getAllStatementsThatAffect/getAllStatementsThatAreAffected") {
    const char program[] = "procedure Proc { "
                           "while (i < 10) {" // 1
                           "  x = y;" // 2
                           "  y = x;" // 3
                           "  if (unrealted == 1) then {" // 4
                           "    y = y;" // 5
                           "  } else {"
                           "    unrelated = 0;" // 6
                           "  }"
                           "  i = i + 1;" // 7
                           "}"
                           "unrelated = x+y+i;" //  8
                           "}";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    PROGRAM_LINE_SET expected = { 2, 3, 5, 7 };
    REQUIRE(pkb.getAllStatementsThatAffect() == expected);
    expected = { 2, 3, 5, 7, 8 };
    REQUIRE(pkb.getAllStatementsThatAreAffected() == expected);
}

TEST_CASE("Test getNextBipStatementOf basic") {
    const char STRUCTURED_STATEMENT[] = "procedure a {         "
                                        "  while (1 == 1) {    " // 1
                                        "    a = 1;            " // 2
                                        "    call b;           " // 3
                                        "  }"
                                        "}"
                                        ""
                                        "procedure b {         "
                                        "  a = 1;              " // 4
                                        "  if (a == 1) then {  " // 5
                                        "    b = 1;            " // 6
                                        "  } else {            "
                                        "    b = 1;            " // 7
                                        "  }                   "
                                        "}";

    Parser parser = testhelpers::GenerateParserFromTokens(STRUCTURED_STATEMENT);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected = { 2 };
    REQUIRE(pkb.getNextBipStatementOf(1, false) == expected);
    expected = { 3 };
    REQUIRE(pkb.getNextBipStatementOf(2, false) == expected);
    expected = { 4 };
    REQUIRE(pkb.getNextBipStatementOf(3, false) == expected);
    expected = { 5 };
    REQUIRE(pkb.getNextBipStatementOf(4, false) == expected);
    expected = { 6, 7 };
    REQUIRE(pkb.getNextBipStatementOf(5, false) == expected);
    expected = { 1 };
    REQUIRE(pkb.getNextBipStatementOf(6, false) == expected);
    REQUIRE(pkb.getNextBipStatementOf(7, false) == expected);

    expected = { 1, 2, 3, 4, 5, 6, 7 };
    REQUIRE(pkb.getNextBipStatementOf(1, true) == expected);
    REQUIRE(pkb.getNextBipStatementOf(2, true) == expected);
    REQUIRE(pkb.getNextBipStatementOf(3, true) == expected);
    REQUIRE(pkb.getNextBipStatementOf(4, true) == expected);
    REQUIRE(pkb.getNextBipStatementOf(5, true) == expected);
    REQUIRE(pkb.getNextBipStatementOf(6, true) == expected);
    REQUIRE(pkb.getNextBipStatementOf(7, true) == expected);
}

TEST_CASE("Test getNextBipRelationship procedure with different scopes are traversed") {
    const char program[] = "procedure A { call B; x = 1; call C; y = 1; } " // 1 2 3 4
                           "procedure B { call C; }" // 5
                           "procedure C { b = 1; }" // 6
    ;
    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);
    REQUIRE(pkb.getNextBipStatementOf(5, true).count(4));
}

TEST_CASE("Test getNextBipRelationship Gandilf regression") {
    const char program[] = "procedure Gandalf {"
                           "read shine;" // 01
                           "ring = shine * 2;" // 02
                           "f = ring + 3;" // 03
                           "read lost;" // 04
                           "call Frodo;" // 05
                           "if (lost == shine) then {" // 06
                           "call Aragon;" // 07
                           "d = (shine + 12) * (lost + 13);}" // 8
                           "else {"
                           "while (f > 10) {" // 9
                           "read g;" // 10
                           "f = ring + 4 + g * a * 2 * 1 + shine;" // 11
                           "call Frodo;}" // 12
                           "d = 2;}" // 13
                           "print d;}" // 14

                           "procedure Frodo {"
                           "a = f + shine;" // 15
                           "if (a < 2) then {" // 16
                           "call Aragon;}" // 17
                           "else{"
                           "f = f + 19;}}" // 18
                           // -4

                           "procedure Aragon {"
                           "while (shine < 3) {" // 19
                           "a = 1 * 2 + lost - f;" // 20
                           "while (lost == a) {" // 21
                           "lost = lost - 1;" // 22
                           "w = a + lost;" // 23
                           "shine = 1 + ring;}" // 24
                           "if (a != shine) then {" // 25
                           "a = lost + 1;" // 26
                           "w = shine * 2;}" // 27
                           "else {"
                           "w = 1;}}}" // 28
                           // -3

                           "procedure Shire {"
                           "call Aragon;"
                           "Frodo = Frodo + 2;"
                           "if (gold == 8) then {"
                           "read elves;"
                           "print elves;"
                           "} else {"
                           "call Elves;"
                           "}"
                           "}"

                           "procedure Elves {"
                           "while (Shire != 3) {"
                           "while (1 < 0) {"
                           "print TRUE;"
                           "}"
                           "}"
                           "lost = lost + 1;"
                           "}";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected;
    expected = { 2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };
    REQUIRE(pkb.getNextBipStatementOf(1, true) == expected);
    expected = { 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };
    REQUIRE(pkb.getNextBipStatementOf(10, true) == expected);
    expected = { 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };
    REQUIRE(pkb.getNextBipStatementOf(11, true) == expected);
    expected = { 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };
    REQUIRE(pkb.getNextBipStatementOf(12, true) == expected);
    expected = { 14 };
    REQUIRE(pkb.getNextBipStatementOf(13, true) == expected);
    expected = { 6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
                 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };
    REQUIRE(pkb.getNextBipStatementOf(15, true) == expected);
    expected = { 6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
                 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };
    REQUIRE(pkb.getNextBipStatementOf(16, true) == expected);
    expected = { 6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
                 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };
    REQUIRE(pkb.getNextBipStatementOf(17, true) == expected);
    expected = { 6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
                 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };
    REQUIRE(pkb.getNextBipStatementOf(18, true) == expected);
    expected = { 6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                 22, 23, 24, 25, 26, 27, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38 };
    REQUIRE(pkb.getNextBipStatementOf(19, true) == expected);
    expected = { 3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };
    REQUIRE(pkb.getNextBipStatementOf(2, true) == expected);
    expected = { 6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                 22, 23, 24, 25, 26, 27, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38 };
    REQUIRE(pkb.getNextBipStatementOf(20, true) == expected);
    expected = { 6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                 22, 23, 24, 25, 26, 27, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38 };
    REQUIRE(pkb.getNextBipStatementOf(21, true) == expected);
    expected = { 6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                 22, 23, 24, 25, 26, 27, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38 };
    REQUIRE(pkb.getNextBipStatementOf(22, true) == expected);
    expected = { 6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                 22, 23, 24, 25, 26, 27, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38 };
    REQUIRE(pkb.getNextBipStatementOf(23, true) == expected);
    expected = { 6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                 22, 23, 24, 25, 26, 27, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38 };
    REQUIRE(pkb.getNextBipStatementOf(24, true) == expected);
    expected = { 6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                 22, 23, 24, 25, 26, 27, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38 };
    REQUIRE(pkb.getNextBipStatementOf(25, true) == expected);
    expected = { 6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                 22, 23, 24, 25, 26, 27, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38 };
    REQUIRE(pkb.getNextBipStatementOf(26, true) == expected);
    expected = { 6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                 22, 23, 24, 25, 26, 27, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38 };
    REQUIRE(pkb.getNextBipStatementOf(27, true) == expected);
    expected = { 6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                 22, 23, 24, 25, 26, 27, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38 };
    REQUIRE(pkb.getNextBipStatementOf(28, true) == expected);
    expected = { 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38 };
    REQUIRE(pkb.getNextBipStatementOf(29, true) == expected);
    expected = { 4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
                 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };
    REQUIRE(pkb.getNextBipStatementOf(3, true) == expected);
    expected = { 32, 33, 34, 35, 36, 37, 38, 31 };
    REQUIRE(pkb.getNextBipStatementOf(30, true) == expected);
    expected = { 32, 33, 34, 35, 36, 37, 38 };
    REQUIRE(pkb.getNextBipStatementOf(31, true) == expected);
    expected = { 33 };
    REQUIRE(pkb.getNextBipStatementOf(32, true) == expected);
    expected = { 35, 36, 37, 38 };
    REQUIRE(pkb.getNextBipStatementOf(34, true) == expected);
    expected = { 35, 36, 37, 38 };
    REQUIRE(pkb.getNextBipStatementOf(35, true) == expected);
    expected = { 35, 36, 37, 38 };
    REQUIRE(pkb.getNextBipStatementOf(36, true) == expected);
    expected = { 35, 36, 37, 38 };
    REQUIRE(pkb.getNextBipStatementOf(37, true) == expected);
    expected = { 5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
                 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };
    REQUIRE(pkb.getNextBipStatementOf(4, true) == expected);
    expected = { 6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
                 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };
    REQUIRE(pkb.getNextBipStatementOf(5, true) == expected);
    expected = { 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };
    REQUIRE(pkb.getNextBipStatementOf(6, true) == expected);
    expected = { 8, 14, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };
    REQUIRE(pkb.getNextBipStatementOf(7, true) == expected);
    expected = { 14 };
    REQUIRE(pkb.getNextBipStatementOf(8, true) == expected);
    expected = { 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };
    REQUIRE(pkb.getNextBipStatementOf(9, true) == expected);
}

TEST_CASE("Test getNextBipStatementOf branching") {
    const char program[] = "procedure left {      "
                           "  y = 1;              " // 1
                           "  call middle;        " // 2
                           "  left = end;         " // 3
                           "}"
                           "procedure middle {    "
                           "  x = 1;              " // 4
                           "}                     "
                           "procedure right {     "
                           "  call middle;        " // 5
                           "  right = end;        " // 6
                           "}                     ";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    // Non-transitive
    STATEMENT_NUMBER_SET expected = { 2 };
    REQUIRE(pkb.getNextBipStatementOf(1, false) == expected);
    expected = { 4 };
    REQUIRE(pkb.getNextBipStatementOf(2, false) == expected);
    expected = {};
    REQUIRE(pkb.getNextBipStatementOf(3, false) == expected);

    expected = { 3, 6 };
    REQUIRE(pkb.getNextBipStatementOf(4, false) == expected);

    expected = { 4 };
    REQUIRE(pkb.getNextBipStatementOf(5, false) == expected);
    expected = {};
    REQUIRE(pkb.getNextBipStatementOf(6, false) == expected);

    // Transitive
    expected = { 2, 4, 3 };
    REQUIRE(pkb.getNextBipStatementOf(1, true) == expected);
    expected = { 3, 6 };
    REQUIRE(pkb.getNextBipStatementOf(4, true) == expected);
    expected = { 4, 6 };
    REQUIRE(pkb.getNextBipStatementOf(5, true) == expected);
}

TEST_CASE("Test getNextBipStatementOf nesting") {
    const char program[] = "procedure top {       "
                           "  y = 1;              " // 1
                           "  call middle;        " // 2
                           "  left = end;         " // 3
                           "}"
                           "procedure middle {    "
                           "  call bottom;        " // 4
                           "}                     "
                           "procedure bottom {    "
                           "  left = end;         " // 5
                           "}                     ";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);


    STATEMENT_NUMBER_SET expected = {};

    expected = { 2, 4, 5, 3 };
    REQUIRE(pkb.getNextBipStatementOf(1, true) == expected);

    expected = { 4, 5, 3 };
    REQUIRE(pkb.getNextBipStatementOf(2, true) == expected);

    expected = {};
    REQUIRE(pkb.getNextBipStatementOf(3, false) == expected);
    REQUIRE(pkb.getNextBipStatementOf(3, true) == expected);

    expected = { 5 };
    REQUIRE(pkb.getNextBipStatementOf(4, false) == expected);
    expected = { 5, 3 };
    REQUIRE(pkb.getNextBipStatementOf(4, true) == expected);

    expected = { 3 };
    REQUIRE(pkb.getNextBipStatementOf(5, false) == expected);
    REQUIRE(pkb.getNextBipStatementOf(5, true) == expected);
}

TEST_CASE("Test getPreviousBipStatementOf") {
    const char program[] = "procedure left {      "
                           "  y = 1;              " // 1
                           "  call middle;        " // 2
                           "  left = end;         " // 3
                           "}"
                           "procedure middle {    "
                           "  x = 1;              " // 4
                           "}                     "
                           "procedure right {     "
                           "  call middle;        " // 5
                           "  right = end;        " // 6
                           "}                     ";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);


    STATEMENT_NUMBER_SET expected = {};

    expected = {};
    REQUIRE(pkb.getPreviousBipStatementOf(1, false) == expected);
    REQUIRE(pkb.getPreviousBipStatementOf(1, true) == expected);

    expected = { 1 };
    REQUIRE(pkb.getPreviousBipStatementOf(2, false) == expected);
    REQUIRE(pkb.getPreviousBipStatementOf(2, true) == expected);

    expected = { 4 };
    REQUIRE(pkb.getPreviousBipStatementOf(3, false) == expected);
    expected = { 1, 2, 4 };
    REQUIRE(pkb.getPreviousBipStatementOf(3, true) == expected);

    expected = { 2, 5 };
    REQUIRE(pkb.getPreviousBipStatementOf(4, false) == expected);
    expected = { 1, 2, 5 };
    REQUIRE(pkb.getPreviousBipStatementOf(4, true) == expected);

    expected = {};
    REQUIRE(pkb.getPreviousBipStatementOf(5, false) == expected);
    REQUIRE(pkb.getPreviousBipStatementOf(5, true) == expected);

    expected = { 4 };
    REQUIRE(pkb.getPreviousBipStatementOf(6, false) == expected);
    expected = { 5, 4 };
    REQUIRE(pkb.getPreviousBipStatementOf(6, true) == expected);
}

TEST_CASE("Test getAllStatementsWithNextBip and getAllStatementsWithPreviousBip") {
    const char program[] = "procedure left {      "
                           "  y = 1;              " // 1
                           "  call middle;        " // 2
                           "  left = end;         " // 3
                           "}"
                           "procedure middle {    "
                           "  x = 1;              " // 4
                           "}                     "
                           "procedure right {     "
                           "  call middle;        " // 5
                           "  right = end;        " // 6
                           "}                     ";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected = { 1, 2, 4, 5 };
    REQUIRE(pkb.getAllStatementsWithNextBip() == expected);

    expected = { 2, 3, 4, 6 };
    REQUIRE(pkb.getAllStatementsWithPreviousBip() == expected);
}

TEST_CASE("Test getStatementsAffectedBipBy/getStatementsAffectedBipBy") {
    const char program[] = "procedure Proc { "
                           "x = 1;" // 1
                           "while (i < 10) {" // 2
                           "  call AnotherProc;" // 3
                           "}"
                           "unrelated = x+y;" //  4
                           "}"
                           "procedure AnotherProc {"
                           "y = x;" // 5
                           "}";
    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected = { 1, 5 };
    REQUIRE(pkb.getAllStatementsThatAffectBip() == expected);
    expected = { 4, 5 };
    REQUIRE(pkb.getAllStatementsThatAreAffectedBip() == expected);
}


TEST_CASE("Test getStatementsAffectedBipBy/getStatementsThatAffectBip") {
    const char program[] = "procedure A { "
                           "x = 1;" // 1
                           "if (zz == 1) then {" // 2
                           "  call B;" // 3
                           "} else {"
                           "  call C;" // 4
                           "}"
                           "sink_one = x + y + z;" // 5
                           "}"
                           ""
                           "procedure B {"
                           "while (yy == 1) {" // 6
                           "  y = y + x;" // 7
                           "  x = z + 1;" // 8
                           "  call C;" // 9
                           "}"
                           "}"
                           ""
                           "procedure C {"
                           "z = y + x;" // 10
                           "}";
    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected = { 7, 10, 5 };
    REQUIRE(pkb.getStatementsAffectedBipBy(1, false) == expected);
    expected = { 7, 8, 10, 5 }; // We affect 8 because call b -> call c -> while loop into 8
    REQUIRE(pkb.getStatementsAffectedBipBy(1, true) == expected);

    expected = {};
    REQUIRE(pkb.getStatementsAffectedBipBy(5, false) == expected);
    REQUIRE(pkb.getStatementsAffectedBipBy(5, true) == expected);

    expected = { 7, 10, 5 };
    REQUIRE(pkb.getStatementsAffectedBipBy(7, false) == expected);
    expected = { 7, 10, 5, 8 };
    REQUIRE(pkb.getStatementsAffectedBipBy(7, true) == expected);

    expected = { 7, 10, 5 };
    REQUIRE(pkb.getStatementsAffectedBipBy(8, false) == expected);
    expected = { 7, 10, 5, 8 };
    REQUIRE(pkb.getStatementsAffectedBipBy(8, true) == expected);

    expected = { 5, 8 };
    REQUIRE(pkb.getStatementsAffectedBipBy(10, false) == expected);
    expected = { 7, 10, 5, 8 };
    REQUIRE(pkb.getStatementsAffectedBipBy(10, true) == expected);


    // Inverse
    expected = { 1, 7, 8, 10 };
    REQUIRE(pkb.getStatementsThatAffectBip(5, false) == expected);
    expected = { 1, 7, 8 };
    REQUIRE(pkb.getStatementsThatAffectBip(7, false) == expected);
    expected = { 10 };
    REQUIRE(pkb.getStatementsThatAffectBip(8, false) == expected);
    expected = { 1, 7, 8 };
    REQUIRE(pkb.getStatementsThatAffectBip(10, false) == expected);

    expected = { 1, 7, 8, 10 };
    REQUIRE(pkb.getStatementsThatAffectBip(5, true) == expected);
    expected = { 1, 7, 8, 10 };
    REQUIRE(pkb.getStatementsThatAffectBip(7, true) == expected);
    expected = { 1, 7, 8, 10 };
    REQUIRE(pkb.getStatementsThatAffectBip(8, true) == expected);
    expected = { 1, 7, 8, 10 };
    REQUIRE(pkb.getStatementsThatAffectBip(10, true) == expected);
}

TEST_CASE("Test getStatementsAffectedBipBy the case in the wiki") {
    const char program[] = "procedure B { "
                           " call C;" // 1
                           " call C;" // 2
                           " call C;" // 3
                           "}"
                           "procedure C {"
                           "d = a;" // 4
                           "a = b;" // 5
                           "b = c;" // 6
                           "c = d;" // 7
                           "}";

    Parser parser = testhelpers::GenerateParserFromTokens(program);
    TNode ast(parser.parse());
    PKBImplementation pkb(ast);

    STATEMENT_NUMBER_SET expected = {};
    REQUIRE(pkb.getStatementsAffectedBipBy(1, false) == expected);

    expected = { 6 };
    REQUIRE(pkb.getStatementsAffectedBipBy(7, false) == expected);
    expected = { 5 };
    REQUIRE(pkb.getStatementsAffectedBipBy(6, false) == expected);
    expected = { 4 };
    REQUIRE(pkb.getStatementsAffectedBipBy(5, false) == expected);

    expected = { 5, 6 };
    REQUIRE(pkb.getStatementsAffectedBipBy(7, true) == expected);
    expected = { 5, 4, 7 };
    REQUIRE(pkb.getStatementsAffectedBipBy(6, true) == expected);
    expected = { 4, 7, 6 };
    REQUIRE(pkb.getStatementsAffectedBipBy(5, true) == expected);
    expected = { 7, 6, 5 };
    REQUIRE(pkb.getStatementsAffectedBipBy(4, true) == expected);
}

} // namespace testpkb
} // namespace backend
