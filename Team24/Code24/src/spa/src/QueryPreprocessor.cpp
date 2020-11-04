#include "QueryPreprocessor.h"

#include "Lexer.h"
#include "Logger.h"
#include "QPTypes.h"
#include "Query.h"

#include <algorithm> // std::find_if
#include <sstream>
#include <tuple>
#include <utility>
#include <vector>


// Constants
const std::string kQppErrorPrefix = "Log[Error-QueryPreprocessor]: ";
const std::string kQppLogWarnPrefix = "Log[WARN-QueryPreprocessor]: ";
const std::string kQppLogInfoPrefix = "Log[INFO-QueryPreprocessor]: ";
const std::string kPatternKeyword = "pattern";

namespace querypreprocessor {

// Declarations.

class State;
// <state of parser, isValidState>
typedef std::pair<State, bool> STATESTATUSPAIR;
// <state of parser, string representation of parsed token(s), isValidState>
typedef std::tuple<State, std::string, qpbackend::ClauseType, bool> STATE_STRING_RESULT_CLAUSE_TYPE_STATUS_QUADRUPLE;
typedef std::tuple<State, qpbackend::ARG, bool> STATE_ARG_RESULT_STATUS_TRIPLE;
void throwIfTokenDoesNotHaveExpectedTokenType(backend::lexer::TokenType expectedTokenType, const TOKEN& token);
static qpbackend::EntityType getEntityTypeFromToken(const TOKEN& token);
State parseSelect(State state);
State parseDeclarations(State state);
STATESTATUSPAIR parseSingleDeclaration(State state);
bool isValidDeclarationDelimiter(const TOKEN& token);
// Declaration for result clause
STATESTATUSPAIR parseResultClause(State state);
STATESTATUSPAIR parseTuple(State state);
STATESTATUSPAIR parseElem(State state);
void parseAttrRef(State state);
void parseAttrName(State state);
// Declaration for 'filtering' clauses
State parseFilteringClauses(State state);
// Declarations for such that clauses
STATESTATUSPAIR parseSingleSuchThatClause(State state);
STATESTATUSPAIR parseRelRef(State state);
STATESTATUSPAIR parseRelationStmtStmtOrLineLine(State state, qpbackend::ClauseType relationClauseType);
STATESTATUSPAIR parseRelationStmtEntOrEntEnt(State state, qpbackend::ClauseType relationType);
STATESTATUSPAIR parseRelationEntEnt(State state, qpbackend::ClauseType relationType);
qpbackend::ARG extractArgFromStmtRefOrLineRefToken(const TOKEN& token, State& state);
bool isStmtRefOrLineRefToken(const TOKEN& token);
STATE_ARG_RESULT_STATUS_TRIPLE parseEntRef(State state);
// Declarations for patten clauses
STATESTATUSPAIR parseSinglePatternClause(State state);
STATESTATUSPAIR parseSingleIfPatternClause(State state);
STATE_STRING_RESULT_CLAUSE_TYPE_STATUS_QUADRUPLE parseExpressionSpec(State state, const TOKEN& synToken);
bool isSynAssignToken(const TOKEN& token, State& state);

// Helper methods

void throwIfTokenDoesNotHaveExpectedTokenType(backend::lexer::TokenType expectedTokenType, const TOKEN& token) {
    if (token.type != expectedTokenType) {
        throw std::runtime_error(kQppErrorPrefix + "throwIfTokenDoesNotHaveExpectedTokenType: Expected a " +
                                 backend::lexer::prettyPrintType(expectedTokenType) + " token, instead received a " +
                                 backend::lexer::prettyPrintType(token.type) + " token");
    }
}

qpbackend::EntityType getEntityTypeFromToken(const TOKEN& token) {
    throwIfTokenDoesNotHaveExpectedTokenType(backend::lexer::TokenType::NAME, token);
    return qpbackend::entityTypeFromString(token.nameValue);
}

/**
 * Encapsulates the state of the parser.
 *
 * That is to say, the State class encapsulates all the information obtained while parsing.
 * Specifically the knowable synonyms declared, values to return and relations to be queried after
 * reading a given amount of tokens supplied.
 *
 * State captures the parser's state any point of time. Thus, it is handy in allowing the parser
 * (a recursive descent parser) to backtrack and apply a new grammar rule.
 *
 * For example, let's try to parse a Uses relation, the QPL's grammar has these followings rules:
 * UsesP : ‘Uses’ ‘(’ entRef ‘,’ entRef ‘)’
 * UsesS : ‘Uses’ ‘(’ stmtRef ‘,’ entRef ‘)’
 *
 * After encountering a 'Uses' and '(' token, the parser is expecting either an `entRef` or
 * `stmtRef`. To handle this, the following can be done:
 * 1. Save the current State object
 * 2. Try to parse the next token(s) with the `entRef` grammar rule.
 * 3. If successful goto VALID
 * 4. Use the saved State object from 1 and parse the next token(s) with the `stmtRef` rule.
 * 5. If successful goto VALID
 * INVALID (6). Parsing is unsuccessful, signal a failure to parse
 * VALID (7). Parsing is successful, continue with the rest of the Uses* rule.
 *
 * State abstracts away all the logic in manipulating the QPL query tokens and Query struct
 * directly. The State will also throw errors or exceptions once it has detected that it is in an
 * invalid state.
 */
class State {
  private:
    qpbackend::Query query;
    TOKENS tokens;
    unsigned int tokenPos{ 0 };
    void logTokenAt(unsigned int tokenPos, std::string methodName) {
        std::stringstream s;
        const TOKEN& token = tokens.at(tokenPos);
        s << kQppLogInfoPrefix << methodName << " Token Position: " << std::to_string(tokenPos)
          << "| value:" << token.nameValue << token.integerValue
          << " type:" << backend::lexer::prettyPrintType(token.type);
        logLine(s.str());
    }


  public:
    State() = default;
    explicit State(const TOKENS& tokens) {
        this->tokens = tokens;
    }

    // Query struct computed properties

    bool hasInvalidQueryDeclarationMap() {
        return std::find_if(query.declarationMap.begin(), query.declarationMap.end(),
                            [](const std::pair<std::string, qpbackend::EntityType>& e) {
                                return e.second == qpbackend::INVALID_ENTITY_TYPE;
                            }) != query.declarationMap.end();
    }

    qpbackend::EntityType getEntityType(const std::string& name) {
        auto iterator = query.declarationMap.find(name);
        if (iterator == query.declarationMap.end()) {
            return qpbackend::INVALID_ENTITY_TYPE;
        }
        return iterator->second;
    }

    // Copy getter(s)

