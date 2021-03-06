#pragma once

#include "DesignExtractor.h"
#include "PKB.h"
#include "TNode.h"

#include <set>
#include <unordered_map>

namespace backend {
class PKBImplementation : virtual public backend::PKB {
  public:
    PKBImplementation() = default;
    explicit PKBImplementation(const TNode& ast);
    const STATEMENT_NUMBER_SET& getAllStatements() const override;
    const VARIABLE_NAME_LIST& getAllVariables() const override;
    const PROCEDURE_NAME_LIST& getAllProcedures() const override;
    const CONSTANT_NAME_SET& getAllConstants() const override;

    bool isRead(STATEMENT_NUMBER s) const override;
    bool isPrint(STATEMENT_NUMBER s) const override;
    bool isCall(STATEMENT_NUMBER s) const override;
    bool isWhile(STATEMENT_NUMBER s) const override;
    bool isIfElse(STATEMENT_NUMBER s) const override;
    bool isAssign(STATEMENT_NUMBER s) const override;

    const STATEMENT_NUMBER_SET getCallStatementsWithProcedureName(PROCEDURE_NAME procedureName) const override;
    const PROCEDURE_NAME getProcedureNameFromCallStatement(STATEMENT_NUMBER callStatementNumber) const override;
    const STATEMENT_NUMBER_SET getReadStatementsWithVariableName(VARIABLE_NAME variableName) const override;
    const VARIABLE_NAME getVariableNameFromReadStatement(STATEMENT_NUMBER readStatementNumber) const override;
    const STATEMENT_NUMBER_SET getPrintStatementsWithVariableName(VARIABLE_NAME variableName) const override;
    const VARIABLE_NAME getVariableNameFromPrintStatement(STATEMENT_NUMBER printStatementNumber) const override;

    STATEMENT_NUMBER_SET getDirectFollow(STATEMENT_NUMBER s) const override;
    STATEMENT_NUMBER_SET getDirectFollowedBy(STATEMENT_NUMBER s) const override;
    STATEMENT_NUMBER_SET getStatementsFollowedBy(STATEMENT_NUMBER s) const override;
    STATEMENT_NUMBER_SET getAllStatementsThatFollows() const override;
    STATEMENT_NUMBER_SET getStatementsThatFollows(STATEMENT_NUMBER s) const override;
    STATEMENT_NUMBER_SET getAllStatementsThatAreFollowed() const override;

    STATEMENT_NUMBER_SET getParent(STATEMENT_NUMBER statementNumber) const override;
    STATEMENT_NUMBER_SET getChildren(STATEMENT_NUMBER statementNumber) const override;
    STATEMENT_NUMBER_SET getAncestors(STATEMENT_NUMBER statementNumber) const override;
    STATEMENT_NUMBER_SET getStatementsThatHaveAncestors() const override;
    STATEMENT_NUMBER_SET getDescendants(STATEMENT_NUMBER statementNumber) const override;
    STATEMENT_NUMBER_SET getStatementsThatHaveDescendants() const override;

    STATEMENT_NUMBER_SET getStatementsThatUse(VARIABLE_NAME v) const override;
    STATEMENT_NUMBER_SET getStatementsThatUseSomeVariable() const override;
    PROCEDURE_NAME_LIST getProceduresThatUse(VARIABLE_NAME v) const override;
    PROCEDURE_NAME_LIST getProceduresThatUseSomeVariable() const override;
    VARIABLE_NAME_LIST getVariablesUsedIn(PROCEDURE_NAME p) const override;
    VARIABLE_NAME_LIST getVariablesUsedBySomeProcedure() const override;
    VARIABLE_NAME_LIST getVariablesUsedIn(STATEMENT_NUMBER s) const override;
    VARIABLE_NAME_LIST getVariablesUsedBySomeStatement() const override;

    STATEMENT_NUMBER_SET getStatementsThatModify(VARIABLE_NAME v) const override;
    STATEMENT_NUMBER_SET getStatementsThatModifySomeVariable() const override;
    PROCEDURE_NAME_LIST getProceduresThatModify(VARIABLE_NAME v) const override;
    PROCEDURE_NAME_LIST getProceduresThatModifySomeVariable() const override;
    VARIABLE_NAME_LIST getVariablesModifiedBy(PROCEDURE_NAME p) const override;
    VARIABLE_NAME_LIST getVariablesModifiedBySomeProcedure() const override;
    VARIABLE_NAME_LIST getVariablesModifiedBy(STATEMENT_NUMBER s) const override;
    VARIABLE_NAME_LIST getVariablesModifiedBySomeStatement() const override;

    PROCEDURE_NAME_SET getProcedureThatCalls(const VARIABLE_NAME& procedureName, bool isTransitive) const override;
    PROCEDURE_NAME_SET getProceduresCalledBy(const VARIABLE_NAME& procedureName, bool isTransitive) const override;
    const PROCEDURE_NAME_SET& getAllProceduresThatCallSomeProcedure() const override;
    const PROCEDURE_NAME_SET& getAllCalledProcedures() const override;

