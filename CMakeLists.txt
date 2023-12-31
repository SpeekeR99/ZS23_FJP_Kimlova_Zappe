cmake_minimum_required(VERSION 3.21)
project(ZS23_FJP_Kimlova_Zappe)

set(CMAKE_CXX_STANDARD 23)

find_package(FLEX)
find_package(BISON)

include_directories(src)

set(LEXER_OUT ${CMAKE_CURRENT_BINARY_DIR}/Tokenizer.cpp)
set(PARSER_OUT ${CMAKE_CURRENT_BINARY_DIR}/Parser.cpp)
set(LEXER_H_OUT ${CMAKE_CURRENT_BINARY_DIR}/Tokenizer.h)
set(PARSER_H_OUT ${CMAKE_CURRENT_BINARY_DIR}/Parser.h)

flex_target(LEXER src/analysis/tokenizer.l ${LEXER_OUT} DEFINES_FILE ${LEXER_H_OUT})
bison_target(PARSER src/analysis/parser.y ${PARSER_OUT} DEFINES_FILE ${PARSER_H_OUT})
add_flex_bison_dependency(LEXER PARSER)

add_executable(
        yadc
        src/main.cpp
        src/SymbolTable.cpp
        src/SymbolTable.h
        src/AbstractSyntaxTree.cpp
        src/AbstractSyntaxTree.h
        src/analysis/SyntaxAnalyzer.cpp
        src/analysis/SyntaxAnalyzer.h
        src/synthesis/SemanticAnalyzer.cpp
        src/synthesis/SemanticAnalyzer.h
        src/synthesis/BuiltinFunctions.cpp
        src/synthesis/InstructionsGenerator.cpp
        src/synthesis/InstructionsGenerator.h
        src/synthesis/Optimizer.cpp
        src/synthesis/Optimizer.h
        ${LEXER_H_OUT}
        ${LEXER_OUT}
        ${PARSER_H_OUT}
        ${PARSER_OUT}
)

target_include_directories(yadc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})