#include "InstructionsGenerator.h"

InstructionsGenerator::InstructionsGenerator(ASTNodeBlock *global_block, std::vector<std::string> &used_builtin_functions) : global_block(global_block), used_builtin_functions(used_builtin_functions), instructions(),
                                                                           instruction_counter(0), symtab(), number_of_declared_variables(),
                                                                           declared_functions(), break_stack(),
                                                                           continue_stack(), sizeof_params_stack(), sizeof_return_type_stack() {
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

void InstructionsGenerator::gen_print_num() {
    this->symtab.insert_scope(0, ACTIVATION_RECORD_SIZE, true);

    this->generate(PL0_INT, 0, ACTIVATION_RECORD_SIZE + 2);
    this->symtab.insert_symbol("__TEMP_PRINT__", VARIABLE, INTEGER, false);
    this->generate(PL0_LOD, 0, -1); /* Load first and only argument */
    auto temp_print_address = this->symtab.get_symbol("__TEMP_PRINT__").address;
    this->generate(PL0_STO, 0, temp_print_address); /* Store the argument in the first temporary variable */

    this->symtab.insert_symbol("__TEMP_COUNTER__", VARIABLE, INTEGER, false);
    auto temp_counter_address = this->symtab.get_symbol("__TEMP_COUNTER__").address;
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_STO, 0, temp_counter_address);

    auto jump_to_start = this->get_instruction_counter();
    this->generate(PL0_INT, 0, 1);
    this->generate(PL0_LOD, 0, temp_print_address); /* Load the number */

    this->generate(PL0_LIT, 0, 10);
    this->generate(PL0_OPR, 0, PL0_MOD); /* Divide the number by 10 */

    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_LIT, 0, 5);
    this->generate(PL0_LOD, 0, temp_counter_address);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_PST, 0, 0); /* Store the result back in the temporary variable */

    this->generate(PL0_LOD, 0, temp_print_address); /* Load the number */
    this->generate(PL0_LIT, 0, 10);
    this->generate(PL0_OPR, 0, PL0_DIV); /* Divide the number by 10 */
    this->generate(PL0_STO, 0, temp_print_address); /* Store the result back in the temporary variable */

    this->generate(PL0_LOD, 0, temp_counter_address);
    this->generate(PL0_LIT, 0, 1);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_STO, 0, temp_counter_address); /* Increment the counter */

    this->generate(PL0_LOD, 0, temp_print_address);
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_OPR, 0, PL0_EQ);

    this->generate(PL0_JMC, 0, jump_to_start);

    /* Buffer of digits is backwards now */
    auto jump_to_print_start = this->get_instruction_counter();
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_LIT, 0, 4);
    this->generate(PL0_LOD, 0, temp_counter_address);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_PLD, 0, 0); /* Load the digit */

    this->generate(PL0_LIT, 0, 48);
    this->generate(PL0_OPR, 0, PL0_ADD);

    this->generate(PL0_WRI, 0, 0); /* Write the digit */

    this->generate(PL0_LOD, 0, temp_counter_address);
    this->generate(PL0_LIT, 0, 1);
    this->generate(PL0_OPR, 0, PL0_SUB);
    this->generate(PL0_STO, 0, temp_counter_address); /* Decrement the counter */

    this->generate(PL0_LOD, 0, temp_counter_address);
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_OPR, 0, PL0_EQ);

    this->generate(PL0_JMC, 0, jump_to_print_start);

    this->generate(PL0_RET, 0, 0);

    this->symtab.remove_scope();
}

