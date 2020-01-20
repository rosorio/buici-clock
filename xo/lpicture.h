/* lpicture.h							-*- C++ -*-
   $Id: lpicture.h,v 1.2 1997/10/18 04:57:37 elf Exp $
   
   written by Marc Singer
   3 Jun 1997

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

   Helper structures and procedures for working with pictures.  The
   primary motivation for this stuff is to allow us to handle varying
   formats without putting them into a windowing system specific data
   structure. 

*/

#if !defined (__LPICTURE_H__)
#    define   __LPICTURE_H__

/* ----- Includes */

#include "memory.h"


/* ----- Class */

typedef struct {
  unsigned8 rgbBlue;
  unsigned8 rgbGreen;
  unsigned8 rgbRed;
  unsigned8 rgbReserved;
} RGBQUAD;			// MS Windows palette structure

class LPicture {
protected:
  void _calc_stride (void) {
    m_cbStride = (((m_width*m_depth)/8 + 3)& ~0x3); }

public:
  int m_width;
  int m_height;
  int m_depth;			// Depth of image, probably 1, 8, 16, or 24
  bool m_fNonNormal;		// Normal is top-down scanline order
  int m_cColors;		// Number of colors in the palette, 0 for none
  int m_cbStride;		// Width of one scanline
  
  void* m_pvMap;		// Location where file is mapped into memory
  int m_cbMap;			// Length of mapping
  int m_fh;			// File handle used in mapping

  void* m_pv;			// Pointer to the bits
  int m_cb;			// Size of region holding the bitmap

  RGBQUAD* m_rgRGB;		// Pointer to palette, if there is one

  bool read (const char* szPath);

  LPicture () {
    zero (); }
  ~LPicture () {
    release_this (); }
  void release_mmap (void);
  void release_this (void);
  void zero (void) {
    memset (this, 0, sizeof (*this)); }
    
};




#endif  /* __LPICTURE_H__ */
