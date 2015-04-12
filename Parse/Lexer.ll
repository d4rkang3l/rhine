/* -*- Bison -*- */
%{
#include "rhine/ParseDriver.h"
#include "rhine/Lexer.h"

#define YY_USER_ACTION yylloc->columns(yyleng);
%}

%option c++ noyywrap warn yylineno stack

SYMBOL  [[:alpha:]][[:alnum:]]+
EXP     [Ee][- +]?[[:digit:]]+
INTEGER [- +]?[[:digit:]]+
RET     [\r\n]+
SPTAB   [ \t]+

%x str

%%

%{
  yylloc->step();
  char string_buf[500];
  char *string_buf_ptr;
%}


{SPTAB} { yylloc->step(); }

{RET} { yylloc->lines(yyleng); yylloc->step(); }

{INTEGER} {
  auto C = ConstantInt::get(atoi(yytext));
  yylval->Integer = C;
  return T::INTEGER;
}

\"	string_buf_ptr = string_buf; BEGIN(str);

<str>\" {
  BEGIN(INITIAL);
  *string_buf_ptr = '\0';
  auto C = GlobalString::get(std::string(string_buf));
  yylval->String = C;
  return T::STRING;
}

<str>\\n  *string_buf_ptr++ = '\n';
<str>\\t  *string_buf_ptr++ = '\t';
<str>\\r  *string_buf_ptr++ = '\r';
<str>\\b  *string_buf_ptr++ = '\b';
<str>\\f  *string_buf_ptr++ = '\f';

<str>\\(.|\n)	*string_buf_ptr++ = yytext[1];

<str>[^\\\n\"]+	{
  char *yptr = yytext;
  while (*yptr)
    *string_buf_ptr++ = *yptr++;
}

"defun" { return T::DEFUN; }
"if" { return T::IF; }
"then" { return T::THEN; }
"&&" { return T::AND; }
"||" { return T::OR; }

"Int" { return T::TINT; }
"Bool" { return T::TBOOL; }
"String" { return T::TSTRING; }

"true" {
  auto B = ConstantBool::get(true);
  yylval->Boolean = B;
  return T::BOOLEAN;
}

"false" {
  auto B = ConstantBool::get(false);
  yylval->Boolean = B;
  return T::BOOLEAN;
}

{SYMBOL} {
  yylval->RawSymbol = new std::string(yytext);
  return T::SYMBOL;
}

[\[ \] \( \) + * ; { } $ ~] {
  return static_cast<P::token_type>(*yytext);
}

. ;

%%

// Required to fill vtable
int yyFlexLexer::yylex()
{
  return 0;
}
