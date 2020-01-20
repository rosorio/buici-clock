/* res.cxx
     $Id: res.cxx,v 1.16 1998/10/21 21:13:57 elf Exp $

   written by Marc Singer
   14 May 1997

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

//#define USE_TEST

#include "standard.h"
#include "ctype.h"
#if defined (HAVE_FCNTL_H)
# include "fcntl.h"
#endif
#if defined (HAVE_MEMORY_H)
# include "memory.h"
#endif

#define IS_PARSER
#include "lres.h"
#include "res_y.h"

#define LONG_PRIME_MAX		(LONG_MAX/10)
#define LONG_PRIME_MAX_REM	(LONG_MAX - LONG_PRIME_MAX*10)
//#define TEST_LEXER

#define CB_NODES		(0x10000)

#define DPRINTF(a)	printf a

void* LResNode::g_pvBase;
void* LResNode::g_pvAlloc;
int*  LResNode::g_piResource;

void* LResNode::alloc (int cb, void* pv)
{
  if (!g_pvBase) {
    g_pvBase = g_pvAlloc = malloc (CB_NODES);
    if (!g_pvBase)
      return NULL;
    g_piResource = (int*) alloc (sizeof (int), NULL);
  }

  void* pvNew = g_pvAlloc;
  if (pv)
    memcpy (pvNew, pv, cb);
  else
    memset (pvNew, 0, cb);
  cb = ((cb + 3) & ~3);

  g_pvAlloc = (unsigned8*) g_pvAlloc + cb;
  return pvNew;
}


/* safe_atol

   converts a string to a long in a safe manner, checking for errors.

*/

int safe_atol (char* sz, signed long* pl)
{
  if (sz[0] == '0' && tolower (sz[1]) == 'x') {
    unsigned long l = 0;
    if (strlen (sz) > 10)
      return 1;			// Overflow on unsigned
    for (sz += 2; *sz; ++sz) {
      switch (tolower (*sz)) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
	l = (l << 4) + (*sz - '0');
	break;
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
	l = (l << 4) + (tolower (*sz) - 'a' + 10);
	break;
      default:
//	assert (false);
	break;
      }  /* switch */
    }  /* for */
    *pl = (long) l;
  }  /* if */
  else {
    register long l = 0;
    int fSigned = 0;

    for (; *sz; ++sz) {
      switch (*sz) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
	/*      DPRINTF (("%d %d %d %d\n", l, *sz - '0', LONG_PRIME_MAX,
		LONG_PRIME_MAX_REM)); */
	if (       l         <  LONG_PRIME_MAX
	    || (   l         == LONG_PRIME_MAX
		&& *sz - '0' <  LONG_PRIME_MAX_REM))
	  l = l*10 + *sz - '0';
	else
	  return 1;		/* Overflow on unsigned number */
	break;

      case '-':
	fSigned = 1;
      case '+':
	break;
      }  /* switch */
    }  /* while */
    if (fSigned && l > LONG_MAX - 1)
      return 2;			/* Overflow on signed number */
    *pl = fSigned ? -l : l;
  }  /* else */
  return 0;
}  /* safe_atol */


int safe_atof (char* sz, float* pr)
{
  *pr = atof (sz);
  return 0;
}


bool LResNode::is_invocation (const char* szKeyword, int id)
{
  return (   m_type						== INVOCATION
	  && m_v.pNode
	  && m_v.pNode->m_type					== KEYWORD
	  && m_v.pNode->m_v.sz
	  && !strcasecmp (m_v.pNode->m_v.sz, szKeyword)
	  && m_v.pNode->m_pNode
	  && m_v.pNode->m_pNode->m_type				== ARGLIST
	  && m_v.pNode->m_pNode->m_v.pNode
	  && m_v.pNode->m_pNode->m_v.pNode->m_type		== INTEGER
	  && m_v.pNode->m_pNode->m_v.pNode->m_v.l		== id
	  );
}


bool LResNode::integer (int* pi)
{
  if (m_type != INTEGER)
    return false;
  *pi = m_v.l;
  return true;
}

