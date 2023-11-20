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
//    auto main_address = global_instructions_generator.get_instruction_counter();
    global_instructions_generator.generate("JMP", 0, 1); /* TODO: jump to main */
    global_instructions_generator.generate("INT", 0, 5); /* TODO: offset 3 + 4 for activation record and 4 variables */

    yyin = fopen(argv[1], "r");
    yyparse();
    fclose(yyin);

    std::cout << global_symbol_table << std::endl;

    for (auto &instruction : global_instructions_generator.get_instructions())
        std::cout << instruction.line << " " << instruction.instruction << " " << instruction.level << " " << instruction.parameter << std::endl;

    return EXIT_SUCCESS;
}
