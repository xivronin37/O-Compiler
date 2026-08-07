#include <vector>
#include <string>
#include "lexer.h"

char Lexer::peek() const {
    if (pos < source.size()) {
        return source[pos];
    }
    return '\0';
}
char Lexer::get() {
    char c = source[pos];
    column++;
    pos++;
    return c;
}

void Lexer::skipWhitespace() {
    while (pos < source.size() && std::isspace(source[pos])) {
        if (source[pos] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        pos++;
    }
}

Token Lexer::nextToken() {
    char c = peek();
    if (std::isalpha(c) || c == '_') {
        std::string identifier;
        while (std::isalnum(peek()) || peek() == '_') {
            identifier += get();
        }

        auto it = keywords.find(identifier);

        if (it != keywords.end()) {
            return {it->second, identifier, line, column};
        } else {
            return {TokenType::Identifier, identifier, line, column - static_cast<int>(identifier.size())};
        }
    }

    if (std::isdigit(c)) {
        std::string number;
        while (std::isdigit(peek())) {
            number += get();
        }
        return {TokenType::Number, number, line, column};
    }

    char op = get();
    switch (op) {
        default: return {TokenType::Unknown, std::string(1, op), line, column};
        case('+'): return {TokenType::Plus, "+", line, column};
        case('-'): return {TokenType::Minus, "-", line, column};
        case('*'): return {TokenType::Star, "*", line, column};
        case('/'): return {TokenType::FSlash, "/", line, column};
        case('\\'): return {TokenType::BSlash, "\\", line, column};
        case(':'): return {TokenType::Colon, ":", line, column};
        case(';'): return {TokenType::Semicolon, ";", line, column};
        case(','): return {TokenType::Comma, ",", line, column};
        case('['): return {TokenType::LBracket, "[", line, column};
        case(']'): return {TokenType::RBracket, "]", line, column};
        case('('): return {TokenType::LParen, "(", line, column};
        case(')'): return {TokenType::RParen, ")", line, column};
        case('{'): return {TokenType::LBrace, "{", line, column};
        case('}'): return {TokenType::RBrace, "}", line, column};
        case('='): {
            if (peek() == '=') {
                get();
                return {TokenType::EqualEqual, "==", line, column};
            }
            return {TokenType::Equal, "=", line, column};
        }
        case('!'): {
            if (peek() == '=') {
                get();
                return {TokenType::NotEqual, "!=", line, column};
            }
            return {TokenType::L_NOT, "!", line, column};
        }
        case('<'): {
            if (peek() == '=') {
                get();
                return {TokenType::LessThanOrEqual, "<=", line, column};
            }
            return {TokenType::LessThan, "<", line, column};
        }
        case('>'): {
            if (peek() == '=') {
                get();
                return {TokenType::GreaterThanOrEqual, ">=", line, column};
            }
            return {TokenType::GreaterThan, ">", line, column};
        }
    }

}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (pos < source.size()) {
        skipWhitespace();
        if (pos >= source.size()) {
            break;
        }
        tokens.push_back(nextToken());
    }
    return tokens;
}

Lexer::Lexer(const std::string& source) : source(source), pos(0), line(1), column(1) {
    
}