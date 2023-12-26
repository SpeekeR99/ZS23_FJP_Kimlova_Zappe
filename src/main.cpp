#include "analysis/SyntaxAnalyzer.h"
#include "synthesis/SemanticAnalyzer.h"
#include "synthesis/InstructionsGenerator.h"
#include "synthesis/Optimizer.h"

void print_usage(const char *program_name) {
    std::cerr << "Usage: " << program_name << " <input file>" << std::endl;
    std::cerr << "Usage: " << program_name << " <input file> -o=<optimizations flag>" << std::endl;
    std::cerr << "Optimizations flags:" << std::endl;
    std::cerr << "    0 - no optimizations" << std::endl;
    std::cerr << "    1 - optimizations" << std::endl;
    std::cerr << "Default optimizations flag is 1" << std::endl;
}

int main(int argc, char **argv) {
    if (argc != 2 && argc != 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    auto optimizations_enabled = true;
    if (argc == 3) {
        if (std::string(argv[2]) == "-o=0" || std::string(argv[1]) == "-o=0") {
            std::cout << "Optimizations disabled" << std::endl;
            optimizations_enabled = false;
        } else if (std::string(argv[2]) == "-o=1" || std::string(argv[1]) == "-o=1") {
            std::cout << "Optimizations enabled" << std::endl;
        } else {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    auto syntax_analyzer = SyntaxAnalyzer(argv[1]);
    auto program_global_block = syntax_analyzer.analyze();

    auto semantic_analyzer = SemanticAnalyzer(program_global_block);
    semantic_analyzer.analyze();
    auto used_builtin_functions = semantic_analyzer.get_used_builtin_functions();

    auto optimizer = Optimizer();
    if (optimizations_enabled)
        optimizer.optimize_ast(program_global_block);

    auto instructions_generator = InstructionsGenerator(program_global_block, used_builtin_functions);
    instructions_generator.generate();
    auto instructions = instructions_generator.get_instructions();

    if (optimizations_enabled)
        optimizer.optimize_instructions(instructions);

    auto instructions_file = std::ofstream("instructions.txt");
    for (auto &instruction: instructions) {
        std::cout << instruction.line << " " << instruction.instruction << " " << instruction.level << " " << instruction.parameter << std::endl;
        instructions_file << instruction.line << " " << instruction.instruction << " " << instruction.level << " " << instruction.parameter << std::endl;
    }
    instructions_file.close();

    return EXIT_SUCCESS;
}
