#include <iostream>
#include "Parser.h"
#include "SymbolTable.h"

extern FILE *yyin;
extern FILE *yyout;

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input file>" << std::endl;
        return EXIT_FAILURE;
    }

    global_symbol_table = SymbolTable();
    global_symbol_table.insert_scope(0, 0);

    yyin = fopen(argv[1], "r");
    yyparse();
    fclose(yyin);

    std::cout << global_symbol_table << std::endl;

    return EXIT_SUCCESS;
}
