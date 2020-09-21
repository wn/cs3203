#include "TestWrapper.h"

#include "Lexer.h"
#include "Logger.h"
#include "PKBImplementation.h"
#include "Parser.h"
#include "QueryPreprocessor.h"

#include <fstream>
#include <iterator>
#include <sstream>

// implementation code of WrapperFactory - do NOT modify the next 5 lines
AbstractWrapper* WrapperFactory::wrapper = 0;
AbstractWrapper* WrapperFactory::createWrapper() {
    if (wrapper == 0) wrapper = new TestWrapper;
    return wrapper;
}
// Do not modify the following line
volatile bool AbstractWrapper::GlobalStop = false;

// a default constructor
TestWrapper::TestWrapper() {
    // create any objects here as instance variables of this class
    // as well as any initialization required for your spa program
}

// method for parsing the SIMPLE source
void TestWrapper::parse(std::string filename) {
    std::ifstream inputFileStream;
    inputFileStream.open(filename);

    backend::TNode ast = backend::Parser(backend::lexer::tokenize(inputFileStream)).parse();
    logLine("AST:");
    logTNode(ast);

    backend::PKBImplementation pkb(ast);

    // call queries on the PKB after this
}

// method to evaluating a query
void TestWrapper::evaluate(std::string query, std::list<std::string>& results) {
    std::vector<backend::lexer::Token> tokens;
    std::stringstream stream(query);
    try {
        tokens = backend::lexer::tokenizeWithWhitespace(stream);
    } catch (const std::exception& e) {
        std::cout << "Invalid query syntax" << std::endl;
        return;
    }
    qpbackend::Query queryStruct = querypreprocessor::parseTokens(tokens);
    // store the answers to the query in the results list (it is initially empty)
    // each result must be a string.
}
