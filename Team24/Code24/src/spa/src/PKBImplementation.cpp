#include "PKBImplementation.h"

#include "DesignExtractor.h"
#include "Foost.hpp"
#include "Logger.h"
#include "PKB.h"
#include "Parser.h"
#include "TNode.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace backend {

PKBImplementation::PKBImplementation(const TNode& ast) {
    logWord("PKB starting with ast");
    logLine(ast.toString());

    if (!extractor::isValidSimpleProgram(ast)) {
        throw std::runtime_error("Provided AST does not represent a valid SIMPLE program");
    }


    std::unordered_map<const TNode*, int> tNodeToStatementNumber = extractor::getTNodeToStatementNumber(ast);
    statementNumberToTNode = extractor::getStatementNumberToTNode(tNodeToStatementNumber);
    tNodeTypeToTNodesMap = extractor::getTNodeTypeToTNodes(ast);
    statementNumberToTNodeType = extractor::getStatementNumberToTNodeTypeMap(statementNumberToTNode);

    for (auto i : statementNumberToTNode) {
        allStatementsNumber.insert(i.first);
    }

    // Get mapping of all procedures that calls (procedureToCalledProcedures) and
    // are called by procedureToCallers) procedure:
    for (const auto& p : extractor::getProcedureToCallees(tNodeTypeToTNodesMap)) {
        PROCEDURE_NAME callerName = p.first->name;

        for (auto calledProcedure : p.second) {
            PROCEDURE_NAME calleeName = calledProcedure->name;

            // Update Call Mapping
            procedureToCalledProcedures[callerName].insert(calleeName);
            procedureToCallers[calleeName].insert(callerName);

            // Register the fact that callee was called by some procedure
            allProceduresThatCall.insert(callerName);
            allCalledProcedures.insert(calleeName);
        }
    }

    // Get all constants name:
    for (auto i : tNodeTypeToTNodesMap[Constant]) {
        allConstantsName.insert(i->constant);
    }

    // Get all variables name:
    std::unordered_set<std::string> varNames;
    for (auto i : tNodeTypeToTNodesMap[Variable]) {
        if (!i->isProcedureVar) {
            varNames.insert(i->name);
        }
    }
    allVariablesName = { varNames.begin(), varNames.end() };

    // Get all procedure name:
    std::unordered_set<std::string> procedureNames;
    for (auto i : tNodeTypeToTNodesMap[Procedure]) {
        procedureNames.insert(i->name);
    }
    allProceduresName = { procedureNames.begin(), procedureNames.end() };

    // Get all assignment statements:
    for (auto i : tNodeTypeToTNodesMap[Assign]) {
        allAssignmentStatements.insert(tNodeToStatementNumber[i]);
    }

    // Get all while statements:
    for (auto i : tNodeTypeToTNodesMap[While]) {
        allWhileStatements.insert(tNodeToStatementNumber[i]);
    }

    // Get all if statements:
    for (auto i : tNodeTypeToTNodesMap[IfElse]) {
        allIfElseStatements.insert(tNodeToStatementNumber[i]);
    }

    // Attribute-based retrieval
    for (auto tNode : tNodeTypeToTNodesMap[Call]) {
        PROCEDURE_NAME calledProcedureName = tNode->children.front().name;
        STATEMENT_NUMBER statementNumber = tNodeToStatementNumber[tNode];
        procedureNameToCallStatements[calledProcedureName].insert(statementNumber);
    }

    for (auto tNode : tNodeTypeToTNodesMap[Read]) {
        VARIABLE_NAME variableName = tNode->children.front().name;
        STATEMENT_NUMBER statementNumber = tNodeToStatementNumber[tNode];
        variableNameToReadStatements[variableName].insert(statementNumber);
    }

    for (auto tNode : tNodeTypeToTNodesMap[Print]) {
        VARIABLE_NAME variableName = tNode->children.front().name;
        STATEMENT_NUMBER statementNumber = tNodeToStatementNumber[tNode];
        variableNameToPrintStatements[variableName].insert(statementNumber);
    }

    // Follow
    std::tie(followFollowedRelation, followedFollowRelation) = extractor::getFollowRelationship(ast);
    allStatementsThatFollows = extractor::getKeysInMap(followFollowedRelation);
    allStatementsThatAreFollowed = extractor::getKeysInMap(followedFollowRelation);

    // Parent
    std::tie(childrenParentRelation, parentChildrenRelation) = extractor::getParentRelationship(ast);
    allStatementsThatHaveAncestors = extractor::getKeysInMap(childrenParentRelation);
    allStatementsThatHaveDescendants = extractor::getKeysInMap(parentChildrenRelation);

    // next
    nextRelationship = extractor::getNextRelationship(tNodeTypeToTNodesMap, tNodeToStatementNumber);
    for (const auto& pair : nextRelationship) {
        statementsWithNext.insert(pair.first);
    }
    previousRelationship = extractor::getPreviousRelationship(nextRelationship);
    for (const auto& pair : previousRelationship) {
        statementsWithPrev.insert(pair.first);
    }

    // Pattern
    patternsMap = extractor::getPatternsMap(tNodeTypeToTNodesMap[Assign], tNodeToStatementNumber);
    conditionVariablesToStatementNumbers =
    extractor::getConditionVariablesToStatementNumbers(statementNumberToTNode);

    // Uses
    std::unordered_map<const TNode*, std::unordered_set<std::string>> usesMapping =
    extractor::getUsesMapping(tNodeTypeToTNodesMap);

    for (auto& p : usesMapping) {
        const TNode* tNode = p.first;
        std::unordered_set<VARIABLE_NAME> usedVariables = p.second;
        // For Statements
        if (tNode->isStatementNode()) {
            STATEMENT_NUMBER statementNumber = tNodeToStatementNumber[tNode];
            // Update statement -> variable
            allStatementsThatUseSomeVariable.insert(statementNumber);
            statementToUsedVariables[statementNumber] =
            VARIABLE_NAME_SET(usedVariables.begin(), usedVariables.end());

            // Update variable -> statement
            for (const VARIABLE_NAME& variable : usedVariables) {
                variableToStatementsThatUseIt[variable].insert(statementNumber);
                allVariablesUsedBySomeStatement.insert(variable);
            }
            // For Procedures
        } else if (tNode->type == TNodeType::Procedure) {
            PROCEDURE_NAME procedureName = tNode->name;
            // Update proc -> variable
            allProceduresThatThatUseSomeVariable.insert(procedureName);
            procedureToUsedVariables[procedureName] =
            PROCEDURE_NAME_SET(usedVariables.begin(), usedVariables.end());
            // Update variable -> proc
            for (const VARIABLE_NAME& variable : usedVariables) {
                variableToProceduresThatUseIt[variable].insert(procedureName);
                allVariablesUsedBySomeProcedure.insert(variable);
            }
        } else {
            throw std::runtime_error("Found a TNode " + tNode->toShortString() +
                                     " that should not be Use-ing any variable");
        }
    }

    // Modifies
    std::unordered_map<const TNode*, std::unordered_set<std::string>> modifiesMapping =
    extractor::getModifiesMapping(tNodeTypeToTNodesMap);
    for (auto& p : modifiesMapping) {
        const TNode* tNode = p.first;
        std::unordered_set<VARIABLE_NAME> modifiedVariables = p.second;
        // For Statements
        if (tNode->isStatementNode()) {
            STATEMENT_NUMBER statementNumber = tNodeToStatementNumber[tNode];
            // Update statement -> variable
            allStatementsThatModifySomeVariable.insert(statementNumber);
            statementToModifiedVariables[statementNumber] =
            VARIABLE_NAME_SET(modifiedVariables.begin(), modifiedVariables.end());

            // Update variable -> statement
            for (const VARIABLE_NAME& variable : modifiedVariables) {
                variableToStatementsThatModifyIt[variable].insert(statementNumber);
                allVariablesModifiedBySomeStatement.insert(variable);
            }
            // For Procedures
        } else if (tNode->type == TNodeType::Procedure) {
            PROCEDURE_NAME procedureName = tNode->name;
            // Update proc -> variable
            allProceduresThatThatModifySomeVariable.insert(procedureName);
            procedureToModifiedVariables[procedureName] =
            PROCEDURE_NAME_SET(modifiedVariables.begin(), modifiedVariables.end());
            // Update variable -> proc
            for (const VARIABLE_NAME& variable : modifiedVariables) {
                variableToProceduresThatModifyIt[variable].insert(procedureName);
                allVariablesModifiedBySomeProcedure.insert(variable);
            }
        } else {
            throw std::runtime_error("Found a TNode " + tNode->toShortString() +
                                     " that should not be Modify-ing any variable");
        }
    }
}

