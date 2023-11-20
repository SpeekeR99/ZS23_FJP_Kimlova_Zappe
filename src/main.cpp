#include <iostream>
#include "parser.h"

int main(int argc, char **argv) {
    yyparse();
    return EXIT_SUCCESS;
}