    qpbackend::Query getQuery() {
        return query;
    }

    // Tokens manipulation

    TOKEN peekToken() {
        if (!hasTokensLeftToParse()) {
            throw std::runtime_error(kQppErrorPrefix +
                                     "State::peekToken: There are no more tokens left to peek.");
        }
        logTokenAt(tokenPos, "peekToken");
        return tokens[tokenPos];
    }

    backend::lexer::Token popToken() {
        if (!hasTokensLeftToParse()) {
            throw std::runtime_error(kQppErrorPrefix +
                                     "State::popToken: QueryPreprocessor has "
                                     "not successfully parsed a Query yet, "
                                     "but has run out of tokens to parse.\n" +
                                     query.toString());
        }
        TOKEN tokenToReturn = tokens[tokenPos];
        logTokenAt(tokenPos, "popToken");
        tokenPos += 1;
        return tokenToReturn;
    }

    backend::lexer::Token popUntilNonWhitespaceToken() {
        TOKEN token = popToken();
        while (token.type == backend::lexer::WHITESPACE) {
            token = popToken();
        }
        return token;
    }

    /**
     * If no next non whitespace token exists, it will pop all the tokens till there are no tokens
     * left to pop.
     */
    void popToNextNonWhitespaceToken() {
        while (hasTokensLeftToParse() && peekToken().type == backend::lexer::WHITESPACE) {
            popToken();
        }
    }

    bool popIfCurrentTokenIsWhitespaceToken() {
        if (!hasTokensLeftToParse() || peekToken().type != backend::lexer::WHITESPACE) {
            return false;
        }
        popToken();
        return true;
    }

    bool hasTokensLeftToParse() {
        return tokenPos < tokens.size();
    }

    // Query arg extraction
    qpbackend::ARG getArgFromSynonymString(const std::string& synonymString) {
        auto iterator = query.declarationMap.find(synonymString);
        if (iterator == query.declarationMap.end()) {
            logLine(kQppErrorPrefix + "getArgFromSynonymString: declarationMap does not contain synonym: " + synonymString);
            return qpbackend::ARG(qpbackend::ArgType::INVALID_ARG, synonymString);
        }
        qpbackend::EntityType entityType = iterator->second;
        switch (entityType) {
        case qpbackend::IF:
        case qpbackend::ASSIGN:
        case qpbackend::PRINT:
        case qpbackend::CALL:
        case qpbackend::WHILE:
        case qpbackend::READ:
        case qpbackend::PROG_LINE:
        case qpbackend::STMT: {
            return { qpbackend::STMT_SYNONYM, synonymString };
        }
        case qpbackend::VARIABLE: {
            return { qpbackend::VAR_SYNONYM, synonymString };
        }
        case qpbackend::CONSTANT: {
            return { qpbackend::CONST_SYNONYM, synonymString };
        }
        case qpbackend::PROCEDURE: {
            return { qpbackend::PROC_SYNONYM, synonymString };
        }
        case qpbackend::INVALID_ENTITY_TYPE:
            return { qpbackend::INVALID_ARG, synonymString };
        }
    }


    // Query struct manipulation

    void addSynonymToQueryDeclarationMap(qpbackend::EntityType entityType, const TOKEN& token) {
        throwIfTokenDoesNotHaveExpectedTokenType(backend::lexer::TokenType::NAME, token);
        if (query.declarationMap.find(token.nameValue) != query.declarationMap.end()) {
            query.declarationMap[token.nameValue] = qpbackend::INVALID_ENTITY_TYPE;
            throw std::runtime_error(kQppErrorPrefix + "State::addSynonymToQueryDeclarationMap: Synonym " +
                                     token.nameValue + " has already been declared.");
        }

        query.declarationMap.insert(std::pair<std::string, qpbackend::EntityType>(token.nameValue, entityType));
    }

    void addSynonymToReturn(const TOKEN& token) {
        throwIfTokenDoesNotHaveExpectedTokenType(backend::lexer::TokenType::NAME, token);
        if (query.declarationMap.find(token.nameValue) == query.declarationMap.end()) {
            throw std::runtime_error(kQppErrorPrefix + "State::addSynonymToReturn: Cannot return values for synonym " +
                                     token.nameValue + " as it has not been declared.");
        }

        qpbackend::ReturnType returnType = qpbackend::DEFAULT_VAL;
        query.returnCandidates.emplace_back(returnType, token.nameValue);
    }

    void addSuchThatClause(qpbackend::ClauseType relationType, const qpbackend::ARG& arg1, const qpbackend::ARG& arg2) {
        query.suchThatClauses.emplace_back(relationType, arg1, arg2);
    }

    void addPatternClause(qpbackend::ClauseType patternType,
                          const qpbackend::ARG& synonym,
                          const qpbackend::ARG& variableName,
                          const std::string& expressionSpec) {

        logLine(kQppLogInfoPrefix + "addPatternClause: " + qpbackend::prettyPrintClauseType(patternType) +
                " " + qpbackend::prettyPrintArg(synonym) + " " +
                qpbackend::prettyPrintArg(variableName) + " " + expressionSpec);
        qpbackend::ARG invalidSyn = { qpbackend::INVALID_ARG, synonym.second };
        // Validate synonym is declared
        if (query.declarationMap.find(synonym.second) == query.declarationMap.end()) {
            addPatternClauseUnchecked(patternType, invalidSyn, variableName, expressionSpec);
            return;
        }

        switch (patternType) {
        case qpbackend::ASSIGN_PATTERN_WILDCARD:
        case qpbackend::ASSIGN_PATTERN_EXACT:
        case qpbackend::ASSIGN_PATTERN_SUB_EXPR: {
            if (query.declarationMap.at(synonym.second) != qpbackend::ASSIGN) {
                break;
            }
            addPatternClauseUnchecked(patternType, synonym, variableName, expressionSpec);
            return;
        }
        case qpbackend::IF_PATTERN:
            if (query.declarationMap.at(synonym.second) != qpbackend::IF || expressionSpec != "_") {
                break;
            }
            addPatternClauseUnchecked(patternType, synonym, variableName, expressionSpec);
            return;
        case qpbackend::WHILE_PATTERN:
            if (query.declarationMap.at(synonym.second) != qpbackend::WHILE || expressionSpec != "_") {
                break;
            }
            addPatternClauseUnchecked(patternType, synonym, variableName, expressionSpec);
            return;
        case qpbackend::FOLLOWS:
        case qpbackend::FOLLOWST:
        case qpbackend::PARENT:
        case qpbackend::PARENTT:
        case qpbackend::USES:
        case qpbackend::MODIFIES:
        case qpbackend::CALLS:
        case qpbackend::CALLST:
        case qpbackend::NEXT:
        case qpbackend::NEXTT:
        case qpbackend::AFFECTS:
        case qpbackend::AFFECTST:
        case qpbackend::WITH:
        case qpbackend::INVALID_CLAUSE_TYPE:
            break;
        }
        addPatternClauseUnchecked(patternType, invalidSyn, variableName, expressionSpec);
    }


