#include <iostream>
#include <string>
#include <format>
#include "lexer.h"
#include "ast.h"
#include "type.h"
#include "codegen.h"

int countVarDecl(ASTNode* node) {
    int counter = 0;

    if (auto varDecl = dynamic_cast<VarDeclNode*>(node)) {
        counter++;
    }

    if (auto block = dynamic_cast<BlockNode*>(node)) {
        for (auto statement : block->statements) {
            counter += countVarDecl(statement);
        }
    }

    if (auto ifNode = dynamic_cast<IfNode*>(node)) {
        counter += countVarDecl(ifNode->thenBranch);
        if (ifNode->elseBranch != nullptr) {
            counter += countVarDecl(ifNode->elseBranch);
        }
    }

    return counter;
}

void CodeGen::emit(const std::string& line, bool indent) {
    std::string current = (indent ? "\t" : "") + line + "\n";
    output += current;
}

void CodeGen::genNode(ASTNode* node) {
    if (auto num = dynamic_cast<NumberLiteralNode*>(node)) {
        std::string number = num->value;
        emit(std::format("movq ${}, %rax", number));
    }

    if (auto bin = dynamic_cast<BinaryExprNode*>(node)) {
        genNode(bin->left);
        emit("pushq %rax");
        genNode(bin->right);
        emit("movq %rax, %rbx");
        emit("popq %rax");


        switch(bin->op.type) {
            case TokenType::Plus: {
                emit("addq %rbx, %rax");
                break;
                }
            case TokenType::Minus: {
                emit("subq %rbx, %rax");
                break;
            }
            case TokenType::Star:{
                emit("imulq %rbx, %rax");
                break;
            }
            case TokenType::FSlash: {
                emit("cqto");
                emit("idivq %rbx");
                break;
            }
            case TokenType::EqualEqual: {
                emit("cmpq %rbx, %rax");
                emit("sete %al");
                emit("movzbq %al, %rax");
                break;
            }
            case TokenType::NotEqual: {
                emit("cmpq %rbx, %rax");
                emit("setne %al");
                emit("movzbq %al, %rax");
                break;
            }
            case TokenType::LessThan: {
                emit("cmpq %rbx, %rax");
                emit("setl %al");
                emit("movzbq %al, %rax");
                break;
            }
            case TokenType::LessThanOrEqual: {
                emit("cmpq %rbx, %rax");
                emit("setle %al");
                emit("movzbq %al, %rax");
                break;
            }
            case TokenType::GreaterThan: {
                emit("cmpq %rbx, %rax");
                emit("setg %al");
                emit("movzbq %al, %rax");
                break;
            }
            case TokenType::GreaterThanOrEqual: {
                emit("cmpq %rbx, %rax");
                emit("setge %al");
                emit("movzbq %al, %rax");
                break;
            }
            case TokenType::L_AND: {
                emit("andq %rbx, %rax");
                emit("movzbq %al, %rax");
                break;
            }
            case TokenType::L_OR: {
                emit("orq %rbx, %rax");
                emit("movzbq %al, %rax");
                break;
            }
        }   
    }

    if (auto block = dynamic_cast<BlockNode*>(node)) {
        for (auto statement : block->statements) {
            genNode(statement);
        }
    }

    if (auto varDecl = dynamic_cast<VarDeclNode*>(node)) {
        genNode(varDecl->value);
        std::string name = varDecl->name.value;


        currentOffset -= 8;
        symbolTable[name] = currentOffset;

        emit(std::format("movq %rax, {}(%rbp)", currentOffset));
    }

    if (auto id = dynamic_cast<IdentifierNode*>(node)) {
        auto it = symbolTable.find(id->value);

        if (it == symbolTable.end()) {
            throw std::runtime_error("Undefined variable: " + id->value);
        }

        int offset = it->second;

        emit(std::format("movq {}(%rbp), %rax", offset));
    }

    if (auto assign = dynamic_cast<AssignNode*>(node)) {
        genNode(assign->value);

        auto it = symbolTable.find(assign->target.value);

        if (it == symbolTable.end()) {
            throw std::runtime_error("Cannot assign to an undefined variable: " + assign->target.value);
        }

        int offset = it->second;

        emit(std::format("movq %rax, {}(%rbp)", offset));
    }

    if (auto ifNode = dynamic_cast<IfNode*>(node)) {
        int id = ifCounter++;

        std::string elseLabel = std::format(".L_else{}", id);
        std::string doneLabel = std::format(".L_done{}", id);

        genNode(ifNode->condition);

        emit("cmpq $0, %rax");

        std::string targetLabel = (ifNode->elseBranch != nullptr) ? elseLabel : doneLabel;
        emit(std::format("je {}", targetLabel));

        genNode(ifNode->thenBranch);

        if (ifNode->elseBranch != nullptr) {

            emit(std::format("jmp {}", doneLabel));

            emit(std::format("{}:", elseLabel), false);

            genNode(ifNode->elseBranch);
        }

        emit(std::format("{}:", doneLabel));
    }

    if (auto whileNode = dynamic_cast<WhileNode*>(node)) {
        int id = whileCounter++;
        std::string startLabel = std::format("START{}", id);
        std::string endLabel = std::format("END{}", id);

        emit(startLabel + ":", false);

        genNode(whileNode->condition);

        emit("cmpq $0, %rax");
        emit(std::format("je {}", endLabel));

        genNode(whileNode->body);

        emit(std::format("jmp {}", startLabel));

        emit(endLabel + ":", false);
    }

}

std::string CodeGen::generate(ASTNode* root) {
    emit(".global main", false);
    emit(".def main; .scl 2; .type 32; .endef", false);
    emit("main:", false);

    int counter = countVarDecl(root) * 8;
    emit("pushq %rbp");
    emit("movq %rsp, %rbp");
    emit(std::format("subq ${}, %rsp", counter));
    genNode(root);
    emit("leave");
    emit("popq %rbp");
    emit("ret");

    return output;
}