const STATEMENT_NUMBER_SET& PKBImplementation::getAllStatements() const {
    return allStatementsNumber;
}

const VARIABLE_NAME_LIST& PKBImplementation::getAllVariables() const {
    return allVariablesName;
}

const PROCEDURE_NAME_LIST& PKBImplementation::getAllProcedures() const {
    return allProceduresName;
}

const CONSTANT_NAME_SET& PKBImplementation::getAllConstants() const {
    return allConstantsName;
}


/** -------------------------- ATTRIBUTE-BASED RETRIEVAL ---------------------------- **/
const STATEMENT_NUMBER_SET PKBImplementation::getCallStatementsWithProcedureName(PROCEDURE_NAME procedureName) const {
    if (procedureNameToCallStatements.find(procedureName) == procedureNameToCallStatements.end()) {
        return STATEMENT_NUMBER_SET();
    }
    return procedureNameToCallStatements.at(procedureName);
}

const PROCEDURE_NAME
PKBImplementation::getProcedureNameFromCallStatement(STATEMENT_NUMBER callStatementNumber) const {
    if (statementNumberToTNode.find(callStatementNumber) == statementNumberToTNode.end()) {
        return VARIABLE_NAME();
    }

    if (statementNumberToTNodeType.at(callStatementNumber) != Call) {
        return VARIABLE_NAME();
    }

    return statementNumberToTNode.at(callStatementNumber)->children.front().name;
}

