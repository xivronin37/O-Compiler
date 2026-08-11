#include <iostream>
#include <fstream>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "type.h"
#include "codegen.h"

int main() {
    try {
        std::string source = readFile("C:/Projects/O Compiler/test.ol");
        
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        for (auto token : tokens) {
            std::cout << "Type: " << tokenTypeName(token.type) << " Lexeme: " << token.value << std::endl;
        }

        Parser parser(tokens);

        ASTNode* root = parser.parse();
        
        TypeChecker checker;

        checker.TypeCheck(root);
        
        TokenType resultType = checker.TypeCheck(root);

        std::cout << "TypeCheck succeeded. Result type: " << tokenTypeName(resultType) << "\n";

        CodeGen codegen;

        std::string assembly = codegen.generate(root); 

        std::ofstream out("output.s");
        out << assembly;
        out.close();
}
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}