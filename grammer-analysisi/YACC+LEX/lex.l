%{
	int calc_num();
	#include "yacc.tab.h"
%}

digit [0-9]
num	{digit}+

%%

{num} {
	yylval=calc_num();
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

int calc_num() {
	int sum = 0, i;
	for(i = 0; yytext[i] != '\0' ;i++)
			sum = sum * 10 + (yytext[i] - '0');
	return sum;
}