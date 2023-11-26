#include "SyntaxAnalyzer.h"

SyntaxAnalyzer::SyntaxAnalyzer(std::string input_file_name) : input_file_name(std::move(input_file_name)) {
    /* Empty */
}

SyntaxAnalyzer::~SyntaxAnalyzer() = default;

ASTNodeBlock *SyntaxAnalyzer::analyze() {
    yyin = fopen(this->input_file_name.c_str(), "r");
    yyparse();
    fclose(yyin);

    return global_block;
}
