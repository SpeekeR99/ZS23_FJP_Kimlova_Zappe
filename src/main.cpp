#include "analysis/SyntaxAnalyzer.h"
#include "synthesis/SemanticAnalyzer.h"
#include "synthesis/InstructionsGenerator.h"
#include "synthesis/Optimizer.h"

/**
 * Prints usage of the program to stderr
 * @param program_name name of the program
 */
void print_usage(const char *program_name) {
    std::cerr << "Usage: " << program_name << " <input file>" << std::endl;
    std::cerr << "Usage: " << program_name << " <input file> -o=<optimizations flag>" << std::endl;
    std::cerr << "Optimizations flags:" << std::endl;
    std::cerr << "    0 - no optimizations" << std::endl;
    std::cerr << "    1 - optimizations" << std::endl;
    std::cerr << "Default optimizations flag is 1" << std::endl;
}

/**
 * Main function of the program
 * @param argc Argument count
 * @param argv Argument values
 * @return EXIT_SUCCESS if program finished successfully, EXIT_FAILURE otherwise
 */
int main(int argc, char **argv) {
    /* Check if at least the input file is provided */
    if (argc != 2 && argc != 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    auto optimizations_enabled = true;
    /* Check if optimizations flag is provided */
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

    /* Syntax analysis */
    auto syntax_analyzer = SyntaxAnalyzer(argv[1]);
    auto program_global_block = syntax_analyzer.analyze();

    /* Semantic analysis */
    auto semantic_analyzer = SemanticAnalyzer(program_global_block);
    semantic_analyzer.analyze();
    /* Used builtin functions are needed for instructions generation */
    auto used_builtin_functions = semantic_analyzer.get_used_builtin_functions();

    /* Optimizations on the AST */
    auto optimizer = Optimizer();
    if (optimizations_enabled)
        optimizer.optimize_ast(program_global_block);

    /* Instructions generation */
    auto instructions_generator = InstructionsGenerator(program_global_block, used_builtin_functions);
    instructions_generator.generate();
    auto instructions = instructions_generator.get_instructions();

    /* Optimizations on the instructions */
    if (optimizations_enabled)
        optimizer.optimize_instructions(instructions);

    /* Output instructions to file (and stdout for debugging) */
    auto instructions_file = std::ofstream("instructions.txt");
    for (auto &instruction: instructions) {
        std::cout << instruction.line << " " << instruction.instruction << " " << instruction.level << " " << instruction.parameter << std::endl;
        instructions_file << instruction.line << " " << instruction.instruction << " " << instruction.level << " " << instruction.parameter << std::endl;
    }
    instructions_file.close();

    return EXIT_SUCCESS;
}
