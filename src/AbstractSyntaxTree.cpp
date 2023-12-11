#include "AbstractSyntaxTree.h"

void ASTNodeBlock::count_breaks_and_continues() {
    for (auto &statement: statements) {
        if (auto *block = dynamic_cast<ASTNodeBlock *>(statement)) {
            block->count_breaks_and_continues();
            this->break_number += block->break_number;
            this->continue_number += block->continue_number;
        } else if (auto *break_continue = dynamic_cast<ASTNodeBreakContinue *>(statement)) {
            if (break_continue->is_break)
                this->break_number++;
            else
                this->continue_number++;
        } else if (auto *if_stmt = dynamic_cast<ASTNodeIf *>(statement)) {
            if_stmt->block->count_breaks_and_continues();
            this->break_number += if_stmt->block->break_number;
            this->continue_number += if_stmt->block->continue_number;
            if (if_stmt->else_block) {
                if_stmt->else_block->count_breaks_and_continues();
                this->break_number += if_stmt->else_block->break_number;
                this->continue_number += if_stmt->else_block->continue_number;
            }
        }
    }
}

int ASTNodeBlock::get_number_of_declared_variables() {
    auto declared_variables = 0;
    for (auto &statement : statements) {
        if (auto *variable_declaration = dynamic_cast<ASTNodeDeclVar *>(statement))
            declared_variables++;
    }
    return declared_variables;
}

std::vector<uint32_t> ASTNodeBlock::get_sizeof_variables() {
    std::vector<uint32_t> sizeof_variables;
    for (auto &statement : statements) {
        if (auto *variable_declaration = dynamic_cast<ASTNodeDeclVar *>(statement))
            sizeof_variables.push_back(variable_declaration->sizeof_type);
    }
    return sizeof_variables;
}

bool ASTNodeBlock::contains_return_statement() {
    if (this->statements.empty())
        return false;

    auto last_statement = this->statements.back();
    if (auto *return_statement = dynamic_cast<ASTNodeReturn *>(last_statement))
        return true;
    else if (auto *if_statement = dynamic_cast<ASTNodeIf *>(last_statement))
        return if_statement->contains_return_statement();

    return false;
}

bool ASTNodeIf::contains_return_statement() {
    if (this->else_block)
        return this->block->contains_return_statement() && this->else_block->contains_return_statement();
    return false; /* If last statement was if, it should contain else to cover all cases */
}

bool ASTNodeBinaryOperator::contains_reference() {
    if (dynamic_cast<ASTNodeReference *>(this->left) || dynamic_cast<ASTNodeReference *>(this->right))
        return true;
    else if (auto *left_binary_operator = dynamic_cast<ASTNodeBinaryOperator *>(this->left))
        return left_binary_operator->contains_reference();
    else if (auto *right_binary_operator = dynamic_cast<ASTNodeBinaryOperator *>(this->right))
        return right_binary_operator->contains_reference();
    return false;
}

std::string ASTNodeBinaryOperator::find_dereference() {
    std::string result;
    if (auto *left_binary_operator = dynamic_cast<ASTNodeBinaryOperator *>(this->left)) {
        result = left_binary_operator->find_dereference();
        if (!result.empty())
            return result;
    } else if (auto *right_binary_operator = dynamic_cast<ASTNodeBinaryOperator *>(this->right)) {
        result = right_binary_operator->find_dereference();
        if (!result.empty())
            return result;
    } else if (auto *left_id = dynamic_cast<ASTNodeIdentifier *>(this->left))
        return left_id->name;
    else if (auto *right_id = dynamic_cast<ASTNodeIdentifier *>(this->right))
        return right_id->name;
    return result;
}

void ASTNodeBinaryOperator::is_float_arithmetic_check() {
    if (auto *left_binary_operator = dynamic_cast<ASTNodeBinaryOperator *>(this->left)) {
        left_binary_operator->is_float_arithmetic_check();
        if (left_binary_operator->is_float_arithmetic)
            this->is_float_arithmetic = true;
    } else if (dynamic_cast<ASTNodeFloatLiteral *>(this->left)) {
        this->is_float_arithmetic = true;
    }
    if (auto *right_binary_operator = dynamic_cast<ASTNodeBinaryOperator *>(this->right)) {
        right_binary_operator->is_float_arithmetic_check();
        if (right_binary_operator->is_float_arithmetic)
            this->is_float_arithmetic = true;
    } else if (dynamic_cast<ASTNodeFloatLiteral *>(this->right)) {
        this->is_float_arithmetic = true;
    }
}

void ASTNodeBinaryOperator::propagate_float() {
    if (auto *left_binary_operator = dynamic_cast<ASTNodeBinaryOperator *>(this->left)) {
        left_binary_operator->propagate_float();
        this->is_float_arithmetic = left_binary_operator->is_float_arithmetic;
    }
    if (auto *right_binary_operator = dynamic_cast<ASTNodeBinaryOperator *>(this->right)) {
        right_binary_operator->propagate_float();
        this->is_float_arithmetic = right_binary_operator->is_float_arithmetic;
    }
}

void ASTNodeDereference::what_do_i_dereference() {
    if (auto *id = dynamic_cast<ASTNodeIdentifier *>(this->expression)) {
        this->identifier = id->name;
    } else if (auto *binary_operator = dynamic_cast<ASTNodeBinaryOperator *>(this->expression)) {
       this->identifier = binary_operator->find_dereference();
    }
}
