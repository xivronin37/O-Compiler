#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <filesystem>
#include "lexer.h"
#include "ast.h"
#include "parser.h"


std::string tokenTypeName(TokenType type) {
    switch (type) {
        case TokenType::Identifier: return "Identifier";
        case TokenType::Keyword: return "Keyword";
        case TokenType::Let: return "Let";
        case TokenType::If: return "If";
        case TokenType::Else: return "Else";
        case TokenType::While: return "While";
        case TokenType::Out: return "Out";
        case TokenType::Create: return "Create";
        case TokenType::Call: return "Call";
        case TokenType::Class: return "Class";
        case TokenType::Insert: return "Insert";
        case TokenType::From: return "From";
        case TokenType::As: return "As";
        case TokenType::Inline: return "Inline";
        case TokenType::Request: return "Request";
        case TokenType::Send: return "Send";
        case TokenType::Decouple: return "Decouple";
        case TokenType::Number: return "Number";
        case TokenType::String: return "String";
        case TokenType::Plus: return "Plus";
        case TokenType::Minus: return "Minus";
        case TokenType::Star: return "Star";
        case TokenType::FSlash: return "FSlash";
        case TokenType::BSlash: return "BSlash";
        case TokenType::Colon: return "Colon";
        case TokenType::Semicolon: return "Semicolon";
        case TokenType::Comma: return "Comma";
        case TokenType::LBracket: return "LBracket";
        case TokenType::RBracket: return "RBracket";
        case TokenType::LParen: return "LParen";
        case TokenType::RParen: return "RParen";
        case TokenType::LBrace: return "LBrace";
        case TokenType::RBrace: return "RBrace";
        case TokenType::Equal: return "Equal";
        case TokenType::EqualEqual: return "EqualEqual";
        case TokenType::NotEqual: return "NotEqual";
        case TokenType::LessThan: return "LessThan";
        case TokenType::GreaterThan: return "GreaterThan";
        case TokenType::LessThanOrEqual: return "LessThanOrEqual";
        case TokenType::GreaterThanOrEqual: return "GreaterThanOrEqual";
        case TokenType::L_AND: return "L_AND";
        case TokenType::L_OR: return "L_OR";
        case TokenType::L_NOT: return "L_NOT";
        case TokenType::L_XOR: return "L_XOR";
        case TokenType::Punctuation: return "Punctuation";
        case TokenType::Int: return "Int";
        case TokenType::Bool: return "Bool";
        case TokenType::Float: return "Float";
        case TokenType::Array: return "Array";
        case TokenType::Hash: return "Hash";
        case TokenType::Null: return "Null";
        case TokenType::UnsignedInt: return "UnsignedInt";
        case TokenType::UnsignedFloat: return "UnsignedFloat";
        case TokenType::EndOfFile: return "EndOfFile";
    }
    return "Unknown"; 
}

Token Parser::peek() const {
    if (pos < tokens.size()) {
        return tokens[pos];
    }
    return {TokenType::EndOfFile, "", 0, 0};
}

Token Parser::advance() {
    if (pos < tokens.size()) {
        return tokens[pos++];
    }
    return {TokenType::EndOfFile, "", 0, 0};
}

Token Parser::expect(TokenType type) {
    Token token = peek();
    if (token.type == type) {
        advance();
        return token;
    }
    
    // Handle error: unexpected token type
    throw std::runtime_error("Unexpected token type " + tokenTypeName(token.type) + " at line " + std::to_string(token.line) + ", column " + std::to_string(token.column)); 
    return {TokenType::EndOfFile, "", 0, 0};
}

Token Parser::expectType() {
    TokenType t = peek().type;
    if (t == TokenType::Int || t == TokenType::Bool || t == TokenType::Float || t == TokenType::Null || t == TokenType::UnsignedInt || t == TokenType::UnsignedFloat) {
        return advance();
    }
    throw std::runtime_error("Expected type token at line " + std::to_string(peek().line) + ", column " + std::to_string(peek().column));
    return {TokenType::EndOfFile, "", 0, 0};
}

bool Parser::match(TokenType type) {
    Token token = peek();
    if (token.type == type) {
        advance();
        return true;
    }
    return false;
}

ASTNode* Parser::statement() {
    if (peek().type == TokenType::Let) {
        return varDecl();
    } 

    if (peek().type == TokenType::If) {
        return ifStatement();
    }

    if (peek().type == TokenType::While) {
        return whileStatement();
    }

    if (peek().type == TokenType::Identifier) {
        Token next = tokens[pos + 1];
        if (next.type == TokenType::Equal || next.type == TokenType::Tilde) {
            return assignStatement();
        }
    }

    if (peek().type == TokenType::Create) {
        return funcDecl();
    }

    if (peek().type == TokenType::Out) {
        return outStatement();
    }

    return exprstatement();
}

