#pragma once

#include <fstream>
#include <utility>
#include "Parser.h"

extern ASTNodeBlock *global_block;
extern FILE *yyin;

class SyntaxAnalyzer {
private:
    std::string input_file_name;

public:
    explicit SyntaxAnalyzer(std::string input_file_name);
    ~SyntaxAnalyzer();

    ASTNodeBlock *analyze();
};