const STATEMENT_NUMBER_SET PKBImplementation::getReadStatementsWithVariableName(VARIABLE_NAME variableName) const {
    if (variableNameToReadStatements.find(variableName) == variableNameToReadStatements.end()) {
        return STATEMENT_NUMBER_SET();
    }
    return variableNameToReadStatements.at(variableName);
}

const VARIABLE_NAME PKBImplementation::getVariableNameFromReadStatement(STATEMENT_NUMBER readStatementNumber) const {
    if (statementNumberToTNode.find(readStatementNumber) == statementNumberToTNode.end()) {
        return VARIABLE_NAME();
    }

    if (statementNumberToTNodeType.at(readStatementNumber) != Read) {
        return VARIABLE_NAME();
    }

    return statementNumberToTNode.at(readStatementNumber)->children.front().name;
}

const STATEMENT_NUMBER_SET PKBImplementation::getPrintStatementsWithVariableName(VARIABLE_NAME variableName) const {
    if (variableNameToPrintStatements.find(variableName) == variableNameToPrintStatements.end()) {
        return STATEMENT_NUMBER_SET();
    }
    return variableNameToPrintStatements.at(variableName);
}

const VARIABLE_NAME
PKBImplementation::getVariableNameFromPrintStatement(STATEMENT_NUMBER printStatementNumber) const {
    if (statementNumberToTNode.find(printStatementNumber) == statementNumberToTNode.end()) {
        return VARIABLE_NAME();
    }

    if (statementNumberToTNodeType.at(printStatementNumber) != Print) {
        return VARIABLE_NAME();
    }

    return statementNumberToTNode.at(printStatementNumber)->children.front().name;
}

/** -------------------------- FOLLOWS ---------------------------- **/

STATEMENT_NUMBER_SET PKBImplementation::getDirectFollow(STATEMENT_NUMBER s) const {
    auto it = followedFollowRelation.find(s);
    if (it == followedFollowRelation.end()) {
        return {};
    }
    return { it->second };
}

