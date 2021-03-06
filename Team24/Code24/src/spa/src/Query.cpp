#include "Query.h"

#include "LegacyQueryShims.h"
#include "Logger.h"
#include "QPTypes.h"

#include <algorithm>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

namespace qpbackend {

const std::map<std::string, EntityType> kEntityStringToTypeMap = {
    { "stmt", EntityType::STMT },
    { "read", EntityType::READ },
    { "print", EntityType::PRINT },
    { "call", EntityType::CALL },
    { "while", EntityType::WHILE },
    { "if", EntityType::IF },
    { "assign", EntityType::ASSIGN },
    { "variable", EntityType::VARIABLE },
    { "constant", EntityType::CONSTANT },
    { "procedure", EntityType::PROCEDURE },
    { "prog_line", EntityType::PROG_LINE },
    { "INVALID ENTITY TYPE", EntityType::INVALID_ENTITY_TYPE },
};

const std::vector<std::pair<std::string, ClauseType>> kRelationClauseStringClauseTypePairs = {
    { "Follows", ClauseType::FOLLOWS },
    { "Follows*", ClauseType::FOLLOWST },
    { "Parent", ClauseType::PARENT },
    { "Parent*", ClauseType::PARENTT },
    { "Uses", ClauseType::USES },
    { "Modifies", ClauseType::MODIFIES },
    { "Next", ClauseType::NEXT },
    { "Next*", ClauseType::NEXTT },
    { "NextBip", ClauseType::NEXTBIP },
    { "NextBip*", ClauseType::NEXTBIPT },
    { "Calls", ClauseType::CALLS },
    { "Calls*", ClauseType::CALLST },
    { "Affects", ClauseType::AFFECTS },
    { "Affects*", ClauseType::AFFECTST },
    { "AffectsBip", ClauseType::AFFECTSBIP },
    { "AffectsBip*", ClauseType::AFFECTSBIPT },
};

bool isEntityString(const std::string& string) {
    return kEntityStringToTypeMap.find(string) != kEntityStringToTypeMap.end();
}

EntityType entityTypeFromString(const std::string& entityString) {
    auto result = kEntityStringToTypeMap.find(entityString);
    if (result == kEntityStringToTypeMap.end()) {
        logLine("Warning:entityTypeFromString: " + entityString + " does not map to any EntityType.");
        return INVALID_ENTITY_TYPE;
    }
    return result->second;
}

std::string stringFromEntityType(EntityType entityType) {
    for (const auto& entityStringTypePair : kEntityStringToTypeMap) {
        if (entityStringTypePair.second == entityType) {
            return entityStringTypePair.first;
        }
    }
    return "UNKNOWN ENTITY TYPE";
}

bool isRelationClauseString(const std::string& string) {
    return std::any_of(kRelationClauseStringClauseTypePairs.begin(),
                       kRelationClauseStringClauseTypePairs.end(),
                       [&](const std::pair<std::string, ClauseType>& pair) {
                           return pair.first == string;
                       });
}

ClauseType relationClauseTypeFromString(const std::string& relationClauseString) {
    for (const auto& relationStringRelationTypePair : kRelationClauseStringClauseTypePairs) {
        if (relationStringRelationTypePair.first == relationClauseString) {
            return relationStringRelationTypePair.second;
        }
    }
    throw std::invalid_argument("Error:relationClauseTypeFromString: " + relationClauseString +
                                " does not map to any EntityType.");
}

std::string stringFromRelationType(ClauseType relationType) {
    for (const auto& relationTypePair : kRelationClauseStringClauseTypePairs) {
        if (relationTypePair.second == relationType) {
            return relationTypePair.first;
        }
    }
    return "UNKNOWN RELATION TYPE";
}

bool Query::operator==(const Query& s) const {
    return declarationMap == s.declarationMap && returnCandidates == s.returnCandidates &&
           suchThatClauses == s.suchThatClauses && patternClauses == s.patternClauses &&
           withClauses == s.withClauses;
}


Query::Query(const std::unordered_map<std::string, EntityType>& declarationMap,
             const RETURN_CANDIDATE_LIST& returnCandidates,
             const std::vector<RELATIONTUPLE>& suchThatClauses,
             const std::vector<PATTERNTUPLE>& patternClauses) {
    this->declarationMap = declarationMap;
    this->returnCandidates = returnCandidates;
    this->suchThatClauses = suchThatClauses;
    this->patternClauses = patternClauses;
}

Query::Query(const DECLARATION_MAP& declarationMap,
             const RETURN_CANDIDATE_LIST& returnCandidates,
             const std::vector<RELATIONTUPLE>& suchThatClauses,
             const CLAUSE_LIST& patternClauses,
             const WITH_CLAUSE_LIST& withClauses) {
    this->declarationMap = declarationMap;
    this->returnCandidates = returnCandidates;
    this->suchThatClauses = suchThatClauses;
    this->patternClauses = patternClauses;
    this->withClauses = withClauses;
}

std::string Query::toString() const {
    std::stringstream stringstream;
    stringstream << "Query {\nDeclaration map:";

    for (const auto& kv : declarationMap) {
        stringstream << "{" << kv.first << ", " << stringFromEntityType(kv.second) << "} ";
    }

    stringstream << "\nReturn candidates to return:";
    for (const auto& returnCandidate : returnCandidates) {
        stringstream << prettyReturnCandidate(returnCandidate) + " ";
    }

    stringstream << "\nSuch that clauses: ";
    ClauseType relationType;
    qpbackend::ARG arg1;
    qpbackend::ARG arg2;
    for (const RELATIONTUPLE& suchThatClause : suchThatClauses) {
        std::tie(relationType, arg1, arg2) = suchThatClause;
        stringstream << "{" << stringFromRelationType(relationType) << ", " << prettyPrintArg(arg1)
                     << ", " << prettyPrintArg(arg2) << "} ";
    }

    stringstream << "\nPattern clauses: ";
    for (const CLAUSE& patternClause : patternClauses) {
        stringstream << prettyPrintCLAUSE(patternClause);
    }

    stringstream << "\nWith clauses: ";
    for (const WITH_CLAUSE& withClause : withClauses) {
        stringstream << "{" << prettyPrintWithClause(withClause) << "} ";
    }

    stringstream << "\n}";
    return stringstream.str();
}

Query::Query(const DECLARATION_MAP& declarationMap,
             const std::vector<std::string>& returnCandidates,
             const std::vector<RELATIONTUPLE>& suchThatClauses,
             const CLAUSE_LIST& patternClauses) {
    this->declarationMap = declarationMap;
    this->suchThatClauses = suchThatClauses;
    this->returnCandidates = convertToReturnValues(returnCandidates, declarationMap);
    this->patternClauses = patternClauses;
}

// Allow for Query struct expansion in Catch framework's error message generation.
std::ostream& operator<<(std::ostream& os, Query const& value) {
    os << value.toString();
    return os;
}

} // namespace qpbackend
