#include "InstructionsGenerator.h"

InstructionsGenerator::InstructionsGenerator(ASTNodeBlock *global_block) : global_block(global_block), instructions(),
                                                                           instruction_counter(0), symtab(),
                                                                           declared_functions(), break_stack(),
                                                                           continue_stack() {
    /* Empty */
}

InstructionsGenerator::~InstructionsGenerator() = default;

void InstructionsGenerator::generate(const std::string &instruction, int level, int parameter) {
    this->instructions.push_back(Instruction{this->instruction_counter++, instruction, level, parameter});
}

void InstructionsGenerator::generate(InstructionIndex instruction, int level, int parameter) {
    this->generate(InstructionsTable[instruction], level, parameter);
}

std::vector<Instruction> &InstructionsGenerator::get_instructions() {
    return this->instructions;
}

Instruction &InstructionsGenerator::get_instruction(std::uint32_t index) {
    return this->instructions[index];
}

std::uint32_t InstructionsGenerator::get_instruction_counter() const {
    return this->instruction_counter;
}

void InstructionsGenerator::set_instruction_counter(std::uint32_t counter) {
    this->instruction_counter = counter;
}

void InstructionsGenerator::generate() {
    this->symtab.insert_scope(0, ACTIVATION_RECORD_SIZE); /* Offset 3 for activation record */
    this->generate("INT", 0, 0);

    for (auto &statement: this->global_block->statements)
        statement->accept(this);

    auto num_global_variables = symtab.get_number_of_variables();
    this->get_instruction(0).parameter = num_global_variables + ACTIVATION_RECORD_SIZE;

    auto main_address = symtab.get_symbol("main").address;
    this->generate("CAL", 0, main_address);
    this->generate("RET", 0, 0);
}

void InstructionsGenerator::visit(ASTNodeBlock *node) {
    auto instruction_line = this->get_instruction_counter();
    this->generate(INT, 0, 0);

    for (auto &statement: node->statements)
        statement->accept(this);

    auto number_of_variables = this->symtab.get_number_of_variables();
    auto &last_int_instr = this->get_instruction(instruction_line);
    last_int_instr.parameter = number_of_variables;

    /* TODO: fix addresses of all sub-scopes between this and BEGIN_BLOCK *OR* change grammar */

    this->generate(INT, 0, -number_of_variables);
    this->symtab.remove_scope();
}

void InstructionsGenerator::visit(ASTNodeDeclVar *node) {
    this->symtab.insert_symbol(node->name, VARIABLE, node->type, node->is_const);
    if (node->expression) {
        node->expression->accept(this);
        auto address = this->symtab.get_symbol(node->name).address;
        this->generate(STO, 0, address);
    }
}

void InstructionsGenerator::visit(ASTNodeDeclFunc *node) { /* TODO: parameters are not implemented yet */
    this->generate(JMP, 0, 0);
    auto func_address = this->get_instruction_counter();
    auto &symbol = this->symtab.get_symbol(node->name);

    if (symbol.name == "") { /* Function was not yet declared */
        this->symtab.insert_symbol(node->name, FUNCTION, node->return_type, false, func_address);
        this->declared_functions[node->name] = func_address;
    } else { /* Function was earlier declared as a header only */
        symbol.address = func_address;
        auto jump_instr_index = this->declared_functions[node->name];
        auto &jump_instr = this->get_instruction(jump_instr_index);
        jump_instr.parameter = func_address;
    }

    if (node->block) {
        this->symtab.insert_scope(0, ACTIVATION_RECORD_SIZE, true); /* Offset 3 for activation record */
        this->generate(INT, 0, ACTIVATION_RECORD_SIZE);

        node->block->accept(this);
    } else {
        this->generate(JMP, 0, 0);
    }

    auto jump_over_func_instr_index = func_address - 1;
    auto &jump_over_func_instr = this->get_instruction(jump_over_func_instr_index);
    jump_over_func_instr.parameter = this->get_instruction_counter();
}

void InstructionsGenerator::visit(ASTNodeIf *node) {
    auto current_scope = this->symtab.get_current_scope();
    auto new_base = current_scope.get_address_base() + current_scope.get_address_offset();
    this->symtab.insert_scope(new_base, 0, false);
    node->condition->accept(this);

    auto jmc_instruction_line = this->get_instruction_counter();
    this->generate(JMC, 0, 0);
    node->block->accept(this);

    auto jmp_instruction_line = this->get_instruction_counter();
    auto &jmc_instruction = this->get_instruction(jmc_instruction_line);
    jmc_instruction.parameter = jmp_instruction_line + 1;
    this->generate(JMP, 0, 0);

    if (node->else_block) {
        auto current_scope = this->symtab.get_current_scope();
        auto new_base = current_scope.get_address_base() + current_scope.get_address_offset();
        this->symtab.insert_scope(new_base, 0, false);

        node->else_block->accept(this);
    }

    auto post_if_instruction_line = this->get_instruction_counter();
    auto &jmp_instruction = this->get_instruction(jmp_instruction_line);
    jmp_instruction.parameter = post_if_instruction_line;
}