void InstructionsGenerator::gen_read_num() {
    /* Initialize the scope and the activation record */
    this->symtab.insert_scope(0, ACTIVATION_RECORD_SIZE, true);
    this->generate(PL0_INT, 0, ACTIVATION_RECORD_SIZE + 2);

    /* Two temporary variables are used to read the number and store the result */
    this->symtab.insert_symbol("__TEMP_READ__", VARIABLE, INTEGER, false);
    this->symtab.insert_symbol("__TEMP_RESULT__", VARIABLE, INTEGER, false);

    /* Result is initialized to 0 */
    auto temp_read_address = this->symtab.get_symbol("__TEMP_READ__").address;
    auto temp_result_address = this->symtab.get_symbol("__TEMP_RESULT__").address;
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_STO, 0, temp_result_address);

    /* Read instruction address is the address to jump to (loop) */
    auto rea_instruction_line = this->get_instruction_counter();
    this->generate(PL0_REA, 0, 0);
    this->generate(PL0_STO, 0, temp_read_address);

    /* Check if the read number is equal to 10 (ASCII for newline) */
    this->generate(PL0_LOD, 0, temp_read_address);
    this->generate(PL0_LIT, 0, 10);
    this->generate(PL0_OPR, 0, PL0_NEQ);

    /* If it is not new line continue reading and adding to the result */
    auto jmc_instruction_line = this->get_instruction_counter();
    this->generate(PL0_JMC, 0, 0);

    /* Load last read digit */
    this->generate(PL0_LOD, 0, temp_read_address);
    /* Subtract '0' to get the actual number */
    this->generate(PL0_LIT, 0, 48);
    this->generate(PL0_OPR, 0, PL0_SUB);
    /* Multiply the result by 10 */
    this->generate(PL0_LOD, 0, temp_result_address);
    this->generate(PL0_LIT, 0, 10);
    this->generate(PL0_OPR, 0, PL0_MUL);
    /* Add the last read digit */
    this->generate(PL0_OPR, 0, PL0_ADD);
    /* Store the result */
    this->generate(PL0_STO, 0, temp_result_address);
    this->generate(PL0_JMP, 0, rea_instruction_line);

    /* Jump here if the read number is equal to 10 (ASCII for newline) */
    auto &jmc_instruction = this->get_instruction(jmc_instruction_line);
    jmc_instruction.parameter = this->get_instruction_counter();

    /* Load the result to be at the top of the stack */
    this->generate(PL0_LOD, 0, temp_result_address);

    /* Return the result */
    this->generate(PL0_STO, 0, -1); /* sizeof int is 1 */
    this->generate(PL0_RET, 0, 0);

    /* Cleanup */
    this->symtab.remove_scope();
}

void InstructionsGenerator::init_builtin_functions() {
    this->symtab.init_builtin_functions();

    if (this->used_builtin_functions.empty())
        return;

    if (std::find(this->used_builtin_functions.begin(), this->used_builtin_functions.end(), "print_num") != this->used_builtin_functions.end()) {
        auto print_num_address = this->get_instruction_counter();
        this->gen_print_num();
        auto &print_num_symbol = this->symtab.get_symbol("print_num");
        print_num_symbol.address = print_num_address;
    }

    if (std::find(this->used_builtin_functions.begin(), this->used_builtin_functions.end(), "read_num") != this->used_builtin_functions.end()) {
        auto read_num_address = this->get_instruction_counter();
        this->gen_read_num();
        auto &read_num_symbol = this->symtab.get_symbol("read_num");
        read_num_symbol.address = read_num_address;
    }
}

void InstructionsGenerator::generate() {
    this->symtab.insert_scope(0, ACTIVATION_RECORD_SIZE); /* Offset 3 for activation record */

    /* Jump over builtin functions */
    this->generate(PL0_JMP, 0, 0);
    this->init_builtin_functions();
    this->get_instruction(0).parameter = this->get_instruction_counter();

    auto int_instruction_line = this->get_instruction_counter();
    this->generate(PL0_INT, 0, 0);

    for (auto &statement: this->global_block->statements)
        statement->accept(this);

    for (auto [key, value] : this->goto_labels_line) {
        auto &goto_instruction = this->get_instruction(value);
        goto_instruction.parameter = this->labels_to_line[key];
    }

    auto sizeof_global_variables = symtab.get_sizeof_variables();
    this->get_instruction(int_instruction_line).parameter = sizeof_global_variables + ACTIVATION_RECORD_SIZE;

    auto main_address = symtab.get_symbol("main").address;
    this->generate(PL0_INT, 0, 1);
    this->generate(PL0_CAL, 0, main_address);
    this->generate(PL0_RET, 0, 0);
}