    void addPatternClauseUnchecked(qpbackend::ClauseType patternType,
                                   const qpbackend::ARG& assignmentSynonym,
                                   const qpbackend::ARG& variableName,
                                   const std::string& expressionSpec) {
        query.patternClauses.emplace_back(patternType, assignmentSynonym, variableName, expressionSpec);
    }


    void setReturnValueToBoolean() {
        query.returnCandidates = { { qpbackend::ReturnType::BOOLEAN, "BOOLEAN" } };
    }


}; // namespace querypreprocessor

// Parser / Business logic methods

/**
 * select-cl : declaration* ‘Select’ synonym ([ suchthat-cl ] | [ pattern-cl ])*
 */
State parseSelect(State state) {
    // If parseDeclaration fails due to re-declaration, an error should be thrown.
    state = parseDeclarations(state);
    logLine(kQppLogInfoPrefix + "parseSelect: Query state after parsing declaration*" +
            state.getQuery().toString());
    const TOKEN& selectToken = state.popUntilNonWhitespaceToken();
    if (selectToken.type != backend::lexer::NAME || selectToken.nameValue != "Select") {
        // Irrecoverable syntax error, only 'Select' tokens come after declaration*. There is no
        // way to backtrack.
        throw std::runtime_error(kQppErrorPrefix +
                                 "parseSelect: Encountered "
                                 "\"" +
                                 selectToken.nameValue +
                                 "\""
                                 " while parsing, when \"Select\" is expected instead.");
    }

    bool isQueryValid;
    std::tie(state, isQueryValid) = parseResultClause(state);
    // Sanity semantic check for invalid declaration map.
    if (state.hasInvalidQueryDeclarationMap() || !isQueryValid) {
        // Received instructions from TA team to redefine handling redeclaration semantic error.
        // Instead of writing False to the results, nothing will be written.
        // Effectively `assign a, a; Select BOOLEAN` should result in '' being projected to the
        // autotester.
        return {};
    }
    return parseFilteringClauses(state);
}

/**
 * declaration*
 *
 * Optimistically parse for declarations until an invalid state is reached. When that happens,
 * return the the most recent valid state.
 */
State parseDeclarations(State state) {
    bool isValidState = true;
    State tempState = state;

    // Optimistically parse for declarations until an invalid state is reached. Only update the
    // `state` with the newly obtained `tempState` if it is valid.
    while (isValidState) {
        state = tempState;
        std::tie(tempState, isValidState) = parseSingleDeclaration(state);
    }
    return state;
}


/**
 * declaration : design-entity synonym (‘,’ synonym)* ‘;’
 *
 * @return <state of parser, isStateInvalid> after attempting to parse a single declaration.
 */
STATESTATUSPAIR parseSingleDeclaration(State state) {
    const TOKEN& designEntity = state.popUntilNonWhitespaceToken();
    if (designEntity.type != backend::lexer::NAME || !qpbackend::isEntityString(designEntity.nameValue)) {
        return STATESTATUSPAIR(state, false);
    }
    qpbackend::EntityType entityType = getEntityTypeFromToken(designEntity);

    TOKEN synonym = state.popUntilNonWhitespaceToken();
    TOKEN delimiter = state.popUntilNonWhitespaceToken();
    logLine(kQppLogInfoPrefix + "parseSingleDeclaration:\n Synonym: " + synonym.nameValue +
            "\nDelimiter type:" + backend::lexer::prettyPrintType(delimiter.type));
    // Handles (‘,’ synonym)* ‘;’
    while (isValidDeclarationDelimiter(delimiter)) {
        // Calling this will throw an error if the synonym is invalid.
        // Case 1.
        // Semantic error: Redeclaring a synonym.
        // Case 2.
        // Irrecoverable syntax error: NAME token expected but not encountered.
        // There is no QPL grammar where a non-NAME token comes after a ',' or a design-entity.
        state.addSynonymToQueryDeclarationMap(entityType, synonym);
        // ';' is the last token of a declaration. Return <state, true> when it is encountered.
        if (delimiter.type == backend::lexer::SEMICOLON) {
            return STATESTATUSPAIR(state, true);
        }
        synonym = state.popUntilNonWhitespaceToken();
        delimiter = state.popUntilNonWhitespaceToken();
    }

    // Encountered an invalid delimiter, return <state, false> to signal this is an invalid state.
    return STATESTATUSPAIR(state, false);
}

/**
 * Checks for the following tokens (',' | ';').
 *
 * In parsing a declaration, a ',' is expected as a delimiter between synonyms, or a ';' is
 * expected as a terminator for the declaration. No other delimeter is expected.
 * @return True if it's one of the tokens, otherwise false.
 */
bool isValidDeclarationDelimiter(const TOKEN& token) {
    return token.type == backend::lexer::SEMICOLON || token.type == backend::lexer::COMMA;
}

/**
 * result-cl : tuple | ‘BOOLEAN’
 */
STATESTATUSPAIR parseResultClause(State state) {
    STATESTATUSPAIR parseTupleStateStatusPair = parseTuple(state);
    if (parseTupleStateStatusPair.second) {
        logLine(kQppLogInfoPrefix +
                "parseResultClause: parsing tuple is successful, query should return tuples");
        return parseTupleStateStatusPair;
    }

    // Parse terminal 'BOOLEAN'
    TOKEN returnValueToken = state.popUntilNonWhitespaceToken();
    if (returnValueToken.type != backend::lexer::NAME || returnValueToken.nameValue != "BOOLEAN") {
        logLine(kQppLogWarnPrefix + "parseResultClause: Unable to parse tuple | 'BOOLEAN' Found:" +
                backend::lexer::prettyPrintType(returnValueToken.type));
        return { state, false };
    }

    state.setReturnValueToBoolean();
    return { state, true };
}

/**
 * tuple: elem | ‘<’ elem ( ‘,’ elem )* ‘>’
 */
STATESTATUSPAIR parseTuple(State state) {
    bool isValidState;
    State backupState = state;

    std::tie(state, isValidState) = parseElem(state);
    if (isValidState) {
        return { state, isValidState };
    }

    // ‘<’ elem ( ‘,’ elem )* ‘>’
    state = backupState;
    TOKEN l_arrow = state.popUntilNonWhitespaceToken();
    if (l_arrow.type != backend::lexer::LT) {
        return { state, false };
    }
    std::tie(state, isValidState) = parseElem(state);
    if (!isValidState) {
        return { state, false };
    }
    while (isValidState) {
        backupState = state;
        TOKEN comma = state.popUntilNonWhitespaceToken();
        if (comma.type != backend::lexer::COMMA) {
            break;
        }
        std::tie(state, isValidState) = parseElem(state);
    }
    state = backupState;
    TOKEN r_arrow = state.popUntilNonWhitespaceToken();
    if (r_arrow.type != backend::lexer::GT) {
        return { state, false };
    }

    return { state, true };
}

/**
 * elem : synonym | attrRef
 * Implemented as `elem: attrRef | synonym` which is easier and still correct.
 */
STATESTATUSPAIR parseElem(State state) {
    // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/338): Parse attrRef before synonym
    try {
        const TOKEN& synonymToken = state.popUntilNonWhitespaceToken();
        state.addSynonymToReturn(synonymToken);
        logLine(kQppLogInfoPrefix + "parseTuple: parsed query: " + state.getQuery().toString());
        return { state, true };
    } catch (const std::runtime_error& e) {
        logLine(e.what());
        return { state, false };
    }
}

/**
 * attrRef : synonym ‘.’ attrName
 *
 */
void parseAttrRef(State state) {
}

/**
 * attrName : ‘procName’| ‘varName’ | ‘value’ | ‘stmt#’
 */
void parseAttrName(State state) {
}

/**
 * ([ suchthat-cl ] | [ pattern-cl ])*
 */
State parseFilteringClauses(State state) {
    state.popToNextNonWhitespaceToken();
    if (!state.hasTokensLeftToParse()) return state;
    State tempState = state;
    bool isParseSuchThatValid = true;
    bool isParsePatternValid = true;
    bool isParsePatternExtendedValid = true;
    while (state.hasTokensLeftToParse() &&
           (isParseSuchThatValid || isParsePatternValid || isParsePatternExtendedValid)) {
        std::tie(tempState, isParseSuchThatValid) = parseSingleSuchThatClause(state);
        if (isParseSuchThatValid) {
            state = tempState;
        }
        std::tie(tempState, isParsePatternValid) = parseSinglePatternClause(state);
        if (isParsePatternValid) {
            state = tempState;
        }

        std::tie(tempState, isParsePatternExtendedValid) = parseSingleIfPatternClause(state);
        if (isParsePatternExtendedValid) {
            state = tempState;
        }
        state.popToNextNonWhitespaceToken();
    }
    if (!isParsePatternValid && !isParseSuchThatValid && !isParsePatternExtendedValid) {
        throw std::runtime_error(kQppErrorPrefix + "parseFilteringClauses: Unable to parse such that or pattern clauses\n" +
                                 state.getQuery().toString());
    }
    return state;
}

/**
 * suchthat-cl : ‘such that’ relRef
 * @return <state of parser, isStateInvalid>
 */
STATESTATUSPAIR parseSingleSuchThatClause(State state) {
    state.popToNextNonWhitespaceToken();
    if (!state.hasTokensLeftToParse()) return STATESTATUSPAIR(state, false);
    TOKEN suchToken = state.popUntilNonWhitespaceToken();
    if (suchToken.type != backend::lexer::NAME || suchToken.nameValue != "such") {
        return STATESTATUSPAIR(state, false);
    }
    TOKEN thatToken = state.popUntilNonWhitespaceToken();
    if (thatToken.type != backend::lexer::NAME || thatToken.nameValue != "that") {
        return STATESTATUSPAIR(state, false);
    }
    return parseRelRef(state);
}

/**
 * relRef : Follows | FollowsT | Parent | ParentT | UsesS | UsesP | ModifiesS | ModifiesP | Calls |
 * CallsT | Next | NextT | Affects | AffectsT
 * @return <state of parser, isStateInvalid>
 */
STATESTATUSPAIR parseRelRef(State state) {
    std::stringstream stringstream;
    TOKEN keywordToken = state.popUntilNonWhitespaceToken();
    if (keywordToken.type != backend::lexer::NAME) {
        return STATESTATUSPAIR(state, false);
    }
    stringstream << keywordToken.nameValue;
    // A "*" may immediately follow the keyword
    if (state.peekToken().type == backend::lexer::TokenType::MULT) {
        state.popToken();
        stringstream << "*";
    }
    std::string possibleRelationString = stringstream.str();
    if (!qpbackend::isRelationClauseString(possibleRelationString)) {
        return STATESTATUSPAIR(state, false);
    }
    qpbackend::ClauseType relationClauseType = qpbackend::relationClauseTypeFromString(possibleRelationString);
    switch (relationClauseType) {
    case qpbackend::FOLLOWS:
    case qpbackend::FOLLOWST:
    case qpbackend::PARENT:
    case qpbackend::PARENTT:
    case qpbackend::NEXT:
    case qpbackend::NEXTT:
    case qpbackend::AFFECTS:
    case qpbackend::AFFECTST:
        return parseRelationStmtStmtOrLineLine(state, relationClauseType);
    case qpbackend::USES:
    case qpbackend::MODIFIES:
        // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/335): Currently
        //  `Modifies(_,...)` and `Uses(_,...)` are coded as syntactic errors, this should
        //  not be the case.
        return parseRelationStmtEntOrEntEnt(state, relationClauseType);
    case qpbackend::CALLS:
    case qpbackend::CALLST:
        return parseRelationEntEnt(state, relationClauseType);
    case qpbackend::ASSIGN_PATTERN_EXACT:
    case qpbackend::ASSIGN_PATTERN_SUB_EXPR:
    case qpbackend::ASSIGN_PATTERN_WILDCARD:
    case qpbackend::IF_PATTERN:
    case qpbackend::WHILE_PATTERN:
    case qpbackend::WITH:
    case qpbackend::INVALID_CLAUSE_TYPE:
        return STATESTATUSPAIR(state, false);
    }
}

/**
 * Follows : ... ‘(’ stmtRef ‘,’ stmtRef ‘)’
 * FollowsT : ... ‘(’ stmtRef ‘,’ stmtRef ‘)’
 * Parent : ... ‘(’ stmtRef ‘,’ stmtRef ‘)’
 * ParentT : ... ‘(’ stmtRef ‘,’ stmtRef ‘)’
 * Next : ... ‘(’ lineRef ‘,’ lineRef ‘)’
 * NextT : ... ‘(’ lineRef ‘,’ lineRef ‘)’
 *
 * Note that stmtRef and lineRef have the same grammar
 * stmtRef : synonym | ‘_’ | INTEGER
 * lineRef : synonym | ‘_’ | INTEGER
 * @return <state of parser, isStateInvalid>
 */
STATESTATUSPAIR parseRelationStmtStmtOrLineLine(State state, qpbackend::ClauseType relationClauseType) {
    TOKEN lParenToken = state.popUntilNonWhitespaceToken();
    if (lParenToken.type != backend::lexer::LPAREN || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationStmtStmtOrLineLine: Expected more tokens but finished "
                "consuming tokens or LPAREN token not found. Obtained " +
                backend::lexer::prettyPrintType(lParenToken.type));
        return STATESTATUSPAIR(state, false);
    }
    TOKEN stmt1Token = state.popUntilNonWhitespaceToken();
    if (!isStmtRefOrLineRefToken(stmt1Token) || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationStmtStmtOrLineLine: Expected more tokens but finished "
                "consuming tokens or STMT token not found. Obtained " +
                backend::lexer::prettyPrintType(stmt1Token.type));
        return STATESTATUSPAIR(state, false);
    }
    TOKEN commaToken = state.popUntilNonWhitespaceToken();
    if (commaToken.type != backend::lexer::COMMA || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationStmtStmtOrLineLine: Expected more tokens but finished "
                "consuming tokens or COMMA token not found. Obtained " +
                backend::lexer::prettyPrintType(commaToken.type));
        return STATESTATUSPAIR(state, false);
    }
    TOKEN stmt2Token = state.popUntilNonWhitespaceToken();
    if (!isStmtRefOrLineRefToken(stmt2Token) || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationStmtStmtOrLineLine: Expected more tokens but finished "
                "consuming tokens or STMT token not found. Obtained " +
                backend::lexer::prettyPrintType(stmt2Token.type));
        return STATESTATUSPAIR(state, false);
    }
    TOKEN rParenToken = state.popUntilNonWhitespaceToken();
    if (rParenToken.type != backend::lexer::RPAREN) {
        logLine(kQppLogWarnPrefix + "parseRelationStmtStmtOrLineLine: RPAREN token not found. Obtained " +
                backend::lexer::prettyPrintType(rParenToken.type));
        return STATESTATUSPAIR(state, false);
    }
    state.addSuchThatClause(relationClauseType, extractArgFromStmtRefOrLineRefToken(stmt1Token, state),
                            extractArgFromStmtRefOrLineRefToken(stmt2Token, state));
    return STATESTATUSPAIR(state, true);
}