STATEMENT_NUMBER_SET PKBImplementation::getDirectFollowedBy(STATEMENT_NUMBER s) const {
    auto it = followFollowedRelation.find(s);
    if (it == followFollowedRelation.end()) {
        return {};
    }
    return { it->second };
}

// TODO(weineng) optimize in the future.
STATEMENT_NUMBER_SET PKBImplementation::getStatementsThatFollows(STATEMENT_NUMBER s) const {
    return extractor::getVisitedPathFromStart(s, followedFollowRelation);
}

// TODO(weineng) optimize in the future.
STATEMENT_NUMBER_SET PKBImplementation::getStatementsFollowedBy(STATEMENT_NUMBER s) const {
    return extractor::getVisitedPathFromStart(s, followFollowedRelation);
}

STATEMENT_NUMBER_SET PKBImplementation::getAllStatementsThatFollows() const {
    return allStatementsThatFollows;
}

STATEMENT_NUMBER_SET PKBImplementation::getAllStatementsThatAreFollowed() const {
    return allStatementsThatAreFollowed;
}

/** -------------------------- PARENTS ---------------------------- **/

STATEMENT_NUMBER_SET PKBImplementation::getParent(STATEMENT_NUMBER statementNumber) const {
    auto it = childrenParentRelation.find(statementNumber);
    if (it == childrenParentRelation.end()) {
        return {};
    }
    return { it->second };
}

STATEMENT_NUMBER_SET PKBImplementation::getChildren(STATEMENT_NUMBER statementNumber) const {
    auto it = parentChildrenRelation.find(statementNumber);
    if (it == parentChildrenRelation.end()) {
        return {};
    }
    return it->second;
}

STATEMENT_NUMBER_SET PKBImplementation::getAncestors(STATEMENT_NUMBER s) const {
    return extractor::getVisitedPathFromStart(s, childrenParentRelation);
}

STATEMENT_NUMBER_SET PKBImplementation::getStatementsThatHaveAncestors() const {
    return allStatementsThatHaveAncestors;
}

