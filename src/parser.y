%{
    #include <iostream>
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

%nonassoc  TYPE ID LITERAL CONSTANT BEGIN_BLOCK END_BLOCK

%nonassoc  IF ELSE FOR WHILE RETURN
%left AND OR NOT
%left EQ NEQ LESS LESSEQ GRT GRTEQ

%left SEMICOLON COMMA L_BRACKET R_BRACKET

%right ASSIGN_OP

%left ADD SUB
%left MUL DIV MOD
%left U_MINUS

%start program

%%

program:
    decl_var_stmt program { printf("decl_var_stmt\n"); }
    | decl_func_stmt program { printf("decl_func_stmt\n"); }
    | /* empty */ { printf("empty\n"); }

decl_var_stmt:
    TYPE ID SEMICOLON { printf("decl_var_stmt: type id \n"); }
    | CONSTANT TYPE ID SEMICOLON { printf("decl_var_stmt: constant type id \n"); }
    | TYPE assign_stmt { printf("decl_var_stmt: type assign_stmt \n"); }
    | CONSTANT TYPE assign_stmt { printf("decl_var_stmt: constant type assign_stmt \n"); }

assign_stmt:
    ID ASSIGN_OP expr SEMICOLON { printf("assign_stmt: id assign_op expr \n"); }

decl_func_stmt:
    TYPE ID L_BRACKET params R_BRACKET block { printf("decl_func_stmt: type id l_bracket params r_bracket block \n"); }
    | TYPE ID L_BRACKET params R_BRACKET SEMICOLON { printf("decl_func_stmt: type id l_bracket params r_bracket semicolon \n"); }

params:
    params_list { printf("params: params_list \n"); }
    | /* empty */ { printf("params: empty \n"); }

params_list:
    TYPE ID COMMA params_list { printf("params_list: type id comma params_list \n"); }
    | TYPE ID { printf("params_list: type id \n"); }

block:
    BEGIN_BLOCK stmts END_BLOCK { printf("block: begin_block stmts end_block \n"); }

stmts:
    stmt stmts { printf("stmts: stmt stmts \n"); }
    | /* empty */ { printf("stmts: empty \n"); }

stmt:
    SEMICOLON { printf("stmt: empty stmt = only semicolon \n"); }
    | decl_var_stmt { printf("stmt: decl_var_stmt \n"); }
    | assign_stmt { printf("stmt: assign_stmt \n"); }
    | if_stmt { printf("stmt: if_stmt \n");}
    | while_stmt { printf("stmt: while_stmt \n");}
    | for_stmt { printf("stmt: for_stmt \n");}
    | call_func_stmt { printf("stmt: call_func_stmt \n");}
    | return_stmt { printf("stmt: return_stmt \n");}

if_stmt:
    IF L_BRACKET expr R_BRACKET block else_stmt { printf("if_stmt: if l_bracket expr r_bracket block else_stmt \n"); }

else_stmt:
    ELSE block { printf("else_stmt: else block \n"); }
    | /* empty */ { printf("else_stmt: empty \n"); }

while_stmt:
    WHILE L_BRACKET expr R_BRACKET block { printf("while_stmt: while l_bracket expr r_bracket block \n"); }

for_stmt:
    FOR L_BRACKET expr SEMICOLON expr SEMICOLON expr R_BRACKET block { printf("for_stmt: for l_bracket expr semicolon expr semicolon expr r_bracket block \n"); }

call_func_stmt:
    call_func_expr SEMICOLON { printf("call_func_stmt: call_func_expr semicolon \n"); }

return_stmt:
    RETURN expr SEMICOLON { printf("return_stmt: return expr semicolon \n"); }

expr:
    ID { printf("expr: id \n");}
    | LITERAL { printf("expr: literal \n");}
    | L_BRACKET expr R_BRACKET { printf("expr: l_bracket expr r_bracket \n");}
    | arithm_expr { printf("expr: arithm_expr \n");}
    | logic_expr { printf("expr: logic_expr \n");}
    | compare_expr { printf("expr: compare_expr \n");}
    | cast_expr { printf("expr: cast_expr \n");}
    | call_func_expr { printf("expr: call_func_expr \n");}

arithm_expr:
    expr ADD expr { printf("arithm_expr: expr add expr \n");}
    | expr SUB expr { printf("arithm_expr: expr sub expr \n");}
    | expr MUL expr { printf("arithm_expr: expr mul expr \n");}
    | expr DIV expr { printf("arithm_expr: expr div expr \n");}
    | expr MOD expr { printf("arithm_expr: expr mod expr \n");}
    | SUB expr %prec U_MINUS { printf("arithm_expr: u_minus expr \n");}

logic_expr:
    expr AND expr { printf("logic_expr: expr and expr \n");}
    | expr OR expr { printf("logic_expr: expr or expr \n");}
    | NOT expr { printf("logic_expr: not expr \n");}

compare_expr:
    expr EQ expr { printf("compare_expr: expr eq expr \n");}
    | expr NEQ expr { printf("compare_expr: expr neq expr \n");}
    | expr LESS expr { printf("compare_expr: expr less expr \n");}
    | expr LESSEQ expr { printf("compare_expr: expr lesseq expr \n");}
    | expr GRT expr { printf("compare_expr: expr grt expr \n");}
    | expr GRTEQ expr { printf("compare_expr: expr grteq expr \n");}

cast_expr:
    L_BRACKET TYPE R_BRACKET expr { printf("cast_expr: l_bracket type r_bracket expr \n");}

call_func_expr:
    ID L_BRACKET args R_BRACKET { printf("call_func_expr: id l_bracket args r_bracket \n");}

args:
    args_list { printf("args: args_list \n");}
    | /* empty */ { printf("args: empty \n");}

args_list:
    expr COMMA args_list { printf("args_list: expr comma args_list \n");}
    | expr { printf("args_list: expr \n");}

%%

int yyerror(const char *s) {
    std::cout << "Error: " << std::string(s) << std::endl;
    return 0;
}
