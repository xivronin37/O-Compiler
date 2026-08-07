#include <iostream>
#include "lexer.h"

int main() {
    std::string source = "let x: int = 5 + y;";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    for (const auto& token : tokens) {
        std::cout << "Token: " << token.value << std::endl;
    }
    return 0;
}
