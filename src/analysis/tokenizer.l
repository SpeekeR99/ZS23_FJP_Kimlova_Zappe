%{
    #include <string>
    #include "../src/AbstractSyntaxTree.h"
    #include "Parser.h"

    #define SAVE_TOKEN yylval->string = new std::string(yytext, yyleng)
    #define TOKEN(t) (yylval->token = t)

    static int next_column = 1;
    int column = 1;

    #define HANDLE_COLUMN column = next_column; next_column += strlen(yytext)

%}

%option bison-bridge bison-locations yylineno noyywrap

type        (int|bool|void)
id          ([a-z_][a-zA-Z0-9_]*)
literal     (([0-9])+)

%%

{type}      { HANDLE_COLUMN; SAVE_TOKEN; return TYPE; }
{literal}   { HANDLE_COLUMN; SAVE_TOKEN; return LITERAL; }
;           { HANDLE_COLUMN; return TOKEN(SEMICOLON); }
const       { HANDLE_COLUMN; return TOKEN(CONSTANT); }
=           { HANDLE_COLUMN; return TOKEN(ASSIGN_OP); }
\(          { HANDLE_COLUMN; return TOKEN(L_BRACKET); }
\)          { HANDLE_COLUMN; return TOKEN(R_BRACKET); }
,           { HANDLE_COLUMN; return TOKEN(COMMA); }
\{          { HANDLE_COLUMN; return TOKEN(BEGIN_BLOCK); }
\}          { HANDLE_COLUMN; return TOKEN(END_BLOCK); }
if          { HANDLE_COLUMN; return TOKEN(IF); }
else        { HANDLE_COLUMN; return TOKEN(ELSE); }
while       { HANDLE_COLUMN; return TOKEN(WHILE); }
for         { HANDLE_COLUMN; return TOKEN(FOR); }
return      { HANDLE_COLUMN; return TOKEN(RETURN); }
\+          { HANDLE_COLUMN; return TOKEN(ADD); }
\-          { HANDLE_COLUMN; return TOKEN(SUB); }
\*          { HANDLE_COLUMN; return TOKEN(MUL); }
\/          { HANDLE_COLUMN; return TOKEN(DIV); }
%           { HANDLE_COLUMN; return TOKEN(MOD); }
&&          { HANDLE_COLUMN; return TOKEN(AND); }
\|\|        { HANDLE_COLUMN; return TOKEN(OR); }
!           { HANDLE_COLUMN; return TOKEN(NOT); }
==          { HANDLE_COLUMN; return TOKEN(EQ); }
!=          { HANDLE_COLUMN; return TOKEN(NEQ); }
\<          { HANDLE_COLUMN; return TOKEN(LESS); }
\<=         { HANDLE_COLUMN; return TOKEN(LESSEQ); }
\>          { HANDLE_COLUMN; return TOKEN(GRT); }
\>=         { HANDLE_COLUMN; return TOKEN(GRTEQ); }
[ \t]+      { HANDLE_COLUMN; }
"\n"        { HANDLE_COLUMN; next_column = 1; }
{id}        { HANDLE_COLUMN; SAVE_TOKEN; return ID; }

%%