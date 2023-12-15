#include "InstructionsGenerator.h"

void InstructionsGenerator::init_builtin_functions() {
    this->symtab.init_builtin_functions();

    if (this->used_builtin_functions.empty())
        return;

    if (std::find(this->used_builtin_functions.begin(), this->used_builtin_functions.end(), "print_int") != this->used_builtin_functions.end()) {
        auto print_int_address = this->get_instruction_counter();
        this->gen_print_int();
        auto &print_int_symbol = this->symtab.get_symbol("print_int");
        print_int_symbol.address = print_int_address;
    }

    if (std::find(this->used_builtin_functions.begin(), this->used_builtin_functions.end(), "read_int") != this->used_builtin_functions.end()) {
        auto read_int_address = this->get_instruction_counter();
        this->gen_read_int();
        auto &read_int_symbol = this->symtab.get_symbol("read_int");
        read_int_symbol.address = read_int_address;
    }

    if (std::find(this->used_builtin_functions.begin(), this->used_builtin_functions.end(), "print_str") != this->used_builtin_functions.end()) {
        auto print_string_address = this->get_instruction_counter();
        this->gen_print_string();
        auto &print_string_symbol = this->symtab.get_symbol("print_str");
        print_string_symbol.address = print_string_address;
    }

    if (std::find(this->used_builtin_functions.begin(), this->used_builtin_functions.end(), "read_str") != this->used_builtin_functions.end()) {
        auto read_string_address = this->get_instruction_counter();
        this->gen_read_string();
        auto &read_string_symbol = this->symtab.get_symbol("read_str");
        read_string_symbol.address = read_string_address;
    }

    if (std::find(this->used_builtin_functions.begin(), this->used_builtin_functions.end(), "strcmp") != this->used_builtin_functions.end()) {
        auto strcmp_address = this->get_instruction_counter();
        this->gen_strcmp();
        auto &strcmp_symbol = this->symtab.get_symbol("strcmp");
        strcmp_symbol.address = strcmp_address;
    }

    if (std::find(this->used_builtin_functions.begin(), this->used_builtin_functions.end(), "strcat") != this->used_builtin_functions.end()) {
        auto strcat_address = this->get_instruction_counter();
        this->gen_strcat();
        auto &strcat_symbol = this->symtab.get_symbol("strcat");
        strcat_symbol.address = strcat_address;
    }

    if (std::find(this->used_builtin_functions.begin(), this->used_builtin_functions.end(), "strlen") != this->used_builtin_functions.end()) {
        auto strlen_address = this->get_instruction_counter();
        this->gen_strlen();
        auto &strlen_symbol = this->symtab.get_symbol("strlen");
        strlen_symbol.address = strlen_address;
    }

    if (std::find(this->used_builtin_functions.begin(), this->used_builtin_functions.end(), "print_float") != this->used_builtin_functions.end()) {
        /* print_float uses print_int */
        if (std::find(this->used_builtin_functions.begin(), this->used_builtin_functions.end(), "print_int") == this->used_builtin_functions.end()) {
            auto print_int_address = this->get_instruction_counter();
            this->gen_print_int();
            auto &print_int_symbol = this->symtab.get_symbol("print_int");
            print_int_symbol.address = print_int_address;
        }

        auto print_float_address = this->get_instruction_counter();
        this->gen_print_float();
        auto &print_float_symbol = this->symtab.get_symbol("print_float");
        print_float_symbol.address = print_float_address;
    }

    if (std::find(this->used_builtin_functions.begin(), this->used_builtin_functions.end(), "read_float") != this->used_builtin_functions.end()) {
        /* read_float uses read_int */
        if (std::find(this->used_builtin_functions.begin(), this->used_builtin_functions.end(), "read_int") == this->used_builtin_functions.end()) {
            auto read_int_address = this->get_instruction_counter();
            this->gen_read_int();
            auto &read_int_symbol = this->symtab.get_symbol("read_int");
            read_int_symbol.address = read_int_address;
        }

        auto read_float_address = this->get_instruction_counter();
        this->gen_read_float();
        auto &read_float_symbol = this->symtab.get_symbol("read_float");
        read_float_symbol.address = read_float_address;
    }
}