void InstructionsGenerator::visit(ASTNodeWhile *node) {
    node->block->count_breaks_and_continues();
    node->break_number = node->block->break_number;
    node->continue_number = node->block->continue_number;

    auto current_scope = this->symtab.get_current_scope();
    auto new_base = current_scope.get_address_base() + current_scope.get_address_offset();
    this->symtab.insert_scope(new_base, 0, false);

    if (node->is_do_while) {
        auto block_instruction_line = this->get_instruction_counter();
        node->block->accept(this);

        auto condition_instruction_line = this->get_instruction_counter();
        node->condition->accept(this);

        auto jmc_instruction_line = this->get_instruction_counter();
        this->generate(JMC, 0, 0);
        this->generate(JMP, 0, block_instruction_line);
        auto post_while_instruction_line = this->get_instruction_counter();
        auto &jmc_instruction = this->get_instruction(jmc_instruction_line);
        jmc_instruction.parameter = post_while_instruction_line;

        for (int i = 0; i < node->break_number; i++) {
            auto break_jmp_instruction_line = this->break_stack.back();
            this->break_stack.pop_back();
            auto &break_jmp_instruction = this->get_instruction(break_jmp_instruction_line);
            break_jmp_instruction.parameter = post_while_instruction_line;
        }
        for (int i = 0; i < node->continue_number; i++) {
            auto continue_jmp_instruction_line = this->continue_stack.back();
            this->continue_stack.pop_back();
            auto &continue_jmp_instruction = this->get_instruction(continue_jmp_instruction_line);
            continue_jmp_instruction.parameter = condition_instruction_line;
        }
    } else {
        auto condition_instruction_line = this->get_instruction_counter();
        node->condition->accept(this);

        auto jmc_instruction_line = this->get_instruction_counter();
        this->generate(JMC, 0, 0);
        node->block->accept(this);

        this->generate(JMP, 0, condition_instruction_line);
        auto post_while_instruction_line = this->get_instruction_counter();
        auto &jmc_instruction = this->get_instruction(jmc_instruction_line);
        jmc_instruction.parameter = post_while_instruction_line;

        for (int i = 0; i < node->break_number; i++) {
            auto break_jmp_instruction_line = this->break_stack.back();
            this->break_stack.pop_back();
            auto &break_jmp_instruction = this->get_instruction(break_jmp_instruction_line);
            break_jmp_instruction.parameter = post_while_instruction_line;
        }
        for (int i = 0; i < node->continue_number; i++) {
            auto continue_jmp_instruction_line = this->continue_stack.back();
            this->continue_stack.pop_back();
            auto &continue_jmp_instruction = this->get_instruction(continue_jmp_instruction_line);
            continue_jmp_instruction.parameter = condition_instruction_line;
        }
    }
}

void InstructionsGenerator::visit(ASTNodeFor *node) { /* TODO: for is not implemented yet */
    /* Empty */
}

void InstructionsGenerator::visit(ASTNodeBreakContinue *node) {
    auto jmp_instruction_line = this->get_instruction_counter();
    this->generate(JMP, 0, 0);
    if (node->is_break)
        this->break_stack.push_back(jmp_instruction_line);
    else
        this->continue_stack.push_back(jmp_instruction_line);
}

void InstructionsGenerator::visit(ASTNodeReturn *node) { /* TODO: return with expression is not implemented yet */
    if (node->expression)
        node->expression->accept(this);
    this->generate(RET, 0, 0);
}

void InstructionsGenerator::visit(ASTNodeExpressionStatement *node) {
    node->expression->accept(this);
}

void InstructionsGenerator::visit(ASTNodeIdentifier *node) {
    auto address = this->symtab.get_symbol(node->name).address;
    auto level = this->symtab.get_symbol_level(node->name);
    this->generate(LOD, level, address);
}

void InstructionsGenerator::visit(ASTNodeIntLiteral *node) {
    this->generate(LIT, 0, node->value);
}

void InstructionsGenerator::visit(ASTNodeBoolLiteral *node) {
    int value = node->value ? 1 : 0;
    this->generate(LIT, 0, value);
}

void InstructionsGenerator::visit(ASTNodeAssignExpression *node) {
    node->expression->accept(this);
    auto address = this->symtab.get_symbol(node->name).address;
    auto level = this->symtab.get_symbol_level(node->name);
    this->generate(STO, level, address);
}

void InstructionsGenerator::visit(ASTNodeBinaryOperator *node) {
    node->left->accept(this);
    node->right->accept(this);

    if (node->op == "&&") { /* AND: 1 * 1 = 1, 1 * 0 = 0, 0 * 1 = 0, 0 * 0 = 0 */
        this->generate(OPR, 0, PL0_MUL);
        this->generate(LIT, 0, 0);
        this->generate(OPR, 0, PL0_NEQ);
    } else if (node->op == "||") { /* OR: 1 + 1 = 2, 1 + 0 = 1, 0 + 1 = 1, 0 + 0 = 0 */
        this->generate(OPR, 0, PL0_ADD);
        this->generate(LIT, 0, 0);
        this->generate(OPR, 0, PL0_NEQ);
    } else {
        this->generate(OPR, 0, OperatorsTable.find(node->op)->second);
    }
}

void InstructionsGenerator::visit(ASTNodeUnaryOperator *node) {
    node->expression->accept(this);

    if(node->op == "!") { /* NOT: true == 0 => false, false == 0 => true */
        this->generate(LIT, 0, 0);
        this->generate(OPR, 0, PL0_EQ);
    } else if (node->op == "-") {
        this->generate(OPR, 0, PL0_NEG);
    }
}

void InstructionsGenerator::visit(ASTNodeCast *node) { /* TODO: cast is not implemented yet */
    /* Empty */
}

void InstructionsGenerator::visit(ASTNodeCallFunc *node) { /* TODO: arguments are not implemented yet */
    auto address = this->symtab.get_symbol(node->name).address;
    auto level = this->symtab.get_symbol_level(node->name);
    this->generate(CAL, level, address);
}