void InstructionsGenerator::visit(ASTNodeBlock *node) {
    auto is_functional_block = this->symtab.get_current_scope().get_is_function_scope();
    auto sizeof_params = 0;
    auto lod_address = -this->sizeof_params_stack.size() - 1;
    std::vector<int> lod_addresses;
    if (is_functional_block) {
        for (auto &sizeof_param: this->sizeof_params_stack) {
            sizeof_params += sizeof_param;
            lod_addresses.push_back(lod_address++);
        }
        this->sizeof_params_stack.clear();
    }
    auto number_of_variables = node->get_number_of_declared_variables();
    auto sizeof_variables = node->get_sizeof_variables();
    auto sum_sizeof = sizeof_params;
    for (auto &sizeof_variable: sizeof_variables)
        sum_sizeof += sizeof_variable;
    this->generate(PL0_INT, 0, sum_sizeof);

    auto sto_param_address = ACTIVATION_RECORD_SIZE;
    for (auto &lod_param_address: lod_addresses) {
        this->generate(PL0_LOD, 0, lod_param_address);
        this->generate(PL0_STO, 0, sto_param_address++);
    }

    auto size_one = INTEGER;
//    auto size_two = ...; /* TODO: once added other types, this should be expanded on */
    for (int i = 0; i < number_of_variables; i++) {
        if (sizeof_variables[i] == 1) /* TODO: don't forget to expand here too :) */
            this->symtab.insert_symbol("__DUMMY_" + std::to_string(i) + "__", VARIABLE, size_one, false);
    }
    this->number_of_declared_variables.push_back(0);

    for (auto &statement: node->statements)
        statement->accept(this);

    this->number_of_declared_variables.pop_back();
    this->generate(PL0_INT, 0, -sum_sizeof);
}

void InstructionsGenerator::visit(ASTNodeDeclVar *node) {
    if (!node->label.empty()) {
        auto jump_instruction_line = this->get_instruction_counter();
        this->labels_to_line[node->label] = jump_instruction_line;
    }

    std::string dummy_name = "__DUMMY_" + std::to_string(this->number_of_declared_variables.back()++) + "__";
    this->symtab.change_symbol_name(dummy_name, node->name);
    auto &symbol = this->symtab.get_symbol(node->name);
    symbol.type = str_to_val_type(node->type);
    symbol.is_const = node->is_const;
    symbol.is_pointer = node->is_pointer;

    if (node->expression) {
        node->expression->accept(this);

        if (dynamic_cast<ASTNodeReference *>(node->expression))
            symbol.is_pointing_to_stack = true;
        else if (dynamic_cast<ASTNodeNew *>(node->expression))
            symbol.is_pointing_to_stack = false;
        else if (auto binary_operator = dynamic_cast<ASTNodeBinaryOperator *>(node->expression)) {
            if (binary_operator->contains_reference())
                symbol.is_pointing_to_stack = true;
        }

        auto address = this->symtab.get_symbol(node->name).address;
        this->generate(PL0_STO, 0, address);
    }
}

