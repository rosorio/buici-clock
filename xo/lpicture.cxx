/* lpicture.cxx
     $Id: lpicture.cxx,v 1.4 1998/10/15 04:11:51 elf Exp $

   written by Marc Singer
   3 June 1997

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
#include <memory.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "lpicture.h"


typedef struct {
//  INT16  bfType; //'B' 'M'
  unsigned32 bfSize;
  int16  bfReserved1;
  int16  bfReserved2;
  unsigned32 bfOffBits;
} HDR_DIBFILE;

typedef struct{
  unsigned32 biSize;
  unsigned32 biWidth;
  unsigned32 biHeight;
  unsigned16  biPlanes;
  unsigned16  biBitCount;
  unsigned32 biCompression;
  unsigned32 biSizeImage;
  unsigned32 biXPelsPerMeter;
  unsigned32 biYPelsPerMeter;
  unsigned32 biClrUsed;
  unsigned32 biClrImportant;
} HDR_DIB;


bool LPicture::read (const char* szPath)
{
  m_fh = open (szPath, O_RDONLY);
  if (m_fh < 0) {
    m_fh = 0;
    return false;
  }

  struct stat stat;
  fstat (m_fh, &stat);

  if (!(m_pvMap = mmap (NULL, m_cbMap = stat.st_size, 
			PROT_READ, MAP_SHARED, m_fh, 0))
      || ((char*)m_pvMap)[0] != 'B'
      || ((char*)m_pvMap)[1] != 'M') {
    release_mmap ();
    return false;
  }


  HDR_DIBFILE& hdrFile = *(HDR_DIBFILE*)((unsigned8*) m_pvMap + 2);
  HDR_DIB&     hdr     = *(HDR_DIB*)((unsigned8*) m_pvMap
				     + 2 + sizeof (HDR_DIBFILE));
    
  m_width = hdr.biWidth;
  m_height = hdr.biHeight;
  m_depth = hdr.biPlanes*hdr.biBitCount;
  _calc_stride ();
  m_fNonNormal = true;
  m_cColors = (hdrFile.bfOffBits - sizeof (hdrFile) - sizeof (hdr))
    /sizeof (RGBQUAD);

  if (m_cColors)
    m_rgRGB = (RGBQUAD*) (&hdr + 1);
  m_pv = (unsigned8*) m_pvMap + hdrFile.bfOffBits;
  m_cb = m_cbStride*m_height;

				// This is catastrophic, but we cannot
				// do much about it unless we handle
				// reading very specially. 
				// ** FIXME??
  if ((unsigned8*) m_pv + m_cb > (unsigned8*) m_pvMap + m_cbMap) {
    release_mmap ();
    return false;
  }

  return true;
}


void LPicture::release_mmap (void)
{
  if (!m_pvMap)
    return;

  if (m_rgRGB >= m_pvMap
      && (unsigned8*) m_rgRGB < (unsigned8*) m_pvMap + m_cbMap)
    m_rgRGB = NULL;
  if (m_pv >= m_pvMap && m_pv < (unsigned8*) m_pvMap + m_cbMap)
    m_pv = NULL;

  if (m_pvMap)
    munmap ((caddr_t) m_pvMap, m_cbMap);
  m_pvMap = NULL;
  if (m_fh) {
    close (m_fh);
    m_fh = 0;
  }
}


void LPicture::release_this (void)
{
  release_mmap ();
  if (m_pv) {
    free (m_pv);
    m_pv = NULL;
    m_cb = 0;
  }
  if (m_rgRGB) {
    free (m_rgRGB);
    m_rgRGB = 0;
    m_cColors = 0;
  }
}

