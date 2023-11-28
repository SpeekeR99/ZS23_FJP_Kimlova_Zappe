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

std::vector<int> ASTNodeBlock::get_sizeof_variables() {
    std::vector<int> sizeof_variables;
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
