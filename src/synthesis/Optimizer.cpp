#include "Optimizer.h"

Optimizer::Optimizer() = default;

Optimizer::~Optimizer() = default;

void Optimizer::optimize_ast(ASTNodeBlock *global_block) {
    global_block->accept(this);
}

void Optimizer::optimize_instructions(std::vector<Instruction> &instructions) {
    for (int i = 0; i < instructions.size(); i++) {
        auto &instruction = instructions[i];

        if (instruction.instruction == "OPR") {
            if (instruction.parameter == PL0_ADD || instruction.parameter == PL0_SUB) {
                auto &previous_instruction = instructions[i - 1];
                if (previous_instruction.instruction == "LIT" && previous_instruction.parameter == 0) {
                    instruction.instruction = "DELETE";
                    previous_instruction.instruction = "DELETE";
                }
            }
            if (instruction.parameter == PL0_MUL || instruction.parameter == PL0_DIV) {
                auto &previous_instruction = instructions[i - 1];
                if (previous_instruction.instruction == "LIT" && previous_instruction.parameter == 1) {
                    instruction.instruction = "DELETE";
                    previous_instruction.instruction = "DELETE";
                }
            }
        }

        if (instruction.instruction == "JMP" || instruction.instruction == "JMC" || instruction.instruction == "CAL") {
            auto &jump_destination = instructions[instruction.parameter];
            while(jump_destination.instruction == "JMP") {
                instruction.parameter = jump_destination.parameter;
                jump_destination.instruction = "DELETE";
                jump_destination = instructions[instruction.parameter];
            }
        }
    }

    instructions.erase(std::remove_if(instructions.begin(), instructions.end(), [](const Instruction &instruction) {
        return instruction.instruction == "DELETE";
    }), instructions.end());

    int line = 0;
    std::map<int, int> old_new_map;
    for (auto &instruction: instructions) {
        old_new_map[instruction.line] = line;
        instruction.line = line++;
    }

    for (auto &instruction: instructions) {
        if (instruction.instruction == "JMP" || instruction.instruction == "JMC" || instruction.instruction == "CAL") {
            instruction.parameter = old_new_map[instruction.parameter];
        }
    }
}

void Optimizer::visit(ASTNodeBlock *node) {
    for (auto &statement: node->statements)
        statement->accept(this);
}

void Optimizer::visit(ASTNodeDeclVar *node) {
    if (node->expression)
        node->expression->accept(this);
}

void Optimizer::visit(ASTNodeDeclFunc *node) {
    if (node->block)
        node->block->accept(this);
}

void Optimizer::visit(ASTNodeIf *node) {
    node->condition->accept(this);
    node->block->accept(this);
    if (node->else_block)
        node->else_block->accept(this);
}

void Optimizer::visit(ASTNodeWhile *node) {
    node->condition->accept(this);
    node->block->accept(this);
}

void Optimizer::visit(ASTNodeFor *node) {
    if (node->init)
        node->init->accept(this);
    if (node->condition)
        node->condition->accept(this);
    if (node->increment)
        node->increment->accept(this);
    node->block->accept(this);
}

void Optimizer::visit(ASTNodeBreakContinue *node) {
    /* Nothing to do */
}

void Optimizer::visit(ASTNodeReturn *node) {
    if (node->expression)
        node->expression->accept(this);
}

void Optimizer::visit(ASTNodeGoto *node) {
    /* Nothing to do */
}

void Optimizer::visit(ASTNodeExpressionStatement *node) {
    node->expression->accept(this);
}

void Optimizer::visit(ASTNodeIdentifier *node) {
    /* Nothing to do */
}

void Optimizer::visit(ASTNodeIntLiteral *node) {
    /* Nothing to do */
}

void Optimizer::visit(ASTNodeBoolLiteral *node) {
    /* Nothing to do */
}

void Optimizer::visit(ASTNodeStringLiteral *node) {
    /* Nothing to do */
}

void Optimizer::visit(ASTNodeFloatLiteral *node) {
    /* Nothing to do */
}

void Optimizer::visit(ASTNodeAssignExpression *node) {
    if (node->lvalue)
        node->lvalue->accept(this);
    if (node->expression)
        node->expression->accept(this);
}

void Optimizer::visit(ASTNodeTernaryOperator *node) {
    node->condition->accept(this);
    node->true_expression->accept(this);
    node->false_expression->accept(this);
}

void Optimizer::visit(ASTNodeBinaryOperator *node) {
    node->left->accept(this);
    node->right->accept(this);
}

void Optimizer::visit(ASTNodeUnaryOperator *node) {
    node->expression->accept(this);
}

void Optimizer::visit(ASTNodeCast *node) {
    node->expression->accept(this);
}

void Optimizer::visit(ASTNodeCallFunc *node) {
    for (auto &argument: node->arguments)
        argument->accept(this);
}

void Optimizer::visit(ASTNodeNew *node) {
    node->expression->accept(this);
}

void Optimizer::visit(ASTNodeDelete *node) {
    node->expression->accept(this);
}

void Optimizer::visit(ASTNodeDereference *node) {
    node->expression->accept(this);
}

void Optimizer::visit(ASTNodeReference *node) {
    /* Nothing to do */
}

void Optimizer::visit(ASTNodeSizeof *node) {
    /* Nothing to do */
}
