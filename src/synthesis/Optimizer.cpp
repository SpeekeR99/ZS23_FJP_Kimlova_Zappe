#include "Optimizer.h"

Optimizer::Optimizer() = default;

Optimizer::~Optimizer() = default;

void Optimizer::optimize_ast(ASTNodeBlock *global_block) {
    global_block->accept(this);
}

void Optimizer::optimize_instructions(std::vector<Instruction> &instructions) {
    /* Remove middle steps of jumps leading to unconditional jumps */
    for (int i = 0; i < instructions.size(); i++) {
        auto &instruction = instructions[i];

        if (instruction.instruction == "JMP" || instruction.instruction == "JMC" || instruction.instruction == "CAL") {
            auto &jump_destination = instructions[instruction.parameter];
            while(jump_destination.instruction == "JMP") {
                instruction.parameter = jump_destination.parameter;
                jump_destination.instruction = "DELETE";
                jump_destination = instructions[instruction.parameter];
            }
        }
    }

    /* Remove the instructions marked for deletion */
    instructions.erase(std::remove_if(instructions.begin(), instructions.end(), [](const Instruction &instruction) {
        return instruction.instruction == "DELETE";
    }), instructions.end());

    /* Map old line numbers to new line numbers */
    int line = 0;
    std::map<int, int> old_new_map;
    for (auto &instruction: instructions) {
        old_new_map[instruction.line] = line;
        instruction.line = line++;
    }

    /* Fix the line numbers in the instructions */
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

void Optimizer::modify_ast_parent_child(ASTNode *node, ASTNode *parent, ASTNodeExpression *new_child) {
    if (auto statement_parent = dynamic_cast<ASTNodeExpressionStatement *>(parent)) {
        statement_parent->expression = new_child;
        new_child->parent = statement_parent;
    } else if (auto assign_parent = dynamic_cast<ASTNodeAssignExpression *>(parent)) {
        assign_parent->expression = new_child;
        new_child->parent = assign_parent;
    } else if (auto decl_var_parent = dynamic_cast<ASTNodeDeclVar *>(parent)) {
        decl_var_parent->expression = new_child;
        new_child->parent = decl_var_parent;
    } else if (auto binary_parent = dynamic_cast<ASTNodeBinaryOperator *>(parent)) {
        if (binary_parent->left == node) {
            binary_parent->left = new_child;
            new_child->parent = binary_parent;
        } else if (binary_parent->right == node) {
            binary_parent->right = new_child;
            new_child->parent = binary_parent;
        }
    } else if (auto if_parent = dynamic_cast<ASTNodeIf *>(parent)) {
        if (if_parent->condition == node) {
            if_parent->condition = new_child;
            new_child->parent = if_parent;
        }
    } else if (auto while_parent = dynamic_cast<ASTNodeWhile *>(parent)) {
        if (while_parent->condition == node) {
            while_parent->condition = new_child;
            new_child->parent = while_parent;
        }
    } else if (auto for_parent = dynamic_cast<ASTNodeFor *>(parent)) {
        if (for_parent->condition == node) {
            for_parent->condition = new_child;
            new_child->parent = for_parent;
        } else if (for_parent->increment == node) {
            for_parent->increment = new_child;
            new_child->parent = for_parent;
        }
    } else if (auto return_parent = dynamic_cast<ASTNodeReturn *>(parent)) {
        if (return_parent->expression == node) {
            return_parent->expression = new_child;
            new_child->parent = return_parent;
        }
    } else if (auto ternary_parent = dynamic_cast<ASTNodeTernaryOperator *>(parent)) {
        if (ternary_parent->condition == node) {
            ternary_parent->condition = new_child;
            new_child->parent = ternary_parent;
        } else if (ternary_parent->true_expression == node) {
            ternary_parent->true_expression = new_child;
            new_child->parent = ternary_parent;
        } else if (ternary_parent->false_expression == node) {
            ternary_parent->false_expression = new_child;
            new_child->parent = ternary_parent;
        }
    }
}

template<typename T>
void Optimizer::binary_op_check_case(ASTNodeBinaryOperator *node, ASTNode *node_to_check, int value_to_check, ASTNodeExpression *node_to_set) {
    /* If node_to_check is of type T */
    if (auto node_to_check_t = dynamic_cast<T *>(node_to_check)) {
        /* and if it has the value value_to_check */
        if (node_to_check_t->value == value_to_check) {
            /* then replace node with node_to_set */
            auto parent = node->parent;
            if (parent) {
                modify_ast_parent_child(node, parent, node_to_set);
                parent->accept(this);
            }
        }
    }
}

void Optimizer::visit(ASTNodeBinaryOperator *node) {
    node->left->accept(this);
    node->right->accept(this);
    if (node->op == "+" || node->op == "-") {
        binary_op_check_case<ASTNodeIntLiteral>(node, node->left, 0, node->right);
        binary_op_check_case<ASTNodeIntLiteral>(node, node->right, 0, node->left);
        binary_op_check_case<ASTNodeFloatLiteral>(node, node->left, 0, node->right);
        binary_op_check_case<ASTNodeFloatLiteral>(node, node->right, 0, node->left);
    } else if (node->op == "*" || node->op == "/") {
        binary_op_check_case<ASTNodeIntLiteral>(node, node->left, 0, node->left);
        binary_op_check_case<ASTNodeIntLiteral>(node, node->right, 0, node->right);
        binary_op_check_case<ASTNodeFloatLiteral>(node, node->left, 0, node->left);
        binary_op_check_case<ASTNodeFloatLiteral>(node, node->right, 0, node->right);
        binary_op_check_case<ASTNodeIntLiteral>(node, node->left, 1, node->right);
        binary_op_check_case<ASTNodeIntLiteral>(node, node->right, 1, node->left);
        binary_op_check_case<ASTNodeFloatLiteral>(node, node->left, 1, node->right);
        binary_op_check_case<ASTNodeFloatLiteral>(node, node->right, 1, node->left);
    } else if (node->op == "&&") {
        binary_op_check_case<ASTNodeBoolLiteral>(node, node->left, false, node->left);
        binary_op_check_case<ASTNodeBoolLiteral>(node, node->right, false, node->right);
        binary_op_check_case<ASTNodeBoolLiteral>(node, node->left, true, node->right);
        binary_op_check_case<ASTNodeBoolLiteral>(node, node->right, true, node->left);
    } else if (node->op == "||") {
        binary_op_check_case<ASTNodeBoolLiteral>(node, node->left, false, node->right);
        binary_op_check_case<ASTNodeBoolLiteral>(node, node->right, false, node->left);
        binary_op_check_case<ASTNodeBoolLiteral>(node, node->left, true, node->left);
        binary_op_check_case<ASTNodeBoolLiteral>(node, node->right, true, node->right);
    }
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
