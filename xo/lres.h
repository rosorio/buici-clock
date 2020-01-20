/* lres.h							-*- C++ -*-
   $Id: lres.h,v 1.5 1998/10/21 21:13:57 elf Exp $
   
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

#if !defined (__LRES_H__)
#    define   __LRES_H__

#if defined (IS_PARSER)

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

#endif

class LResNode {
private:
  int m_type;
  LResNode* m_pNode;
  union {
    long l;
    float r;
    char* sz;
    LResNode* pNode;
  } m_v;

  static void* g_pvBase;	// Base allocation for resource nodes
  static void* g_pvAlloc;	// Source for allocation of new nodes
  static int* g_piResource;	// Offset from base to root node

  void _fixup (void);

public:
  
  static void*
    alloc (int cb, void *pv); // Allocate resource memory
  void emit (int index);
  LResNode* 
    enum_args (void);
  LResNode* 
    find_resources (void);
  static void
    fixup (LResNode* pNode);
  static void
    generate (LResNode* pNode);
  char* 
    keyword (void);
  bool
    is_invocation (const char* szKeyword, int id);
  static LResNode* 
    locate (const char* szInvocation, int id);
  LResNode* next (void) {
    return m_pNode; }

  bool integer (int* pi);
  bool real (float* pr);
  bool string (char** sz);

};

extern void* g_pvNodeAlloc;	/* Base allocation for all NODEs */

void* alloc_resource (int cb, void* pv);
int safe_atol (char* sz, signed long* pl);
int safe_atof (char* sz, float* pr);

bool as_bool (const XrmValue& value, bool fDefault);

extern FILE* yyin;

extern "C" int yylex (void);
int yyparse (void);

#endif  /* __LRES_H__ */