void InstructionsGenerator::gen_print_int() {
    this->symtab.insert_scope(0, ACTIVATION_RECORD_SIZE, true);

    this->generate(PL0_INT, 0, ACTIVATION_RECORD_SIZE + 2);
    this->symtab.insert_symbol("__TEMP_PRINT__", VARIABLE, int_t, false);
    this->generate(PL0_LOD, 0, -1); /* Load first and only argument */
    auto temp_print_address = this->symtab.get_symbol("__TEMP_PRINT__").address;
    this->generate(PL0_STO, 0, temp_print_address); /* Store the argument in the first temporary variable */

    this->symtab.insert_symbol("__TEMP_COUNTER__", VARIABLE, int_t, false);
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

void InstructionsGenerator::gen_read_int() {
    /* Initialize the scope and the activation record */
    this->symtab.insert_scope(0, ACTIVATION_RECORD_SIZE, true);
    this->generate(PL0_INT, 0, ACTIVATION_RECORD_SIZE + 2);

    /* Two temporary variables are used to read the number and store the result */
    this->symtab.insert_symbol("__TEMP_READ__", VARIABLE, int_t, false);
    this->symtab.insert_symbol("__TEMP_RESULT__", VARIABLE, int_t, false);

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

void InstructionsGenerator::gen_print_string() {
    this->symtab.insert_scope(0, ACTIVATION_RECORD_SIZE, true);

    this->generate(PL0_INT, 0, ACTIVATION_RECORD_SIZE + 3);
    this->symtab.insert_symbol("__TEMP_PRINT__", VARIABLE, int_t, false);
    this->generate(PL0_LOD, 0, -1); /* Load first and only argument */
    auto temp_print_address = this->symtab.get_symbol("__TEMP_PRINT__").address;
    this->generate(PL0_STO, 0, temp_print_address); /* Store the argument in the first temporary variable */

    this->symtab.insert_symbol("__TEMP_STRING_SIZE__", VARIABLE, int_t, false);
    auto temp_string_size_address = this->symtab.get_symbol("__TEMP_STRING_SIZE__").address;
    this->generate(PL0_LIT, 0, -1);
    this->generate(PL0_LOD, 0, temp_print_address);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_LDA, 0, 0);
    this->generate(PL0_STO, 0, temp_string_size_address);

    this->symtab.insert_symbol("__TEMP_COUNTER__", VARIABLE, int_t, false);
    auto temp_counter_address = this->symtab.get_symbol("__TEMP_COUNTER__").address;
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_STO, 0, temp_counter_address);

    auto jump_to_start = this->get_instruction_counter();
    this->generate(PL0_LOD, 0, temp_print_address); /* Load the string */
    this->generate(PL0_LOD, 0, temp_counter_address); /* Load the counter */
    this->generate(PL0_OPR, 0, PL0_ADD); /* Add the counter to the string address */
    this->generate(PL0_LDA, 0, 0); /* Load the character */
    this->generate(PL0_WRI, 0, 0); /* Write the character */
    this->generate(PL0_LOD, 0, temp_counter_address); /* Load the counter */
    this->generate(PL0_LIT, 0, 1);
    this->generate(PL0_OPR, 0, PL0_ADD); /* Increment the counter */
    this->generate(PL0_STO, 0, temp_counter_address); /* Store the counter */
    this->generate(PL0_LOD, 0, temp_counter_address); /* Load the counter */
    this->generate(PL0_LOD, 0, temp_string_size_address); /* Load the string size */
    this->generate(PL0_OPR, 0, PL0_EQ); /* Check if the counter is equal to the string size */
    this->generate(PL0_JMC, 0, jump_to_start); /* If not equal, jump to the start */

    this->generate(PL0_RET, 0, 0);

    this->symtab.remove_scope();
}

