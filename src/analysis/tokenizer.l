%{
    #include <string>
    #include "../src/AbstractSyntaxTree.h"
    #include "Parser.h"

    #define SAVE_TOKEN yylval->string = new std::string(yytext, yyleng)
    #define TOKEN(t) (yylval->token = t)

    static int next_column = 1;
    int column = 1;

    #define HANDLE_COLUMN column = next_column; next_column += strlen(yytext)

    void handle_error(const char* text, int line, int column);

    int inside_comment = 0;
%}

%option bison-bridge bison-locations yylineno noyywrap

block_comment_start     \/\*
block_comment_end       \*\/
type                    (int|bool|void)
id                      ([a-z_][a-zA-Z0-9_]*)
int_literal             (([0-9])+)
bool_literal            (true|false)

%%

{block_comment_start}   { HANDLE_COLUMN; inside_comment++; }
{block_comment_end}     { HANDLE_COLUMN; inside_comment--; }
{type}                  { HANDLE_COLUMN; if (!inside_comment) { SAVE_TOKEN; return TYPE; } }
{int_literal}           { HANDLE_COLUMN; if (!inside_comment) { SAVE_TOKEN; return INT_LITERAL; } }
{bool_literal}          { HANDLE_COLUMN; if (!inside_comment) { SAVE_TOKEN; return BOOL_LITERAL; } }
;                       { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(SEMICOLON); } }
const                   { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(CONSTANT); } }
=                       { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(ASSIGN_OP); } }
\(                      { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(L_BRACKET); } }
\)                      { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(R_BRACKET); } }
,                       { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(COMMA); } }
\{                      { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(BEGIN_BLOCK); } }
\}                      { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(END_BLOCK); } }
if                      { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(IF); } }
else                    { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(ELSE); } }
while                   { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(WHILE); } }
do                      { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(DO); } }
for                     { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(FOR); } }
break                   { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(BREAK); } }
continue                { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(CONTINUE); } }
return                  { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(RETURN); } }
new                     { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(NEW); } }
delete                  { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(DELETE); } }
\+                      { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(ADD); } }
\-                      { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(SUB); } }
\*                      { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(MUL); } }
\/                      { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(DIV); } }
%                       { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(MOD); } }
&&                      { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(AND); } }
\|\|                    { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(OR); } }
!                       { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(NOT); } }
==                      { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(EQ); } }
!=                      { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(NEQ); } }
\<                      { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(LESS); } }
\<=                     { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(LESSEQ); } }
\>                      { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(GRT); } }
\>=                     { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(GRTEQ); } }
\@                      { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(REF); } }
\^                      { HANDLE_COLUMN; if (!inside_comment) { return TOKEN(DEREF); } }
[ \t]+                  { HANDLE_COLUMN; }
\r\n                    { HANDLE_COLUMN; next_column = 1; }
\n                      { HANDLE_COLUMN; next_column = 1; }
\r                      { HANDLE_COLUMN; next_column = 1; }
{id}                    { HANDLE_COLUMN; if (!inside_comment) { SAVE_TOKEN; return ID; } }
.                       { HANDLE_COLUMN; if (!inside_comment) { handle_error(yytext, yylineno, column); } }

%%

void handle_error(const char* text, int line, int column) {
    std::cerr << "Lexical error: unexpected character \'" << text << "\', at line " << line << ", column " << column << std::endl;
    exit(1);
}