bool LResNode::real (float* pr)
{
  if (m_type != REAL)
    return false;
  *pr = m_v.r;
  return true;
}

bool LResNode::string (char** psz)
{
  if (m_type != STRING)
    return false;
  *psz = m_v.sz;
  return true;
}


/* find_resources

   returns a pointer to the first invocation contained by this
   invocation.  This function recurses into the current invocation.

*/

LResNode* LResNode::find_resources (void)
{
  if (m_type != INVOCATION)
    return NULL;
  for (LResNode* pNode = m_v.pNode; pNode; pNode = pNode->m_pNode)
    if (pNode->m_type == RESOURCE)
      return pNode->m_v.pNode;
  return NULL;
}

/* LResNode::keyword

   returns a pointer to the keyword string for the invocation.  The
   return value is NULL if the node is not an invocation.

*/

char* LResNode::keyword (void)
{
  if (    m_type != INVOCATION
      || !m_v.pNode
      ||  m_v.pNode->m_type != KEYWORD)
    return NULL;
  return m_v.pNode->m_v.sz;
}


/* LResNode::locate

   locates a typed resource with the given ID.  It is required that
   all resources have the ID as the first argument if they are found
   with this primitive.

*/

LResNode* LResNode::locate (const char* szInvocation, int id)
{
  LResNode* pNode = (LResNode*) ((unsigned8*) g_pvBase + *g_piResource);
  if (pNode->m_v.pNode)
    pNode = pNode->m_v.pNode;

  for (; pNode; pNode = pNode->m_pNode) {
    if (pNode->is_invocation (szInvocation, id))
      return pNode;
  }

  return NULL;
}


/* LResNode::enum_args

   enumerates the argument vector for an invocation.  On the first
   call, pass the pointer to the invocation.  For successive calls,
   pass the pointer returned by the function.

*/


LResNode* LResNode::enum_args (void)
{
  if (!this)
    return NULL;
				// Initial case
  if (m_type == INVOCATION) {
    for (LResNode* pNode = m_v.pNode; pNode; pNode = pNode->m_pNode)
      if (pNode->m_type == ARGLIST)
	return pNode->m_v.pNode;
    return NULL;
  }

				// Iterative case
  return m_pNode;
}


void LResNode::emit (int index)
{
/*   DPRINTF (("[emit 0x%x]", (char*) pNode - (char*) g_pvNodeAlloc)); */

  switch (m_type) {
  case ARGLIST:
    DPRINTF ((" ("));
    for (LResNode* pNode = m_v.pNode; pNode; pNode = pNode->m_pNode)
      pNode->emit (index + 1);
    DPRINTF ((" )"));
    break;
  case INVOCATION:
    DPRINTF (("%*.*s", index, index, ""));
    for (LResNode* pNode = m_v.pNode; pNode; pNode = pNode->m_pNode)
      pNode->emit (index + 1);
    DPRINTF (("\n"));
    break;
  case KEYWORD:
    DPRINTF (("%s", m_v.sz));
    break;
  case INTEGER:
    DPRINTF ((" %ld", m_v.l));
    break;
  case REAL:
    DPRINTF ((" %g", m_v.r));
    break;
  case STRING:
    DPRINTF ((" \"%s\"", m_v.sz));
    break;
  case RESOURCE:
    if (index > 0)
      DPRINTF ((" {\n"));
    for (LResNode* pNode = m_v.pNode; pNode; pNode = pNode->m_pNode) {
      pNode->emit (index + 1);
/*      DPRINTF (("[ in resource 0x%x=>0x%x]",
	      (char*) pNode - (char*) g_pvNodeAlloc,
	      (char*) pNode->m_pNode - (char*) g_pvNodeAlloc));
*/
    }
    if (index > 0)
      DPRINTF (("%*.*s}", index - 1, index - 1, ""));
    break;
  }
}

