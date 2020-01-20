%{
/* res_y.y
     $Id: res_y.y,v 1.7 1997/10/18 04:57:42 elf Exp $

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

   Grammar for dialog parser.

*/

#include "standard.h"
#define IS_PARSER
#include "lres.h"
#include <string.h>

#if defined (__cplusplus)
extern "C" int yylex (void);
#else
int yylex (void);
#endif

int yyerror (const char* sz);
extern FILE* yyout;

int g_cBraceOpen;		/* Count of open braces */
int g_cParenOpen;		/* Count of open parentheses */
int g_cConst;			/* Count of open constant expressions */

//#define DPRINTF(a)	printf a
#define DPRINTF(a)
//#define _in(p)	((char*) p - (char*) g_pvNodeAlloc)

%}

%token PO PC			/* Parenthesis Open and Close */
%token BO BC			/* Brace Open and Close */
%token QUOTE			/* Quote mark to inhibit lambda binding */

%union {
    long l;			/* Constant numeric value */
    float r;			/* Real number value */
    char* sz;			/* String literal */
    int er;			/* Error value */
    NODE* pNode;
}
%token <l> INTEGER 
%token <r> REAL 
%token <sz> STRING
%token <sz> KEYWORD
%token <er> ERROR
%token <pNode> RESOURCE
%token <pNode> INVOCATION
%token <pNode> ARGLIST

%type <pNode> keyword
%type <pNode> argument   argument_list   argument_list_d
%type <pNode> invocation invocation_list invocation_list_d
%type <pNode> resource_file

%%


resource_file:
	invocation_list
		{ 
		  LResNode::fixup ((LResNode*) $$);
		  LResNode::generate ((LResNode*) $$); 
		}
 ;

invocation_list:
	  invocation
		{
		  $$ = (NODE*) LResNode::alloc (sizeof (NODE), NULL);
		  DPRINTF (("RESOURCE 0x%x -> 0x%x\n", _in ($$), _in ($1)));
		  $$->type = RESOURCE;
		  $$->v.pNode = $1;
		}
	| invocation_list invocation
		{ 
		  DPRINTF (("RESOURCE+ 0x%x -> 0x%x\n", _in ($1), _in ($2)));
		  $2->pNode = $1->v.pNode;
		  $1->v.pNode = $2;
		  $$ = $1;
		}
 ;

invocation:
	  keyword
		{
		  $$ = (NODE*) LResNode::alloc (sizeof (NODE), NULL);
		  DPRINTF (("INVOCATION <%s> @0x%x\n", $1->v.sz,  _in ($$)));
		  $$->type = INVOCATION;
		  $$->v.pNode = $1;
		}
	| keyword argument_list_d
		{
		  $$ = (NODE*) LResNode::alloc (sizeof (NODE), NULL);
		  DPRINTF (("INVOCATION <%s> @0x%x a 0x%x\n",
			    $1->v.sz, _in ($$), _in ($2)));
		  $$->type = INVOCATION;
		  $$->v.pNode = $1;
		  $1->pNode = $2;
		}
	| keyword invocation_list_d
		{
		  $$ = (NODE*) LResNode::alloc (sizeof (NODE), NULL);
		  DPRINTF (("INVOCATION <%s> @0x%x i 0x%x\n",
			    $1->v.sz, _in ($$), _in ($2)));
		  $$->type = INVOCATION;
		  $$->v.pNode = $1;
		  $1->pNode = $2;
		}
	| keyword argument_list_d invocation_list_d
		{
		  $$ = (NODE*) LResNode::alloc (sizeof (NODE), NULL);
		  DPRINTF (("INVOCATION <%s> @0x%x a 0x%x i 0x%x\n", 
			    $1->v.sz, _in ($$), _in ($2), _in ($3)));
		  $$->type = INVOCATION;
		  $$->v.pNode = $1;
		  $1->pNode = $2;
		  $2->pNode = $3;
		}
 ;

invocation_list_d:
	  BO invocation_list BC				{ $$ = $2; }
	| BO		     BC				
		{
		  $$ = (NODE*) LResNode::alloc (sizeof (NODE), NULL);
		  DPRINTF (("invocation_list @0x%x\n", _in ($$)));
		  $$->type = INVOCATION;
		}
 ;	  

argument_list_d:
	  PO argument_list PC				{ $$ = $2; }
	| PO		   PC
		{
		  $$ = (NODE*) LResNode::alloc (sizeof (NODE), NULL);
		  DPRINTF (("argument_list @0x%x\n", _in ($$)));
		  $$->type = ARGLIST;
		}
 ;

argument_list:
	  argument
		{
		  $$ = (NODE*) LResNode::alloc (sizeof (NODE), NULL);
		  DPRINTF (("argument_list @0x%x => @0x%x\n", 
			    _in ($$), _in ($1)));
		  $$->type = ARGLIST;
		  $$->v.pNode = $1;
		}
	| argument_list ',' argument
		{ 
		  DPRINTF ((" 0x%x => 0x%x\n", _in ($1), _in ($3)));
		  $3->pNode = $1->v.pNode;
		  $1->v.pNode = $3;
		  $$ = $1;
		}
 ;

keyword:
	  KEYWORD
		{
		  $$ = (NODE*) LResNode::alloc (sizeof (NODE), NULL);
		  DPRINTF (("KEYWORD @0x%x\n", _in ($$)));
		  $$->type = KEYWORD;
		  $$->v.sz = $1;
		}
 ;

argument:
	  STRING
		{
		  $$ = (NODE*) LResNode::alloc (sizeof (NODE), NULL);
		  DPRINTF (("STRING @0x%x\n", _in ($$)));
		  $$->type = STRING;
		  $$->v.sz = $1;
		}
	| INTEGER
		{
		  $$ = (NODE*) LResNode::alloc (sizeof (NODE), NULL);
		  DPRINTF (("INTEGER @0x%x\n", _in ($$)));
		  $$->type = INTEGER;
		  $$->v.l = $1;
		}
	| REAL
		{
		  $$ = (NODE*) LResNode::alloc (sizeof (NODE), NULL);
		  DPRINTF (("REAL @0x%x\n", _in ($$)));
		  $$->type = REAL;
		  $$->v.r = $1;
		}
 ;

%%

int yyerror (const char* sz)
{
  extern int g_iLineParse;
  extern char *yytext;
/*    printf ("%d: %s at '%s'\n", yylineno, sz, yytext); */
    printf("error: line %d, %s at '%s'\n", g_iLineParse, sz, yytext);
//    printf ("yyerror: %s\n", sz);
    return 0;
}

