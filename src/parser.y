%{
    #include "SymbolTable.h"
    #include "InstructionsGenerator.h"

    SymbolTable global_symbol_table;
    InstructionsGenerator global_instructions_generator;

    std::vector<int> stack{};
%}

%code provides {
  int yyerror(const char *s);
  int yylex(YYSTYPE*, YYLTYPE*);
}

%locations
%define api.pure
%union {
    int number;
    char *string;
}

%nonassoc TYPE ID LITERAL CONSTANT BEGIN_BLOCK END_BLOCK

%nonassoc IF ELSE FOR WHILE RETURN
%left AND OR NOT
%left EQ NEQ LESS LESSEQ GRT GRTEQ

%left SEMICOLON COMMA L_BRACKET R_BRACKET

%right ASSIGN_OP

%left ADD SUB
%left MUL DIV MOD
%left U_MINUS

%start program

%type <string> ID TYPE
%type <number> LITERAL expr

%%

program:
    decl_var_stmt program {
        printf("decl_var_stmt\n");
    }
    | decl_func_stmt program {
        printf("decl_func_stmt\n");
    }
    | /* empty */ {
        printf("empty\n");
    }

decl_var_stmt:
    TYPE ID SEMICOLON {
        global_symbol_table.insert_symbol($2, VARIABLE, $1, false);
        free($1);
        free($2);
    }
    | CONSTANT TYPE ID SEMICOLON {
        global_symbol_table.insert_symbol($3, VARIABLE, $2, true);
        free($2);
        free($3);
    }
    | TYPE ID ASSIGN_OP expr SEMICOLON {
        global_symbol_table.insert_symbol($2, VARIABLE, $1, false);
        auto address = global_symbol_table.get_symbol($2).address;
        auto level = global_symbol_table.get_symbol_level($2);
        global_instructions_generator.generate(STO, level, address);
        free($1);
        free($2);
    }
    | CONSTANT TYPE ID ASSIGN_OP expr SEMICOLON {
        global_symbol_table.insert_symbol($3, VARIABLE, $2, true);
        auto address = global_symbol_table.get_symbol($3).address;
        auto level = global_symbol_table.get_symbol_level($3);
        global_instructions_generator.generate(STO, level, address);
        free($2);
        free($3);
    }

assign_stmt:
    ID ASSIGN_OP expr SEMICOLON {
        auto address = global_symbol_table.get_symbol($1).address;
        auto level = global_symbol_table.get_symbol_level($1);
        global_instructions_generator.generate(STO, level, address);
        free($1);
    }

decl_func_stmt:
    TYPE ID L_BRACKET params R_BRACKET { /* TODO: call a deklarace jsou rozdilny veci */
        auto instruction_line = global_instructions_generator.get_instruction_counter() + 1;
        auto &symbol = global_symbol_table.get_symbol($2);
        if (symbol.name == "")
            global_symbol_table.insert_symbol($2, FUNCTION, $1, false, instruction_line);
        else
            symbol.address = instruction_line;
        free($1);
        free($2);

        global_symbol_table.insert_scope(0, 3, true);

        auto instruction_line_jmp = global_instructions_generator.get_instruction_counter();
        stack.emplace_back(instruction_line_jmp);
        global_instructions_generator.generate(JMP, 0, 0);
        global_instructions_generator.generate(INT, 0, 3);
    }
    block {
        auto old_instruction_line_jmp = stack.back();
        stack.pop_back();
        auto &jmp_over_func_instr = global_instructions_generator.get_instruction(old_instruction_line_jmp);
        jmp_over_func_instr.parameter = global_instructions_generator.get_instruction_counter();
    }
    | TYPE ID L_BRACKET params R_BRACKET SEMICOLON {
        auto instruction_line = global_instructions_generator.get_instruction_counter();
        global_symbol_table.insert_symbol($2, FUNCTION, $1, false, instruction_line);
        free($1);
        free($2);
    }

params:
    params_list {
        printf("params: params_list \n");
    }
    | /* empty */ {
        printf("params: empty \n");
    }

params_list:
    TYPE ID COMMA params_list {
        printf("params_list: type id comma params_list \n");
        free($1);
        free($2);
    }
    | TYPE ID {
        printf("params_list: type id \n");
        free($1);
        free($2);
    }

block:
    BEGIN_BLOCK {
        auto number_of_variables = global_symbol_table.get_number_of_variables();
        auto instruction_line = global_instructions_generator.get_instruction_counter();
        stack.emplace_back(instruction_line);
        global_instructions_generator.generate(INT, 0, 0);
    }
    stmts END_BLOCK {
        auto number_of_variables = global_symbol_table.get_number_of_variables();
        auto old_instruction_line = stack.back();
        stack.pop_back();
        auto &last_int_instr = global_instructions_generator.get_instruction(old_instruction_line);
        last_int_instr.parameter = number_of_variables;
        global_instructions_generator.generate(INT, 0, -number_of_variables);
        global_symbol_table.remove_scope();
    }

stmts:
    stmt stmts {
        printf("stmts: stmt stmts \n");
    }
    | /* empty */ {
        printf("stmts: empty \n");
    }