// Helper methods for stmtRef : synonym | ‘_’ | INTEGER and lineRef : synonym | ‘_’ | INTEGER

qpbackend::ARG extractArgFromStmtRefOrLineRefToken(const TOKEN& token, State& state) {
    switch (token.type) {
    case backend::lexer::INTEGER:
        return { qpbackend::ArgType::NUM_ENTITY, token.integerValue };
    case backend::lexer::UNDERSCORE:
        return { qpbackend::ArgType::WILDCARD, "_" };
    case backend::lexer::NAME:
        return state.getArgFromSynonymString(token.nameValue);
    default:
        throw std::invalid_argument(kQppErrorPrefix + "extractArgFromStmtRefOrLineRefToken: A non StmtRef or LineRef token is supplied of type:" +
                                    backend::lexer::prettyPrintType(token.type));
    }
}

bool isStmtRefOrLineRefToken(const TOKEN& token) {
    switch (token.type) {
    case backend::lexer::INTEGER:
    case backend::lexer::UNDERSCORE:
    case backend::lexer::NAME:
        return true;
    default:
        return false;
    }
};

/**
 * UsesS : ... ‘(’ stmtRef ‘,’ entRef ‘)’
 * UsesP : ... ‘(’ entRef ‘,’ entRef ‘)’
 * ModifiesS : ... ‘(’ stmtRef ‘,’ entRef ‘)’
 * ModifiesP : ... ‘(’ entRef ‘,’ entRef ‘)’
 * @return <state of parser, isStateInvalid>
 */
