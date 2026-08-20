#include <iostream>
#include <string>
#include <format>
#include "lexer.h"
#include "ast.h"
#include "type.h"
#include "codegen.h"

int countVarDecl(ASTNode* node) {
    int counter = 0;

    if (auto arrDecl = dynamic_cast<ArrayDeclNode*>(node)) {
        counter += arrDecl->elements.size();
        counter++;
    }

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

    if (auto whileNode = dynamic_cast<WhileNode*>(node)) {
        counter += countVarDecl(whileNode->body);
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

    if (auto arrDecl = dynamic_cast<ArrayDeclNode*>(node)) {
        std::string name = arrDecl->name.value;

        currentOffset -= 8;
        emit(std::format("movq ${}, {}(%rbp)", arrDecl->elements.size(), currentOffset));

        int baseOffset = currentOffset - 8;

        for (size_t i = 0; i < arrDecl->elements.size(); i++) {
            genNode(arrDecl->elements[i]);
            currentOffset -= 8;

            emit(std::format("movq %rax, {}(%rbp)", currentOffset));
        }

        symbolTable[name] = baseOffset;
    }
    
    else if (auto varDecl = dynamic_cast<VarDeclNode*>(node)) {
        std::string name = varDecl->name.value;

        if (auto inst = dynamic_cast<InstanceNode*>(varDecl->value)) {
            int baseOffset = currentOffset - 8;

            for (size_t i = 0; i < arrDecl->elements.size(); i++) {
                genNode(inst->arguments[i]);

                currentOffset -= 8;
                emit(std::format("movq %rax, {}(%rbp)", currentOffset));
            }

            symbolTable[name] = baseOffset;
        }
        else {
            genNode(varDecl->value);
            currentOffset -= 8;
            symbolTable[name] = currentOffset;
            emit(std::format("movq %rax, {}(%rbp)", currentOffset));
        }
    }
    
    if (auto idx = dynamic_cast<IndexNode*>(node)) {
    genNode(idx->index);
    int baseOffset = symbolTable[idx->name.value];
    emit(std::format("movq {}(%rbp, %rax, 8), %rax", baseOffset));
    }

    if (auto field = dynamic_cast<FieldAccessNode*>(node)) {
        auto target = dynamic_cast<IdentifierNode*>(field->target);

        if (!target) {
            throw std::runtime_error("CG: E36-1 | No identifier found for field access");
        }

        int baseOffset = symbolTable[target->value];
        std::vector<Param> fields = typeCheck.structTable[typeCheck.instances[target->value]];

        bool found = false;
        int count = -1;

        for (auto& checkedField : fields) {
            count++;
            if (checkedField.name.value == field->field.value) {
                found = true;
                break;
            }
        }

        if (!found) {
            throw std::runtime_error("CG: E35-1 | Undefined field: " + target->value);
        }

        int finalOffset = baseOffset - count*8;

        emit(std::format("movq {}(%rbp), %rax", finalOffset));
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
        if (auto id = dynamic_cast<IdentifierNode*>(assign->target)) {
            genNode(assign->value);
            auto it = symbolTable.find(id->value);

            if (it == symbolTable.end()) {
                throw std::runtime_error("CG: E37 | Cannot assign to an undefined variable: " + id->value);
            }

            emit(std::format("movq %rax, {}(%rbp)", it->second));
        }

        else if (auto field = dynamic_cast<FieldAccessNode*>(assign->target)) {
            genNode(assign->value);

            auto target = dynamic_cast<IdentifierNode*>(field->target);

            if (!target) {
                throw std::runtime_error("CG: E36-2 | No identifier found for field access");
            }

            int baseOffset = symbolTable[target->value];
            std::vector<Param> fields = typeCheck.structTable[typeCheck.instances[target->value]];

            bool found = false;
            int count = -1;

            for (auto& checkedField : fields) {
                count++;
                if (checkedField.name.value == field->field.value) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                throw std::runtime_error("CG: E35-2 | Undefined field: " + target->value);
            }

            int finalOffset = baseOffset - count*8;

            emit(std::format("movq {}(%rbp), %rax", finalOffset));
        }

        else if (auto idx = dynamic_cast<IndexNode*>(assign->target)) {
            genNode(assign->value);
            emit("pushq %rax");

            genNode(idx->index);
            emit("movq %rax, %rbx");

            emit("popq %rax");

            int baseOffset = symbolTable[idx->name.value];
            emit(std::format("movq %rax, {}(%rbp,%rbx,8)", baseOffset));
        }

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

    if (auto funcDecl = dynamic_cast<FuncDeclNode*>(node)) {
        emit(std::format("jmp .L_skip_{}", funcDecl->name.value));
        emit(funcDecl->name.value + ":", false);
        emit("pushq %rbp");
        emit("movq %rsp, %rbp");

        int allocatedBytes = (countVarDecl(funcDecl->body) + funcDecl->parameters.size()) * 8;

        emit(std::format("subq ${}, %rsp", allocatedBytes));

        for (int i = 1; i <= funcDecl->parameters.size(); i++) {
            funcOffset-=8;
            symbolTable[funcDecl->parameters[i-1].name.value] = funcOffset;
            switch(i) {
                default: {
                    throw std::runtime_error("Cannot have more than four parameters: " + funcDecl->name.value);
                }
                case 1: {
                    emit(std::format("movq %rcx, {}(%rbp)", funcOffset));
                    break;
                }
                case 2: {
                    emit(std::format("movq %rdx, {}(%rbp)", funcOffset));
                    break;
                }
                case 3: {
                    emit(std::format("movq %r8, {}(%rbp)", funcOffset));
                    break;
                }
                case 4: {
                    emit(std::format("movq %r9, {}(%rbp)", funcOffset));
                    break;
                }
                }
            }


        genNode(funcDecl->body);

        funcOffset = 0;

        emit("leave");
        emit("ret");
        emit(std::format(".L_skip_{}:", funcDecl->name.value), false);
    }

    if (auto callNode = dynamic_cast<CallNode*>(node)) {
        for (int i = 0; i < callNode->arguments.size(); i++) {
            genNode(callNode->arguments[i]);
            switch(i + 1) {
                default:
                    throw std::runtime_error("Cannot have more than six arguments: " + callNode->name.value);
                case 1: {
                    emit("movq %rax, %rcx");
                    break;
                }
                case 2: {
                    emit("movq %rax, %rdx");
                    break;
                }
                case 3: {
                    emit("movq %rax, %r8");
                    break;
                }
                case 4: {
                    emit("movq %rax, %r9");
                    break;
                }
            }
        }
        
        emit("subq $32, %rsp");
        emit(std::format("call {}", callNode->name.value));
        emit("addq $32, %rsp");
    }

    if (auto outNode = dynamic_cast<OutNode*>(node)) {
        genNode(outNode->output);

        emit("leave");
        emit("ret");
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
    emit("ret");

    return output;
}