ASTNode* Parser::varDecl() {
    expect(TokenType::Let);
    Token name = expect(TokenType::Identifier);
    expect(TokenType::Colon);
    if (peek().type == TokenType::LBrace) {
        std::vector<ASTNode*> elements;
        typeBlockInfo newInfo = typeBlock();
        expect(TokenType::Comma);
        expect(TokenType::BSlash);

        while (peek().type != TokenType::BSlash) {
            if (peek().type == newInfo.elementType.type) {
                elements.push_back(expression());
            } else {
                throw std::runtime_error("Expected type: " + tokenTypeName(newInfo.elementType.type) + "Got: " +tokenTypeName(peek().type));
            }
            if (peek().type == TokenType::Comma) advance();
        }

        expect(TokenType::BSlash);
        expect(TokenType::Semicolon);

        return new ArrayDeclNode{name, newInfo.elementType, newInfo.params[0], elements};
    }

    Token type = expectType();
    expect(TokenType::Comma);
    ASTNode* value = expression(); 
    expect(TokenType::Semicolon);
    return new VarDeclNode{name, type, value};
}

typeBlockInfo Parser::typeBlock() {
    typeBlockInfo info;
    int size = 0;
    Token foundType;
    expect(TokenType::LBrace);
    expect(TokenType::Array);
    expect(TokenType::LParen);
    size = std::stoi(expect(TokenType::Int).value);
    expect(TokenType::RParen);
    
    expect(TokenType::Comma);

    if (peek().type == TokenType::Int || peek().type == TokenType::Float || peek().type == TokenType::UnsignedInt
    || peek().type == TokenType::UnsignedFloat) {
        foundType = advance();
    } else {
        throw std::runtime_error("Expected valid type, got: " + peek().value);
    }

    expect(TokenType::RBrace);

    info.kind = TokenType::Array;
    info.elementType = foundType;
    info.params.push_back(size);

    return info;
}

ASTNode* Parser::ifStatement() {
    expect(TokenType::If);
    ASTNode* condition = expression();
    ASTNode* thenBranch = block();
    ASTNode* elseBranch = nullptr;
    if (peek().type == TokenType::Else) {
        advance();
        elseBranch = block();
        return new IfNode{condition, thenBranch, elseBranch};
    }
    
    return new IfNode{condition, thenBranch, elseBranch};
}


ASTNode* Parser::whileStatement() {
    expect(TokenType::While);
    ASTNode* condition = expression();
    ASTNode* body = block();
    
    return new WhileNode{condition, body};
}

ASTNode* Parser::funcDecl() {
    expect(TokenType::Create);
    Token returnType = expectType();
    Token name = expect(TokenType::Identifier);
    std::vector<Param> parameters;
    expect(TokenType::LParen);
    while (peek().type != TokenType::RParen) {
        Param tempParam;
        tempParam.type = expectType();
        expect(TokenType::Colon);
        tempParam.name = expect(TokenType::Identifier);
        parameters.push_back(tempParam);
        if (peek().type != TokenType::RParen) {
            expect(TokenType::Comma);
        }       
    }

    expect(TokenType::RParen);

    ASTNode* body = block();

    return new FuncDeclNode{returnType, name, parameters, body};
}

ASTNode* Parser::outStatement() {
    expect(TokenType::Out);
    ASTNode* output = expression();
    expect(TokenType::Semicolon);

    return new OutNode{output};
}

ASTNode* Parser::assignStatement() {
    Token target = expect(TokenType::Identifier);
    Token op = advance();
    ASTNode* value = expression();
    expect(TokenType::Semicolon);
    return new AssignNode{target, op, value};
}

ASTNode* Parser::exprstatement() {
    ASTNode* expr = expression();
    expect(TokenType::Semicolon);
    return expr;
}

ASTNode* Parser::expression() {
    return comparison();
}

ASTNode* Parser::comparison() {
    ASTNode* left = term(); // arithmetic first

    while (peek().type == TokenType::LessThan || peek().type == TokenType::LessThanOrEqual ||
           peek().type == TokenType::GreaterThan || peek().type == TokenType::GreaterThanOrEqual ||
           peek().type == TokenType::EqualEqual || peek().type == TokenType::NotEqual) {
        Token op = advance();
        ASTNode* right = term();
        left = new BinaryExprNode(left, op, right);
    }

    return left;
}

ASTNode* Parser::term() {
    ASTNode* left = factor();

    while (peek().type == TokenType::Plus || peek().type == TokenType::Minus) {
        Token op = advance();
        ASTNode* right = factor();
        left = new BinaryExprNode(left, op, right);
    }
    return left;
}

ASTNode* Parser::factor() {
    ASTNode* left = primary();

    while (peek().type == TokenType::Star || peek().type == TokenType::FSlash) {
        Token op = advance();
        ASTNode* right = primary();
        left = new BinaryExprNode(left, op, right);
    }
    return left;
}

