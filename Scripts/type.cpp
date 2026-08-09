#include <unordered_map>
#include "type.h"
#include <stdexcept>


void SymbolTable::declare(const std::string& name, TokenType& type) {
    table[name] = type;
}

TokenType SymbolTable::lookup(const std::string& name) const {
    auto it = table.find(name);
    if (it == table.end()) {
        throw std::runtime_error("Variable " + name + " not declared.");
    }

    return it->second;
}

bool SymbolTable::exists(const std::string& name) const {
    return table.find(name) != table.end();
}

TokenType TypeChecker::TypeCheck(ASTNode* node) {
    if (auto num = dynamic_cast<NumberLiteralNode*>(node)) {
        return TokenType::Int; // Placeholder before floats
    }

    if (auto id = dynamic_cast<IdentifierNode*>(node)) {
        return symbols.lookup(id->value);
    }

    if (auto bin = dynamic_cast<BinaryExprNode*>(node)) {
        TokenType leftType = TypeCheck(bin->left);
        TokenType rightType = TypeCheck(bin->right);

        if (leftType != rightType) {
            throw std::runtime_error("Type mismatch in binary expression: '" + tokenTypeName(leftType) + "' -> '" + tokenTypeName(rightType) + "'");
        }
        
        switch(bin->op.type) {
            case TokenType::LessThan:
            case TokenType::GreaterThan:
            case TokenType::LessThanOrEqual:
            case TokenType::GreaterThanOrEqual:
            case TokenType::EqualEqual:
            case TokenType::NotEqual:
                return TokenType::Bool;
            
            default: return leftType;
        }
    }

    if (auto varDecl = dynamic_cast<VarDeclNode*>(node)) {
        TokenType value = TypeCheck(varDecl->value);
        TokenType declaredType = varDecl->type.type;

        if (value != declaredType) {
            throw std::runtime_error("Type mismatch in variable declaration for '" + varDecl->name.value + "'");
        }

        symbols.declare(varDecl->name.value, declaredType);

        return declaredType;
    }

    if (auto block = dynamic_cast<BlockNode*>(node)) {
        TokenType last = TokenType::Null;
        for (auto statement: block->statements) {
            last = TypeCheck(statement);
        }
        return last;
    }

    if (auto ifNode = dynamic_cast<IfNode*>(node)) {
        TokenType condition = TypeCheck(ifNode->condition);

        if (condition != TokenType::Bool) {
            throw std::runtime_error("If condition must be Bool");
        }
        TypeCheck(ifNode->thenBranch);

        if (ifNode->elseBranch != nullptr) {
            TypeCheck(ifNode->elseBranch);
        }

        return TokenType::Null;
    }

    if (auto assignNode = dynamic_cast<AssignNode*>(node)) {
        TokenType targetType = symbols.lookup(assignNode->target.value);
        TokenType value = TypeCheck(assignNode->value);

        if (targetType != value) {
            throw std::runtime_error("Cannot assign due to type mismatch: '" + assignNode->target.value + "'");
        }

        return targetType;
    }

    // Add more cases for other node types
    throw std::runtime_error("Unhandled node type");
}