void InstructionsGenerator::gen_read_string() {
    this->symtab.insert_scope(0, ACTIVATION_RECORD_SIZE, true);
    this->generate(PL0_INT, 0, ACTIVATION_RECORD_SIZE + 2);

    this->symtab.insert_symbol("__TEMP_STRING_SIZE__", VARIABLE, int_t, false);
    auto temp_string_size_address = this->symtab.get_symbol("__TEMP_STRING_SIZE__").address;
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_STO, 0, temp_string_size_address);
    this->symtab.insert_symbol("__TEMP_STRING_COUNTER__", VARIABLE, int_t, false);
    auto temp_string_counter_address = this->symtab.get_symbol("__TEMP_STRING_COUNTER__").address;

    auto rea_instruction_line = this->get_instruction_counter();
    this->generate(PL0_INT, 0, 1);
    this->generate(PL0_REA, 0, 0);
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_LOD, 0, temp_string_size_address);
    this->generate(PL0_LIT, 0, 5);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_PST, 0, 0);

    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_LOD, 0, temp_string_size_address);
    this->generate(PL0_LIT, 0, 5);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_PLD, 0, 0);
    this->generate(PL0_LIT, 0, 10);
    this->generate(PL0_OPR, 0, PL0_NEQ);
    auto jmc_instruction_line = this->get_instruction_counter();
    this->generate(PL0_JMC, 0, 0);

    this->generate(PL0_LOD, 0, temp_string_size_address);
    this->generate(PL0_LIT, 0, 1);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_STO, 0, temp_string_size_address);

    this->generate(PL0_JMP, 0, rea_instruction_line);
    auto &jmc_instruction = this->get_instruction(jmc_instruction_line);
    jmc_instruction.parameter = this->get_instruction_counter();

    this->generate(PL0_INT, 0, 1);
    this->generate(PL0_LOD, 0, temp_string_size_address);
    this->generate(PL0_NEW, 0, 0);
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_LOD, 0, temp_string_size_address);
    this->generate(PL0_LIT, 0, 6);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_PST, 0, 0);

    /* copy the string to the heap */
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_LOD, 0, temp_string_size_address);
    this->generate(PL0_LIT, 0, 6);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_PLD, 0, 0);
    this->generate(PL0_LIT, 0, -1);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_LOD, 0, temp_string_size_address);
    this->generate(PL0_STA, 0, 0);

    this->generate(PL0_LOD, 0, temp_string_size_address);
    this->generate(PL0_STO, 0, temp_string_counter_address);

    /* copy untill the the temp string size address is 0 */
    auto jump_to_start = this->get_instruction_counter();
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_LOD, 0, temp_string_size_address);
    this->generate(PL0_LIT, 0, 6);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_PLD, 0, 0);
    this->generate(PL0_LOD, 0, temp_string_counter_address);
    this->generate(PL0_LIT, 0, -1);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_LOD, 0, temp_string_counter_address);
    this->generate(PL0_LIT, 0, 4);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_PLD, 0, 0);
    this->generate(PL0_STA, 0, 0);

    this->generate(PL0_LOD, 0, temp_string_counter_address);
    this->generate(PL0_LIT, 0, -1);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_STO, 0, temp_string_counter_address);

    this->generate(PL0_LOD, 0, temp_string_counter_address);
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_OPR, 0, PL0_EQ);
    this->generate(PL0_JMC, 0, jump_to_start);

    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_LOD, 0, temp_string_size_address);
    this->generate(PL0_LIT, 0, 6);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_PLD, 0, 0);
    this->generate(PL0_STO, 0, -1);
    this->generate(PL0_RET, 0, 0);

    this->symtab.remove_scope();
}

