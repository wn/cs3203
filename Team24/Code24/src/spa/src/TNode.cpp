#include "TNode.h"
#include <Logger.h>
#include <sstream>

namespace backend {
std::string getTNodeTypeString(TNodeType t) {
    switch (t) {
    case TNodeType::Program:
        return "Program";
    case TNodeType::Procedure:
        return "Procedure";
    case TNodeType::StatementList:
        return "StatementList";
    case TNodeType::Assign:
        return "Assign";
    case TNodeType::Call:
        return "Call";
    case TNodeType::IfElse:
        return "IfElse";
    case TNodeType::Print:
        return "Print";
    case TNodeType::Read:
        return "Read";
    case TNodeType::While:
        return "While";
    case TNodeType::Not:
        return "Not";
    case TNodeType::And:
        return "And";
    case TNodeType::Or:
        return "Or";
    case TNodeType::Greater:
        return "Greater";
    case TNodeType::GreaterThanOrEqual:
        return "GreaterThanOrEqual";
    case TNodeType::Lesser:
        return "Lesser";
    case TNodeType::LesserThanOrEqual:
        return "LesserThanOrEqual";
    case TNodeType::Equal:
        return "Equal";
    case TNodeType::NotEqual:
        return "NotEqual";
    case TNodeType::Plus:
        return "Plus";
    case TNodeType::Minus:
        return "Minus";
    case TNodeType::Multiply:
        return "Multiply";
    case TNodeType::Divide:
        return "Divide";
    case TNodeType::Modulo:
        return "Modulo";
    case TNodeType::Variable:
        return "Variable";
    case TNodeType::Constant:
        return "Constant";
    case TNodeType::INVALID:
        return "INVALID";
    default:
        // if TNodeType indeed cover all possible cases, throw statement should not occur.
        throw "Error: Unknown TNodeType is not implemented yet.";
    };
}

std::string TNode::toString() const {
    return toStringHelper(0);
}

std::string TNode::toStringHelper(int tabs) const {
    std::string nodeName = getTNodeTypeString(type);
    std::ostringstream stringStream;

    for (int i = 0; i < tabs; ++i)
        stringStream << " ";
    stringStream << nodeName;
    if (line >= 0) {
        stringStream << " @ " << line << "(" << name << ", " << constant << ")";
    }
    stringStream << " : [\n";

    for (const auto& child : children) {
        stringStream << child.toStringHelper(tabs + 1);
    }

    for (int i = 0; i < tabs; ++i)
        stringStream << " ";
    stringStream << "]\n";
    return stringStream.str();
}

bool TNode::operator==(const TNode& rhs) const {
    if (type != rhs.type || line != rhs.line || name != rhs.name || constant != rhs.constant) {
        // Helpful to resolve test.
        logLine("type not equal: " + getTNodeTypeString(type) + " : " + getTNodeTypeString(rhs.type),
                type != rhs.type);
        logLine("type line not equal: " + std::to_string(line) + " : " + std::to_string(rhs.line),
                line != rhs.line);
        logLine("name not equal: " + name + " : " + rhs.name, name != rhs.name);
        logLine("constant not equal: " + std::to_string(constant) + " : " + std::to_string(rhs.constant),
                constant != rhs.constant);
        logLine();
        return false;
    }
    if (children.size() != rhs.children.size()) {
        return false;
    }
    for (int i = 0; i < children.size(); ++i) {
        if (children[i] == rhs.children[i]) {
            continue;
        }
        return false;
    }
    return true;
}

void TNode::addChild(const TNode& c) {
    children.emplace_back(c);
}
} // namespace backend
