#pragma once
#include <unordered_map>
#include <string>
#include "lexer.h"
#include "parser.h"

class SymbolTable  {
    private:
        std::unordered_map<std::string, TokenType> table;
    public:
        void declare(const std::string& name, TokenType& type);

        TokenType lookup(const std::string& name) const;

        bool exists(const std::string& name) const;

        void remove(const std::string& name);
};

class TypeChecker {
    private:
        SymbolTable symbols;
    public:
        TypeChecker() = default;
        TokenType TypeCheck(ASTNode* node);
};
