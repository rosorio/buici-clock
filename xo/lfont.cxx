/* lfont.cxx
     $Id: lfont.cxx,v 1.5 1997/10/18 04:57:35 elf Exp $

   written by Marc Singer
   10 May 1997

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

#include "standard.h"
#include "ldisplay.h"
#include "lfont.h"

void LFontCache::init (LDisplay* pDisplay)
{
  release_this ();		// Just in case we are doing something stupid
  m_pDisplay = pDisplay;
  m_hashFontName.init (sizeof (XFontStruct*), 101);
  m_hashFontID.init   (sizeof (XFontStruct*), 101);
}


XFontStruct* LFontCache::find (char* szFont)
{
  HashKey key = LHashTable::make_case_string_key (szFont);
  XFontStruct* pFont = NULL;
  m_hashFontName.find (key, &pFont);
  if (pFont)
    return pFont;
  pFont = XLoadQueryFont (m_pDisplay->display (), szFont);
  if (pFont) {
    m_hashFontName.add (key, &pFont);
    m_hashFontID.add (HashKey (pFont->fid), &pFont);
  }
  return pFont;
}

XFontStruct* LFontCache::find (Font fid)
{
  XFontStruct* pFont = NULL;
  m_hashFontID.find (HashKey (fid), &pFont);
  return pFont;
}


void LFontCache::release_this (void)
{
  if (!m_pDisplay)
    return;
  HashKey key;
  XFontStruct** ppFont;
  for (unsigned32 index = 0;
       index = m_hashFontName.enumerate_reverse (index, &key, 
					     (const void**) &ppFont); ) {
    m_hashFontID.remove (HashKey ((*ppFont)->fid));
    XUnloadFont (m_pDisplay->display (), (*ppFont)->fid);
    XFreeFontInfo (NULL, *ppFont, 1);
    m_hashFontName.remove (key);
  }
}