STATEMENT_NUMBER_SET PKBImplementation::getDescendants(STATEMENT_NUMBER statementNumber) const {
    std::unordered_set<int> visited;
    std::vector<int> toVisit = { statementNumber };
    while (!toVisit.empty()) {
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
    return STATEMENT_NUMBER_SET(visited.begin(), visited.end());
}

STATEMENT_NUMBER_SET PKBImplementation::getStatementsThatHaveDescendants() const {
    return allStatementsThatHaveDescendants;
}

/** -------------------------- USES ---------------------------- **/
STATEMENT_NUMBER_SET PKBImplementation::getStatementsThatUse(VARIABLE_NAME v) const {
    auto it = variableToStatementsThatUseIt.find(v);
    if (it == variableToStatementsThatUseIt.end()) {
        return STATEMENT_NUMBER_SET();
    } else {
        return STATEMENT_NUMBER_SET(it->second.begin(), it->second.end());
    }
}
STATEMENT_NUMBER_SET PKBImplementation::getStatementsThatUseSomeVariable() const {
    return STATEMENT_NUMBER_SET(allStatementsThatUseSomeVariable.begin(),
                                allStatementsThatUseSomeVariable.end());
}
PROCEDURE_NAME_LIST PKBImplementation::getProceduresThatUse(VARIABLE_NAME v) const {
    auto it = variableToProceduresThatUseIt.find(v);
    if (it == variableToProceduresThatUseIt.end()) {
        return PROCEDURE_NAME_LIST();
    } else {
        return PROCEDURE_NAME_LIST(it->second.begin(), it->second.end());
    }
}
PROCEDURE_NAME_LIST PKBImplementation::getProceduresThatUseSomeVariable() const {
    return PROCEDURE_NAME_LIST(allProceduresThatThatUseSomeVariable.begin(),
                               allProceduresThatThatUseSomeVariable.end());
}
VARIABLE_NAME_LIST PKBImplementation::getVariablesUsedIn(PROCEDURE_NAME p) const {
    auto it = procedureToUsedVariables.find(p);
    if (it == procedureToUsedVariables.end()) {
        return VARIABLE_NAME_LIST();
    } else {
        return VARIABLE_NAME_LIST(it->second.begin(), it->second.end());
    }
}
VARIABLE_NAME_LIST PKBImplementation::getVariablesUsedBySomeProcedure() const {
    return VARIABLE_NAME_LIST(allVariablesUsedBySomeProcedure.begin(),
                              allVariablesUsedBySomeProcedure.end());
}
VARIABLE_NAME_LIST PKBImplementation::getVariablesUsedIn(STATEMENT_NUMBER s) const {
    auto it = statementToUsedVariables.find(s);
    if (it == statementToUsedVariables.end()) {
        return VARIABLE_NAME_LIST();
    } else {
        return VARIABLE_NAME_LIST(it->second.begin(), it->second.end());
    }
}
VARIABLE_NAME_LIST PKBImplementation::getVariablesUsedBySomeStatement() const {
    return VARIABLE_NAME_LIST(allVariablesUsedBySomeStatement.begin(),
                              allVariablesUsedBySomeStatement.end());
}

/** -------------------------- MODIFIES ---------------------------- **/
STATEMENT_NUMBER_SET PKBImplementation::getStatementsThatModify(VARIABLE_NAME v) const {
    auto it = variableToStatementsThatModifyIt.find(v);
    if (it == variableToStatementsThatModifyIt.end()) {
        return STATEMENT_NUMBER_SET();
    } else {
        return STATEMENT_NUMBER_SET(it->second.begin(), it->second.end());
    }
}
STATEMENT_NUMBER_SET PKBImplementation::getStatementsThatModifySomeVariable() const {
    return STATEMENT_NUMBER_SET(allStatementsThatModifySomeVariable.begin(),
                                allStatementsThatModifySomeVariable.end());
}
PROCEDURE_NAME_LIST PKBImplementation::getProceduresThatModify(VARIABLE_NAME v) const {
    auto it = variableToProceduresThatModifyIt.find(v);
    if (it == variableToProceduresThatModifyIt.end()) {
        return PROCEDURE_NAME_LIST();
    } else {
        return PROCEDURE_NAME_LIST(it->second.begin(), it->second.end());
    }
}
PROCEDURE_NAME_LIST PKBImplementation::getProceduresThatModifySomeVariable() const {
    return PROCEDURE_NAME_LIST(allProceduresThatThatModifySomeVariable.begin(),
                               allProceduresThatThatModifySomeVariable.end());
}
VARIABLE_NAME_LIST PKBImplementation::getVariablesModifiedBy(PROCEDURE_NAME p) const {
    auto it = procedureToModifiedVariables.find(p);
    if (it == procedureToModifiedVariables.end()) {
        return VARIABLE_NAME_LIST();
    } else {
        return VARIABLE_NAME_LIST(it->second.begin(), it->second.end());
    }
}
VARIABLE_NAME_LIST PKBImplementation::getVariablesModifiedBySomeProcedure() const {
    return VARIABLE_NAME_LIST(allVariablesModifiedBySomeProcedure.begin(),
                              allVariablesModifiedBySomeProcedure.end());
}
VARIABLE_NAME_LIST PKBImplementation::getVariablesModifiedBy(STATEMENT_NUMBER s) const {
    auto it = statementToModifiedVariables.find(s);
    if (it == statementToModifiedVariables.end()) {
        return VARIABLE_NAME_LIST();
    } else {
        return VARIABLE_NAME_LIST(it->second.begin(), it->second.end());
    }
}
VARIABLE_NAME_LIST PKBImplementation::getVariablesModifiedBySomeStatement() const {
    return VARIABLE_NAME_LIST(allVariablesModifiedBySomeStatement.begin(),
                              allVariablesModifiedBySomeStatement.end());
}

/** -------------------------- Pattern ---------------------------- **/
STATEMENT_NUMBER_SET
PKBImplementation::getAllAssignmentStatementsThatMatch(const std::string& assignee,
                                                       const std::string& pattern,
                                                       bool isSubExpr) const {
    std::string strippedPattern = pattern;
    auto res = std::remove_if(strippedPattern.begin(), strippedPattern.end(), isspace);
    strippedPattern.erase(res, strippedPattern.end());

    // catch `pattern = "        "` case
    if (strippedPattern.empty()) {
        if (!isSubExpr) {
            return {};
        }
        if (assignee == "_") {
            // since both pattern and assignee is empty, we return all assignment statements.
            return { allAssignmentStatements.begin(), allAssignmentStatements.end() };
        }
        // Return all s such that Modifies(assignee, s);
        if (variableToStatementsThatModifyIt.find(assignee) == variableToStatementsThatModifyIt.end()) {
            return STATEMENT_NUMBER_SET();
        }
        STATEMENT_NUMBER_SET statementsThatModifyAssignee =
        variableToStatementsThatModifyIt.find(assignee)->second;
        return STATEMENT_NUMBER_SET(statementsThatModifyAssignee.begin(),
                                    statementsThatModifyAssignee.end());
    }

    // Preprocess pattern using the parser, to set precedence.
    std::string searchPattern = Parser::parseExpr(pattern);
    if (searchPattern.empty() || patternsMap.find(searchPattern) == patternsMap.end()) {
        return {};
    }
    std::vector<std::tuple<std::string, STATEMENT_NUMBER, bool>> candidateResult = patternsMap.at(searchPattern);
    if (assignee != "_") {
        // remove results that have a different assignee
        auto res = std::remove_if(candidateResult.begin(), candidateResult.end(),
                                  [&](const std::tuple<std::string, STATEMENT_NUMBER, bool>& x) {
                                      return std::get<0>(x) != assignee;
                                  });
        candidateResult.erase(res, candidateResult.end());
    }
    if (!isSubExpr) {
        // Remove results that are sub expressions
        auto res = std::remove_if(candidateResult.begin(), candidateResult.end(),
                                  [&](const std::tuple<std::string, STATEMENT_NUMBER, bool>& x) {
                                      return std::get<2>(x);
                                  });
        candidateResult.erase(res, candidateResult.end());
    }

    STATEMENT_NUMBER_SET result;
    for (auto tup : candidateResult) {
        result.insert(std::get<1>(tup));
    }
    return result;
}

bool PKBImplementation::isRead(STATEMENT_NUMBER s) const {
    auto it = statementNumberToTNodeType.find(s);
    if (it == statementNumberToTNodeType.end()) {
        return false;
    }
    return it->second == Read;
}

bool PKBImplementation::isPrint(STATEMENT_NUMBER s) const {
    auto it = statementNumberToTNodeType.find(s);
    if (it == statementNumberToTNodeType.end()) {
        return false;
    }
    return it->second == Print;
}

bool PKBImplementation::isCall(STATEMENT_NUMBER s) const {
    auto it = statementNumberToTNodeType.find(s);
    if (it == statementNumberToTNodeType.end()) {
        return false;
    }
    return it->second == Call;
}

bool PKBImplementation::isWhile(STATEMENT_NUMBER s) const {
    auto it = statementNumberToTNodeType.find(s);
    if (it == statementNumberToTNodeType.end()) {
        return false;
    }
    return it->second == While;
}

bool PKBImplementation::isIfElse(STATEMENT_NUMBER s) const {
    auto it = statementNumberToTNodeType.find(s);
    if (it == statementNumberToTNodeType.end()) {
        return false;
    }
    return it->second == IfElse;
}

bool PKBImplementation::isAssign(STATEMENT_NUMBER s) const {
    auto it = statementNumberToTNodeType.find(s);
    if (it == statementNumberToTNodeType.end()) {
        return false;
    }
    return it->second == Assign;
}

PROCEDURE_NAME_SET PKBImplementation::getProcedureThatCalls(const PROCEDURE_NAME& procedureName,
                                                            bool isTransitive) const {
    return foost::getVisitedInDFS(procedureName, procedureToCallers, isTransitive);
}

PROCEDURE_NAME_SET PKBImplementation::getProceduresCalledBy(const PROCEDURE_NAME& procedureName,
                                                            bool isTransitive) const {
    return foost::getVisitedInDFS(procedureName, procedureToCalledProcedures, isTransitive);
}
const PROCEDURE_NAME_SET& PKBImplementation::getAllProceduresThatCallSomeProcedure() const {
    return allProceduresThatCall;
}
const PROCEDURE_NAME_SET& PKBImplementation::getAllCalledProcedures() const {
    return allCalledProcedures;
}

STATEMENT_NUMBER_SET
PKBImplementation::getNextStatementOf(STATEMENT_NUMBER statementNumber, bool isTransitive) const {
    return foost::getVisitedInDFS(statementNumber, nextRelationship, isTransitive);
}

STATEMENT_NUMBER_SET PKBImplementation::getPreviousStatementOf(STATEMENT_NUMBER statementNumber,
                                                               bool isTransitive) const {
    return foost::getVisitedInDFS(statementNumber, previousRelationship, isTransitive);
}

STATEMENT_NUMBER_SET PKBImplementation::getAllWhileStatementsThatMatch(const VARIABLE_NAME& variable,
                                                                       const std::string& pattern,
                                                                       bool isSubExpr) const {
    if (!pattern.empty() || !isSubExpr) {
        throw std::runtime_error(
        "getAllWhileStatementsThatMatch: body-pattern match is not implemented yet.");
    }
    // Get all while statements, and get all statements whose cond uses variable, and find intersection.
    auto it = conditionVariablesToStatementNumbers.find(variable);
    if (it == conditionVariablesToStatementNumbers.end()) {
        return {};
    }
    const STATEMENT_NUMBER_SET& conditionsThatMatches = it->second;

    return foost::SetIntersection(allWhileStatements, conditionsThatMatches);
}

STATEMENT_NUMBER_SET PKBImplementation::getAllIfElseStatementsThatMatch(const VARIABLE_NAME& variable,
                                                                        const std::string& ifPattern,
                                                                        bool ifPatternIsSubExpr,
                                                                        const std::string& elsePattern,
                                                                        bool elsePatternIsSubExpr) const {
    if (!ifPattern.empty() || !elsePattern.empty() || !ifPatternIsSubExpr || !elsePatternIsSubExpr) {
        throw std::runtime_error(
        "getAllIfElseStatementsThatMatch: body-pattern match is not implemented yet.");
    }
    if (variable == "_") {
        return allIfElseStatements;
    }
    // Get all if statements, and get all statements whose cond uses variable, and find intersection.
    auto it = conditionVariablesToStatementNumbers.find(variable);
    if (it == conditionVariablesToStatementNumbers.end()) {
        return {};
    }
    const STATEMENT_NUMBER_SET& conditionsThatMatches = it->second;

    return foost::SetIntersection(allIfElseStatements, conditionsThatMatches);
}

const STATEMENT_NUMBER_SET& PKBImplementation::getAllStatementsWithNext() const {
    return statementsWithNext;
}

const STATEMENT_NUMBER_SET& PKBImplementation::getAllStatementsWithPrev() const {
    return statementsWithPrev;
}

PROGRAM_LINE_SET PKBImplementation::getStatementsAffectedBy(PROGRAM_LINE statementNumber, bool isTransitive) const {
    return PROGRAM_LINE_SET();
}
PROGRAM_LINE_SET PKBImplementation::getStatementsThatAffect(PROGRAM_LINE statementNumber, bool isTransitive) const {
    return PROGRAM_LINE_SET();
}
const PROGRAM_LINE_SET& PKBImplementation::getAllStatementsThatAffect() const {
    return PROGRAM_LINE_SET();
}
const PROGRAM_LINE_SET& PKBImplementation::getAllStatementsThatAreAffected() const {
    return PROGRAM_LINE_SET();
}

} // namespace backend
