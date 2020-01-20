/* dmalloc.cxx
     $Id: dmalloc.cxx,v 1.7 1998/10/15 07:12:41 elf Exp $

   written by Marc Singer
   27 May 1997

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


#define BY_DMALLOC
#include "standard.h"
#include "dmalloc.h"

typedef struct _WRAPPER {
  unsigned32 marker;		// Marker for start of block
  _WRAPPER* pPrev;
  _WRAPPER* pNext;
  unsigned long cb;		// User's size of block
  char szModule[16];		// Word aligned name of allocating source
} WRAPPER;

#define CB_WRAPPER  (sizeof (WRAPPER) + sizeof (unsigned32))
//#define	MARKER_HEAD (unsigned32) 0xefacdaed
#define	MARKER_HEAD ((unsigned32) 0xfecade2b)
#define	MARKER_TAIL ((unsigned32) 0xd1cefaef)

#define B_WRAP	'+'
#define B_FREE	'-'

WRAPPER* g_pWrapperHead;		// Head of list of wrapped blocks


void _tell_ (WRAPPER* pWrapper);
bool _validate_one_ (WRAPPER* pWrapper);
bool _validate_ (WRAPPER* pWrapper);
void* _wrap_ (void* pv, int cb, const char* szModule, int iLine, 
	      bool fFill, WRAPPER** ppWrapLink);



/* ::operator new & delete

   replace global new and delete operators.

*/

void* operator new (size_t cb)
{
  return _malloc_ (cb, "unknown", 0);
}

void* operator new (size_t cb, const char* szModule, int iLine)
{
  return _malloc_ (cb, szModule, iLine);
}

void operator delete (void* pv)
{
  _free_ (pv);
}


void dmalloc_exit (void)
{
  for (WRAPPER* pWrapper = g_pWrapperHead; pWrapper;
       pWrapper = pWrapper->pNext)
    _tell_ (pWrapper);
}



void dmalloc_validate (void)
{
  if (!_validate_ (g_pWrapperHead))
    fprintf (stderr, "heap invalid on dmalloc_validate\n");
}



/* _free_ 

*/

void _free_ (void* pv)
{
  if (!pv)
    return;

				// -- Locate
  WRAPPER* pWrapper = (WRAPPER*) ((unsigned8*) pv - sizeof (WRAPPER));
  if (!_validate_one_ (pWrapper)) {
    _tell_ (pWrapper);
    fprintf (stderr, "heap invalid on free\n");
    return;
  }
				// -- Unlink
    if (pWrapper->pPrev)
    pWrapper->pPrev->pNext = pWrapper->pNext;
  if (pWrapper->pNext)
    pWrapper->pNext->pPrev = pWrapper->pPrev;
  if (g_pWrapperHead == pWrapper)
    g_pWrapperHead = pWrapper->pNext;

				// -- Release
  free (pWrapper);
}


/* _calloc_

   allocate memory and wrap it with tracing information.

*/

void* _calloc_ (size_t c, size_t cb, const char* szModule, int iLine)  
{
  if (!_validate_ (g_pWrapperHead)) {
    fprintf (stderr, "heap invalid on malloc\n");
    return NULL;
  }

  cb *= c;
  void* pv = malloc (cb + CB_WRAPPER);
  if (pv && CB_WRAPPER)
    pv = _wrap_ (pv, cb, szModule, iLine, false, &g_pWrapperHead);

  //  ASSERT_FAIL (pv);

  return pv;
}


/* _malloc_

   allocate memory and wrap it with tracing information.

*/

void* _malloc_ (size_t cb, const char* szModule, int iLine)  
{
  if (!_validate_ (g_pWrapperHead)) {
    fprintf (stderr, "heap invalid on malloc\n");
    return NULL;
  }

  void* pv = malloc (cb + CB_WRAPPER);
  if (pv && CB_WRAPPER)
    pv = _wrap_ (pv, cb, szModule, iLine, true, &g_pWrapperHead);

  //  ASSERT_FAIL (pv);

  return pv;
}


/* _realloc_

   re-allocate memory and wrap it with tracing information.

*/

void* _realloc_ (void* pvOld, size_t cb, const char* szModule, int iLine)  
{
  if (!_validate_ (g_pWrapperHead)) {
    fprintf (stderr, "heap invalid on malloc\n");
    return NULL;
  }

  void* pv = _malloc_ (cb + CB_WRAPPER, szModule, iLine);
  if (pv) {
    unsigned int cbCopy = ((WRAPPER*) pvOld)->cb;
    if (cbCopy > cb)
      cbCopy = cb;
    memcpy ((unsigned8*) pv + sizeof (WRAPPER), 
	    (unsigned8*) pvOld + sizeof (WRAPPER), cbCopy);
    _free_ (pvOld);
  }
  if (pv && CB_WRAPPER)
    pv = _wrap_ (pv, cb, szModule, iLine, true, &g_pWrapperHead);

  //  ASSERT_FAIL (pv);

  return pv;
}


void* _wrap_ (void* pv, int cb, const char* szModule, int iLine, 
	      bool fFill, WRAPPER** ppWrapLink)
{
				// -- Fill
  if (fFill)
    memset ((unsigned8*) pv + sizeof (WRAPPER), B_WRAP, cb);
  else
    memset ((unsigned8*) pv + sizeof (WRAPPER), 0, cb);	// calloc requires this

				// -- Wrap
  WRAPPER* pWrapper = (WRAPPER*) pv;
  pWrapper->marker = MARKER_HEAD;
  pWrapper->cb = cb;		// User's requested size
  memset (pWrapper->szModule, 0, sizeof (pWrapper->szModule));
  sprintf (pWrapper->szModule, "%.10s.%.4d", szModule, iLine);
  *(unsigned32*) ((unsigned char*) pv + sizeof (WRAPPER) + cb) = MARKER_TAIL;

				// -- Link
  pWrapper->pPrev = NULL;
  pWrapper->pNext = *ppWrapLink;
  if (*ppWrapLink)
    (*ppWrapLink)->pPrev = pWrapper;
  *ppWrapLink = pWrapper;
  return (void*)(pWrapper + 1);
}


void _tell_ (WRAPPER* pWrapper)
{
  fprintf (stderr, "block 0x%lx %ld (0x%lx) bytes for %.*s",
	   (unsigned long) pWrapper + 1, pWrapper->cb, pWrapper->cb, 
	   (int) sizeof (pWrapper->szModule), pWrapper->szModule);
  if (pWrapper->marker != MARKER_HEAD)
    fprintf (stderr, " [HEAD]");
  if (*(unsigned32*)((char*) (pWrapper + 1) + pWrapper->cb) != MARKER_TAIL)
    fprintf (stderr, " [TAIL]");
  fprintf (stderr, "\n");
}


bool _validate_one_ (WRAPPER* pWrapper)
{
  return (pWrapper->marker == MARKER_HEAD 
	  && *(unsigned32*)(((unsigned char*) pWrapper)
			   + sizeof (WRAPPER) + pWrapper->cb) == MARKER_TAIL);
}

bool _validate_ (WRAPPER* pWrapper) 
{
  for (; pWrapper; pWrapper = pWrapper->pNext) {
    if (!_validate_one_ (pWrapper)) {
      _tell_ (pWrapper);
      fprintf (stderr, "heap invalid\n");
      return false;
    }
  }
  return true;
}
