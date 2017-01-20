%{
#include "Format.tab.h"
extern char * yylval;
%}

id      [0-9]+
eq      [=]
delim   [ \t]
newline [\n]+
dot     [.]   
chinaese [\x80-\xff]{2} 
string  [A-Za-z0-9]+[_]*[A-Za-z0-9]*
comment #{delim}*{chinaese}*[ A-Za-z0-9]*{chinaese}*[ A-Za-z0-9]*
%%
{newline}       {
                return NEWLINE;
                }
TRANTYPE        {
		return TRANTYPE;
	        }
FMTDESC|MAC_CREATE|MAC_CHECK  {
		return HEAD;
	        }
{eq}            {
	        return EQ;
	        }
{id}            {
	        return ID;
	        }	
","             {
	        return COMMA;
	        }	
";"|"|"         {
	        return SEM;
	        }	
"("             {
	        return LPAREN;
	        }
")"             {
	        return RPAREN;
	        }
"[TDF]"	        {
	        return TDF;
	        }
<<EOF>>         { yyterminate();}
"&"{string}|"*"{string}|"*&"{string}  {
	        return TEMP;
                }
{string}|{string}"*" {
	        return STRING;
	        }	     
{string}*{chinaese}+*    {
                }
%%
int yywrap()
{
  return(1);
}