    STATEMENT_NUMBER_SET getNextStatementOf(STATEMENT_NUMBER statementNumber, bool isTransitive) const override;
    STATEMENT_NUMBER_SET getPreviousStatementOf(STATEMENT_NUMBER statementNumber, bool isTransitive) const override;
    const STATEMENT_NUMBER_SET& getAllStatementsWithNext() const override;
    const STATEMENT_NUMBER_SET& getAllStatementsWithPrev() const override;

    STATEMENT_NUMBER_SET getNextBipStatementOf(STATEMENT_NUMBER statementNumber, bool isTransitive) const override;
    STATEMENT_NUMBER_SET getPreviousBipStatementOf(STATEMENT_NUMBER statementNumber, bool isTransitive) const override;
    STATEMENT_NUMBER_SET getAllStatementsWithNextBip() const override;
    STATEMENT_NUMBER_SET getAllStatementsWithPreviousBip() const override;

    PROGRAM_LINE_SET getStatementsAffectedBy(PROGRAM_LINE statementNumber, bool isTransitive) const override;
    PROGRAM_LINE_SET getStatementsThatAffect(PROGRAM_LINE statementNumber, bool isTransitive) const override;
    const PROGRAM_LINE_SET& getAllStatementsThatAffect() const override;
    const PROGRAM_LINE_SET& getAllStatementsThatAreAffected() const override;

    PROGRAM_LINE_SET getStatementsAffectedBipBy(PROGRAM_LINE statementNumber, bool isTransitive) const override;
    PROGRAM_LINE_SET getStatementsThatAffectBip(PROGRAM_LINE statementNumber, bool isTransitive) const override;
    const PROGRAM_LINE_SET& getAllStatementsThatAffectBip() const override;
    const PROGRAM_LINE_SET& getAllStatementsThatAreAffectedBip() const override;

    // Pattern
    STATEMENT_NUMBER_SET
    getAllAssignmentStatementsThatMatch(const std::string& assignee, const std::string& pattern, bool isSubExpr) const override;
    STATEMENT_NUMBER_SET getAllWhileStatementsThatMatch(const VARIABLE_NAME& variable,
                                                        const std::string& pattern,
                                                        bool isSubExpr) const override;
    STATEMENT_NUMBER_SET getAllIfElseStatementsThatMatch(const VARIABLE_NAME& variable,
                                                         const std::string& ifPattern,
                                                         bool ifPatternIsSubExpr,
                                                         const std::string& elsePattern,
                                                         bool elsePatternIsSubExpr) const override;

  private:
    std::unordered_map<const TNode*, int> tNodeToStatementNumber;

    // Follows helper:
    // for k, v in map, follow(v, k).
    std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER> followedFollowRelation;
    // for k, v in map, follow(k, v).
    std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER> followFollowedRelation;
    // Stmt list is private to prevent modification.
    STATEMENT_NUMBER_SET allStatementsThatFollows;
    STATEMENT_NUMBER_SET allStatementsThatAreFollowed;
    std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET> transitiveFollows;
    std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET> transitiveFollowed;

    // Parent helper:
    // for k, v in map, parent(k, j) for j in v
    std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET> parentChildrenRelation;
    // for k, v in map, parent(v, k).
    std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER> childrenParentRelation;
    // Stmt list is private to prevent modification.
    STATEMENT_NUMBER_SET allStatementsThatHaveAncestors;
    STATEMENT_NUMBER_SET allStatementsThatHaveDescendants;


    // Uses helper:
    std::unordered_map<const TNode*, std::unordered_set<std::string>> usesMapping;
    std::unordered_map<VARIABLE_NAME, STATEMENT_NUMBER_SET> variableToStatementsThatUseIt;
    STATEMENT_NUMBER_SET allStatementsThatUseSomeVariable;
    std::unordered_map<VARIABLE_NAME, PROCEDURE_NAME_SET> variableToProceduresThatUseIt;
    PROCEDURE_NAME_SET allProceduresThatThatUseSomeVariable;
    std::unordered_map<PROCEDURE_NAME, VARIABLE_NAME_SET> procedureToUsedVariables;
    VARIABLE_NAME_SET allVariablesUsedBySomeProcedure;
    std::unordered_map<STATEMENT_NUMBER, VARIABLE_NAME_SET> statementToUsedVariables;
    VARIABLE_NAME_SET allVariablesUsedBySomeStatement;

