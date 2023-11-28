#include "analysis/SyntaxAnalyzer.h"
#include "synthesis/SemanticAnalyzer.h"
#include "synthesis/InstructionsGenerator.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input file>" << std::endl;
        return EXIT_FAILURE;
    }

    auto syntax_analyzer = SyntaxAnalyzer(argv[1]);
    auto program_global_block = syntax_analyzer.analyze();

//    program_global_block->debug_print();

    auto semantic_analyzer = SemanticAnalyzer(program_global_block);
    semantic_analyzer.analyze();

    auto instructions_generator = InstructionsGenerator(program_global_block);
    instructions_generator.generate();

    for (auto &instruction: instructions_generator.get_instructions())
        std::cout << instruction.line << " " << instruction.instruction << " " << instruction.level << " "
                  << instruction.parameter << std::endl;

    return EXIT_SUCCESS;
}
