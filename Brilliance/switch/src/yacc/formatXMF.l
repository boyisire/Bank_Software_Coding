%{
#include "XMF.tab.h"
%}
string  [A-Za-z0-9]+[_]*[A-Za-z0-9]*
id      [0-9]+
eq      [=]
delim   [ \t]
newline [\n]+
dot     [.]   
comment #{delim}*{chinaese}*[ A-Za-z0-9]*{chinaese}*[ A-Za-z0-9]*
chinaese [\x80-\xff]{2}
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
IF              { 
                return IF;
                }
THEN            { 
                return THEN;}
GOTO            { 
                return GOTO;
                }
RETURN          { 
                return RETURN;
                }
TRACE           {
                return TRACE;
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
VAR             {
                return VAR;
                }
FIELD           {
                return FIELD;
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
"["             {
	        return LBRACKET;
	        }
"]"             {
	        return RBRACKET;
	        }
"[TDF]"	        {
	        return TDF;
	        }
{string}|{string}"*"        {
	        return STRING;
	        }	     
{comment}       {     
                }
{string}*{chinaese}+* {
                }

%%
int yywrap()
{
  return(1);
}