void InstructionsGenerator::gen_strcmp() {
    this->symtab.insert_scope(0, ACTIVATION_RECORD_SIZE, true);
    this->generate(PL0_INT, 0, ACTIVATION_RECORD_SIZE + 5);

    this->symtab.insert_symbol("__TEMP_STR_1__", VARIABLE, int_t, false);
    this->symtab.insert_symbol("__TEMP_STR_2__", VARIABLE, int_t, false);
    this->symtab.insert_symbol("__TEMP_STR_SIZE__", VARIABLE, int_t, false);
    this->symtab.insert_symbol("__TEMP_COUNTER__", VARIABLE, int_t, false);
    this->symtab.insert_symbol("__TEMP_RESULT__", VARIABLE, int_t, false);

    auto temp_str_1_address = this->symtab.get_symbol("__TEMP_STR_1__").address;
    auto temp_str_2_address = this->symtab.get_symbol("__TEMP_STR_2__").address;
    auto temp_str_size_address = this->symtab.get_symbol("__TEMP_STR_SIZE__").address;
    auto temp_counter_address = this->symtab.get_symbol("__TEMP_COUNTER__").address;
    auto temp_result_address = this->symtab.get_symbol("__TEMP_RESULT__").address;

    this->generate(PL0_LOD, 0, -2);
    this->generate(PL0_STO, 0, temp_str_1_address);
    this->generate(PL0_LOD, 0, -1);
    this->generate(PL0_STO, 0, temp_str_2_address);
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_STO, 0, temp_counter_address);

    /* look to heap for the -1 index and compare sizes */
    this->generate(PL0_LOD, 0, temp_str_1_address);
    this->generate(PL0_LIT, 0, -1);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_LDA, 0, 0);
    this->generate(PL0_LOD, 0, temp_str_2_address);
    this->generate(PL0_LIT, 0, -1);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_LDA, 0, 0);
    this->generate(PL0_STO, 0, temp_str_size_address);
    this->generate(PL0_LOD, 0, temp_str_size_address);
    this->generate(PL0_OPR, 0, PL0_EQ);
    this->generate(PL0_STO, 0, temp_result_address);
    this->generate(PL0_LOD, 0, temp_result_address);
    auto jmc_instruction_line = this->get_instruction_counter();
    this->generate(PL0_JMC, 0, 0);

    /* compare the strings byte by byte */
    auto byte_by_byte_start = this->get_instruction_counter();
    this->generate(PL0_LOD, 0, temp_str_size_address);
    this->generate(PL0_LOD, 0, temp_counter_address);
    this->generate(PL0_OPR, 0, PL0_NEQ);
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_OPR, 0, PL0_EQ);
    this->generate(PL0_STO, 0, temp_result_address);
    this->generate(PL0_LOD, 0, temp_result_address);
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_OPR, 0, PL0_EQ);
    auto jmc_equal_instruction_line = this->get_instruction_counter();
    this->generate(PL0_JMC, 0, 0);
    this->generate(PL0_LOD, 0, temp_str_1_address);
    this->generate(PL0_LOD, 0, temp_counter_address);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_LDA, 0, 0);
    this->generate(PL0_LOD, 0, temp_str_2_address);
    this->generate(PL0_LOD, 0, temp_counter_address);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_LDA, 0, 0);
    this->generate(PL0_OPR, 0, PL0_NEQ);
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_OPR, 0, PL0_EQ);
    this->generate(PL0_STO, 0, temp_result_address);
    this->generate(PL0_LOD, 0, temp_counter_address);
    this->generate(PL0_LIT, 0, 1);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_STO, 0, temp_counter_address);
    this->generate(PL0_LOD, 0, temp_result_address);
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_OPR, 0, PL0_EQ);
    this->generate(PL0_JMC, 0, byte_by_byte_start);

    auto &jmc_instruction = this->get_instruction(jmc_instruction_line);
    jmc_instruction.parameter = this->get_instruction_counter();
    auto &jmc_equal_instruction = this->get_instruction(jmc_equal_instruction_line);
    jmc_equal_instruction.parameter = this->get_instruction_counter();
    this->generate(PL0_LOD, 0, temp_result_address);
    this->generate(PL0_STO, 0, -3);
    this->generate(PL0_RET, 0, 0);

    this->symtab.remove_scope();
}

