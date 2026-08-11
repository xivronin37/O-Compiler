#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include "lexer.h"

class Parser {
    public:
        Parser(const std::vector<Token>& tokens);
        ASTNode* parse();
    private:
        std::vector<Token> tokens;
        size_t pos = 0;

        Token peek() const;
        Token advance();
        Token expect(TokenType type);
        Token expectType();
        bool match(TokenType type);
        ASTNode* statement();
        ASTNode* varDecl();
        ASTNode* ifStatement();
        ASTNode* expression();
        ASTNode* comparison();
        ASTNode* term();
        ASTNode* factor();
        ASTNode* primary();
        ASTNode* block();
        ASTNode* exprstatement();
        ASTNode* assignStatement();

};

std::string tokenTypeName(TokenType type);

void printAST(ASTNode* node, int depth = 0);