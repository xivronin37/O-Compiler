#pragma once

#include <vector>
#include <string>
#include <unordered_map>

enum class TokenType {
    Identifier,
    Keyword,
    Number,
    String,
    Plus, Minus, Star, FSlash, BSlash, Colon, Semicolon, Comma, LBracket, RBracket, LParen, RParen, LBrace, RBrace,
    Equal, EqualEqual, NotEqual, LessThan, GreaterThan, LessThanOrEqual, GreaterThanOrEqual, L_AND, L_OR, L_NOT, L_XOR,
    Punctuation,
    Bool,
    Null,
    Unknown,
    EndOfFile
};

inline const std::unordered_map<std::string, TokenType> keywords = {
    {"let", TokenType::Keyword},
    {"if", TokenType::Keyword},
    {"else", TokenType::Keyword},
    {"while", TokenType::Keyword},
    {"for", TokenType::Keyword},
    {"return", TokenType::Keyword},
    {"call", TokenType::Keyword},
    {"class", TokenType::Keyword},
    {"insert", TokenType::Keyword},
    {"from", TokenType::Keyword},
    {"as", TokenType::Keyword},
    {"inline", TokenType::Keyword},
    {"request", TokenType::Keyword},
    {"send", TokenType::Keyword},
    {"decouple", TokenType::Keyword}
};

struct Token {
    TokenType type;
    std::string value;
    int line = 1;
    int column = 1;
};

class Lexer {
    public:
        Lexer(const std::string& source);
        std::vector<Token> tokenize();
    private:
        std::string source;
        size_t pos = 0;
        int line = 1;
        int column = 1;

        char peek() const;
        char get();
        void skipWhitespace();
        Token nextToken();
};