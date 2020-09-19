#include "PKBImplementation.h"

#include "DesignExtractor.h"
#include "Logger.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace backend {

PKBImplementation::PKBImplementation(const TNode& ast) {
    logWord("PKB starting with ast");
    logLine(ast.toString());
    auto tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    auto statementNumberToTNode = extractor::getStatementNumberToTNode(tNodeToStatementNumber);

    std::tie(followFollowedRelation, followedFollowRelation) = extractor::getFollowRelationship(ast);
    allStatementsThatFollows = extractor::getKeysInMap<>(followFollowedRelation);
    allStatementsThatAreFollowed = extractor::getKeysInMap<>(followedFollowRelation);

    std::tie(childrenParentRelation, parentChildrenRelation) = extractor::getParentRelationship(ast);
    allStatementsThatHaveAncestors = extractor::getKeysInMap<>(childrenParentRelation);
    allStatementsThatHaveDescendants = extractor::getKeysInMap<>(parentChildrenRelation);
}

STATEMENT_NUMBER_LIST PKBImplementation::getAllStatements() const {
    return STATEMENT_NUMBER_LIST();
}

VARIABLE_LIST PKBImplementation::getAllVariables() const {
    return VARIABLE_LIST();
}

PROCEDURE_LIST PKBImplementation::getAllProcedures() const {
    return PROCEDURE_LIST();
}

/** -------------------------- FOLLOWS ---------------------------- **/
// TODO(weineng) optimize in the future.
STATEMENT_NUMBER_LIST PKBImplementation::getStatementsThatFollows(STATEMENT_NUMBER s) const {
    return extractor::getVisitedPathFromStart(s, followedFollowRelation);
}

// TODO(weineng) optimize in the future.
STATEMENT_NUMBER_LIST PKBImplementation::getStatementsFollowedBy(STATEMENT_NUMBER s) const {
    return extractor::getVisitedPathFromStart(s, followFollowedRelation);
}

STATEMENT_NUMBER_LIST PKBImplementation::getAllStatementsThatFollows() const {
    return allStatementsThatFollows;
}

STATEMENT_NUMBER_LIST PKBImplementation::getAllStatementsThatAreFollowed() const {
    return allStatementsThatAreFollowed;
}

/** -------------------------- PARENTS ---------------------------- **/
STATEMENT_NUMBER_LIST PKBImplementation::getAncestors(STATEMENT_NUMBER s) const {
    return extractor::getVisitedPathFromStart(s, childrenParentRelation);
}

STATEMENT_NUMBER_LIST PKBImplementation::getStatementsThatHaveAncestors() const {
    return allStatementsThatHaveAncestors;
}

STATEMENT_NUMBER_LIST PKBImplementation::getDescendants(STATEMENT_NUMBER statementNumber) const {
    std::unordered_set<int> visited;
    std::vector<int> toVisit = { statementNumber };
    while (toVisit.size() > 0) {
        int visiting = toVisit.back();
        toVisit.pop_back();
        if (visited.find(visiting) != visited.end()) {
            continue;
        }
        visited.insert(visiting);
        auto it = parentChildrenRelation.find(visiting);
        if (it == parentChildrenRelation.end()) {
            continue;
        }
        toVisit.insert(toVisit.end(), it->second.begin(), it->second.end());
    }
    visited.erase(statementNumber);
    return std::vector<int>(visited.begin(), visited.end());
}

STATEMENT_NUMBER_LIST PKBImplementation::getStatementsThatHaveDescendants() const {
    return allStatementsThatHaveDescendants;
}

} // namespace backend
