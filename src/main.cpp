#include <iostream>
#include "Parser.h"
#include "SymbolTable.h"
#include "InstructionsGenerator.h"

extern FILE *yyin;
extern FILE *yyout;

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input file>" << std::endl;
        return EXIT_FAILURE;
    }

    global_symbol_table = SymbolTable();
    global_symbol_table.insert_scope(0, 3); /* Offset 3 for activation record */

    global_instructions_generator = InstructionsGenerator();
    global_instructions_generator.generate("INT", 0, 0);

    yyin = fopen(argv[1], "r");
    yyparse();
    fclose(yyin);

    auto num_global_variables = global_symbol_table.get_number_of_variables();
    global_instructions_generator.get_instruction(0).parameter = num_global_variables + 3;

    auto main_address = global_symbol_table.get_symbol("main").address;
    global_instructions_generator.generate("CAL", 0, main_address);
    global_instructions_generator.generate("RET", 0, 0);

    std::cout << global_symbol_table << std::endl;

    for (auto &instruction : global_instructions_generator.get_instructions())
        std::cout << instruction.line << " " << instruction.instruction << " " << instruction.level << " " << instruction.parameter << std::endl;

    return EXIT_SUCCESS;
}
