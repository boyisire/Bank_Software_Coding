%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef YYDEBUG
#define YYDEBUG 1
#endif
int LineCount=1;
%}

%union {
       long id;
       char * string;  
       }
       
%token STRING TEMP COMMENT NEWLINE
%token HEAD TRANTYPE
%token TDF EQ LPAREN RPAREN LBRACKET RBRACKET COMMA SEM
%token ID EXPRESS  
%token IF THEN GOTO RETURN TRACE DO
%token WHILE CONTINUE EXIT ENDWHILE  
%token SWITCH CASE BREAK DEFAULT ENDSWITCH
%token SET VAR FIELD 

%%
/* 
 *格式转换脚本语法框架
 */
file          :head TDF NEWLINE    {LineCount++;}
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
head_desc     :HEAD EQ  NEWLINE
              |HEAD EQ express NEWLINE   
              |error {}
              ;
script        :ID statement NEWLINE {LineCount++;} 
              |ID block     
              |error {printf("\n[Abort line %d:Format script syntax error]\n",LineCount);LineCount++;}
              ;
block         :while 
              |switch   
              ;
while         :WHILE condition NEWLINE segment ID ENDWHILE NEWLINE  {LineCount++;}       
              ;
segment       :
              |segment ID statement NEWLINE  {LineCount++;} 
              |segment ID CONTINUE NEWLINE   {LineCount++;} 
              |segment ID EXIT  NEWLINE      {LineCount++;} 
              |segment ID block     
              ; 
switch        :SWITCH condition NEWLINE snatch ID ENDSWITCH NEWLINE {LineCount++;} 
              ;
snatch        : 
              |snatch ID CASE condition NEWLINE {LineCount++;} 
              |snatch ID statement NEWLINE     {LineCount++;} 
              |snatch ID DEFAULT  NEWLINE      {LineCount++;} 
              |snatch ID BREAK NEWLINE         {LineCount++;} 
              |snatch ID block          
              ;              
statement     :GOTO rickle                
              |VAR temp  
              |SET temp 
              |TDF tdfgroup
              |RETURN condition
              |TRACE express
              |DO express
              |FIELD rickle EQ rickle 
              |IF condition THEN express
              |IF condition THEN statement
              ;
temp          :STRING EQ rickle 
              |STRING LBRACKET STRING RBRACKET EQ rickle 
              ;
tdfgroup      :ID
              |tdfgroup COMMA express
              ;
condition     :rickle
              |condition EQ rickle 
              ;
express       :STRING
              |STRING LPAREN RPAREN      
              |STRING LPAREN parameter RPAREN
              |VAR LPAREN parameter RPAREN
              ;
parameter     :rickle
              |parameter COMMA SEM
              |parameter COMMA COMMA 
              |parameter COMMA parameter
              ;
rickle        :ID
              |express
              ;
%% 

int main()
{

  yyparse();
  return(0);
}

int yyerror(char *msg)
{
  printf("",msg);
  return;
}
