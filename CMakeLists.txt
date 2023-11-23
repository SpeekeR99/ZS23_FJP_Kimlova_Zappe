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

flex_target(LEXER src/tokenizer.l ${LEXER_OUT} DEFINES_FILE ${LEXER_H_OUT})
bison_target(PARSER src/parser.y ${PARSER_OUT} DEFINES_FILE ${PARSER_H_OUT})
add_flex_bison_dependency(LEXER PARSER)

add_executable(
        yadc
        src/main.cpp
        src/SymbolTable.cpp
        src/SymbolTable.h
        src/InstructionsGenerator.cpp
        src/InstructionsGenerator.h
        src/AbstractSyntaxTree.h
        ${LEXER_H_OUT}
        ${LEXER_OUT}
        ${PARSER_H_OUT}
        ${PARSER_OUT}
)

target_include_directories(yadc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
