#include <unordered_map>
#include "ast.h"
#include "type.h"
#include <stdexcept>


void SymbolTable::declare(const std::string& name, TokenType& type) {
    if (isDeclared(name)) {
        throw std::runtime_error("TC | '" + name + "' is already declared");
    }

    table[name] = type;
}

bool SymbolTable::isDeclared(const std::string& name) const {
    return exists(name) || arrayExists(name);
}

TokenType SymbolTable::lookup(const std::string& name) const {
    auto t_it = table.find(name);
    if (t_it != table.end()) return t_it->second;

    auto a_it = arrays.find(name);
    if (a_it != arrays.end()) return a_it->second;

    throw std::runtime_error("TC | '" + name + "' is not declared");
}


bool SymbolTable::exists(const std::string& name) const {
    return table.find(name) != table.end();
}

bool SymbolTable::arrayExists(const std::string& name) const {
    return arrays.find(name) != arrays.end();
}

void SymbolTable::arrayDeclare(const std::string& name, TokenType& elementType) {
    if (isDeclared(name)) {
        throw std::runtime_error("TC | '" + name + "' is already declared");
    }

    arrays[name] = elementType;
}

void SymbolTable::remove(const std::string& name) {
    table.erase(name);
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
            throw std::runtime_error("TC | Type mismatch in binary expression: '" + tokenTypeName(leftType) + "' -> '" + tokenTypeName(rightType) + "'");
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

    if (auto arr = dynamic_cast<ArrayDeclNode*>(node)) {
        TokenType declaredType = arr->elementType.type;
        int counter = 0;
        for (auto element : arr->elements) {
            TokenType elementValue = TypeCheck(element);
            if (elementValue != declaredType) {
                throw std::runtime_error("TC | Type mismatch in array declaration at index " + std::to_string(counter) + ", expected: " + tokenTypeName(arr->elementType.type) + " Got: " + tokenTypeName(elementValue));
            }
            counter++;
        }

        if (arr->elements.size() != arr->size) {
            throw std::runtime_error("TC | Array '" + arr->name.value + "' declared size " 
                + std::to_string(arr->size) + " but got " + std::to_string(arr->elements.size()) + " elements");
        }

        symbols.arrayDeclare(arr->name.value, declaredType);

        return declaredType;
    }

    if (auto idx = dynamic_cast<IndexNode*>(node)) {
        if (TokenType::Int != TypeCheck(idx->index)) {
            throw std::runtime_error("Non-integer index for identifier '" + idx->name.value + "'");
        }

        if (!(symbols.arrayExists(idx->name.value))) {
            throw std::runtime_error("'" + idx->name.value + "' is not an array");
        } 

        return symbols.lookup(idx->name.value);
    }

    if (auto varDecl = dynamic_cast<VarDeclNode*>(node)) {
        TokenType value = TypeCheck(varDecl->value);
        TokenType declaredType = varDecl->type.type;

        if (value != declaredType) {
            throw std::runtime_error("TC | Type mismatch in variable declaration for '" + varDecl->name.value + "'");
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
            throw std::runtime_error("TC | If condition must be Bool");
        }
        TypeCheck(ifNode->thenBranch);

        if (ifNode->elseBranch != nullptr) {
            TypeCheck(ifNode->elseBranch);
        }

        return TokenType::Null;
    }

    if (auto whileNode = dynamic_cast<WhileNode*>(node)) {
        TokenType condition = TypeCheck(whileNode->condition);

        if (condition != TokenType::Bool) {
            throw std::runtime_error("TC | If condition must be Bool");
        }

        TypeCheck(whileNode->body);

        return TokenType::Null;
    }

    if (auto assignNode = dynamic_cast<AssignNode*>(node)) {
        TokenType targetType = symbols.lookup(assignNode->target.value);
        TokenType value = TypeCheck(assignNode->value);

        if (targetType != value) {
            throw std::runtime_error("TC | Cannot assign due to type mismatch: '" + assignNode->target.value + "'");
        }

        return targetType;
    }

    if (auto funcDecl = dynamic_cast<FuncDeclNode*>(node)) {
        FuncType current;
        current.type = funcDecl->returnType.type;
        for (auto param : funcDecl->parameters) {
            current.paramTypes.push_back(param.type.type);
        }

        functions[funcDecl->name.value] = current;

        for (auto param : funcDecl->parameters) {
            symbols.declare(param.name.value, param.type.type);
        }

        returnType = current.type;

        TypeCheck(funcDecl->body);

        returnType = TokenType::Sentinel;

        for (auto param : funcDecl->parameters) {
            symbols.remove(param.name.value);
        }

        return current.type;

    }

    if (auto callNode = dynamic_cast<CallNode*>(node)) {
        auto it = functions.find(callNode->name.value);
        if (it == functions.end()) {
            throw std::runtime_error("TC | Undefined function:" + callNode->name.value);
        }

        FuncType typeStruct = it->second;

        if (callNode->arguments.size() != typeStruct.paramTypes.size()) {
            throw std::runtime_error("TC | Expected " + std::to_string(typeStruct.paramTypes.size()) + " arguments, got " + std::to_string(callNode->arguments.size()));
        }

        int index = 0;
        for (auto argument : callNode->arguments) {
            TokenType type = TypeCheck(argument);
            if (type != typeStruct.paramTypes[index]) {
                throw std::runtime_error("TC | Expected type: " + tokenTypeName(typeStruct.paramTypes[index]) + "Got: " + tokenTypeName(type));
            }
            index++;
        }
        return typeStruct.type;
    }
    
    if (auto outNode = dynamic_cast<OutNode*>(node)) {
        TokenType outType = TypeCheck(outNode->output);

        if (outType == returnType) {
            return outType;
        } else {
            throw std::runtime_error("TC | Expected: " + tokenTypeName(returnType) + " Got: " + tokenTypeName(outType));
        }
    }

    throw std::runtime_error("TC | Unhandled node type");
}