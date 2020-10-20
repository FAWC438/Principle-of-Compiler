%{
#include <stdio.h>
#include <string.h>

int wordCount = 0;
int charCount = 0;
int columnCount = 0;
int lineCount = 1;
int IDCount = 0;
int stringCount = 0;
FILE *fp;


%}

delim     	 [' '\t]+
WARP      	 [\n]
NOTWARP   	 [^\n]
letter    	 [A-Za-z]
digit     	 [0-9]
NOTSTR		 [^\"]
NOTCHAR      [^\']
KEYWORD		 include|define|char|short|int|unsigned|long|float|double|struct|union|void|enum|signed|const|volatile|typedef|auto|register|static|extern|break|case|continue|default|do|else|for|goto|if|return|switch|while|sizeof
PUNC		 \,|\;
DOT			 \.
RELOP        \=|\=\=|\<|\>|\<\=|\>\=
OPERATOR     \+|\-|\*|\\|\&|\||\^|\?|\%
BRACKET      \(|\)|\{|\}|\[|\]
ID        	 ({letter}|\_)({letter}|{digit}|\_)*
NUM       	 {digit}+(\.{digit}+)?(E(\+|\-)?digit+)?
STR     	 \"{NOTWARP}*\"
CHAR         \'{NOTWARP}\'
MACRO		 \#
INCLUDELIB	 \<({letter}|{digit}|{DOT})*\>
LINECOMMENT  \/\/{NOTWARP}*
error1   	 {digit}+{letter}+({letter}|{digit})*
error2       (\"{NOTSTR}*)|(\'{NOTCHAR})
error3       @|~|`
%x        	 IN_COMMENT

%%
{delim}        {
	columnCount+=yyleng;
	charCount+=yyleng;
	}
{KEYWORD}      {
	printf("KEYWORD: %s\n",yytext);
	fprintf(fp,"KEYWORD: %s\n",yytext);
	columnCount+=yyleng;
	charCount+=yyleng;
	wordCount++;
	}
{ID}           {
	printf("ID%d: %s\n",IDCount,yytext);
	fprintf(fp,"ID%d: %s\n",IDCount,yytext);
	IDCount++;
	columnCount+=yyleng;
	charCount+=yyleng;
	wordCount++;
	}
{NUM}          {
	printf("NUM: %s\n",yytext);
	fprintf(fp,"NUM: %s\n",yytext);
	columnCount+=yyleng;
	charCount+=yyleng;
	}
{LINECOMMENT}  {
	printf("LINENOTE: %s\n",yytext);
	fprintf(fp,"LINENOTE: %s\n",yytext);
	}
{STR}          {
	printf("STRING%d: %s\n",stringCount,yytext);
	fprintf(fp,"STRING%d: %s\n",stringCount,yytext);
	stringCount++;
	columnCount+=yyleng;
	charCount+=yyleng;
	}
{CHAR}         {
	printf("CHAR: %s\n",yytext);
	fprintf(fp,"CHAR: %s\n",yytext);
	columnCount++;
	charCount++;
	}
{PUNC}	       {
	printf("PUNCTUATION: %s\n",yytext);
	fprintf(fp,"PUNCTUATION: %s\n",yytext);
	columnCount++;
	charCount++;
	}
{DOT}		   {
	printf("DOT: %s\n",yytext);
	fprintf(fp,"DOT: %s\n",yytext);
	columnCount++;
	charCount++;
	}
{MACRO}		   {
	printf("MACRO: %s\n",yytext);
	fprintf(fp,"MACRO: %s\n",yytext);
	columnCount++;
	charCount++;
	}
{OPERATOR}     {
	printf("OPERATOR: %s\n",yytext);
	fprintf(fp,"OPERATOR: %s\n",yytext);
	columnCount++;
	charCount++;
	}
{RELOP}        {
	printf("RELOP: %s\n",yytext);
	fprintf(fp,"RELOP: %s\n",yytext);
	columnCount+=yyleng;
	charCount+=yyleng;
	}
{BRACKET}      {
	printf("BRACKET: %s\n",yytext);
	fprintf(fp,"BRACKET: %s\n",yytext);
	columnCount+=yyleng;
	charCount+=yyleng;
	}
{INCLUDELIB}   {
	printf("INCLUDE: %s\n",yytext);
	fprintf(fp,"INCLUDE: %s\n",yytext);
	columnCount+=yyleng;
	charCount+=yyleng;
	}
{error1}       {
	printf("ERROR: Line %d, Column %d: Identity cannot start with number.\"%s\"\n",lineCount,columnCount+1,yytext);
	fprintf(fp,"ERROR: Line %d, Column %d: Identity cannot start with number.\"%s\"\n",lineCount,columnCount+1,yytext);
	columnCount+=yyleng;
	charCount+=yyleng;
	}
{error3}       {
	printf("ERROR: Line %d, Column %d: Invalid symbol.\"%s\"\n",lineCount,columnCount+1,yytext);
	fprintf(fp,"ERROR: Line %d, Column %d: Invalid symbol.\"%s\"\n",lineCount,columnCount+1,yytext);
	columnCount+=yyleng;
	charCount+=yyleng;}

<INITIAL>{
"/*"           BEGIN(IN_COMMENT);yymore();
}
<IN_COMMENT>{
"*/"           printf("ANNODATE: %s\n",yytext);BEGIN(INITIAL);
[^*\n]+	       yymore();
"*"            yymore();
\n             lineCount++;yymore();
}

{error2}       {printf("ERROR:Line %d, Column %d: Cannot Match the Left Puncuation.\n",lineCount,columnCount+1);fprintf(fp,"ERROR:Line %d, Column %d: Cannot Match the Left Puncuation.\n",lineCount,columnCount+1);columnCount+=yyleng;charCount+=yyleng;}
{WARP}         {lineCount++;columnCount=0;}
.              {printf("UNKNOW: %s\n",yytext);fprintf(fp,"UNKNOW: %s\n",yytext);charCount++;columnCount++;}

%%

int main()
{
	fp = fopen("result.txt", "w");
	yylex();
	printf("\nFinished.\nResult:\n");
	printf("Chars: %d \n",charCount);
	printf("Lines: %d \n",lineCount);
	printf("Words: %d \n",wordCount);
	fclose(fp);
	return 0;
}

int yyWARP()
{
	return 1;
}