    // Modifies helper:
    std::unordered_map<const TNode*, std::unordered_set<std::string>> modifiesMapping;
    std::unordered_map<VARIABLE_NAME, STATEMENT_NUMBER_SET> variableToStatementsThatModifyIt;
    STATEMENT_NUMBER_SET allStatementsThatModifySomeVariable;
    std::unordered_map<VARIABLE_NAME, PROCEDURE_NAME_SET> variableToProceduresThatModifyIt;
    PROCEDURE_NAME_SET allProceduresThatThatModifySomeVariable;
    std::unordered_map<PROCEDURE_NAME, VARIABLE_NAME_SET> procedureToModifiedVariables;
    VARIABLE_NAME_SET allVariablesModifiedBySomeProcedure;
    std::unordered_map<STATEMENT_NUMBER, VARIABLE_NAME_SET> statementToModifiedVariables;
    VARIABLE_NAME_SET allVariablesModifiedBySomeStatement;

    // Pattern helper:
    std::unordered_set<int> allWhileCondWithVariables;
    std::unordered_set<int> allIfElseCondWithVariables;
    std::unordered_map<std::string, std::vector<std::tuple<std::string, STATEMENT_NUMBER, bool>>> patternsMap;
    std::unordered_map<VARIABLE_NAME, STATEMENT_NUMBER_SET> conditionVariablesToStatementNumbers;

    // Call helper:
    // {key, values} of all procedures, where values are the procedures that is called by key.
    std::unordered_map<std::string, std::unordered_set<std::string>> procedureToCalledProcedures;
    // {key, values} of all procedures, where values are the procedures that calls the key.
    std::unordered_map<std::string, std::unordered_set<std::string>> procedureToCallers;
    PROCEDURE_NAME_SET allProceduresThatCall;
    PROCEDURE_NAME_SET allCalledProcedures;

    // Next helper:
    std::unordered_map<STATEMENT_NUMBER, std::unordered_set<STATEMENT_NUMBER>> nextRelationship;
    std::unordered_map<STATEMENT_NUMBER, std::unordered_set<STATEMENT_NUMBER>> previousRelationship;
    STATEMENT_NUMBER_SET statementsWithNext;
    STATEMENT_NUMBER_SET statementsWithPrev;

    // NextBip helper:
    std::unordered_map<PROGRAM_LINE, std::unordered_set<extractor::NextBipEdge>> nextBipRelationship;
    std::unordered_map<PROGRAM_LINE, std::unordered_set<extractor::NextBipEdge>> previousBipRelationship;
    std::unordered_map<STATEMENT_NUMBER, std::unique_ptr<const TNode>> procedureEndNodes;

    // Affects helper:
    mutable std::unordered_map<PROGRAM_LINE, PROGRAM_LINE_SET> affectsMapping;
    mutable std::unordered_map<PROGRAM_LINE, PROGRAM_LINE_SET> affectedMapping;
    mutable STATEMENT_NUMBER_SET statementsThatAffect;
    mutable STATEMENT_NUMBER_SET statementsThatAreAffected;

    // AffectsBip helper:
    std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET> affectsBipMapping;
    mutable std::map<ScopedStatement, ScopedStatements> affectsBipStarMemo;
    mutable std::map<ScopedStatement, ScopedStatements> affectedBipStarMemo;
    std::map<ScopedStatement, ScopedStatements> affectsBipStarMapping;
    std::unordered_map<STATEMENT_NUMBER, STATEMENT_NUMBER_SET> affectedBipMapping;
    std::map<ScopedStatement, ScopedStatements> affectedBipStarMapping;
    STATEMENT_NUMBER_SET statementsThatAffectBip;
    STATEMENT_NUMBER_SET statementsThatAreAffectedBip;

    // Performance booster fields:
    std::unordered_map<TNodeType, std::vector<const TNode*>, EnumClassHash> tNodeTypeToTNodesMap;
    std::unordered_map<int, TNodeType> statementNumberToTNodeType;

    /// Entities retrieval helper
    VARIABLE_NAME_LIST allVariablesName;
    CONSTANT_NAME_SET allConstantsName;
    PROCEDURE_NAME_LIST allProceduresName;
    STATEMENT_NUMBER_SET allStatementsNumber;
    STATEMENT_NUMBER_SET allAssignmentStatements;
    STATEMENT_NUMBER_SET allWhileStatements;
    STATEMENT_NUMBER_SET allIfElseStatements;
    std::unordered_map<STATEMENT_NUMBER, const TNode*> statementNumberToTNode;

    std::unordered_map<PROCEDURE_NAME, STATEMENT_NUMBER_SET> procedureNameToCallStatements;
    std::unordered_map<VARIABLE_NAME, STATEMENT_NUMBER_SET> variableNameToReadStatements;
    std::unordered_map<STATEMENT_NUMBER, VARIABLE_NAME> readStatementsToVariableName;
    std::unordered_map<STATEMENT_NUMBER, VARIABLE_NAME> printStatementsToVariableName;
    std::unordered_map<STATEMENT_NUMBER, PROCEDURE_NAME> callStatementsToProcedureName;
    std::unordered_map<VARIABLE_NAME, STATEMENT_NUMBER_SET> variableNameToPrintStatements;
};
} // namespace backend
