#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <stdexcept>

enum class TokenType {
    Identifier,
    Keyword,
    Let, If, Else, While, For, Return, Call, Create, Class, Insert, From, As, Inline, Request, Send, Decouple,
    Number,
    String,
    Plus, Minus, Star, FSlash, BSlash, Colon, Semicolon, Comma, LBracket, RBracket, LParen, RParen, LBrace, RBrace, Tilde,
    Equal, EqualEqual, NotEqual, LessThan, GreaterThan, LessThanOrEqual, GreaterThanOrEqual,
    L_AND, L_OR, L_NOT, L_XOR, B_AND, B_OR, B_NOT, B_XOR,
    Punctuation,
    Int, Bool, Float, Null, UnsignedInt, UnsignedFloat,
    Unknown,
    EndOfFile
};

inline const std::unordered_map<std::string, TokenType> keywords = {
    {"let", TokenType::Let},
    {"if", TokenType::If},
    {"else", TokenType::Else},
    {"while", TokenType::While},
    {"for", TokenType::For},
    {"return", TokenType::Return},
    {"call", TokenType::Call},
    {"create", TokenType::Create},
    {"class", TokenType::Class},
    {"insert", TokenType::Insert},
    {"from", TokenType::From},
    {"as", TokenType::As},
    {"inline", TokenType::Inline},
    {"request", TokenType::Request},
    {"send", TokenType::Send},
    {"decouple", TokenType::Decouple}
};

inline const std::unordered_map<std::string, TokenType> types = {
    {"i", TokenType::Int},
    {"b", TokenType::Bool},
    {"f", TokenType::Float},
    {"n", TokenType::Null},
    {"ui", TokenType::UnsignedInt},
    {"uf", TokenType::UnsignedFloat}
};


struct Token {
    TokenType type;
    std::string value;
    int line = 1;
    int column = 0;
};

class Lexer {
    public:
        Lexer(const std::string& source);
        std::vector<Token> tokenize();
    private:
        std::string source;
        size_t pos = 0;
        int line = 1;
        int column = 0;

        char peek() const;
        char advance();
        void skipWhitespace();
        Token nextToken();
};

std::string readFile(const std::string& filename);