void InstructionsGenerator::visit(ASTNodeDeclFunc *node) {
    if (!node->label.empty()) {
        auto jump_instruction_line = this->get_instruction_counter();
        this->labels_to_line[node->label] = jump_instruction_line;
    }

    auto jump_over_func_instr_index = this->get_instruction_counter();
    this->generate(PL0_JMP, 0, 0);
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
        this->generate(PL0_INT, 0, ACTIVATION_RECORD_SIZE);

        auto &decl_func_symbol = this->symtab.get_symbol(node->name);
        for (auto &parameter: node->parameters) {
            this->symtab.insert_symbol(parameter->name, VARIABLE, str_to_val_type(parameter->type), false);
            this->symtab.get_symbol(parameter->name).is_pointer = parameter->is_pointer;
            decl_func_symbol.parameters.push_back(this->symtab.get_symbol(parameter->name));
            this->sizeof_params_stack.push_back(sizeof_val_type(str_to_val_type(parameter->type)));
        }

        this->sizeof_return_type_stack.push_back(sizeof_val_type(str_to_val_type(node->return_type)));
        node->block->accept(this);
        this->sizeof_return_type_stack.pop_back();
    } else {
        this->generate(PL0_JMP, 0, 0);
    }

    this->symtab.remove_scope();

    auto &jump_over_func_instr = this->get_instruction(jump_over_func_instr_index);
    jump_over_func_instr.parameter = this->get_instruction_counter();
}

void InstructionsGenerator::visit(ASTNodeIf *node) {
    if (!node->label.empty()) {
        auto jump_instruction_line = this->get_instruction_counter();
        this->labels_to_line[node->label] = jump_instruction_line;
    }

    auto current_scope = this->symtab.get_current_scope();
    auto new_base = current_scope.get_address_base() + current_scope.get_address_offset();
    this->symtab.insert_scope(new_base, 0, false);
    node->condition->accept(this);

    auto jmc_instruction_line = this->get_instruction_counter();
    this->generate(PL0_JMC, 0, 0);
    node->block->accept(this);
    this->symtab.remove_scope();

    auto jmp_instruction_line = this->get_instruction_counter();
    auto &jmc_instruction = this->get_instruction(jmc_instruction_line);
    jmc_instruction.parameter = jmp_instruction_line + 1;
    this->generate(PL0_JMP, 0, 0);

    if (node->else_block) {
        auto current_scope = this->symtab.get_current_scope();
        auto new_base = current_scope.get_address_base() + current_scope.get_address_offset();
        this->symtab.insert_scope(new_base, 0, false);

        node->else_block->accept(this);
        this->symtab.remove_scope();
    }

    auto post_if_instruction_line = this->get_instruction_counter();
    auto &jmp_instruction = this->get_instruction(jmp_instruction_line);
    jmp_instruction.parameter = post_if_instruction_line;
}

void InstructionsGenerator::visit(ASTNodeWhile *node) {
    if (!node->label.empty()) {
        auto jump_instruction_line = this->get_instruction_counter();
        this->labels_to_line[node->label] = jump_instruction_line;
    }

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

        if (node->is_repeat_until) {
            this->generate(PL0_LIT, 0, 0);
            this->generate(PL0_OPR, 0, PL0_EQ);
        }

        auto jmc_instruction_line = this->get_instruction_counter();
        this->generate(PL0_JMC, 0, 0);
        this->generate(PL0_JMP, 0, block_instruction_line);
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
        this->generate(PL0_JMC, 0, 0);
        node->block->accept(this);

        this->generate(PL0_JMP, 0, condition_instruction_line);
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

    this->symtab.remove_scope();
}

