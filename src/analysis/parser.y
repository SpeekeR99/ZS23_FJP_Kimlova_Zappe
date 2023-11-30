%{
    #include "../src/AbstractSyntaxTree.h"

    extern int yylineno;
    extern int column;

    ASTNodeBlock *global_block;
%}

%code requires {
    #include "AbstractSyntaxTree.h"
}

%code provides {
  int yyerror(const char *s);
  int yylex(YYSTYPE*, YYLTYPE*);
}

%locations
%define api.pure
%union {
    ASTNode *node;
    ASTNodeExpression *expr;
    ASTNodeStatement *stmt;
    ASTNodeBlock *block;
    std::vector<ASTNodeDeclVar *> *params;
    std::vector<ASTNodeExpression *> *args;
    std::string *string;
    int token;
}
%define parse.error verbose

%nonassoc TYPE ID LABEL INT_LITERAL BOOL_LITERAL CONSTANT BEGIN_BLOCK END_BLOCK

%nonassoc IF ELSE FOR WHILE DO REPEAT UNTIL BREAK CONTINUE RETURN NEW DELETE GOTO
%left AND OR NOT
%left EQ NEQ LESS LESSEQ GRT GRTEQ

%left SEMICOLON COMMA L_BRACKET R_BRACKET COLON

%right ASSIGN_OP

%nonassoc DEREF REF
%left ADD SUB
%left MUL DIV MOD
%left U_MINUS

%type <string> ID TYPE INT_LITERAL BOOL_LITERAL ADD SUB MUL DIV MOD AND OR NOT EQ NEQ LESS LESSEQ GRT GRTEQ ASSIGN_OP
%type <expr> expr arithm_expr logic_expr compare_expr cast_expr call_func_expr assign_expr memory_expr
%type <stmt> stmt decl_var_stmt decl_func_stmt if_stmt loop_stmt while_stmt do_while_stmt until_do_stmt repeat_until_stmt for_stmt jump_stmt break_stmt continue_stmt return_stmt goto_stmt
%type <block> program stmts block else_stmt
%type <params> params params_list
%type <args> args args_list
%type <token> ptr_modifier

%start program

%%

program:
    program decl_var_stmt {
        $1->statements.emplace_back($2);
    }
    | program decl_func_stmt {
        $1->statements.emplace_back($2);
    }
    | %empty {
        global_block = new ASTNodeBlock();
        $$ = global_block;
    }
;

decl_var_stmt:
    TYPE ID SEMICOLON {
        $$ = new ASTNodeDeclVar(*$1, 0, *$2, false, nullptr, yylineno);
        delete $1;
        delete $2;
    }
    | TYPE ptr_modifier ID SEMICOLON {
        $$ = new ASTNodeDeclVar(*$1, $2, *$3, false, nullptr, yylineno);
        delete $1;
        delete $3;
    }
    | CONSTANT TYPE ID SEMICOLON {
        $$ = new ASTNodeDeclVar(*$2, 0, *$3, true, nullptr, yylineno);
        delete $2;
        delete $3;
    }
    | CONSTANT TYPE ptr_modifier ID SEMICOLON {
        $$ = new ASTNodeDeclVar(*$2, $3, *$4, true, nullptr, yylineno);
        delete $2;
        delete $4;
    }
    | TYPE ID ASSIGN_OP expr SEMICOLON {
        $$ = new ASTNodeDeclVar(*$1, 0, *$2, false, $4, yylineno);
        delete $1;
        delete $2;
    }
    | TYPE ptr_modifier ID ASSIGN_OP expr SEMICOLON {
        $$ = new ASTNodeDeclVar(*$1, $2, *$3, false, $5, yylineno);
        delete $1;
        delete $3;
    }
    | CONSTANT TYPE ID ASSIGN_OP expr SEMICOLON {
        $$ = new ASTNodeDeclVar(*$2, 0, *$3, true, $5, yylineno);
        delete $2;
        delete $3;
    }
    | CONSTANT TYPE ptr_modifier ID ASSIGN_OP expr SEMICOLON {
        $$ = new ASTNodeDeclVar(*$2, $3, *$4, true, $6, yylineno);
        delete $2;
        delete $4;
    }
;

ptr_modifier:
    ptr_modifier DEREF {
        $$ = $1 + 1;
    }
    | DEREF {
        $$ = 1;
    }
;

decl_func_stmt:
    TYPE ID L_BRACKET params R_BRACKET block {
        $$ = new ASTNodeDeclFunc(*$1, *$2, *$4, $6, yylineno);
        delete $1;
        delete $2;
        delete $4;
    }
    | TYPE ID L_BRACKET params R_BRACKET SEMICOLON {
        $$ = new ASTNodeDeclFunc(*$1, *$2, *$4, nullptr, yylineno);
        delete $1;
        delete $2;
        delete $4;
    }