void InstructionsGenerator::gen_strcat() {
    this->symtab.insert_scope(0, ACTIVATION_RECORD_SIZE, true);
    this->generate(PL0_INT, 0, ACTIVATION_RECORD_SIZE + 7);

    this->symtab.insert_symbol("__TEMP_STR_1__", VARIABLE, int_t, false);
    this->symtab.insert_symbol("__TEMP_STR_2__", VARIABLE, int_t, false);
    this->symtab.insert_symbol("__TEMP_STR_1_SIZE__", VARIABLE, int_t, false);
    this->symtab.insert_symbol("__TEMP_STR_2_SIZE__", VARIABLE, int_t, false);
    this->symtab.insert_symbol("__TEMP_RESULT__", VARIABLE, int_t, false);
    this->symtab.insert_symbol("__TEMP_RESULT_SIZE_", VARIABLE, int_t, false);
    this->symtab.insert_symbol("__TEMP_COUNTER__", VARIABLE, int_t, false);

    auto temp_str_1_address = this->symtab.get_symbol("__TEMP_STR_1__").address;
    auto temp_str_2_address = this->symtab.get_symbol("__TEMP_STR_2__").address;
    auto temp_str_1_size_address = this->symtab.get_symbol("__TEMP_STR_1_SIZE__").address;
    auto temp_str_2_size_address = this->symtab.get_symbol("__TEMP_STR_2_SIZE__").address;
    auto temp_result_address = this->symtab.get_symbol("__TEMP_RESULT__").address;
    auto temp_result_size_address = this->symtab.get_symbol("__TEMP_RESULT_SIZE_").address;
    auto temp_counter_address = this->symtab.get_symbol("__TEMP_COUNTER__").address;

    this->generate(PL0_LOD, 0, -2);
    this->generate(PL0_STO, 0, temp_str_1_address);
    this->generate(PL0_LOD, 0, -1);
    this->generate(PL0_STO, 0, temp_str_2_address);

    /* look to heap for the -1 index and store sizes */
    this->generate(PL0_LOD, 0, temp_str_1_address);
    this->generate(PL0_LIT, 0, -1);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_LDA, 0, 0);
    this->generate(PL0_STO, 0, temp_str_1_size_address);
    this->generate(PL0_LOD, 0, temp_str_2_address);
    this->generate(PL0_LIT, 0, -1);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_LDA, 0, 0);
    this->generate(PL0_STO, 0, temp_str_2_size_address);

    /* calculate the size of the result string */
    this->generate(PL0_LOD, 0, temp_str_1_size_address);
    this->generate(PL0_LOD, 0, temp_str_2_size_address);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_STO, 0, temp_result_size_address);

    /* allocate the result string */
    this->generate(PL0_LOD, 0, temp_result_size_address);
    this->generate(PL0_NEW, 0, 0);
    this->generate(PL0_STO, 0, temp_result_address);
    this->generate(PL0_LOD, 0, temp_result_address);
    this->generate(PL0_LIT, 0, -1);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_LOD, 0, temp_result_size_address);
    this->generate(PL0_STA, 0, 0);

    /* copy the first string to the result string */
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_STO, 0, temp_counter_address);

    auto jump_to_start_str1 = this->get_instruction_counter();
    this->generate(PL0_LOD, 0, temp_result_address);
    this->generate(PL0_LOD, 0, temp_counter_address);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_LOD, 0, temp_str_1_address);
    this->generate(PL0_LOD, 0, temp_counter_address);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_LDA, 0, 0);
    this->generate(PL0_STA, 0, 0);
    this->generate(PL0_LOD, 0, temp_counter_address);
    this->generate(PL0_LIT, 0, 1);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_STO, 0, temp_counter_address);
    this->generate(PL0_LOD, 0, temp_counter_address);
    this->generate(PL0_LOD, 0, temp_str_1_size_address);
    this->generate(PL0_OPR, 0, PL0_EQ);
    this->generate(PL0_JMC, 0, jump_to_start_str1);

    /* copy the second string to the result string */
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_STO, 0, temp_counter_address);

    auto jump_to_start_str2 = this->get_instruction_counter();
    this->generate(PL0_LOD, 0, temp_result_address);
    this->generate(PL0_LOD, 0, temp_counter_address);
    this->generate(PL0_LOD, 0, temp_str_1_size_address);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_LOD, 0, temp_str_2_address);
    this->generate(PL0_LOD, 0, temp_counter_address);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_LDA, 0, 0);
    this->generate(PL0_STA, 0, 0);
    this->generate(PL0_LOD, 0, temp_counter_address);
    this->generate(PL0_LIT, 0, 1);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_STO, 0, temp_counter_address);
    this->generate(PL0_LOD, 0, temp_counter_address);
    this->generate(PL0_LOD, 0, temp_str_2_size_address);
    this->generate(PL0_OPR, 0, PL0_EQ);
    this->generate(PL0_JMC, 0, jump_to_start_str2);

    this->generate(PL0_LOD, 0, temp_result_address);
    this->generate(PL0_STO, 0, -3);
    this->generate(PL0_RET, 0, 0);

    this->symtab.remove_scope();
}

