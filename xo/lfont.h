/* lfont.h							-*- C++ -*-
   $Id: lfont.h,v 1.3 1997/10/18 04:57:35 elf Exp $
   
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

   -----

   Font management.  We provide these routines in order to streamline
   font requests without requiring toolkit window classes from
   monitoring their font usage.

*/

#if !defined (__LFONT_H__)
#    define   __LFONT_H__

/* ----- Includes */

#include "lhash.h"

class LDisplay;

class LFontCache {
protected:
  LDisplay* m_pDisplay;
  LHashTable m_hashFontName;	// szFont->XFontStruct*
  LHashTable m_hashFontID;	// fontID->XFontStruct*

public:
  LFontCache () {
    zero (); }
  ~LFontCache () {
    release_this (); }
  void zero (void) {
    memset (this, 0, sizeof (*this)); }
  void init (LDisplay* pDisplay);
  void release_this (void);
  
  XFontStruct* find (char* szFont);
  XFontStruct* find (Font fid);
};



#endif  /* __LFONT_H__ */
