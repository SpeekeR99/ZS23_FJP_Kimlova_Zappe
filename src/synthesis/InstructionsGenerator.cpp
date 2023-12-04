#include "InstructionsGenerator.h"

InstructionsGenerator::InstructionsGenerator(ASTNodeBlock *global_block, std::vector<std::string> &used_builtin_functions) : global_block(global_block), used_builtin_functions(used_builtin_functions), instructions(),
                                                                           instruction_counter(0), symtab(),
                                                                           declared_functions(), break_stack(),
                                                                           continue_stack(), sizeof_params_stack(), sizeof_return_type_stack(), sizeof_arguments_stack(),
                                                                           labels_to_line(), goto_labels_line() {
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

void InstructionsGenerator::register_label(ASTNode *node) {
    if (!node->label.empty()) {
        auto jump_instruction_line = this->get_instruction_counter();
        this->labels_to_line[node->label] = jump_instruction_line;
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
    auto lod_address = -this->sizeof_params_stack.size();
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

    this->symtab.allocate_symbols(number_of_variables, sizeof_variables);

    for (auto &statement: node->statements)
        statement->accept(this);

    this->generate(PL0_INT, 0, -sum_sizeof);
}

void InstructionsGenerator::visit(ASTNodeDeclVar *node) {
    this->register_label(node);

    auto &symbol = this->symtab.get_first_empty_symbol(sizeof_val_type(str_to_val_type(node->type)));
    this->symtab.change_symbol_name(symbol.name, node->name);
    symbol.type = {str_to_val_type(node->type), node->is_pointer, true};
    symbol.is_const = node->is_const;

    if (node->expression) {
        if (dynamic_cast<ASTNodeNew *>(node->expression))
            symbol.type.is_pointing_to_stack = false;
        else
            symbol.type.is_pointing_to_stack = true;

        node->expression->accept(this);

        this->generate(PL0_STO, 0, symbol.address);
    }
}

void InstructionsGenerator::visit(ASTNodeDeclFunc *node) {
    this->register_label(node);

    auto jump_over_func_instr_index = this->get_instruction_counter();
    this->generate(PL0_JMP, 0, 0);
    auto func_address = this->get_instruction_counter();

    auto &symbol = this->symtab.get_symbol(node->name);
    if (symbol.name.empty()) { /* Function was not yet declared */
        Type type{str_to_val_type(node->return_type), false, false};
        this->symtab.insert_symbol(node->name, FUNCTION, type, false, func_address);
        this->declared_functions[node->name] = func_address;
    } else { /* Function was earlier declared as a header only */
        symbol.address = func_address;
        auto jump_instr_index = this->declared_functions[node->name];
        auto &jump_instr = this->get_instruction(jump_instr_index);
        jump_instr.parameter = func_address;
    }

    if (node->block) {
        this->symtab.insert_scope(0, ACTIVATION_RECORD_SIZE, true); /* Offset 3 for activation record */
        auto &func_symbol = this->symtab.get_symbol(node->name);
        this->generate(PL0_INT, 0, ACTIVATION_RECORD_SIZE);

        auto sizeof_params_sum = 0;
        for (auto &parameter: node->parameters) {
            Type type{str_to_val_type(parameter->type), parameter->is_pointer, false};
            this->symtab.insert_symbol(parameter->name, VARIABLE, type, false);
            auto &param_symbol = this->symtab.get_symbol(parameter->name);
            func_symbol.parameters.push_back(param_symbol);
            this->sizeof_params_stack.push_back(sizeof_val_type(str_to_val_type(parameter->type)));
            sizeof_params_sum += sizeof_val_type(str_to_val_type(parameter->type));
        }

        this->sizeof_arguments_stack.push_back(sizeof_params_sum);
        this->sizeof_return_type_stack.push_back(sizeof_val_type(str_to_val_type(node->return_type)));
        node->block->accept(this);
        this->sizeof_return_type_stack.pop_back();
        this->sizeof_arguments_stack.pop_back();

        this->symtab.remove_scope();
    } else {
        this->generate(PL0_JMP, 0, 0);
    }

    auto &jump_over_func_instr = this->get_instruction(jump_over_func_instr_index);
    jump_over_func_instr.parameter = this->get_instruction_counter();
}

void InstructionsGenerator::visit(ASTNodeIf *node) {
    this->register_label(node);

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
    this->register_label(node);

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

        if (node->is_repeat_until) {
            this->generate(PL0_LIT, 0, 0);
            this->generate(PL0_OPR, 0, PL0_EQ);
        }

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
    this->register_label(node);

    node->block->count_breaks_and_continues();
    node->break_number = node->block->break_number;
    node->continue_number = node->block->continue_number;

    auto current_scope = this->symtab.get_current_scope();
    auto new_base = current_scope.get_address_base() + current_scope.get_address_offset();
    this->symtab.insert_scope(new_base, 0, false);

    uint32_t sizeof_init = 0;
    if (auto decl_var = dynamic_cast<ASTNodeDeclVar *>(node->init)) {
        Type type{str_to_val_type(decl_var->type), decl_var->is_pointer, true};
        sizeof_init = type.size;
        this->generate(PL0_INT, 0, sizeof_init);
        this->symtab.allocate_symbols(1, {sizeof_init});
        node->init->accept(this);
    }
    else
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
    this->register_label(node);

    auto jmp_instruction_line = this->get_instruction_counter();
    this->generate(PL0_JMP, 0, 0);
    if (node->is_break)
        this->break_stack.push_back(jmp_instruction_line);
    else
        this->continue_stack.push_back(jmp_instruction_line);
}

void InstructionsGenerator::visit(ASTNodeReturn *node) {
    this->register_label(node);

    if (node->expression)
        node->expression->accept(this);

    auto sizeof_return_type = this->sizeof_return_type_stack.back();
    auto sizeof_arguments = this->sizeof_arguments_stack.back();
    this->generate(PL0_STO, 0, -sizeof_return_type - sizeof_arguments);

    this->generate(PL0_RET, 0, 0);
}

void InstructionsGenerator::visit(ASTNodeGoto *node) {
    this->register_label(node);

    auto jump_instruction_line = this->get_instruction_counter();
    this->generate(PL0_JMP, 0, 0);
    this->goto_labels_line[node->label_to_go_to] = jump_instruction_line;
}

void InstructionsGenerator::visit(ASTNodeExpressionStatement *node) {
    this->register_label(node);
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

void InstructionsGenerator::visit(ASTNodeStringLiteral *node) {
    this->symtab.insert_symbol("__TEMP_STRING__", VARIABLE, string_t, false);
    auto string_literal_address = this->symtab.get_symbol("__TEMP_STRING__").address + is_array_of_strings;
    /* String literals are fun to play with */
    /* For real tho, +1 because if array of strings is used, this basically writes over our computed address, since
     * the adress will only be LIT as literal, this variable will overwrite it as it is the next "free" address */
    this->generate(PL0_INT, 0, 1);
    this->generate(PL0_LIT, 0, node->value.length());
    this->generate(PL0_NEW, 0, 0);
    this->generate(PL0_STO, 0, string_literal_address);

    this->generate(PL0_LOD, 0, string_literal_address);
    this->generate(PL0_LIT, 0, -1);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_LIT, 0, node->value.length());
    this->generate(PL0_STA, 0, 0);

    for (int i = 0; i < node->value.length(); i++) {
        this->generate(PL0_LOD, 0, string_literal_address);
        this->generate(PL0_LIT, 0, i);
        this->generate(PL0_OPR, 0, PL0_ADD);
        this->generate(PL0_LIT, 0, node->value[i]);
        this->generate(PL0_STA, 0, 0);
    }

    this->generate(PL0_INT, 0, -1);
    this->generate(PL0_LOD, 0, string_literal_address);
    this->symtab.remove_symbol("__TEMP_STRING__");
}

void InstructionsGenerator::visit(ASTNodeAssignExpression *node) {
    if (node->lvalue) {
        if (auto dereference = dynamic_cast<ASTNodeDereference *>(node->lvalue)) {
            auto &symbol = this->symtab.get_symbol(dereference->identifier);
            if (symbol.type.is_pointing_to_stack) {
                node->expression->accept(this);
                auto level = this->symtab.get_symbol_level(dereference->identifier);
                this->generate(PL0_LIT, 0, level);
                node->lvalue->accept(this);
                this->generate(PL0_PST, 0, 0);
            } else {
                if (symbol.type.type == string_t.type)
                    is_array_of_strings = true;
                node->lvalue->accept(this);
                node->expression->accept(this);
                this->generate(PL0_STA, 0, 0);
                is_array_of_strings = false;
            }
        }
    } else {
        auto &symbol = this->symtab.get_symbol(node->name);

        if (dynamic_cast<ASTNodeNew *>(node->expression))
            symbol.type.is_pointing_to_stack = false;
        else
            symbol.type.is_pointing_to_stack = true;

        node->expression->accept(this);

        auto level = this->symtab.get_symbol_level(node->name);
        this->generate(PL0_STO, level, symbol.address);
    }
}

void InstructionsGenerator::visit(ASTNodeTernaryOperator *node) {
    node->condition->accept(this);

    auto jmc_instruction_line = this->get_instruction_counter();
    this->generate(PL0_JMC, 0, 0);

    node->true_expression->accept(this);

    auto jmp_instruction_line = this->get_instruction_counter();
    this->generate(PL0_JMP, 0, 0);

    auto &jmc_instruction = this->get_instruction(jmc_instruction_line);
    jmc_instruction.parameter = this->get_instruction_counter();

    node->false_expression->accept(this);

    auto &jmp_instruction = this->get_instruction(jmp_instruction_line);
    jmp_instruction.parameter = this->get_instruction_counter();
}

void InstructionsGenerator::visit(ASTNodeBinaryOperator *node) {
    if (node->is_pointer_arithmetic) {
        if (auto left_id = dynamic_cast<ASTNodeIdentifier *>(node->left)) {
            auto &symbol = this->symtab.get_symbol(left_id->name);
            node->left->accept(this);
            node->right->accept(this);
            this->generate(PL0_LIT, 0, symbol.type.size);
            this->generate(PL0_OPR, 0, PL0_MUL);
            this->generate(PL0_OPR, 0, OperatorsTable.find(node->op)->second);
        }
        else if (auto right_id = dynamic_cast<ASTNodeIdentifier *>(node->right)) {
            auto &symbol = this->symtab.get_symbol(right_id->name);
            node->right->accept(this);
            node->left->accept(this);
            this->generate(PL0_LIT, 0, symbol.type.size);
            this->generate(PL0_OPR, 0, PL0_MUL);
            this->generate(PL0_OPR, 0, OperatorsTable.find(node->op)->second);
        }
        else {
            node->left->accept(this);
            node->right->accept(this);
            this->generate(PL0_OPR, 0, OperatorsTable.find(node->op)->second);
        }
    }
    else {
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

    this->generate(PL0_INT, 0, symbol.type.size);

    for (auto &argument: node->arguments)
        argument->accept(this);

    this->generate(PL0_CAL, level, symbol.address);

    auto sizeof_params = 0;
    for (auto &parameter: symbol.parameters)
        sizeof_params += parameter.type.size;

    this->generate(PL0_INT, 0, -sizeof_params);
}

void InstructionsGenerator::visit(ASTNodeNew *node) {
    Type type{str_to_val_type(node->type), false, false};
    this->symtab.insert_symbol("__TEMP_NEW_SIZE__", VARIABLE, type, false);
    auto temp_new_size_address = this->symtab.get_symbol("__TEMP_NEW_SIZE__").address;

    this->generate(PL0_INT, 0, 2);
    node->expression->accept(this);
    this->generate(PL0_LIT, 0, type.size);
    this->generate(PL0_OPR, 0, PL0_MUL);
    this->generate(PL0_STO, 0, temp_new_size_address);
    this->generate(PL0_LOD, 0, temp_new_size_address);
    this->generate(PL0_NEW, 0, 0);

    this->symtab.insert_symbol("__TEMP_NEW__", VARIABLE, type, false);
    auto temp_new_address = this->symtab.get_symbol("__TEMP_NEW__").address;

    this->generate(PL0_STO, 0, temp_new_address);
    this->generate(PL0_LOD, 0, temp_new_address);
    this->generate(PL0_LIT, 0, -1);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_LOD, 0, temp_new_size_address);
    this->generate(PL0_STA, 0, 0);

    this->generate(PL0_INT, 0, -2);

    this->generate(PL0_LOD, 0, temp_new_address);

    this->symtab.remove_symbol("__TEMP_NEW_SIZE__");
    this->symtab.remove_symbol("__TEMP_NEW__");
}

void InstructionsGenerator::visit(ASTNodeDelete *node) {
    node->expression->accept(this);
    this->generate(PL0_DEL, 0, 0);
}

void InstructionsGenerator::visit(ASTNodeDereference *node) {
    if (auto binary_op = dynamic_cast<ASTNodeBinaryOperator *>(node->expression))
        binary_op->is_pointer_arithmetic = true;
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

void InstructionsGenerator::visit(ASTNodeSizeof *node) {
    this->generate(PL0_LIT, 0, sizeof_val_type(str_to_val_type(node->type)));
}