;

params:
    params_list {
        $$ = $1;
    }
    | %empty {
        $$ = new std::vector<ASTNodeDeclVar *>();
    }
;

params_list:
    params_list COMMA TYPE ID {
        $1->emplace_back(new ASTNodeDeclVar(*$3, 0, *$4, false, nullptr, yylineno));
        $$ = $1;
        delete $3;
        delete $4;
    }
    | params_list COMMA TYPE ptr_modifier ID {
        $1->emplace_back(new ASTNodeDeclVar(*$3, $4, *$5, false, nullptr, yylineno));
        $$ = $1;
        delete $3;
        delete $5;
    }
    | TYPE ID {
        $$ = new std::vector<ASTNodeDeclVar *>();
        $$->emplace_back(new ASTNodeDeclVar(*$1, 0, *$2, false, nullptr, yylineno));
        delete $1;
        delete $2;
    }
    | TYPE ptr_modifier ID {
        $$ = new std::vector<ASTNodeDeclVar *>();
        $$->emplace_back(new ASTNodeDeclVar(*$1, $2, *$3, false, nullptr, yylineno));
        delete $1;
        delete $3;
    }
;

block:
    BEGIN_BLOCK stmts END_BLOCK {
        $$ = $2;
    }
;

stmts:
    stmts stmt {
        $1->statements.emplace_back($2);
    }
    | stmts ID COLON stmt {
        $4->label = *$2;
        $1->statements.emplace_back($4);
        delete $2;
    }
    | %empty {
        $$ = new ASTNodeBlock();
    }
;

stmt:
    SEMICOLON {
        /* Empty */
    }
    | decl_func_stmt {
        $$ = $1;
    }
    | decl_var_stmt {
        $$ = $1;
    }
    | if_stmt {
        $$ = $1;
    }
    | loop_stmt {
        $$ = $1;
    }
    | jump_stmt {
        $$ = $1;
    }
    | expr SEMICOLON {
        $$ = new ASTNodeExpressionStatement($1, yylineno);
    }
;

if_stmt:
    IF L_BRACKET expr R_BRACKET block else_stmt {
        $$ = new ASTNodeIf($3, $5, $6, yylineno);
    }
;

else_stmt:
    ELSE block {
        $$ = $2;
    }
    | %empty {
        $$ = nullptr;
    }
;

loop_stmt:
    while_stmt {
        $$ = $1;
    }
    | do_while_stmt {
        $$ = $1;
    }
    | until_do_stmt {
        $$ = $1;
    }
    | repeat_until_stmt {
        $$ = $1;
    }
    | for_stmt {
        $$ = $1;
    }
;

while_stmt:
    WHILE L_BRACKET expr R_BRACKET block {
        $$ = new ASTNodeWhile($3, $5, false, false, yylineno);
    }
;

do_while_stmt:
    DO block WHILE L_BRACKET expr R_BRACKET SEMICOLON {
        $$ = new ASTNodeWhile($5, $2, true, false, yylineno);
    }
;

until_do_stmt:
    UNTIL L_BRACKET expr R_BRACKET DO block SEMICOLON {
        $$ = new ASTNodeWhile($3, $6, false, true, yylineno);
    }
;

repeat_until_stmt:
    REPEAT block UNTIL L_BRACKET expr R_BRACKET SEMICOLON {
        $$ = new ASTNodeWhile($5, $2, true, true, yylineno);
    }
;

for_stmt:
    FOR L_BRACKET expr SEMICOLON expr SEMICOLON expr R_BRACKET block {
        auto temp = new ASTNodeExpressionStatement($3, yylineno);
        $$ = new ASTNodeFor(temp, $5, $7, $9, yylineno);
    }
    | FOR L_BRACKET decl_var_stmt expr SEMICOLON expr R_BRACKET block {
        $$ = new ASTNodeFor($3, $4, $6, $8, yylineno);
    }
;

jump_stmt:
    break_stmt {
        $$ = $1;
    }
    | continue_stmt {
        $$ = $1;
    }
    | return_stmt {
        $$ = $1;
    }
    | goto_stmt {
        $$ = $1;
    }
;

break_stmt:
    BREAK SEMICOLON {
        $$ = new ASTNodeBreakContinue(true, yylineno);
    }
;

continue_stmt:
    CONTINUE SEMICOLON {
        $$ = new ASTNodeBreakContinue(false, yylineno);
    }
;

return_stmt:
    RETURN expr SEMICOLON {
        $$ = new ASTNodeReturn($2, yylineno);
    }
    | RETURN SEMICOLON {
        $$ = new ASTNodeReturn(nullptr, yylineno);
    }
;