STATESTATUSPAIR parseRelationStmtEntOrEntEnt(State state, qpbackend::ClauseType relationType) {
    // Mutable variables in function.
    qpbackend::ARG stmtOrEntArg;
    bool isValidState = true;
    qpbackend::ARG entArg;
    // state argument is also mutable.

    TOKEN lParenToken = state.popUntilNonWhitespaceToken();
    if (lParenToken.type != backend::lexer::LPAREN || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationStmtEntOrEntEnt: Expected more tokens but finished "
                "consuming tokens or LPAREN token not found. Obtained " +
                backend::lexer::prettyPrintType(lParenToken.type));
        return STATESTATUSPAIR(state, false);
    }

    // Check the current token to see if it should be handled as an EntRef or StmtRef
    state.popIfCurrentTokenIsWhitespaceToken();
    if (!state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix + "parseRelationStmtEntOrEntEnt: Expected more tokens but "
                                    "finished consuming tokens");
        return STATESTATUSPAIR(state, false);
    }
    TOKEN stmtOrEntToken = state.peekToken();
    if (isStmtRefOrLineRefToken(stmtOrEntToken)) {
        state.popToken();
        stmtOrEntArg = extractArgFromStmtRefOrLineRefToken(stmtOrEntToken, state);
    } else {
        std::tie(state, stmtOrEntArg, isValidState) = parseEntRef(state);
    }
    if (!isValidState || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationStmtEntOrEntEnt: Expected more tokens but finished "
                "consuming tokens or STMT | ENT not found. Obtained " +
                backend::lexer::prettyPrintType(stmtOrEntToken.type));
        return STATESTATUSPAIR(state, false);
    }

    TOKEN commaToken = state.popUntilNonWhitespaceToken();
    if (commaToken.type != backend::lexer::COMMA || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationStmtEntOrEntEnt: Expected more tokens but finished "
                "consuming tokens or COMMA token not found. Obtained " +
                backend::lexer::prettyPrintType(commaToken.type));
        return STATESTATUSPAIR(state, false);
    }

    std::tie(state, entArg, isValidState) = parseEntRef(state);
    if (!isValidState || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationStmtEntOrEntEnt: Expected more tokens but finished "
                "consuming tokens or ENT not found. Obtained token of value:" +
                entArg.second);
        return STATESTATUSPAIR(state, false);
    }

    TOKEN rParenToken = state.popUntilNonWhitespaceToken();
    if (rParenToken.type != backend::lexer::RPAREN) {
        logLine(kQppLogWarnPrefix + "parseRelationStmtEntOrEntEnt: RPAREN token not found. Obtained " +
                backend::lexer::prettyPrintType(rParenToken.type));
        return STATESTATUSPAIR(state, false);
    }
    state.addSuchThatClause(relationType, stmtOrEntArg, entArg);
    return STATESTATUSPAIR(state, true);
}

