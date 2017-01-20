%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char * string;
#define YYSYTPE string
int LineCount=1;

%}

%token STRING TEMP COMMENT NEWLINE
%token HEAD TRANTYPE
%token TDF EQ LPAREN RPAREN COMMA SEM 
%token ID EXPRESS  

%%
/* 
 *格式转换脚本语法框架
 */
file          :head TDF NEWLINE  {LineCount++;} 
              |file script          
              ;
/*
 * 格式转换脚本格式
 */
head          :head_trantype  {LineCount++;}
              |head_desc head {LineCount++;}
              |head head_desc {LineCount++;}
              ;
head_trantype :TRANTYPE EQ ID NEWLINE
              |error {} 
              ;
head_desc     :HEAD EQ STRING NEWLINE
              ;
script        :statement NEWLINE {LineCount++;}
              |error { printf("\n[Abort line %d:Format script syntax error]\n",LineCount); }
              ;
statement     :TEMP EQ rickle
              |EQ rickle 
              |rickle EQ rickle 
              ;
rickle        :ID
              |express
              ;
express       :STRING
              |STRING LPAREN RPAREN      
              |STRING LPAREN parameter RPAREN
              ;
parameter     :rickle
              |parameter COMMA SEM
              |parameter COMMA COMMA 
              |parameter COMMA parameter
              ;
%% 

int main()
{

  yyparse();
  return(0);
}

int yyerror(char *msg)
{
  return;
}
