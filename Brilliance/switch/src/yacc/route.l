%{
#include "Route.tab.h"
%}

string  [A-Za-z0-9]+[_]*[A-Za-z0-9]*
id      [0-9]+
eq      [=]
delim   [ \t]
newline [\n]+
dot     [.]   
chinaese [\x80-\xff]{2} 
comment #{delim}*{chinaese}*[A-Za-z0-9]*[#,*]*{chinaese}*[A-Za-z0-9]
%%
{newline}       {
                return NEWLINE;
                }
ROUTEDESC       {
		return DESC;
	        }
SEND            { 
                return SEND;
                }
CALL            { 
                return CALL;
                }
ROLLBACK        { 
                return ROLLBACK;
                }
FTPPUT          { 
                return FTPPUT;
                }
FTPGET          { 
                return FTPGET;
                }
BINARY|binary   { 
                return BINARY;
                }
ASCII|ascii     { 
                return ASCII;
                }
QPUT            { 
                return QPUT;
                }
QGET            { 
                return QGET;
                }
IF              { 
                return IF;
                }
THEN            { 
                return THEN;}
GOTO            { 
                return GOTO;
                }
WITH            { 
                return WITH;
                }
GROUP           { 
                return GROUP;
                }
REVMODE         { 
                return REVMODE;
                }
REVGROUP        { 
                return REVGROUP;
                }
SAF             { 
                return SAF;
                }
END             { 
                return END;
                }
DO              {
                return DO;
                }
WHILE           { 
                return WHILE;
                }
CONTINUE        {
                return CONTINUE;
                }
EXIT            {
                return EXIT;
                }
ENDWHILE        {
                return ENDWHILE;
                }

SWITCH          {
                return SWITCH;
                }
CASE            {
                return CASE;
                }
BREAK           {
                return BREAK;
                }
DEFAULT         {
                return DEFAULT;
                }
ENDSWITCH       {
                return ENDSWITCH;
                }
SET             {
                return SET;
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
"("             {
	        return LPAREN;
	        }
")"             {
	        return RPAREN;
	        }
"[ROUTE]"  	{
	        return ROUTE;
	        }
"."             {
	        return POINT;
	        }
<<EOF>>         { yyterminate();}
"&"{string}|"*"{string}|"*&"{string}  {
	        return TEMP;
                }
{string}        {
	        return STRING;
	        }	     
{comment}       {
	        }   
{string}*{chinaese}**    {
                }
%%

int yywrap()
{
  return(1);
}
