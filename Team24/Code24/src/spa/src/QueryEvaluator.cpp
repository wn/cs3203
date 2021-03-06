#include "QueryEvaluator.h"

#include "Logger.h"
#include "Optimisation.h"
#include "PKB.h"
#include "QEHelper.h"
#include "QPTypes.h"
#include "Query.h"

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace qpbackend {
namespace queryevaluator {
std::vector<std::string> QueryEvaluator::evaluateQuery(Query query) {
    SingleQueryEvaluator sqe(query);
    return sqe.evaluateQuery(pkb);
}

// helper structure to map such-that clause arguments to SubRelationType
SRT_LOOKUP_TABLE SingleQueryEvaluator::srt_table = generateSrtTable();
ATTR_CONVERT_TABLE SingleQueryEvaluator::attr_convert_table = generateAttrConvertTable();

std::vector<std::string> SingleQueryEvaluator::evaluateQuery(const backend::PKB* pkb) {
    // initialize the table of candidates
    if (hasEvaluationCompleted) {
        handleError("same single query evaluator should not be called twice");
    }

    if (query.returnCandidates.empty()) {
        handleError("Invalid query");
    }

    for (const auto& requested : query.returnCandidates) {
        if (requested.first == INVALID_RETURN_TYPE) {
            handleError("invalid return type");
            break;
        }

        if (requested.first == BOOLEAN) {
            if (query.returnCandidates.size() != 1) {
                handleError("BOOLEAN as return value should not appear in a tuple");
                break;
            }
            continue;
        }

        initializeIfSynonym(pkb, requested.second);
    }

    // sort and group clauses
    std::vector<std::vector<CLAUSE_LIST>> clauses = getClausesSortedAndGrouped(pkb);

    // evaluate clauses
    for (const auto& group : clauses) {
        if (failed) {
            break;
        }
        ResultTable stGroupRT;
        for (const auto& subgroup : group) {
            if (failed) {
                break;
            }
            ResultTable subGroupRT;
            for (const auto& clause : subgroup) {
                if (failed) {
                    break;
                }
                failed = failed || !(evaluateClause(pkb, clause, subGroupRT));
                const std::string& argName1 = std::get<1>(clause).second;
                const std::string& argName2 = std::get<2>(clause).second;
                if (isSynonym(argName1)) {
                    synonymCounters[argName1]--;
                }
                if (isSynonym(argName2)) {
                    synonymCounters[argName2]--;
                }
            }
            failed = failed || !stGroupRT.mergeTable(std::move(subGroupRT));
            updateSynonymsWithResultTable(stGroupRT, true);
        }
        failed = failed || !resultTable.mergeTable(std::move(stGroupRT));
        updateSynonymsWithResultTable(resultTable);
    }

    // prepare output
    hasEvaluationCompleted = true;
    return produceResult(pkb);
}

/**
 * convert evaluation result to a string
 */
std::vector<std::string> SingleQueryEvaluator::produceResult(const backend::PKB* pkb) {
    if (query.returnCandidates.empty()) {
        return std::vector<std::string>();
    }

    if (query.returnCandidates.at(0).first == BOOLEAN) {
        const std::string result = (failed) ? "FALSE" : "TRUE";
        return { result };
    }

    if (failed) {
        return std::vector<std::string>();
    }

    // evaluate attribute reference
    std::vector<std::string> synNames;
    for (const auto& returnSyn : query.returnCandidates) {
        const std::string& synName = returnSyn.second;
        const EntityType entityType = query.declarationMap.at(synName);
        const ReturnType returnType = returnSyn.first;

        ArgType argType = getAttrArgType(returnType, synName);
        if (argType == INVALID_ARG) {
            handleError("invalid return type");
            return std::vector<std::string>();
        }

        bool needConversion = needAttrConversion(argType);

        if (!needConversion) {
            if (!resultTable.isSynonymContained(synName)) {
                std::unordered_set<std::string> tempSet(synonym_candidates[synName].begin(),
                                                        synonym_candidates[synName].end());
                ResultTable tempTable(synName, tempSet);
                if (resultTable.isEmpty()) {
                    resultTable = tempTable;
                } else {
                    resultTable.mergeTable(std::move(tempTable));
                }
            }
            synNames.push_back(synName);
        } else {
            std::unordered_set<std::vector<std::string>, StringVectorHash> pairs;
            for (const auto& c1 : synonym_candidates[synName]) {
                const std::string& attr = inquirePKBForAttribute(pkb, argType, c1);
                pairs.insert({ c1, attr });
            }
            // assign cl.procName to cl_0
            std::string attrName = assignSynonymToAttribute(synName, returnType);
            ResultTable tempTable({ synName, attrName }, pairs);
            if (resultTable.isEmpty()) {
                resultTable = tempTable;
            } else {
                resultTable.mergeTable(std::move(tempTable));
            }
            synNames.push_back(attrName);
        }
    }

    // write tuple to string
    std::vector<std::vector<std::string>> resultTuples;
    resultTable.updateSynonymValueTupleVector(synNames, resultTuples);
    std::vector<std::string> resultStrs;
    for (const auto& row : resultTuples) {
        resultStrs.push_back(tupleToStr(row));
    }
    return resultStrs;
};

/**
 * check if a name is a synonym's name and initialized.
 * if it's a synonym but not in the synonym_candidates, call initialization method
 * @param pkb : the pkb to look up for synonym's candidate values.
 * @param synonym : the name of th synonym. i.e., for synonym x, "x" is its name.
 */
void SingleQueryEvaluator::initializeIfSynonym(const backend::PKB* pkb, std::string const& synonymName) {
    if ((query.declarationMap.find(synonymName) != query.declarationMap.end()) &&
        (synonym_candidates.find(synonymName) == synonym_candidates.end())) {
        initializeCandidate(pkb, synonymName, query.declarationMap[synonymName]);
    }
}

/**
 * store the name in the synonym_candidates and intialize its liss of candidate values.
 * @param pkb : the pkb to look for candidate synonyms
 * @param synonym : the synonym's name
 * @param entityType : the type of the synonym in declaration
 */
void SingleQueryEvaluator::initializeCandidate(const backend::PKB* pkb,
                                               std::string const& synonymName,
                                               EntityType entityType) {
    // initialize the possible values of all synonyms
    if (entityType == VARIABLE) {
        VARIABLE_NAME_LIST variables = pkb->getAllVariables();
        synonym_candidates[synonymName] = std::vector<std::string>(variables.begin(), variables.end());
    } else if (entityType == PROCEDURE) {
        PROCEDURE_NAME_LIST procs = pkb->getAllProcedures();
        synonym_candidates[synonymName] = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
    } else if (entityType == CONSTANT) {
        CONSTANT_NAME_SET consts = pkb->getAllConstants();
        std::vector<std::string> constStrs;
        std::copy(consts.begin(), consts.end(), std::back_inserter(constStrs));
        synonym_candidates[synonymName] = constStrs;
    } else {
        std::map<EntityType, std::function<bool(STATEMENT_NUMBER)>> entityTypeToUnaryPredictor = {
            { ASSIGN, [pkb](STATEMENT_NUMBER x) { return pkb->isAssign(x); } },
            { CALL, [pkb](STATEMENT_NUMBER x) { return pkb->isCall(x); } },
            { IF, [pkb](STATEMENT_NUMBER x) { return pkb->isIfElse(x); } },
            { PRINT, [pkb](STATEMENT_NUMBER x) { return pkb->isPrint(x); } },
            { READ, [pkb](STATEMENT_NUMBER x) { return pkb->isRead(x); } },
            { STMT, [pkb](STATEMENT_NUMBER x) { return true; } },
            { PROG_LINE, [pkb](STATEMENT_NUMBER x) { return true; } },
            { WHILE, [pkb](STATEMENT_NUMBER x) { return pkb->isWhile(x); } },
        };
        if (entityTypeToUnaryPredictor.find(entityType) == entityTypeToUnaryPredictor.end()) {
            handleError("invalid entity type");
            return;
        }
        // Only keep statements that fulfill the predicate
        STATEMENT_NUMBER_SET allStatements = pkb->getAllStatements();
        STATEMENT_NUMBER_SET result;
        auto predicate = entityTypeToUnaryPredictor[entityType];
        for (STATEMENT_NUMBER statementNumber : allStatements) {
            if (predicate(statementNumber)) {
                result.insert(statementNumber);
            }
        }

        synonym_candidates[synonymName] = castToStrVector<>(result);
    }
}

/**
 * evaluate single clause
 * @param pkb
 * @param clause
 * @param groupResultTable: IRT of the group the clause belongs to
 * @return return false if (i) semantic errors encountered (ii) no result found
 */
bool SingleQueryEvaluator::evaluateClause(const backend::PKB* pkb, const CLAUSE& clause, ResultTable& groupResultTable) {
    ClauseType rt = std::get<0>(clause);

    ArgType arg_type_1 = std::get<1>(clause).first;
    ArgType arg_type_2 = std::get<2>(clause).first;

    std::string arg1 = std::get<1>(clause).second;
    std::string arg2 = std::get<2>(clause).second;

    const std::string& patternStr = std::get<3>(clause);

    SubRelationType srt = getSubRelationType(rt, arg_type_1, arg_type_2);

    // Initializes arguments if they point to declared synonyms.
    initializeIfSynonym(pkb, arg1);
    initializeIfSynonym(pkb, arg2);

    ClauseArgsType argsType = getClauseArgsType(arg_type_1, arg_type_2);

    switch (argsType) {
    case SYNONYM_SYNONYM:
        return evaluateSynonymSynonym(pkb, srt, arg_type_2, arg_type_1, arg2, arg1, patternStr, groupResultTable);
    case SYNONYM_ENTITY:
        return evaluateEntitySynonym(pkb, srt, arg_type_1, arg2, arg1, patternStr,
                                     groupResultTable); // swap the arguments as the called method required
    case SYNONYM_WILDCARD:
        return evaluateSynonymWildcard(pkb, srt, arg1, patternStr, groupResultTable);
    case ENTITY_SYNONYM:
        return evaluateEntitySynonym(pkb, srt, arg_type_2, arg1, arg2, patternStr, groupResultTable);
    case ENTITY_ENTITY:
        return evaluateEntityEntity(pkb, srt, arg1, arg2);
    case ENTITY_WILDCARD:
        return evaluateEntityWildcard(pkb, srt, arg1);
    case WILDCARD_SYNONYM:
        return evaluateSynonymWildcard(pkb, srt, arg2, patternStr, groupResultTable);
    case WILDCARD_ENTITY:
        return evaluateEntityWildcard(pkb, srt, arg2);
    case WILDCARD_WILDCARD:
        return evaluateWildcardWildcard(pkb, srt);
    case INVALID_1:
        handleError("invalid arg1 type: " + arg1);
        return false;
    case INVALID_2:
        handleError("invalid arg2 type: " + arg2);
        return false;
    default:
        handleError("Clauses argument is not implemented yet.");
        return false;
    }
}

/**
 * evaluate the clause against a pair of synonyms
 * after evaluation, update two synonyms' candidate list
 * @param pkb
 * @param subRelationType
 * @param arg1 : the name of first synonym
 * @param arg2 : the name of second synonym
 * @param groupResultTable: the result table of the group the clause belongs to
 * @param patternStr: the pattern string
 * @return : false if any synonym's candidate value list got empty. otherwise true
 */
bool SingleQueryEvaluator::evaluateSynonymSynonym(const backend::PKB* pkb,
                                                  SubRelationType subRelationType,
                                                  ArgType argType1,
                                                  ArgType argType2,
                                                  std::string const& arg1,
                                                  std::string const& arg2,
                                                  std::string const& patternStr,
                                                  ResultTable& groupResultTable) {
    bool isSelfRelation = (arg1 == arg2);
    std::vector<std::string> candidates_1 = synonym_candidates[arg1];
    std::vector<std::string> candidates_2 = synonym_candidates[arg2];

    if (candidates_1.empty() || candidates_2.empty()) {
        return false;
    }

    // check all pairs
    std::unordered_set<std::string> singleEntity;
    std::unordered_set<std::vector<std::string>, StringVectorHash> pairs;
    if (subRelationType == WITH_SRT) {
        const std::string& attrName = arg1 + "_" + arg2;
        std::unordered_set<std::vector<std::string>, StringVectorHash> attrPairs1 =
        evaluateSynonymAttrForWith(pkb, subRelationType, argType1, arg1);
        std::unordered_set<std::vector<std::string>, StringVectorHash> attrPairs2 =
        evaluateSynonymAttrForWith(pkb, subRelationType, argType2, arg2);
        ResultTable rt1({ arg1, attrName }, attrPairs1);
        ResultTable rt2({ arg2, attrName }, attrPairs2);
        rt1.mergeTable(std::move(rt2));
        if (isSelfRelation) {
            rt1.updateSynonymValueSet(arg1, singleEntity);
        } else {
            rt1.updateSynonymValueTupleSet({ arg1, arg2 }, pairs);
        }
    } else {
        for (const auto& c1 : candidates_1) {
            std::vector<std::string> c1_result;
            c1_result = inquirePKBForRelationOrPattern(pkb, subRelationType, c1, patternStr);
            if (isSelfRelation) {
                if (isFoundInVector<std::string>(c1_result, c1)) {
                    singleEntity.insert(c1);
                }
            } else {
                for (const auto& c2 : candidates_2) {
                    if (isFoundInVector<std::string>(c1_result, c2)) {
                        pairs.insert({ c1, c2 });
                    }
                }
            }
        }
    }

    // update IRT table
    if (isSelfRelation) {
        ResultTable newRT(arg1, singleEntity);
        groupResultTable.mergeTable(std::move(newRT));
    } else {
        ResultTable newRT({ arg1, arg2 }, pairs);
        groupResultTable.mergeTable(std::move(newRT));
    }
    bool isNotFailed = !groupResultTable.isEmpty();
    updateSynonymsWithResultTable(groupResultTable);
    return isNotFailed;
}

/**
 * evaluate the clause against an entity and a synonym
 * after evaluation, update the candidate value list of the synonym
 * @param pkb
 * @param subRelationType
 * @param arg1 : an entity--stetment number or procedure name or variable name
 * @param arg2 : the name of a synonym
 * @param groupResultTable: the IRT table of the group the clause belongs to
 * @param patternStr : the pattern string
 * @return false if no candidates of synonym makes the relation hold, otherwise true
 */
bool SingleQueryEvaluator::evaluateEntitySynonym(const backend::PKB* pkb,
                                                 SubRelationType subRelationType,
                                                 ArgType synonymArgType,
                                                 std::string const& arg1,
                                                 std::string const& arg2,
                                                 std::string const& patternStr,
                                                 ResultTable& groupResultTable) {
    std::unordered_set<std::string> resultSet;
    if (subRelationType == WITH_SRT) {
        std::unordered_set<std::vector<std::string>, StringVectorHash> attrPairs =
        evaluateSynonymAttrForWith(pkb, subRelationType, synonymArgType, arg2);
        for (const auto& elem : attrPairs) {
            if (arg1 == elem[1]) {
                resultSet.insert(elem[0]);
            }
        }
    } else {
        std::vector<std::string> arg1_result =
        inquirePKBForRelationOrPattern(pkb, subRelationType, arg1, patternStr);
        std::vector<std::string> result = vectorIntersection<>(arg1_result, synonym_candidates[arg2]);
        resultSet = std::unordered_set<std::string>(result.begin(), result.end());
    }
    ResultTable newRT(arg2, resultSet);
    groupResultTable.mergeTable(std::move(newRT));
    bool isNotFailed = !groupResultTable.isEmpty();
    updateSynonymsWithResultTable(groupResultTable);
    return isNotFailed;
}

/**
 * evaluate the clause
 * @param pkb
 * @param subRelationType
 * @param arg1 : an entity--stetment number or procedure name or variable name
 * @param arg2 : an entity--stetment number or procedure name or variable name
 * @return false if the relation does not hold for two entities, otherwise true
 */
bool SingleQueryEvaluator::evaluateEntityEntity(const backend::PKB* pkb,
                                                SubRelationType subRelationType,
                                                std::string const& arg1,
                                                std::string const& arg2) {
    if (subRelationType == WITH_SRT) {
        return arg1 == arg2;
    }
    std::vector<std::string> arg1_result = inquirePKBForRelationOrPattern(pkb, subRelationType, arg1, "");
    return isFoundInVector<std::string>(arg1_result, arg2);
}

/**
 * evaluate the clause against a synonym and a wildcard (placeholde '_')
 * after the evaluation, update the synonm's candidate list
 * @param pkb
 * @param subRelationType
 * @param arg : the name of the synonym
 * @param groupResultTable: the IRT table of the group the clause belongs to
 * @return false if the synonm's candidate list gets empty
 */
bool SingleQueryEvaluator::evaluateSynonymWildcard(const backend::PKB* pkb,
                                                   SubRelationType subRelationType,
                                                   std::string const& arg,
                                                   std::string const& patternStr,
                                                   ResultTable& groupResultTable) {
    std::vector<std::string> inquired_result = inquirePKBForRelationWildcard(pkb, subRelationType, patternStr);
    std::vector<std::string> result = vectorIntersection(inquired_result, synonym_candidates[arg]);
    std::unordered_set<std::string> resultSet(result.begin(), result.end());
    ResultTable newRT(arg, resultSet);
    groupResultTable.mergeTable(std::move(newRT));
    bool isNotFailed = !groupResultTable.isEmpty();
    updateSynonymsWithResultTable(groupResultTable);
    return isNotFailed;
}

/**
 * evaluate the clause against an entity and a wildcard (place hol)
 * @param pkb
 * @param subRelationType
 * @param arg : an entity--stetment number or procedure name or variable name
 * @return false if the entity cannot fulfill the relation anyway, otherwise true
 */
bool SingleQueryEvaluator::evaluateEntityWildcard(const backend::PKB* pkb,
                                                  SubRelationType subRelationType,
                                                  std::string const& arg) {
    std::vector<std::string> result = inquirePKBForRelationWildcard(pkb, subRelationType, "");
    return isFoundInVector<std::string>(result, arg);
}

/**
 * evaluate a pair of wildcard
 * @param pkb
 * @param subRelationType
 * @return false if no such relations exist in the source code, otherwise true
 */
bool SingleQueryEvaluator::evaluateWildcardWildcard(const backend::PKB* pkb, SubRelationType subRelationType) {
    std::vector<std::string> result = inquirePKBForRelationWildcard(pkb, subRelationType, "");
    return !(result.empty());
}

std::unordered_set<std::vector<std::string>, StringVectorHash>
SingleQueryEvaluator::evaluateSynonymAttrForWith(const backend::PKB* pkb,
                                                 SubRelationType srt,
                                                 ArgType argType,
                                                 const std::string& arg) {
    std::unordered_set<std::vector<std::string>, StringVectorHash> result;
    for (const auto& val : synonym_candidates[arg]) {
        const std::string& attr = inquirePKBForAttribute(pkb, argType, val);
        result.insert({ val, attr });
    }
    return result;
}

/**
 * call PKB API methods to retrieve answer for the given relation and argument
 * @param pkb
 * @param subRelationType
 * @param arg : an entity--stetment number or procedure name or variable name
 * @return the list of values that together with the given entity make the relation hold
 */
std::vector<std::string> SingleQueryEvaluator::inquirePKBForRelationOrPattern(const backend::PKB* pkb,
                                                                              SubRelationType subRelationType,
                                                                              const std::string& arg,
                                                                              const std::string& patternStr) {
    std::vector<std::string> result;
    STATEMENT_NUMBER_SET stmts;
    PROCEDURE_NAME_SET procs;
    VARIABLE_NAME_LIST vars;
    switch (subRelationType) {
    case PREFOLLOWS:
        stmts = pkb->getDirectFollow(std::stoi(arg));
        result = castToStrVector<>(stmts);
        break;
    case POSTFOLLOWS:
        stmts = pkb->getDirectFollowedBy(std::stoi(arg));
        result = castToStrVector<>(stmts);
        break;
    case PREFOLLOWST:
        stmts = pkb->getStatementsThatFollows(std::stoi(arg));
        result = castToStrVector<>(stmts);
        break;
    case POSTFOLLOWST:
        stmts = pkb->getStatementsFollowedBy(std::stoi(arg));
        result = castToStrVector<>(stmts);
        break;
    case PREPARENT:
        stmts = pkb->getChildren(std::stoi(arg));
        result = castToStrVector<>(stmts);
        break;
    case POSTPARENT:
        stmts = pkb->getParent(std::stoi(arg));
        result = castToStrVector<>(stmts);
        break;
    case PREPARENTT:
        stmts = pkb->getDescendants(std::stoi(arg));
        result = castToStrVector<>(stmts);
        break;
    case POSTPARENTT:
        stmts = pkb->getAncestors(std::stoi(arg));
        result = castToStrVector<>(stmts);
        break;
    case PREUSESS: {
        VARIABLE_NAME_LIST variables = pkb->getVariablesUsedIn(std::stoi(arg));
        result = std::vector<VARIABLE_NAME>(variables.begin(), variables.end());
        break;
    }
    case POSTUSESS:
        stmts = pkb->getStatementsThatUse(arg);
        result = castToStrVector<>(stmts);
        break;
    case PREUSESP: {
        VARIABLE_NAME_LIST variables = pkb->getVariablesUsedIn(arg);
        result = std::vector<VARIABLE_NAME>(variables.begin(), variables.end());
        break;
    }
    case POSTUSESP: {
        auto procs = pkb->getProceduresThatUse(arg);
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    }
    case PREMODIFIESS: {
        VARIABLE_NAME_LIST variables = pkb->getVariablesModifiedBy(std::stoi(arg));
        result = std::vector<VARIABLE_NAME>(variables.begin(), variables.end());
        break;
    }
    case POSTMODIFIESS:
        stmts = pkb->getStatementsThatModify(arg);
        result = castToStrVector<>(stmts);
        break;
    case PREMODIFIESP: {
        VARIABLE_NAME_LIST variables = pkb->getVariablesModifiedBy(arg);
        result = std::vector<VARIABLE_NAME>(variables.begin(), variables.end());
        break;
    }
    case POSTMODIFIESP: {
        auto procs = pkb->getProceduresThatModify(arg);
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    }
    case PRENEXT: {
        stmts = pkb->getNextStatementOf(std::stoi(arg), false);
        result = castToStrVector<>(stmts);
        break;
    }
    case POSTNEXT: {
        stmts = pkb->getPreviousStatementOf(std::stoi(arg), false);
        result = castToStrVector<>(stmts);
        break;
    }
    case PRENEXTT: {
        stmts = pkb->getNextStatementOf(std::stoi(arg), true);
        result = castToStrVector<>(stmts);
        break;
    }
    case POSTNEXTT: {
        stmts = pkb->getPreviousStatementOf(std::stoi(arg), true);
        result = castToStrVector<>(stmts);
        break;
    }
    case PREAFFECTS: {
        auto lines = pkb->getStatementsAffectedBy(std::stoi(arg), false);
        result = castToStrVector<>(lines);
        break;
    }
    case POSTAFFECTS: {
        auto lines = pkb->getStatementsThatAffect(std::stoi(arg), false);
        result = castToStrVector<>(lines);
        break;
    }
    case PREAFFECTST: {
        auto lines = pkb->getStatementsAffectedBy(std::stoi(arg), true);
        result = castToStrVector<>(lines);
        break;
    }
    case POSTAFFECTST: {
        auto lines = pkb->getStatementsThatAffect(std::stoi(arg), true);
        result = castToStrVector<>(lines);
        break;
    }
    case PRENEXTBIP: {
        stmts = pkb->getNextBipStatementOf(std::stoi(arg), false);
        result = castToStrVector<>(stmts);
        break;
    }
    case POSTNEXTBIP: {
        stmts = pkb->getPreviousBipStatementOf(std::stoi(arg), false);
        result = castToStrVector<>(stmts);
        break;
    }
    case PRENEXTBIPT: {
        stmts = pkb->getNextBipStatementOf(std::stoi(arg), true);
        result = castToStrVector<>(stmts);
        break;
    }
    case POSTNEXTBIPT: {
        stmts = pkb->getPreviousBipStatementOf(std::stoi(arg), true);
        result = castToStrVector<>(stmts);
        break;
    }
    case PREAFFECTSBIP: {
        auto lines = pkb->getStatementsAffectedBipBy(std::stoi(arg), false);
        result = castToStrVector<>(lines);
        break;
    }
    case POSTAFFECTSBIP: {
        auto lines = pkb->getStatementsThatAffectBip(std::stoi(arg), false);
        result = castToStrVector<>(lines);
        break;
    }
    case PREAFFECTSBIPT: {
        auto lines = pkb->getStatementsAffectedBipBy(std::stoi(arg), true);
        result = castToStrVector<>(lines);
        break;
    }
    case POSTAFFECTSBIPT: {
        auto lines = pkb->getStatementsThatAffectBip(std::stoi(arg), true);
        result = castToStrVector<>(lines);
        break;
    }
    case PRECALLS: {
        procs = pkb->getProceduresCalledBy(arg, false);
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    }
    case POSTCALLS: {
        procs = pkb->getProcedureThatCalls(arg, false);
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    }
    case PRECALLST: {
        procs = pkb->getProceduresCalledBy(arg, true);
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    }
    case POSTCALLST: {
        procs = pkb->getProcedureThatCalls(arg, true);
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    }
    case ASSIGN_PATTERN_EXACT_SRT: {
        stmts = pkb->getAllAssignmentStatementsThatMatch(arg, patternStr, false);
        result = castToStrVector<>(stmts);
        break;
    }
    case ASSIGN_PATTERN_SUBEXPR_SRT: {
        stmts = pkb->getAllAssignmentStatementsThatMatch(arg, patternStr, true);
        result = castToStrVector<>(stmts);
        break;
    }
    case ASSIGN_PATTERN_WILDCARD_SRT: {
        stmts = pkb->getAllAssignmentStatementsThatMatch(arg, "", true);
        result = castToStrVector<>(stmts);
        break;
    }
    case IF_PATTERN_SRT: {
        stmts = pkb->getAllIfElseStatementsThatMatch(arg, "", true, "", true);
        result = castToStrVector<>(stmts);
        break;
    }
    case WHILE_PATTERN_SRT: {
        stmts = pkb->getAllWhileStatementsThatMatch(arg, "", true);
        result = castToStrVector<>(stmts);
        break;
    }
    default:
        handleError("unknown sub-relation type");
    }
    return result;
}

/**
 * call PKB API methods to retrieve answer for the given relation
 * @param pkb
 * @param subRelationType
 * @return the list of values that make the relation hold
 */
std::vector<std::string> SingleQueryEvaluator::inquirePKBForRelationWildcard(const backend::PKB* pkb,
                                                                             SubRelationType subRelationType,
                                                                             const std::string& patternStr) {
    std::vector<std::string> result;
    STATEMENT_NUMBER_SET stmts;
    PROCEDURE_NAME_SET procs;
    switch (subRelationType) {
    case PREFOLLOWS_WILD:
        stmts = pkb->getAllStatementsThatAreFollowed();
        result = castToStrVector<>(stmts);
        break;
    case POSTFOLLOWS_WILD:
        stmts = pkb->getAllStatementsThatFollows();
        result = castToStrVector<>(stmts);
        break;
    case PREPARENT_WILD:
        stmts = pkb->getStatementsThatHaveDescendants();
        result = castToStrVector<>(stmts);
        break;
    case POSTPARENT_WILD:
        stmts = pkb->getStatementsThatHaveAncestors();
        result = castToStrVector<>(stmts);
        break;
    case USES_WILDCARD:
        stmts = pkb->getStatementsThatUseSomeVariable();
        result = castToStrVector<>(stmts);
        break;
    case USEP_WILDCARD: {
        auto procs = pkb->getProceduresThatUseSomeVariable();
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    }
    case MODIFIESS_WILDCARD:
        stmts = pkb->getStatementsThatModifySomeVariable();
        result = castToStrVector<>(stmts);
        break;
    case MODIFIESP_WILDCARD: {
        auto procs = pkb->getProceduresThatModifySomeVariable();
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    }
    case PRENEXT_WILD:
        stmts = pkb->getAllStatementsWithNext();
        result = castToStrVector<>(stmts);
        break;
    case POSTNEXT_WILD:
        stmts = pkb->getAllStatementsWithPrev();
        result = castToStrVector<>(stmts);
        break;
    case PREAFFECTS_WILD:
        stmts = pkb->getAllStatementsThatAffect();
        result = castToStrVector<>(stmts);
        break;
    case POSTAFFECTS_WILD:
        stmts = pkb->getAllStatementsThatAreAffected();
        result = castToStrVector<>(stmts);
        break;
    case PRENEXTBIP_WILD:
        stmts = pkb->getAllStatementsWithNextBip();
        result = castToStrVector<>(stmts);
        break;
    case POSTNEXTBIP_WILD:
        stmts = pkb->getAllStatementsWithPreviousBip();
        result = castToStrVector<>(stmts);
        break;
    case PREAFFECTSBIP_WILD:
        stmts = pkb->getAllStatementsThatAffectBip();
        result = castToStrVector<>(stmts);
        break;
    case POSTAFFECTSBIP_WILD:
        stmts = pkb->getAllStatementsThatAreAffectedBip();
        result = castToStrVector<>(stmts);
        break;
    case PRECALL_WILD:
        procs = pkb->getAllProceduresThatCallSomeProcedure();
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    case POSTCALL_WILD:
        procs = pkb->getAllCalledProcedures();
        result = std::vector<PROCEDURE_NAME>(procs.begin(), procs.end());
        break;
    case ASSIGN_PATTERN_EXACT_SRT: {
        stmts = pkb->getAllAssignmentStatementsThatMatch("_", patternStr, false);
        result = castToStrVector<>(stmts);
        break;
    }
    case ASSIGN_PATTERN_SUBEXPR_SRT: {
        stmts = pkb->getAllAssignmentStatementsThatMatch("_", patternStr, true);
        result = castToStrVector<>(stmts);
        break;
    }
    case ASSIGN_PATTERN_WILDCARD_SRT: {
        stmts = pkb->getAllAssignmentStatementsThatMatch("_", "", true);
        result = castToStrVector<>(stmts);
        break;
    }
    case IF_PATTERN_SRT: {
        stmts = pkb->getAllIfElseStatementsThatMatch("_", "", true, "", true);
        result = castToStrVector<>(stmts);
        break;
    }
    case WHILE_PATTERN_SRT: {
        stmts = pkb->getAllWhileStatementsThatMatch("_", "", true);
        result = castToStrVector<>(stmts);
        break;
    }
    default:
        handleError("unknown sub-relation type");
    }
    return result;
}

const std::string
SingleQueryEvaluator::inquirePKBForAttribute(const backend::PKB* pkb, ArgType argType, const std::string& arg) {
    switch (argType) {
    case CALL_TO_PROC_SYNONYM:
        return pkb->getProcedureNameFromCallStatement(std::stoi(arg));
    case READ_TO_VAR_SYNONYM:
        return pkb->getVariableNameFromReadStatement(std::stoi(arg));
    case PRINT_TO_VAR_SYNONYM:
        return pkb->getVariableNameFromPrintStatement(std::stoi(arg));
    case INVALID_ARG:
        handleError("cannot retrieve the attribute");
        return "";
    default:
        return arg;
    }
}

/**
 * optimisation: sort the clauses
 * @param pkb
 * @return empty list if any clause is invalid
 */
std::vector<std::vector<CLAUSE_LIST>> SingleQueryEvaluator::getClausesSortedAndGrouped(const backend::PKB* pkb) {
    CLAUSE_LIST clauses;
    for (const auto& relationClause : query.suchThatClauses) {
        clauses.emplace_back(std::get<0>(relationClause), std::get<1>(relationClause),
                             std::get<2>(relationClause), "");
    }

    for (const auto& patternClause : query.patternClauses) {
        clauses.push_back(patternClause);
    }

    for (const auto& rawWithClause : query.withClauses) {
        ARG arg1 = getWithArgType(std::get<0>(rawWithClause));
        ARG arg2 = getWithArgType(std::get<1>(rawWithClause));
        clauses.emplace_back(WITH, arg1, arg2, "");
    }

    // check if there are invalid clauses
    for (const auto& clause : clauses) {
        if (!validateClause(clause)) {
            handleError("the clause is invalid");
            return {};
        }
    }

    // sort and group the clauses
    return optimisation::optimizeQueries(clauses, query.returnCandidates);
}

SubRelationType SingleQueryEvaluator::getSubRelationType(ClauseType clauseType, ArgType argType1, ArgType argType2) {
    if (clauseType == WITH) {
        if ((isNameArg(argType1) && isNameArg(argType2)) || (isNumArg(argType1) && isNumArg(argType2))) {
            return WITH_SRT;
        } else {
            return INVALID;
        }
    } else {
        SubRelationType srt;
        try {
            srt = srt_table.at(clauseType).at(argType1).at(argType2);
        } catch (const std::out_of_range& oor) {
            (void)oor;
            srt = INVALID;
        }
        return srt;
    }
}

ArgType SingleQueryEvaluator::getAttrArgType(ReturnType returnType, const std::string& synonym) {
    if (query.declarationMap.find(synonym) == query.declarationMap.end()) {
        return INVALID_ARG;
    }
    EntityType entityType = query.declarationMap.at(synonym);
    ArgType argType;
    try {
        argType = attr_convert_table.at(entityType).at(returnType);
    } catch (const std::out_of_range& oor) {
        // invalid return type, e.g. stmt s; Select s.varName
        // since only variable, read statement, print statement have .varName
        // a synonym declared as statement type does not have 'varName' attribute
        (void)oor;
        argType = INVALID_ARG;
    }

    return argType;
}

ARG SingleQueryEvaluator::getWithArgType(const ATTR_ARG& attrArg) {
    ArgType rawArgType;
    ReturnType returnType;
    std::string synonym;
    std::tie(rawArgType, returnType, synonym) = attrArg;

    // only prog_line synonym is allowed for with
    if (returnType == DEFAULT_VAL) {
        if (rawArgType == NAME_ENTITY || rawArgType == NUM_ENTITY ||
            (rawArgType == STMT_SYNONYM && query.declarationMap.at(synonym) == PROG_LINE)) {
            return { rawArgType, synonym };
        } else {
            return { INVALID_ARG, "" };
        }
    }
    ArgType argType = getAttrArgType(returnType, synonym);
    return { argType, synonym };
}

bool SingleQueryEvaluator::validateClause(const CLAUSE& clause) {
    // check if anything invalid
    ClauseType clauseType = std::get<0>(clause);
    ArgType argType1 = std::get<1>(clause).first;
    ArgType argType2 = std::get<2>(clause).first;
    if (clauseType == INVALID_CLAUSE_TYPE || argType1 == INVALID_ARG || argType2 == INVALID_ARG) {
        return false;
    }

    // check if found in srt table
    SubRelationType srt = getSubRelationType(clauseType, argType1, argType2);
    return srt != INVALID;
}

/**
 * update synonym candidates with a given Intermediate Result Table
 * @param table : the IRT used
 */
void SingleQueryEvaluator::updateSynonymsWithResultTable(ResultTable& table, bool prune) {
    for (const auto& p : synonym_candidates) {
        table.updateSynonymValueVector(p.first, synonym_candidates[p.first]);
    }
    if (prune) {
        for (const auto& i : synonymCounters) {
            if (returnSynonyms.find(i.first) == returnSynonyms.end() && i.second == 0) {
                // flush and compress
                table.DeleteColumn(i.first);
            }
        }
        table.FlushTable();
    }
}

/**
 * handle exception or error
 */
void SingleQueryEvaluator::handleError(std::string const& msg) {
    logLine(msg);
    failed = true;
}

/**
 * check if a string is the name of a synonym
 * @param pkb : used for synonym candidates initialization
 * @param str : the string to test
 * @return true if it's the name of a synonym, otherwis false
 */
bool SingleQueryEvaluator::isSynonym(std::string const& str) {
    return (query.declarationMap.find(str) != query.declarationMap.end());
}

/**
 * cast vector to vector of strings
 */
template <typename T> std::vector<std::string> castToStrVector(const std::vector<T>& vect) {
    std::vector<std::string> result;
    for (const auto& element : vect) {
        result.push_back(std::to_string(element));
    }
    return result;
}

/**
 * cast set to vector of strings
 */
template <typename T> std::vector<std::string> castToStrVector(const std::unordered_set<T>& s) {
    std::vector<std::string> result;
    for (const auto& element : s) {
        result.push_back(std::to_string(element));
    }
    return result;
}

/**
 * check if vector v contains argument arg
 * @tparam T
 * @param v
 * @param arg
 * @return return true if contained, otherwise false
 */
template <typename T> bool isFoundInVector(const std::vector<T>& v, T arg) {
    return std::find(v.begin(), v.end(), arg) != v.end();
}

/**
 * return the intersection of two vectors lst1 and lst2
 * @tparam T
 * @param lst1 : the first vector
 * @param lst2 : the second vector
 * @return : return the intersection of two vectors
 */
template <typename T>
std::vector<T> vectorIntersection(const std::vector<T>& lst1, const std::vector<T>& lst2) {
    std::vector<T> result;
    std::unordered_set<T> lst1_set(lst1.begin(), lst1.end());
    for (const auto& element : lst2) {
        if (lst1_set.find(element) != lst1_set.end()) {
            result.push_back(element);
        }
    }
    return result;
}
} // namespace queryevaluator
} // namespace qpbackend