/**
 * Calls* : ... ‘(’ entRef ‘,’ entRef ‘)’
 * Calls : ... ‘(’ entRef ‘,’ entRef ‘)’
 * @return <state of parser, isStateInvalid>
 */
STATESTATUSPAIR parseRelationEntEnt(State state, qpbackend::ClauseType relationType) {
    // Mutable variables in function.
    qpbackend::ARG entArg1;
    bool isValidState = true;
    qpbackend::ARG entArg2;
    // state argument is also mutable.

    TOKEN lParenToken = state.popUntilNonWhitespaceToken();
    if (lParenToken.type != backend::lexer::LPAREN || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationEntEnt: Expected more tokens but finished "
                "consuming tokens or LPAREN token not found. Obtained " +
                backend::lexer::prettyPrintType(lParenToken.type));
        return STATESTATUSPAIR(state, false);
    }

    state.popIfCurrentTokenIsWhitespaceToken();
    if (!state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix + "parseRelationEntEnt: Expected more tokens but "
                                    "finished consuming tokens");
        return STATESTATUSPAIR(state, false);
    }
    TOKEN entToken = state.peekToken();
    std::tie(state, entArg1, isValidState) = parseEntRef(state);
    if (!isValidState || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationEntEnt: Expected more tokens but finished "
                "consuming tokens or ENT not found. Obtained " +
                backend::lexer::prettyPrintType(entToken.type));
        return STATESTATUSPAIR(state, false);
    }

    TOKEN commaToken = state.popUntilNonWhitespaceToken();
    if (commaToken.type != backend::lexer::COMMA || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationEntEnt: Expected more tokens but finished "
                "consuming tokens or COMMA token not found. Obtained " +
                backend::lexer::prettyPrintType(commaToken.type));
        return STATESTATUSPAIR(state, false);
    }

    std::tie(state, entArg2, isValidState) = parseEntRef(state);
    if (!isValidState || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix +
                "parseRelationEntEnt: Expected more tokens but finished "
                "consuming tokens or ENT not found. Obtained token of value:" +
                entArg2.second);
        return STATESTATUSPAIR(state, false);
    }

    TOKEN rParenToken = state.popUntilNonWhitespaceToken();
    if (rParenToken.type != backend::lexer::RPAREN) {
        logLine(kQppLogWarnPrefix + "parseRelationEntEnt: RPAREN token not found. Obtained " +
                backend::lexer::prettyPrintType(rParenToken.type));
        return STATESTATUSPAIR(state, false);
    }
    state.addSuchThatClause(relationType, entArg1, entArg2);
    return STATESTATUSPAIR(state, true);
}

/**
 * entRef : synonym | ‘_’ | ‘"’ IDENT ‘"’
 * @return <state of parser, string representation of valid ent ref, isValidState>
 */
STATE_ARG_RESULT_STATUS_TRIPLE parseEntRef(State state) {
    TOKEN firstToken = state.popUntilNonWhitespaceToken();
    // Handle (synonym | '_')
    switch (firstToken.type) {
    case backend::lexer::NAME: {
        qpbackend::ARG arg = state.getArgFromSynonymString(firstToken.nameValue);
        return STATE_ARG_RESULT_STATUS_TRIPLE(state, arg, true);
    }
    case backend::lexer::UNDERSCORE: {
        return STATE_ARG_RESULT_STATUS_TRIPLE(state, qpbackend::ARG(qpbackend::ArgType::WILDCARD, "_"), true);
    }
    case backend::lexer::LBRACE:
    case backend::lexer::RBRACE:
    case backend::lexer::LPAREN:
    case backend::lexer::RPAREN:
    case backend::lexer::SEMICOLON:
    case backend::lexer::COMMA:
    case backend::lexer::DOUBLE_QUOTE:
    case backend::lexer::SINGLE_EQ:
    case backend::lexer::NOT:
    case backend::lexer::ANDAND:
    case backend::lexer::OROR:
    case backend::lexer::EQEQ:
    case backend::lexer::NEQ:
    case backend::lexer::GT:
    case backend::lexer::GTE:
    case backend::lexer::LT:
    case backend::lexer::LTE:
    case backend::lexer::PLUS:
    case backend::lexer::MINUS:
    case backend::lexer::MULT:
    case backend::lexer::DIV:
    case backend::lexer::MOD:
    case backend::lexer::INTEGER:
    case backend::lexer::WHITESPACE:
    case backend::lexer::PERIOD:
    case backend::lexer::HASH:
        break;
    }

    // Handle ‘"’ IDENT ‘"’
    if (firstToken.type != backend::lexer::DOUBLE_QUOTE || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix + "parseEntRef: Either ran out of tokens or expected DOUBLE_QUOTE Token, found " +
                backend::lexer::prettyPrintType(firstToken.type));
        return STATE_ARG_RESULT_STATUS_TRIPLE(state, qpbackend::ARG(qpbackend::INVALID_ARG, ""), false);
    }
    TOKEN identToken = state.popToken();
    if (identToken.type != backend::lexer::NAME || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix + "parseEntRef: Either ran out of tokens or expected NAME Token, found " +
                backend::lexer::prettyPrintType(identToken.type));
        return STATE_ARG_RESULT_STATUS_TRIPLE(state, qpbackend::ARG(qpbackend::INVALID_ARG, ""), false);
    }
    TOKEN closingDoubleQuoteToken = state.popToken();
    state.popIfCurrentTokenIsWhitespaceToken();
    if (closingDoubleQuoteToken.type != backend::lexer::DOUBLE_QUOTE || !state.hasTokensLeftToParse()) {
        logLine(kQppLogWarnPrefix + "parseEntRef: Either ran out of tokens or expected DOUBLE_QUOTE Token, found " +
                backend::lexer::prettyPrintType(closingDoubleQuoteToken.type));
        return STATE_ARG_RESULT_STATUS_TRIPLE(state, qpbackend::ARG(qpbackend::INVALID_ARG, ""), false);
    }
    return STATE_ARG_RESULT_STATUS_TRIPLE(state, qpbackend::ARG(qpbackend::NAME_ENTITY, identToken.nameValue), true);
}

/**
 * if : syn-if ‘(’ entRef ‘,’ ‘_’ ‘,’ ‘_’ ‘)’
 * Semantic: // syn-if must be of type ‘if’
 * @param state
 * @return
 */
