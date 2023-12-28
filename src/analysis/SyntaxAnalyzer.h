#pragma once

#include <fstream>
#include <utility>
#include "Parser.h"

/** Global "block" of the program; basically just root of the AST */
extern ASTNodeBlock *global_block;
/** Input file stream */
extern FILE *yyin;

/**
 * Class for syntax analysis
 */
class SyntaxAnalyzer {
private:
    /** Input file name */
    std::string input_file_name;

public:
    /**
     * Constructor
     * @param input_file_name Input file name
     */
    explicit SyntaxAnalyzer(std::string input_file_name);
    /**
     * Destructor
     */
    ~SyntaxAnalyzer();

    /**
     * Analyze the syntax
     * @return Root of the AST
     */
    ASTNodeBlock *analyze();
};