stmt:
    SEMICOLON {
        ;
    }
    | decl_var_stmt {
        ;
    }
    | assign_stmt {
        ;
    }
    | if_stmt {
        printf("stmt: if_stmt \n");
    }
    | while_stmt {
        printf("stmt: while_stmt \n");
    }
    | for_stmt {
        printf("stmt: for_stmt \n");
    }
    | call_func_stmt {
        printf("stmt: call_func_stmt \n");
    }
    | return_stmt {
        printf("stmt: return_stmt \n");
    }

if_stmt:
    IF L_BRACKET expr R_BRACKET block else_stmt {
        printf("if_stmt: if l_bracket expr r_bracket block else_stmt \n");
    }

else_stmt:
    ELSE block {
        printf("else_stmt: else block \n");
    }
    | /* empty */ {
        printf("else_stmt: empty \n");
    }

while_stmt:
    WHILE L_BRACKET expr R_BRACKET block {
        printf("while_stmt: while l_bracket expr r_bracket block \n");
    }

for_stmt:
    FOR L_BRACKET expr SEMICOLON expr SEMICOLON expr R_BRACKET block {
        printf("for_stmt: for l_bracket expr semicolon expr semicolon expr r_bracket block \n");
    }

call_func_stmt:
    call_func_expr SEMICOLON {
        printf("call_func_stmt: call_func_expr semicolon \n");
    }

return_stmt:
    RETURN expr SEMICOLON {
        global_instructions_generator.generate(RET, 0, 0);
    }
    | RETURN SEMICOLON {
        global_instructions_generator.generate(RET, 0, 0);
    }

expr:
    ID {
        auto address = global_symbol_table.get_symbol($1).address;
        auto level = global_symbol_table.get_symbol_level($1);
        global_instructions_generator.generate(LOD, level, address);
        free($1);
    }
    | LITERAL {
        global_instructions_generator.generate(LIT, 0, $1);
    }
    | L_BRACKET expr R_BRACKET {
        ; /* Empty */
    }
    | arithm_expr {
        ; /* Empty */
    }
    | logic_expr {
        ; /* Empty */
    }
    | compare_expr {
        ; /* Empty */
    }
    | cast_expr {
        ;
    }
    | call_func_expr {
        ;
    }

arithm_expr:
    expr ADD expr {
        global_instructions_generator.generate(OPR, 0, PL0_ADD);
    }
    | expr SUB expr {
        global_instructions_generator.generate(OPR, 0, PL0_SUB);
    }
    | expr MUL expr {
        global_instructions_generator.generate(OPR, 0, PL0_MUL);
    }
    | expr DIV expr {
        global_instructions_generator.generate(OPR, 0, PL0_DIV);
    }
    | expr MOD expr {
        global_instructions_generator.generate(OPR, 0, PL0_MOD);
    }
    | SUB expr %prec U_MINUS {
        global_instructions_generator.generate(OPR, 0, PL0_NEG);
    }

logic_expr:
    expr AND expr {
        global_instructions_generator.generate(OPR, 0, PL0_ADD);
        global_instructions_generator.generate(LIT, 0, 2);
        global_instructions_generator.generate(OPR, 0, PL0_EQ);
    }
    | expr OR expr {
        global_instructions_generator.generate(OPR, 0, PL0_ADD);
        global_instructions_generator.generate(LIT, 0, 0);
        global_instructions_generator.generate(OPR, 0, PL0_NEQ);
    }
    | NOT expr {
        global_instructions_generator.generate(LIT, 0, 0);
        global_instructions_generator.generate(OPR, 0, PL0_EQ);
    }

compare_expr:
    expr EQ expr {
        global_instructions_generator.generate(OPR, 0, PL0_EQ);
    }
    | expr NEQ expr {
        global_instructions_generator.generate(OPR, 0, PL0_NEQ);
    }
    | expr LESS expr {
        global_instructions_generator.generate(OPR, 0, PL0_LT);
    }
    | expr LESSEQ expr {
        global_instructions_generator.generate(OPR, 0, PL0_LEQ);
    }
    | expr GRT expr {
        global_instructions_generator.generate(OPR, 0, PL0_GRT);
    }
    | expr GRTEQ expr {
        global_instructions_generator.generate(OPR, 0, PL0_GEQ);
    }

cast_expr:
    L_BRACKET TYPE R_BRACKET expr {
        printf("cast_expr: l_bracket type r_bracket expr \n");
    }

call_func_expr:
    ID L_BRACKET args R_BRACKET {
        auto &func_symbol = global_symbol_table.get_symbol($1);
        auto level = global_symbol_table.get_symbol_level($1);
        global_instructions_generator.generate(CAL, level, func_symbol.address);
        free($1);
    }

args:
    args_list {
        printf("args: args_list \n");
    }
    | /* empty */ {
        printf("args: empty \n");
    }

args_list:
    expr COMMA args_list {
        printf("args_list: expr comma args_list \n");
    }
    | expr {
        printf("args_list: expr \n");
    }

%%

int yyerror(const char *s) {
    std::cout << "Error: " << std::string(s) << std::endl;
    return 0;
}