void LResNode::generate (LResNode* pNode)
{
  *g_piResource = (unsigned8*) pNode - (unsigned8*) g_pvBase;

				// Emit all of the resources
  //  for (; pNode; pNode = pNode->m_pNode)
  //    emit (pNode, -1);

  DPRINTF (("resource templates require %d bytes\n", 
	    int ((unsigned8*) g_pvAlloc - (unsigned8*) g_pvBase)));

  {
    int fh = open ("resources", O_WRONLY | O_CREAT | O_TRUNC, 0660);
    write (fh, g_pvBase, (unsigned8*) g_pvAlloc - (unsigned8*) g_pvBase);
    close (fh);
  }

  //  pNode = locate ("menu", 1);
  //  pNode->emit (0);
}


/* _fixup

   reverses the order of the argument and resource lists.  These lists
   are collected in reverse order because of the way that the parser
   reduces the grammer.  Yes, we could fix it, but the overhead of
   reordering is low.

   Note that the invocation itself is not reordered, but it must be
   enumerated in order to reorder the argument list, the possible
   resource list. 

*/

void LResNode::_fixup (void)
{
  switch (m_type) {
  default:
    break;
  case ARGLIST:
  case RESOURCE:
    {
      LResNode* pNodeNext = NULL;
      LResNode* pNodeLink;
      for (LResNode* pNodeStep = m_v.pNode; pNodeStep; 
	   pNodeStep = pNodeLink) {
	pNodeLink = pNodeStep->m_pNode;
	pNodeStep->m_pNode = pNodeNext;
	pNodeNext = pNodeStep;
      }	
      m_v.pNode = pNodeNext;
    }
  case INVOCATION:
    for (LResNode* pNode = m_v.pNode; pNode; pNode = pNode->m_pNode)
      pNode->_fixup ();
    break;
  }
}


void LResNode::fixup (LResNode* pNode) 
{
  for (; pNode; pNode = pNode->m_pNode)
    pNode->_fixup ();
}

/* as_bool

   returns a boolean value interpretation for a X Resource Manager
   value.  This is a convenience feature to make the specification of
   booleans simple.  The default is returned if the value is not
   valid.

*/

bool as_bool (const XrmValue& value, bool fDefault)
{
  if (!value.addr || !value.size)
    return fDefault;
  
  if (   *value.addr == 't' || *value.addr == 'T'
      || *value.addr == 'y' || *value.addr == 'Y' 
      || *value.addr == '1')
    return true;
  if (   *value.addr == 'f' || *value.addr == 'F'
      || *value.addr == 'n' || *value.addr == 'N' 
      || *value.addr == '0')
    return false;
  return fDefault;
}


extern "C" int yywrap (void)
{
  return 1;
}  /* yywrap */


#if defined (USE_TEST)

int main (int /* argc */, char** /* argv */)
{
  yyin = stdin;

#if defined (TEST_LEXER)
  int result;
  while (result = yylex ()) {
    switch (result) {
    case KEYWORD:
      DPRINTF ((" <%s>", yylval.sz));
      break;
    case PO:
      DPRINTF ((" PO"));
      break;
    case PC:
      DPRINTF ((" PC\n"));
      break;
    case BO:
      DPRINTF ((" BO\n"));
      break;
    case BC:
      DPRINTF ((" BC\n"));
      break;
    case QUOTE:
      DPRINTF ((" QUOTE"));
      break;
    case INTEGER:
      DPRINTF ((" INTEGER %d", yylval.l));
      break;
    case REAL:
      DPRINTF ((" REAL"));
      break;
    case STRING:
      DPRINTF ((" STRING '%s'", yylval.sz));
      break;
    case ERROR:
      DPRINTF ((" ERROR\n"));
      break;
    default:
      if (isprint (result))
	DPRINTF ((" '%c'", result));
      else
	DPRINTF ((" [%d]", result));
      break;
    }
  }
#else

  yyparse ();
#endif

#if 0
  LResNode* pNodeMenu = LResNode::locate ("dialog", 1);
  for (LResNode* pNode = pNodeMenu->find_resources (); pNode; 
       pNode = pNode->next ()) {
    pNode->emit (1);
    for (LResNode* pNodeArg = pNode; pNodeArg = pNodeArg->enum_args (); ) {
      pNodeArg->emit (3);
    }
  }
#endif

}

#endif
