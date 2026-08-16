#pragma once
#include <unordered_map>
#include <string>
#include "lexer.h"
#include "parser.h"

class SymbolTable  {
    private:
        std::unordered_map<std::string, TokenType> table;
        std::unordered_map<std::string, TokenType> arrays;
    public:
        void declare(const std::string& name, TokenType& type);

        void arrayDeclare(const std::string& name, TokenType& elementType);

        TokenType lookup(const std::string& name) const;

        bool exists(const std::string& name) const;

        bool isDeclared(const std::string& name) const;

        bool arrayExists(const std::string& name) const;

        bool onlyArray(const std::string& name) const;

        void remove(const std::string& name);
};

struct FuncType {
    TokenType type;
    std::vector<TokenType> paramTypes;
};

inline std::unordered_map<std::string, FuncType> functions;

class TypeChecker {
    private:
        SymbolTable symbols;

        TokenType returnType = TokenType::Sentinel;
    public:
        TypeChecker() = default;
        TokenType TypeCheck(ASTNode* node);
};