goto_stmt:
    GOTO ID SEMICOLON {
        $$ = new ASTNodeGoto(*$2, yylineno);
        delete $2;
    }
;

expr:
    ID {
        $$ = new ASTNodeIdentifier(*$1, yylineno);
        delete $1;
    }
    | INT_LITERAL {
        $$ = new ASTNodeIntLiteral(atoi($1->c_str()), yylineno);
        delete $1;
    }
    | BOOL_LITERAL {
        $$ = new ASTNodeBoolLiteral(*$1 == "true", yylineno);
        delete $1;
    }
    | L_BRACKET expr R_BRACKET {
        $$ = $2;
    }
    | assign_expr {
        $$ = $1;
    }
    | arithm_expr {
        $$ = $1;
    }
    | logic_expr {
        $$ = $1;
    }
    | compare_expr {
        $$ = $1;
    }
    | cast_expr {
        $$ = $1;
    }
    | call_func_expr {
        $$ = $1;
    }
    | memory_expr {
        $$ = $1;
    }
;

assign_expr:
    ID ASSIGN_OP expr {
        $$ = new ASTNodeAssignExpression(*$1, nullptr, $3, yylineno);
        delete $1;
    }
    | expr ASSIGN_OP expr {
        if (auto deref = dynamic_cast<ASTNodeDereference *>($1))
            deref->is_lvalue = true;
        $$ = new ASTNodeAssignExpression("", $1, $3, yylineno);
    }
;

arithm_expr:
    expr ADD expr {
        $$ = new ASTNodeBinaryOperator($1, "+", $3, yylineno);
    }
    | expr SUB expr {
        $$ = new ASTNodeBinaryOperator($1, "-", $3, yylineno);
    }
    | expr MUL expr {
        $$ = new ASTNodeBinaryOperator($1, "*", $3, yylineno);
    }
    | expr DIV expr {
        $$ = new ASTNodeBinaryOperator($1, "/", $3, yylineno);
    }
    | expr MOD expr {
        $$ = new ASTNodeBinaryOperator($1, "%", $3, yylineno);
    }
    | SUB expr %prec U_MINUS {
        $$ = new ASTNodeUnaryOperator("-", $2, yylineno);
    }
;


logic_expr:
    expr AND expr {
        $$ = new ASTNodeBinaryOperator($1, "&&", $3, yylineno);
    }
    | expr OR expr {
        $$ = new ASTNodeBinaryOperator($1, "||", $3, yylineno);
    }
    | NOT expr {
        $$ = new ASTNodeUnaryOperator("!", $2, yylineno);
    }
;

compare_expr:
    expr EQ expr {
        $$ = new ASTNodeBinaryOperator($1, "==", $3, yylineno);
    }
    | expr NEQ expr {
        $$ = new ASTNodeBinaryOperator($1, "!=", $3, yylineno);
    }
    | expr LESS expr {
        $$ = new ASTNodeBinaryOperator($1, "<", $3, yylineno);
    }
    | expr LESSEQ expr {
        $$ = new ASTNodeBinaryOperator($1, "<=", $3, yylineno);
    }
    | expr GRT expr {
        $$ = new ASTNodeBinaryOperator($1, ">", $3, yylineno);
    }
    | expr GRTEQ expr {
        $$ = new ASTNodeBinaryOperator($1, ">=", $3, yylineno);
    }
;

cast_expr:
    L_BRACKET TYPE R_BRACKET expr {
        $$ = new ASTNodeCast(*$2, $4, yylineno);
    }
;

call_func_expr:
    ID L_BRACKET args R_BRACKET {
        $$ = new ASTNodeCallFunc(*$1, *$3, yylineno);
        delete $1;
        delete $3;
    }
;

args:
    args_list {
        $$ = $1;
    }
    | %empty {
        $$ = new std::vector<ASTNodeExpression *>();
    }
;

args_list:
    args_list COMMA expr {
        $1->emplace_back($3);
        $$ = $1;
    }
    | expr {
        $$ = new std::vector<ASTNodeExpression *>();
        $$->emplace_back($1);
    }
;

memory_expr:
    NEW L_BRACKET TYPE COMMA expr R_BRACKET {
        $$ = new ASTNodeNew(*$3, $5, yylineno);
        delete $3;
    }
    | DELETE expr {
        $$ = new ASTNodeDelete($2, yylineno);
    }
    | DEREF expr {
        $$ = new ASTNodeDereference($2, yylineno);
    }
    | REF ID {
        $$ = new ASTNodeReference(*$2, yylineno);
        delete $2;
    }
;

%%

int yyerror(const char *s) {
    std::string error = std::string(s);
    error = error.substr(error.find_first_of(",") + 2, error.length());
    std::cerr << "Syntax error: " << error << ", in line " << yylineno << ", column " << column << std::endl;
    exit(1);
}