STATESTATUSPAIR parseSingleIfPatternClause(State state) {
    state.popToNextNonWhitespaceToken();
    if (!state.hasTokensLeftToParse()) return STATESTATUSPAIR(state, false);
    // Mutable vars
    bool isValidState;
    qpbackend::ClauseType clauseType;
    qpbackend::ARG entRefArg;
    std::string expressionSpec;
    // state is also mutable
    logLine(kQppLogInfoPrefix + "parseSingleIfPatternClause: Begin");
    TOKEN patternToken = state.popUntilNonWhitespaceToken();
    state.popIfCurrentTokenIsWhitespaceToken();
    if (patternToken.type != backend::lexer::NAME || patternToken.nameValue != kPatternKeyword ||
        !state.hasTokensLeftToParse()) {
        return STATESTATUSPAIR(state, false);
    }

    TOKEN synIfToken = state.popUntilNonWhitespaceToken();
    state.popIfCurrentTokenIsWhitespaceToken();
    if (synIfToken.type != backend::lexer::NAME || !state.hasTokensLeftToParse()) {
        return STATESTATUSPAIR(state, false);
    }

    TOKEN lParenToken = state.popUntilNonWhitespaceToken();
    if (lParenToken.type != backend::lexer::LPAREN || !state.hasTokensLeftToParse()) {
        return STATESTATUSPAIR(state, false);
    }

    std::tie(state, entRefArg, isValidState) = parseEntRef(state);
    if (!isValidState) return STATESTATUSPAIR(state, false);

    TOKEN firstCommaToken = state.popUntilNonWhitespaceToken();
    if (firstCommaToken.type != backend::lexer::COMMA || !state.hasTokensLeftToParse()) {
        return STATESTATUSPAIR(state, false);
    }

    TOKEN firstUnderscoreToken = state.popUntilNonWhitespaceToken();
    if (firstUnderscoreToken.type != backend::lexer::UNDERSCORE || !state.hasTokensLeftToParse()) {
        return STATESTATUSPAIR(state, false);
    }

    TOKEN secondCommaToken = state.popUntilNonWhitespaceToken();
    if (secondCommaToken.type != backend::lexer::COMMA || !state.hasTokensLeftToParse()) {
        return STATESTATUSPAIR(state, false);
    }

    TOKEN secondUnderscoreToken = state.popUntilNonWhitespaceToken();
    if (secondUnderscoreToken.type != backend::lexer::UNDERSCORE || !state.hasTokensLeftToParse()) {
        return STATESTATUSPAIR(state, false);
    }

    TOKEN rParenToken = state.popUntilNonWhitespaceToken();
    if (rParenToken.type != backend::lexer::RPAREN) {
        return STATESTATUSPAIR(state, false);
    }

    state.addPatternClause(qpbackend::IF_PATTERN,
                           state.getArgFromSynonymString(synIfToken.nameValue), entRefArg, "_");
    state.popIfCurrentTokenIsWhitespaceToken();
    logLine(kQppLogInfoPrefix + "parseSinglePatternIfClause: Success End");
    return STATESTATUSPAIR(state, true);
}


/**
 * pattern-cl : ‘pattern’ (assign | while)
 * assign : syn-assign ‘(‘ entRef ‘,’ expression-spec ‘)’
 * entRef : synonym | ‘_’ | ‘"’ IDENT ‘"’
 * expression-spec :  ‘"‘ expr’"’ | ‘_’ ‘"’ expr ‘"’ ‘_’ | ‘_’
 * while : syn-while ‘(’ entRef ‘,’ ‘_’ ‘)’
 */
STATESTATUSPAIR parseSinglePatternClause(State state) {
    state.popToNextNonWhitespaceToken();
    if (!state.hasTokensLeftToParse()) return STATESTATUSPAIR(state, false);
    // Mutable vars
    bool isValidState;
    qpbackend::ClauseType clauseType;
    qpbackend::ARG entRefArg;
    std::string expressionSpec;
    // state is also mutable
    logLine(kQppLogInfoPrefix + "parseSinglePatternClause: Begin");
    TOKEN patternToken = state.popUntilNonWhitespaceToken();
    state.popIfCurrentTokenIsWhitespaceToken();
    if (patternToken.type != backend::lexer::NAME || patternToken.nameValue != kPatternKeyword ||
        !state.hasTokensLeftToParse()) {
        return STATESTATUSPAIR(state, false);
    }

    TOKEN synAssignToken = state.popUntilNonWhitespaceToken();
    state.popIfCurrentTokenIsWhitespaceToken();
    if (synAssignToken.type != backend::lexer::NAME || !state.hasTokensLeftToParse()) {
        return STATESTATUSPAIR(state, false);
    }

    TOKEN lParenToken = state.popUntilNonWhitespaceToken();
    if (lParenToken.type != backend::lexer::LPAREN || !state.hasTokensLeftToParse()) {
        return STATESTATUSPAIR(state, false);
    }

    std::tie(state, entRefArg, isValidState) = parseEntRef(state);
    if (!isValidState) return STATESTATUSPAIR(state, false);

    TOKEN commaToken = state.popUntilNonWhitespaceToken();
    if (commaToken.type != backend::lexer::COMMA || !state.hasTokensLeftToParse()) {
        return STATESTATUSPAIR(state, false);
    }

    bool isSyntacticallyValid;
    std::tie(state, expressionSpec, clauseType, isSyntacticallyValid) =
    parseExpressionSpec(state, synAssignToken);
    if (!isSyntacticallyValid) return STATESTATUSPAIR(state, false);

    TOKEN rParenToken = state.popUntilNonWhitespaceToken();
    if (rParenToken.type != backend::lexer::RPAREN) {
        return STATESTATUSPAIR(state, false);
    }

    // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/272):
    // Modify State::addPatternClause to take in an ARG rather than value strings.
    state.addPatternClause(clauseType, state.getArgFromSynonymString(synAssignToken.nameValue),
                           entRefArg, expressionSpec);
    state.popIfCurrentTokenIsWhitespaceToken();
    logLine(kQppLogInfoPrefix + "parseSinglePatternClause: Success End");
    return STATESTATUSPAIR(state, true);
}

bool isSynAssignToken(const TOKEN& token, State& state) {
    if (token.type != backend::lexer::NAME) {
        return false;
    }
    const std::unordered_map<std::string, qpbackend::EntityType>& declarationMap = state.getQuery().declarationMap;
    auto declaration = declarationMap.find(token.nameValue);
    if (declaration == declarationMap.end()) {
        return false;
    }
    return declaration->second == qpbackend::EntityType::ASSIGN;
}

