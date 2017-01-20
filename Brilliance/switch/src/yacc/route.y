%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char * string;
#define YYSYTPE string
int LineCount=1;

%}
       
%token STRING TEMP COMMENT NEWLINE
%token DESC 
%token ROUTE EQ LPAREN RPAREN COMMA 
%token ID IF THEN GOTO DO SET POINT 
%token WHILE CONTINUE EXIT ENDWHILE  
%token SWITCH CASE BREAK DEFAULT ENDSWITCH
%token FTPPUT FTPGET QPUT QGET BINARY ASCII
%token SEND CALL WITH GROUP REVMODE REVGROUP ROLLBACK SAF END
%%
/* 
 *·�ɽű��﷨���
 */
file          :head ROUTE NEWLINE {LineCount++;}
              |file script
              ;
/*
 * ·�ɽű��������� ·�ɽű���ʶ����ROUTER��
 */
head          :
              |DESC EQ rickle NEWLINE 
              ;
/*
 * ·�ɽű����ⲿ�� 
 */
script        :
              |ID WITH REVMODE ID COMMA REVGROUP rickle NEWLINE {LineCount++;}
              |ID statement NEWLINE {LineCount++;}
              |ID block  {LineCount++;}
              |error {printf("\n[Abort line %d:Route script syntax error]\n",LineCount);LineCount++;}           
              ;
block         :while 
              |switch   
              ;
while         :WHILE condition NEWLINE segment ID ENDWHILE NEWLINE
              ;
segment       :
              |segment ID statement NEWLINE 
              |segment ID CONTINUE  NEWLINE 
              |segment ID EXIT    NEWLINE 
              |segment ID block     
              ; 
switch        :SWITCH condition NEWLINE snatch ID ENDSWITCH NEWLINE 
              ;
snatch        :
              |snatch ID CASE condition NEWLINE 
              |snatch ID statement NEWLINE            
              |snatch ID DEFAULT NEWLINE      
              |snatch ID BREAK   NEWLINE      
              |snatch ID block          
              ;              
statement     :GOTO rickle               
              |SET STRING EQ rickle
              |SET TEMP EQ express 
              |DO express
              |ROLLBACK
              |SEND send
              |CALL call
              |FTPPUT LPAREN ftpstatement RPAREN 
              |FTPGET LPAREN ftpstatement RPAREN
              |QPUT LPAREN qstatement RPAREN
              |QGET LPAREN qstatement RPAREN
              |IF condition THEN express
              |IF condition THEN statement
              ;
call          :rickle 
              |call WITH callbranch
              ;
callbranch    :GROUP rickle
              |REVMODE ID
              |REVMODE ID COMMA REVGROUP rickle
              ;
send          :rickle
              |send WITH sendbranch
              ;
sendbranch    :END
              |SAF
              |GROUP rickle
              |END COMMA GROUP rickle
              |SAF COMMA GROUP rickle
              |GROUP rickle COMMA END
              |GROUP rickle COMMA SAF
              ;
/*      FTPPUT��Զ���������û������룬�����ļ���Զ���ļ�����ʱʱ�䣬����ģʽ�� */
ftpstatement  :rickle COMMA rickle COMMA filename COMMA filename COMMA rickle COMMA BINARY
              |rickle COMMA rickle COMMA filename COMMA filename COMMA rickle COMMA ASCII
              ;
/*       QGET����ţ������ļ���Զ���ļ�������С����ʱʱ�䣬�ط������� */
qstatement    :rickle COMMA filename COMMA filename COMMA rickle COMMA rickle COMMA rickle
              ;
filename      :rickle
              |rickle POINT rickle
              ;
condition     :rickle
              |condition EQ ID
              |condition EQ express
              ;
express       :STRING
              |STRING LPAREN RPAREN      
              |STRING LPAREN parameter RPAREN
              ;
parameter     :rickle
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
  return;
}
