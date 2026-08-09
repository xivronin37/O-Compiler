#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include "lexer.h"


struct ASTNode {
    virtual ~ASTNode() = default;
};

struct IdentifierNode : ASTNode {
    std::string value;
    IdentifierNode(const std::string& value) : value(value) {}
};

struct CollectionNode : ASTNode {
    std::string value;
    std::vector<ASTNode*> elements;
    CollectionNode(const std::string& value, const std::vector<ASTNode*>& elements) : value(value), elements(elements) {}
};

struct NumberLiteralNode : ASTNode {
    std::string value;
    NumberLiteralNode(const std::string& value) : value(value) {}
};

struct BlockNode : ASTNode {
    std::vector<ASTNode*> statements;
    BlockNode(const std::vector<ASTNode*>& statements) : statements(statements) {}
};

struct IfNode : ASTNode {
    ASTNode* condition;
    ASTNode* thenBranch;
    ASTNode* elseBranch;

    IfNode(ASTNode* condition, ASTNode*& thenBranch, ASTNode*& elseBranch)
        : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}
};

struct BinaryExprNode : ASTNode {
    ASTNode* left;
    Token op;
    ASTNode* right;

    BinaryExprNode(ASTNode* left, Token op, ASTNode* right) : left(left), op(op), right(right) {}
};

struct VarDeclNode : ASTNode {
    Token name;
    Token type;
    ASTNode* value;

    VarDeclNode(Token name, Token type, ASTNode* value) : name(name), type(type), value(value) {}
};

struct AssignNode : ASTNode {
    Token target;
    Token op;
    ASTNode* value;

    AssignNode(Token target, Token op, ASTNode* value) : target(target), op(op), value(value) {}
};

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