void InstructionsGenerator::visit(ASTNodeFor *node) {
    if (!node->label.empty()) {
        auto jump_instruction_line = this->get_instruction_counter();
        this->labels_to_line[node->label] = jump_instruction_line;
    }

    node->block->count_breaks_and_continues();
    node->break_number = node->block->break_number;
    node->continue_number = node->block->continue_number;

    auto current_scope = this->symtab.get_current_scope();
    auto new_base = current_scope.get_address_base() + current_scope.get_address_offset();
    this->symtab.insert_scope(new_base, 0, false);

    int sizeof_init = 0;
    if (auto decl_var = dynamic_cast<ASTNodeDeclVar *>(node->init)) {
        sizeof_init = sizeof_val_type(str_to_val_type(decl_var->type));
        this->generate(PL0_INT, 0, sizeof_init);
        this->symtab.insert_symbol("__DUMMY_0__", VARIABLE, decl_var->type, false);
        this->number_of_declared_variables.push_back(0);
        node->init->accept(this);
        this->number_of_declared_variables.pop_back();
    }
    else if (dynamic_cast<ASTNodeAssignExpression *>(node->init))
        node->init->accept(this);

    auto condition_instruction_line = this->get_instruction_counter();
    node->condition->accept(this);
    auto jmc_instruction_line = this->get_instruction_counter();
    this->generate(PL0_JMC, 0, 0);
    node->block->accept(this);
    node->increment->accept(this);
    this->generate(PL0_JMP, 0, condition_instruction_line);
    auto &jmc_instruction = this->get_instruction(jmc_instruction_line);
    jmc_instruction.parameter = this->get_instruction_counter();

    if (sizeof_init)
        this->generate(PL0_INT, 0, -sizeof_init);

    this->symtab.remove_scope();
}

void InstructionsGenerator::visit(ASTNodeBreakContinue *node) {
    if (!node->label.empty()) {
        auto jump_instruction_line = this->get_instruction_counter();
        this->labels_to_line[node->label] = jump_instruction_line;
    }

    auto jmp_instruction_line = this->get_instruction_counter();
    this->generate(PL0_JMP, 0, 0);
    if (node->is_break)
        this->break_stack.push_back(jmp_instruction_line);
    else
        this->continue_stack.push_back(jmp_instruction_line);
}

void InstructionsGenerator::visit(ASTNodeReturn *node) {
    if (!node->label.empty()) {
        auto jump_instruction_line = this->get_instruction_counter();
        this->labels_to_line[node->label] = jump_instruction_line;
    }

    if (node->expression)
        node->expression->accept(this);

    auto sizeof_return_type = this->sizeof_return_type_stack.back();
    this->generate(PL0_STO, 0, -sizeof_return_type);

    this->generate(PL0_RET, 0, 0);
}

void InstructionsGenerator::visit(ASTNodeExpressionStatement *node) {
    if (!node->label.empty()) {
        auto jump_instruction_line = this->get_instruction_counter();
        this->labels_to_line[node->label] = jump_instruction_line;
    }

    node->expression->accept(this);
}

void InstructionsGenerator::visit(ASTNodeIdentifier *node) {
    auto address = this->symtab.get_symbol(node->name).address;
    auto level = this->symtab.get_symbol_level(node->name);
    this->generate(PL0_LOD, level, address);
}

void InstructionsGenerator::visit(ASTNodeIntLiteral *node) {
    this->generate(PL0_LIT, 0, node->value);
}

void InstructionsGenerator::visit(ASTNodeBoolLiteral *node) {
    int value = node->value ? 1 : 0;
    this->generate(PL0_LIT, 0, value);
}

void InstructionsGenerator::visit(ASTNodeAssignExpression *node) {
    auto &symbol = this->symtab.get_symbol(node->name);

    node->expression->accept(this);

    if (dynamic_cast<ASTNodeReference *>(node->expression))
        symbol.is_pointing_to_stack = true;
    else if (dynamic_cast<ASTNodeNew *>(node->expression))
        symbol.is_pointing_to_stack = false;
    else if (auto binary_operator = dynamic_cast<ASTNodeBinaryOperator *>(node->expression)) {
        if (binary_operator->contains_reference())
            symbol.is_pointing_to_stack = true;
    }

    auto address = symbol.address;
    auto level = this->symtab.get_symbol_level(node->name);
    this->generate(PL0_STO, level, address);
}