ASTNode* Parser::primary() {
    if (peek().type == TokenType::Int || peek().type == TokenType::Float) {
        Token num = advance();
        return new NumberLiteralNode(num.value);
    }

    if (peek().type == TokenType::Identifier) {
        Token id = advance();
        
        if (peek().type == TokenType::BSlash) {
            advance();
            std::vector<ASTNode*> elements;
            elements.push_back(expression());

            while (peek().type == TokenType::Comma) {
                advance();
                elements.push_back(expression());
            }

            expect(TokenType::BSlash);
            return new CollectionNode(id.value, elements);
        }

        if (peek().type == TokenType::Hash) {
            bool isGrouped = false;
            advance();
            if (peek().type == TokenType::LParen) {
                advance();
                isGrouped = true;
            }

            ASTNode* index = expression();

            if (isGrouped) expect(TokenType::RParen);
        }

        return new IdentifierNode(id.value);
    }

    if (peek().type == TokenType::Call) {
        expect(TokenType::Call);
        Token name = expect(TokenType::Identifier);
        expect(TokenType::LParen);

        std::vector<ASTNode*> arguments;

        while (peek().type != TokenType::RParen) {
            ASTNode* argument = expression();
            arguments.push_back(argument);
            if (peek().type != TokenType::RParen) {
                expect(TokenType::Comma);
            }    
        }

        expect(TokenType::RParen);

        return new CallNode{name, arguments};
    }

    throw std::runtime_error("Expected expression at line " + std::to_string(peek().line));
}

ASTNode* Parser::block() {
    expect(TokenType::LBracket);

    std::vector<ASTNode*> statements;
    while (peek().type != TokenType::RBracket && peek().type != TokenType::EndOfFile) {
        statements.push_back(statement());
    }

    expect(TokenType::RBracket);

    return new BlockNode(statements);
}

Parser::Parser(const std::vector<Token>& tokens, std::filesystem::path currentDir) : tokens(tokens), currentDir(currentDir) {}

ASTNode* Parser::parse() {
    std::vector<ASTNode*> statements;
    while (peek().type == TokenType::Insert) {
        expect(TokenType::Insert);
        Token StringLiteral = expect(TokenType::String);
        expect(TokenType::Semicolon);

        std::filesystem::path insertedPath = currentDir / StringLiteral.value;
        std::string insertSource = readFile(insertedPath.string()); // we read the inserted file

        Lexer lexer(insertSource);
        std::vector<Token> tokens = lexer.tokenize();

        for (auto token : tokens) {
            std::cout << "[insert] Type: " << tokenTypeName(token.type) << " Lexeme: " << token.value << std::endl;
        }

        Parser parser(tokens, insertedPath.parent_path());

        ASTNode* insertedFile = parser.parse(); // recursively parse: if insertedFile also has an insert, it will return to this code block

        for (auto statement : dynamic_cast<BlockNode*>(insertedFile)->statements) {
            statements.push_back(statement);
        }

    }
    while (peek().type != TokenType::EndOfFile) {
        statements.push_back(statement());
    }

    return new BlockNode(statements);
}

void printAST(ASTNode* node, int depth) {
    std::string indent(depth * 2, ' ');

    if (auto arr = dynamic_cast<ArrayDeclNode*>(node)) {
        std::cout << indent << "Array: " << arr->elementType.value << ", size " << arr->size << "\n";
        for (auto element : arr->elements) {
            printAST(element, depth + 1);
        }
    }

    else if (auto v = dynamic_cast<VarDeclNode*>(node)) {
        std::cout << indent << "VarDecl: " << v->name.value << " : " << v->type.value << "\n";
        printAST(v->value, depth + 1);
    }
    else if (auto b = dynamic_cast<BinaryExprNode*>(node)) {
        std::cout << indent << "BinaryExpr: " << b->op.value << "\n";
        printAST(b->left, depth + 1);
        printAST(b->right, depth + 1);
    }

    else if (auto n = dynamic_cast<NumberLiteralNode*>(node)) {
        std::cout << indent << "Number: " << n->value << "\n";
    }

    else if (auto i = dynamic_cast<IdentifierNode*>(node)) {
        std::cout << indent << "Identifier: " << i->value << "\n";
    }

    else if (auto blk = dynamic_cast<BlockNode*>(node)) {
        std::cout << indent << "Block:\n";
        for (auto stmt : blk->statements) {
            printAST(stmt, depth + 1);
        }
    }

    else if (auto ifNode = dynamic_cast<IfNode*>(node)) {
        std::cout << indent << "If:\n";
        std::cout << indent << "  Condition:\n";
        printAST(ifNode->condition, depth + 2);
        std::cout << indent << "  Then:\n";
        printAST(ifNode->thenBranch, depth + 2);
        if (ifNode->elseBranch != nullptr) {
            std::cout << indent << "  Else:\n";
            printAST(ifNode->elseBranch, depth + 2);
        }
    }

    else {
        std::cout << indent << "Unknown node\n";
    }
}