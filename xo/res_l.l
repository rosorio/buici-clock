%{
/* res_l.l
     $Id: res_l.l,v 1.8 1997/10/18 04:57:41 elf Exp $

   This file is part of the project XO.  See the file README for
   more information.

   Copyright (C) 1997 Marc Singer

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   in a file called COPYING along with this program; if not, write to
   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA
   02139, USA.

   -----------
   DESCRIPTION
   -----------

   Lexical anaylzer for dialog language parser.

*/

#include "standard.h"
#define IS_PARSER
#include "lres.h"
#include "res_y.h"
#include <limits.h>

#if defined (__cplusplus)
extern "C" int yylex (void);
#else
int yylex (void);
#endif

int g_iLineParse = 1;

  /* We need to read from the buffered input stream because we do some
     magic with the first line that requires an ungetc() call. */
#define YY_INPUT(pv,result,cbMax) \
  if ((result = fread ((char*) pv, 1, cbMax, yyin)) < 0 ) \
    YY_FATAL_ERROR ("input in flex scanner failed");

#define DPRINTF(a)

#define YY_NO_UNPUT

%}

%s COMMENT

%%

	/* comments are ignored */
"/*"				{ BEGIN COMMENT; }
"*/"				{ BEGIN 0; }
<COMMENT>.*			;
<COMMENT>\n			{ ++g_iLineParse; }

	/* real number */
([+-]?[0-9]+\.[0-9]*) {
		  float r;
		  if (safe_atof (yytext, &r)) {
 /*		    yylval.er = erInteger; */
		    return ERROR;
		  }
		  yylval.r = r;
		  return REAL;
		}

	/* integer */
([+-]?[0-9]+)	{
		  long l;
		  if (safe_atol (yytext, &l)) {
 /*		    yylval.er = erInteger; */
		    return ERROR;
		  }
		  yylval.l = l;					
		  return INTEGER;
		}

	/* hex integer */
(0[xX][0-9a-fA-F]+) {
		  long l;
		  if (safe_atol (yytext, &l)) {
 /*			yylval.er = erInteger; */
	            return ERROR;
		  }
		  yylval.l = l;					
		  return INTEGER;
		}

	/* white space is ignored */
\n		{ ++g_iLineParse; }
[ \t\r\032]	;


	/* string surrounded by double quotes, \" to escape a " character */
\"([^\"\n]|(\\\"))*\" {
		  yytext[yyleng - 1] = 0;
		  yylval.sz = (char*) LResNode::alloc (yyleng - 1, yytext + 1);
		  return STRING;
		}

[a-zA-Z_]+	{ 
		  yylval.sz = (char*) LResNode::alloc (yyleng + 1, yytext);
		  return KEYWORD;
		}

"\""		{ return QUOTE; }
"{"		{ return BO; }
"}"		{ return BC; }
"("		{ return PO; }
")"		{ return PC; }

.		{ return yytext[0]; } 

%%