void InstructionsGenerator::visit(ASTNodeBinaryOperator *node) {
    node->left->accept(this);
    node->right->accept(this);

    if (node->op == "&&") { /* AND: 1 * 1 = 1, 1 * 0 = 0, 0 * 1 = 0, 0 * 0 = 0 */
        this->generate(PL0_OPR, 0, PL0_MUL);
        this->generate(PL0_LIT, 0, 0);
        this->generate(PL0_OPR, 0, PL0_NEQ);
    } else if (node->op == "||") { /* OR: 1 + 1 = 2, 1 + 0 = 1, 0 + 1 = 1, 0 + 0 = 0 */
        this->generate(PL0_OPR, 0, PL0_ADD);
        this->generate(PL0_LIT, 0, 0);
        this->generate(PL0_OPR, 0, PL0_NEQ);
    } else {
        this->generate(PL0_OPR, 0, OperatorsTable.find(node->op)->second);
    }
}

void InstructionsGenerator::visit(ASTNodeUnaryOperator *node) {
    node->expression->accept(this);

    if(node->op == "!") { /* NOT: true == 0 => false, false == 0 => true */
        this->generate(PL0_LIT, 0, 0);
        this->generate(PL0_OPR, 0, PL0_EQ);
    } else if (node->op == "-") {
        this->generate(PL0_OPR, 0, PL0_NEG);
    }
}

void InstructionsGenerator::visit(ASTNodeCast *node) { /* TODO: cast is not implemented yet */
    /* Empty */
}

void InstructionsGenerator::visit(ASTNodeCallFunc *node) {
    auto &symbol = this->symtab.get_symbol(node->name);
    auto level = this->symtab.get_symbol_level(node->name);

    for (auto &argument: node->arguments)
        argument->accept(this);

    this->generate(PL0_INT, 0, sizeof_val_type(symbol.type));
    this->generate(PL0_CAL, level, symbol.address);
}

void InstructionsGenerator::visit(ASTNodeNew *node) {
    node->expression->accept(this);
    this->generate(PL0_LIT, 0, sizeof_val_type(str_to_val_type(node->type)));
    this->generate(PL0_OPR, 0, PL0_MUL);
    this->generate(PL0_NEW, 0, 0);
}

void InstructionsGenerator::visit(ASTNodeDelete *node) {
    node->expression->accept(this);
    this->generate(PL0_DEL, 0, 0);
}

void InstructionsGenerator::visit(ASTNodeDereference *node) {
    node->expression->accept(this);
    if (!node->is_lvalue && !node->is_pointing_to_stack)
        this->generate(PL0_LDA, 0, 0);
    else if (!node->is_lvalue && node->is_pointing_to_stack)
        this->generate(PL0_LOD, 0, 0);
}

void InstructionsGenerator::visit(ASTNodeReference *node) {
    auto address = this->symtab.get_symbol(node->identifier).address;
    this->generate(PL0_LIT, 0, address);
}

void InstructionsGenerator::visit(ASTNodeDynamicAssignExpression *node) {
    if (auto dereference = dynamic_cast<ASTNodeDereference *>(node->left)) {
        auto &symbol = this->symtab.get_symbol(dereference->identifier);
        if (symbol.is_pointing_to_stack) {
            node->right->accept(this);
            auto level = this->symtab.get_symbol_level(dereference->identifier);
            this->generate(PL0_LIT, 0, level);
            node->left->accept(this);
            this->generate(PL0_PST, 0, 0);
        } else {
            node->left->accept(this);
            node->right->accept(this);
            this->generate(PL0_STA, 0, 0);
        }
    }
}

void InstructionsGenerator::visit(ASTNodeGoto *node) {
    if (!node->label.empty()) {
        auto jump_instruction_line = this->get_instruction_counter();
        this->labels_to_line[node->label] = jump_instruction_line;
    }

    auto jump_instruction_line = this->get_instruction_counter();
    this->generate(PL0_JMP, 0, 0);
    this->goto_labels_line[node->label_to_go_to] = jump_instruction_line;
}
