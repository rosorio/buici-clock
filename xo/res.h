/* res.h
   $Id: res.h,v 1.3 1997/10/18 04:57:41 elf Exp $
   
   written by Marc Singer
   15 May 1997

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

*/

#if !defined (__RES_H__)
#    define   __RES_H__

typedef struct _NODE {
  int type;
  struct _NODE* pNode;
  union {
    long l;
    float r;
    char* sz;
    struct _NODE* pNode;
  } v;
} NODE;

extern void* g_pvNodeAlloc;	/* Base allocation for all NODEs */

void* alloc_resource (int cb, void* pv);
int safe_atol (char* sz, signed long* pl);
int safe_atof (char* sz, float* pr);

extern FILE* yyin;

extern "C" int yylex (void);
int yyparse (void);

NODE* find_dialog (void);

#endif  /* __RES_H__ */
