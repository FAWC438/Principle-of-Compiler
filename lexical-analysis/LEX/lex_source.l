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

delim     	 ([' ']|[\t])+
ANY			 [.]
WARP      	 [\n]
NOTWARP   	 [^\n]
letter    	 [A-Za-z]
digit     	 [0-9]
NOTSTR		 [^\"]
NOTCHAR      [^\']
ESC			 [\\]
NOTESC		 [^\\]
KEYWORD		 include|define|char|short|int|unsigned|long|float|double|struct|union|void|enum|signed|const|volatile|typedef|auto|register|static|extern|break|case|continue|default|do|else|for|goto|if|return|switch|while|sizeof
STR     	 \"{NOTWARP}*\"
CHAR         ({delim}?\'{NOTESC}?\')|({delim}?\'{ESC}{letter}?\')
PUNC		 \,|\;
DOT			 \.
RELOP        \=\=|\<|\>|\<\=|\>\=
EQUAL		 \=
OPERATOR     \+|\-|\*|\\|\&|\||\^|\?|\%|\+\+|\-\-|\+\=|\-\=|\*\=|\/\=
BRACKET      \(|\)|\{|\}|\[|\]
ID        	 ({letter}|\_)({letter}|{digit}|\_)*
NUM       	 {digit}+(\.{digit}+)?(E(\+|\-)?digit+)?
MACRO		 \#
INCLUDELIB	 \<({letter}|{digit}|{DOT})*\>
LINECOMMENT  \/\/{NOTWARP}*
error1   	 {digit}+{letter}+({letter}|{digit})*
error2       (\"{NOTSTR}*)
error3       @|~|`
error4		 (({delim}?\'{NOTESC}{NOTCHAR}))|(({delim}?\'{ESC}{NOTCHAR}{NOTCHAR}))
%x        	 IN_COMMENT

%%
{delim}        {
	columnCount+=yyleng;
	charCount+=yyleng;
	}
{KEYWORD}      {
	fprintf(fp,"KEYWORD(%d, %d): %s\n",lineCount,columnCount,yytext);
	columnCount+=yyleng;
	charCount+=yyleng;
	wordCount++;
	}
{ID}           {
	fprintf(fp,"ID%d(%d, %d): %s\n",IDCount,lineCount,columnCount,yytext);
	IDCount++;
	columnCount+=yyleng;
	charCount+=yyleng;
	wordCount++;
	}
{NUM}          {
	fprintf(fp,"NUM(%d, %d): %s\n",lineCount,columnCount,yytext);
	columnCount+=yyleng;
	charCount+=yyleng;
	}
{LINECOMMENT}  {
	fprintf(fp,"LINENOTE(%d, %d): %s\n",lineCount,columnCount,yytext);
	}
{STR}          {
	fprintf(fp,"STRING%d(%d, %d): %s\n",stringCount,lineCount,columnCount,yytext);
	stringCount++;
	columnCount+=yyleng;
	charCount+=yyleng;
	}
{CHAR}         {
	fprintf(fp,"CHAR(%d, %d): %s\n",lineCount,columnCount,yytext);
	columnCount+=yyleng;
	charCount+=yyleng;
	}
{PUNC}	       {
	fprintf(fp,"PUNCTUATION(%d, %d): %s\n",lineCount,columnCount,yytext);
	columnCount++;
	charCount++;
	}
{DOT}		   {
	fprintf(fp,"DOT(%d, %d): %s\n",lineCount,columnCount,yytext);
	columnCount++;
	charCount++;
	}
{MACRO}		   {
	fprintf(fp,"MACRO(%d, %d): %s\n",lineCount,columnCount,yytext);
	columnCount++;
	charCount++;
	}
{OPERATOR}     {
	fprintf(fp,"OPERATOR(%d, %d): %s\n",lineCount,columnCount,yytext);
	columnCount++;
	charCount++;
	}
{RELOP}        {
	fprintf(fp,"RELOP(%d, %d): %s\n",lineCount,columnCount,yytext);
	columnCount+=yyleng;
	charCount+=yyleng;
	}
{EQUAL}		   {
	fprintf(fp,"EQUAL(%d, %d): %s\n",lineCount,columnCount,yytext);
	columnCount++;
	charCount++;
	}
{BRACKET}      {
	fprintf(fp,"BRACKET(%d, %d): %s\n",lineCount,columnCount,yytext);
	columnCount+=yyleng;
	charCount+=yyleng;
	}
{INCLUDELIB}   {
	fprintf(fp,"INCLUDE(%d, %d): %s\n",lineCount,columnCount,yytext);
	columnCount+=yyleng;
	charCount+=yyleng;
	}
{error1}       {
	fprintf(fp,"ERROR: Line %d, Column %d: Identity cannot start with number \"%s\"\n",lineCount,columnCount+1,yytext);
	columnCount+=yyleng;
	charCount+=yyleng;
	}
{error2}       {
	fprintf(fp,"ERROR:Line %d, Column %d: String cannot match the left puncuation \n",lineCount,columnCount+1);
	columnCount+=yyleng;
	charCount+=yyleng;
	}
{error3}       {
	fprintf(fp,"ERROR: Line %d, Column %d: Invalid symbol \"%s\"\n",lineCount,columnCount+1,yytext);
	columnCount+=yyleng;
	charCount+=yyleng;
	}
{error4}	   {
	fprintf(fp,"ERROR: Line %d, Column %d: Invalid char pattern \"%s\"\n",lineCount,columnCount+1,yytext);
	columnCount+=yyleng;
	charCount+=yyleng;
}

<INITIAL>{
"/*"           BEGIN(IN_COMMENT);yymore();
}
<IN_COMMENT>{
"*/"           fprintf(fp,"ANNODATE(%d, %d): %s\n",lineCount,columnCount,yytext);BEGIN(INITIAL);
[^*\n]+	       yymore();
"*"            yymore();
\n             lineCount++;yymore();
}


{WARP}         {
	lineCount++;
	columnCount=0;
	}
.              {
	fprintf(fp,"UNKNOW(%d, %d): %s\n",lineCount,columnCount,yytext);
	charCount++;
	columnCount++;
	}

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