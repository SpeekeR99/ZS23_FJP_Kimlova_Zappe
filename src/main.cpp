#include <iostream>
#include "Parser.h"
#include "AbstractSyntaxTree.h"
#include "synthesis/InstructionsGenerator.h"

extern ASTNodeBlock *global_block;
extern FILE *yyin;

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input file>" << std::endl;
        return EXIT_FAILURE;
    }

    yyin = fopen(argv[1], "r");
    yyparse();
    fclose(yyin);

    auto instructions_generator = InstructionsGenerator(global_block);
    instructions_generator.generate();

    for (auto &instruction: instructions_generator.get_instructions())
        std::cout << instruction.line << " " << instruction.instruction << " " << instruction.level << " "
                  << instruction.parameter << std::endl;

    return EXIT_SUCCESS;
}
