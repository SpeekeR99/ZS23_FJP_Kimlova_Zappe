%{

#include <stdlib.h>
#include <stdio.h>

#include "grammar.tab.h"

%}

%option bison-bridge bison-locations

type        (int|bool|void)
id          ([a-z_][a-zA-Z0-9_]*)
literal     (([0-9])+)

%%

{type}      { yylval->string = yytext; return TYPE; }
{literal}   { yylval->number = atoi(yytext); return LITERAL; }
;           { return SEMICOLON; }
const       { return CONSTANT; }
=           { return ASSIGN_OP; }
\(          { return L_BRACKET; }
\)          { return R_BRACKET; }
,           { return COMMA; }
\{          { return BEGIN_BLOCK; }
\}          { return END_BLOCK; }
if          { return IF; }
else        { return ELSE; }
while       { return WHILE; }
for         { return FOR; }
return      { return RETURN; }
\+          { return ADD; }
\-          { return SUB; }
\*          { return MUL; }
\/          { return DIV; }
%           { return MOD; }
&&          { return AND; }
\|\|        { return OR; }
!           { return NOT; }
==          { return EQ; }
!=          { return NEQ; }
\<          { return LESS; }
\<=         { return LESSEQ; }
\>          { return GRT; }
\>=         { return GRTEQ; }
[ \t\n]+    ;
{id}        { yylval->string = yytext; return ID; }

%%