/**
 * The original grammar is:
 * expression-spec :  ‘"‘ expr’"’ | ‘_’ ‘"’ expr ‘"’ ‘_’ | ‘_’
 * expr will be parsed by the PKB's parser to build an AST tree.
 * @return _"<expr>"_ | "<expr>"
 */
STATE_STRING_RESULT_CLAUSE_TYPE_STATUS_QUADRUPLE parseExpressionSpec(State state, const TOKEN& synToken) {
    const TOKEN& firstToken = state.popUntilNonWhitespaceToken();

    if (!state.hasTokensLeftToParse())
        return STATE_STRING_RESULT_CLAUSE_TYPE_STATUS_QUADRUPLE(state, "", qpbackend::INVALID_CLAUSE_TYPE, false);
    state.popToNextNonWhitespaceToken();
    const TOKEN& secondToken = state.peekToken();

    bool isSubExpression;
    int doubleQuotesPoppedCount = 0; // When doubleQuotesPoppedCount = 2, it means the end of the expr is reached.
    int nonWhitespaceTokensInExpr = 0;
    std::string expressionSpec;
    // TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/231):
    //  Use switch case statements to optimise.
    if (firstToken.type == backend::lexer::DOUBLE_QUOTE) {
        isSubExpression = false;
        doubleQuotesPoppedCount += 1;
    } else if (firstToken.type == backend::lexer::UNDERSCORE && secondToken.type == backend::lexer::DOUBLE_QUOTE) {
        isSubExpression = true;
    } else if (firstToken.type == backend::lexer::UNDERSCORE) {
        qpbackend::EntityType synEntityType = state.getEntityType(synToken.nameValue);

        if (synEntityType == qpbackend::ASSIGN) {
            return STATE_STRING_RESULT_CLAUSE_TYPE_STATUS_QUADRUPLE(state, "_", qpbackend::ASSIGN_PATTERN_WILDCARD,
                                                                    true);
        } else if (synEntityType == qpbackend::WHILE) {
            return STATE_STRING_RESULT_CLAUSE_TYPE_STATUS_QUADRUPLE(state, "_", qpbackend::WHILE_PATTERN, true);
        }
        return STATE_STRING_RESULT_CLAUSE_TYPE_STATUS_QUADRUPLE(state, "_", qpbackend::INVALID_CLAUSE_TYPE, true);
    } else {
        return STATE_STRING_RESULT_CLAUSE_TYPE_STATUS_QUADRUPLE(state, "", qpbackend::INVALID_CLAUSE_TYPE, false);
    }

    // Stringify all tokens that are in between the double quotes
    // "<stringify all tokens here>"
    while (doubleQuotesPoppedCount < 2) {
        const TOKEN& currToken = state.popToken();
        if (currToken.type != backend::lexer::WHITESPACE) {
            nonWhitespaceTokensInExpr += 1;
        }
        switch (currToken.type) {
        case backend::lexer::NAME:
            expressionSpec += currToken.nameValue;
            break;
        case backend::lexer::INTEGER:
            expressionSpec += currToken.integerValue;
            break;
        case backend::lexer::LPAREN:
            expressionSpec += '(';
            break;
        case backend::lexer::RPAREN:
            expressionSpec += ')';
            break;
        case backend::lexer::MULT:
            expressionSpec += '*';
            break;
        case backend::lexer::PLUS:
            expressionSpec += '+';
            break;
        case backend::lexer::MINUS:
            expressionSpec += '-';
            break;
        case backend::lexer::DIV:
            expressionSpec += '/';
            break;
        case backend::lexer::MOD:
            expressionSpec += '%';
            break;
        case backend::lexer::DOUBLE_QUOTE:
            doubleQuotesPoppedCount += 1;
            break;
        case backend::lexer::WHITESPACE:
            // Keep whitespace so that the SIMPLE parser can tell the difference between
            // "1 + 23" (valid) and "1 + 2 3" (invalid)
            expressionSpec += ' ';
            break;
        default:
            return STATE_STRING_RESULT_CLAUSE_TYPE_STATUS_QUADRUPLE(state, "", qpbackend::INVALID_CLAUSE_TYPE, false);
        }
    }

    if (nonWhitespaceTokensInExpr == 0) {
        logLine(kQppLogWarnPrefix + "parseExpressionSpec: no EXPR is matched between 2 "
                                    "DOUBLE_QUOTE tokens.");
        return STATE_STRING_RESULT_CLAUSE_TYPE_STATUS_QUADRUPLE(state, "", qpbackend::INVALID_CLAUSE_TYPE, false);
    }


    if (isSubExpression) {
        const TOKEN& endingUnderscoreToken = state.popUntilNonWhitespaceToken();
        if (endingUnderscoreToken.type != backend::lexer::UNDERSCORE) {
            logLine(kQppLogWarnPrefix +
                    "parseExpressionSpec: Missing ending UNDERSCORE for _\"expr\"_ group.");
            return STATE_STRING_RESULT_CLAUSE_TYPE_STATUS_QUADRUPLE(state, "", qpbackend::INVALID_CLAUSE_TYPE, false);
        }
    }

    qpbackend::ClauseType clauseType;
    if (state.getEntityType(synToken.nameValue) != qpbackend::ASSIGN) {
        clauseType = qpbackend::INVALID_CLAUSE_TYPE;
    } else if (isSubExpression) {
        clauseType = qpbackend::ASSIGN_PATTERN_SUB_EXPR;
    } else {
        clauseType = qpbackend::ASSIGN_PATTERN_EXACT;
    }

    state.popIfCurrentTokenIsWhitespaceToken();
    return STATE_STRING_RESULT_CLAUSE_TYPE_STATUS_QUADRUPLE(state, expressionSpec, clauseType, true);
}

// QueryPreprocessor API definitions.

/**
 * Parses tokens of a QPL query into a Query struct for easier processing.
 *
 * TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/166):
 * tokens can be converted to an rvalue reference for optimization in the future.
 * `Tokens` are currently copied within state.
 *
 * @return A Query struct representing the valid QPL query. If the QPL query is invalid,
 * return an empty Query struct.
 */
qpbackend::Query parseTokens(const TOKENS& tokens) {
    State initialState = State(tokens);
    try {
        State completedState = parseSelect(initialState);
        logLine(kQppLogInfoPrefix + "parseTokens: completed parsing.\n" + completedState.getQuery().toString());
        return completedState.getQuery();
    } catch (const std::runtime_error& e) {
        logLine(e.what());
    } catch (const std::invalid_argument& e) {
        logLine(e.what());
    }
    return qpbackend::Query();
}

} // namespace querypreprocessor