void InstructionsGenerator::gen_strlen() {
    this->symtab.insert_scope(0, ACTIVATION_RECORD_SIZE, true);

    this->generate(PL0_INT, 0, ACTIVATION_RECORD_SIZE);
    this->generate(PL0_LOD, 0, -1);
    this->generate(PL0_LIT, 0, -1);
    this->generate(PL0_OPR, 0, PL0_ADD);
    this->generate(PL0_LDA, 0, 0);
    this->generate(PL0_STO, 0, -2);
    this->generate(PL0_RET, 0, 0);

    this->symtab.remove_scope();
}

void InstructionsGenerator::gen_print_float() {
    this->symtab.insert_scope(0, ACTIVATION_RECORD_SIZE, true);

    this->generate(PL0_INT, 0, ACTIVATION_RECORD_SIZE + 2);
    this->symtab.insert_symbol("__TEMP_FLOAT__", VARIABLE, float_t, false);
    auto temp_float_address = this->symtab.get_symbol("__TEMP_FLOAT__").address;
    this->generate(PL0_LOD, 0, -1);
    this->generate(PL0_STO, 0, temp_float_address + 1);
    this->generate(PL0_LOD, 0, -2);
    this->generate(PL0_STO, 0, temp_float_address);
    this->generate(PL0_LOD, 0, temp_float_address);
    this->generate(PL0_LOD, 0, temp_float_address + 1);

    auto print_int_address = this->symtab.get_symbol("print_int").address;
    this->generate(PL0_RTI, 0, 1);
    this->generate(PL0_CAL, 0, print_int_address);

    this->generate(PL0_LIT, 0, 46);
    this->generate(PL0_WRI, 0, 0);

    this->generate(PL0_LOD, 0, temp_float_address);
    this->generate(PL0_LOD, 0, temp_float_address + 1);
    this->generate(PL0_RTI, 0, 0);
    this->generate(PL0_CAL, 0, print_int_address);

    this->generate(PL0_RET, 0, 0);

    this->symtab.remove_scope();
}

void InstructionsGenerator::gen_read_float() {
    this->symtab.insert_scope(0, ACTIVATION_RECORD_SIZE, true);

    this->generate(PL0_INT, 0, ACTIVATION_RECORD_SIZE + 2);

    /* Two temporary variables are used to read the number and store the result */
    this->symtab.insert_symbol("__TEMP_READ__", VARIABLE, int_t, false);
    this->symtab.insert_symbol("__TEMP_RESULT__", VARIABLE, int_t, false);

    /* Result is initialized to 0 */
    auto temp_read_address = this->symtab.get_symbol("__TEMP_READ__").address;
    auto temp_result_address = this->symtab.get_symbol("__TEMP_RESULT__").address;
    this->generate(PL0_LIT, 0, 0);
    this->generate(PL0_STO, 0, temp_result_address);

    /* Read instruction address is the address to jump to (loop) */
    auto rea_instruction_line = this->get_instruction_counter();
    this->generate(PL0_REA, 0, 0);
    this->generate(PL0_STO, 0, temp_read_address);

    /* Check if the read number is equal to 46 (ASCII for '.') */
    this->generate(PL0_LOD, 0, temp_read_address);
    this->generate(PL0_LIT, 0, 46);
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
    this->generate(PL0_INT, 0, 1);

    auto read_int_address = this->symtab.get_symbol("read_int").address;
    this->generate(PL0_CAL, 0, read_int_address);

    this->generate(PL0_ITR, 0, 0);

    this->generate(PL0_STO, 0, -1);
    this->generate(PL0_STO, 0, -2);

    this->generate(PL0_RET, 0, 0);

    this->symtab.remove_scope();
}
