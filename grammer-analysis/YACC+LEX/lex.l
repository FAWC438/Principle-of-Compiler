%{
	int getNum();
	#include "yacc.tab.h"
%}

digit [0-9]
num	{digit}+

%%

{num} {
	yylval = getNum();
	return(num);
}

"+" {return('+');}
"-" {return('-');}
"*" {return('*');}
"/" {return('/');}
"(" {return('(');}
")" {return(')');}
\n  {return('\n');}
%%
int yywrap() {
	return 1;
}

int getNum() {
	int sum = 0;
	for(int i = 0; yytext[i] != '\0' ;i++)
			sum = sum * 10 + (yytext[i] - '0');
	return sum;
}