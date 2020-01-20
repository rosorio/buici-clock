/* dither.cxx
     $Id: dither.cxx,v 1.3 1997/10/18 04:57:32 elf Exp $

   written by Marc Singer
   22 March 1997

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

   Creates an 8bpp bitmap from the given 24bpp image using a fast
   ordered dither.

*/

#include "standard.h"
#include "dither.h"

//#include <math.h>
#include <assert.h>
#include <stdlib.h>


typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef short UINT;
typedef unsigned char BYTE;

typedef struct {
  //  UINT  bfType; //'B' 'M'
  DWORD bfSize;
  UINT  bfReserved1;
  UINT  bfReserved2;
  DWORD bfOffBits;
} HDR_DIBFILE;

typedef struct{
  DWORD biSize;
  DWORD biWidth;
  DWORD biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  DWORD biXPelsPerMeter;
  DWORD biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} HDR_DIB;

typedef struct {
  unsigned8 rgbBlue;
  unsigned8 rgbGreen;
  unsigned8 rgbRed;
  unsigned8 rgbReserved;
} RGBQUAD;

#define ROWWIDTH(width,depth) ((((width)*(depth))/8 + 3)& ~0x3)
#define _rgb(r,g,b) (b),(g),(r)
#define _error(peThis,pePrev,i,c)    \
   ((((peThis[((i) - 1)*3 + (c)])*7) \
   + ((pePrev[((i) - 1)*3 + (c)])*3) \
   + ((pePrev[((i)    )*3 + (c)])*5))/16)


/* LDither::create_matrix

   creates the dithering matrix for the specified dimension.  The
   values are scaled by C_SHADES_MAX.

*/

void LDither::create_matrix (int dim)
{
  if (m_dim == dim && m_pMatrix)
    return;

  if (m_pMatrix) {
    free (m_pMatrix);
    m_pMatrix = NULL;
  }

  int x, y, *dat;

  assert (dim == 4);		// If this fails, we need to eliminate
				// some of the performance code in
				// dither.h

  m_dim = (1 << dim);
  m_dim2 = m_dim*m_dim;

				// Precompute some inner-loop values
  m_level_red   = levels (m_cRed);
  m_level_green = levels (m_cGreen);
  m_level_blue  = levels (m_cBlue);
  m_dither_divisor = m_dim2*C_SHADES_MAX;

  m_pMatrix = (int **) malloc ((m_dim *sizeof(int*)) +		// pointers
			       (m_dim2*sizeof(int)));	// data

  assert (m_pMatrix);

  dat = (int*) &m_pMatrix[m_dim];
  for (y = 0; y < m_dim; y++)
    m_pMatrix[y] = &dat[y*m_dim];

  for (y = 0; y < m_dim; y++)
    for (x = 0; x < m_dim; x++)
      m_pMatrix[y][x] = C_SHADES_MAX*matrix_value (y, x, dim);
}


/* LDither::dither

   dithers cy rows of cx pixels.  The source pixels are in Windows 24
   bit DIB format.  The output is Windows 8 bit DIB format.

*/

void LDither::dither (int xOrg, int yOrg, int cx, int cy,
		      unsigned8* pbSrc, unsigned8* pbDst)
{
  int y, mask = (m_dim - 1);
  register int x, d;
  register int *pMatrix;
  int cbRowSrcExtra = ROWWIDTH (cx, 24) - cx*3;
  int cbRowDstExtra = ROWWIDTH (cx, 8)  - cx;

  for (y = 0; y < cy; ++y, pbSrc += cbRowSrcExtra, pbDst += cbRowDstExtra)
    for (pMatrix = m_pMatrix[(y + yOrg) & mask], x = cx; x--; pbSrc += 3) {
      d = pMatrix[(x + xOrg) & mask];
#if 0
      *pbDst++ = color_index (dither (pbSrc[2], d, m_cRed),
			      dither (pbSrc[1], d, m_cGreen),
			      dither (pbSrc[0], d, m_cBlue));
#else
      *pbDst++ = color_index (dither_red   (pbSrc[2], d),
			      dither_green (pbSrc[1], d),
			      dither_blue  (pbSrc[0], d));
#endif
    }
}


/* LDither::init

   initialize the dithering control structures.  This includes the
   scalar parameters, the lookup table and the dithering matrix.

*/

void LDither::init (int dim, int cRed, int cGreen, int cBlue)
{
  if (m_cRed == cRed && m_cGreen == cGreen && m_cBlue == cBlue && m_rgRGB)
    return;

  m_cRed        = cRed;
  m_cGreen      = cGreen;
  m_cBlue       = cBlue;
  m_cColors     = cRed*cGreen*cBlue;

  if (m_cColors > C_COLOR_MAX) {
    fprintf (stderr, "too many shades %d, max %d\n", m_cColors, C_COLOR_MAX);
    return;
  }
  if (m_cRed < 2) {
    fprintf (stderr, "too few shades for red, minimum of 2\n");
    return;
  }
  if (m_cGreen < 2)
    fprintf (stderr, "too few shades for green, minimum of 2\n");
  if (m_cBlue < 2)
    fprintf (stderr, "too few shades for blue, minimum of 2\n");

  register int r, g, b, i;

  if (m_rgRGB) {
    free (m_rgRGB);
    m_rgRGB = NULL;
  }

  m_rgRGB = (RGBQUAD*) malloc (m_cColors*sizeof (RGBQUAD));
  memset (m_rgRGB, 0, m_cColors*sizeof (RGBQUAD));

  RGBQUAD* rgRGB = (RGBQUAD*) m_rgRGB;

  for (r = 0; r < m_cRed; r++)
    for (g = 0; g < m_cGreen; g++)
      for (b = 0; b < m_cBlue; b++) {
	i = color_index (r,g,b);
	rgRGB[i].rgbRed   = (r*(C_COLOR_MAX-1)/(m_cRed   - 1));
	rgRGB[i].rgbGreen = (g*(C_COLOR_MAX-1)/(m_cGreen - 1));
	rgRGB[i].rgbBlue  = (b*(C_COLOR_MAX-1)/(m_cBlue  - 1));
      }

  create_matrix (dim);
}


/* LDither::matrix_value

   returns the value for a dithering matrix of square matrix, size
   high and wide, at the point x, y.

   From netpbm-mar-1994 which got it from Graphics Gems, p. 714.

*/

int LDither::matrix_value (int y, int x, int size)
{
  assert (size > 0);

  register int d;

  /* Think of d as the density. At every iteration, d is shifted left
   * one and a new bit is put in the low bit based on x and y.  If x
   * is odd and y is even, or visa versa, then a bit is shifted in.
   * This generates the checkerboard pattern seen in dithering.  This
   * quantity is shifted again and the low bit of y is added in.  This
   * whole thing interleaves a checkerboard pattern and y's bits which
   * is what you want.  */

  for (d = 0; size--; x >>= 1, y >>= 1)
    d = (d << 2) | (((x & 1) ^ (y & 1)) << 1) | (y & 1);
  return d;
}

void LDither::release_this (void)
{
  if (m_pMatrix) {
    free (m_pMatrix);
    m_pMatrix = NULL;
  }

  if (m_rgRGB) {
    free (m_rgRGB);
    m_rgRGB = NULL;
  }
}


#if defined (TEST_DITHER)

main (int argc, char** argv)
{
  if (argc != 2) {
    fprintf (stderr, "usage: <source_bitmap_filename>\n");

    exit (1);
  }
  char* sz = argv[1];

  HDR_DIBFILE hdr_file;
  HDR_DIB hdr;
  int height;
  int width;

  FILE* fp = fopen (sz, "rb");
  UINT bftype;
  fread (&bftype, 1, sizeof (bftype), fp);
  if (   bftype != (('B' << 8) | 'M')
      && bftype != (('M' << 8) | 'B')) {
    fprintf (stderr, "file %s is not a bitmap\n", sz);
    exit (2);
  }

  fread (&hdr_file, 1, sizeof (hdr_file), fp);
  fread (&hdr, 1, sizeof (hdr), fp);
  if (hdr.biBitCount != 24) {
    fprintf (stderr, "source file must be 24 bits/pel");
    exit (3);
  }

  height = hdr.biHeight;
  width  = hdr.biWidth;
  int cbRowSrc = ROWWIDTH (width, 24);

  unsigned8* pbSrc = (unsigned8*) malloc (height*cbRowSrc);
  fseek (fp, hdr_file.bfOffBits, SEEK_SET);
  fread (pbSrc, 1, height*cbRowSrc, fp);
  fclose (fp);

  int cbRowDst = ROWWIDTH (width, 8);
  memset (&hdr_file, 0, sizeof (hdr_file));
  hdr_file.bfOffBits  = sizeof (hdr_file) + 2 + sizeof (hdr)
    + sizeof (RGBQUAD)*256;
  hdr_file.bfSize = hdr_file.bfOffBits + cbRowDst*height;
  memset (&hdr, 0, sizeof (hdr));
  hdr.biSize = sizeof (hdr);
  hdr.biWidth = width;
  hdr.biHeight = height;
  hdr.biPlanes = 1;
  hdr.biBitCount = 8;
  hdr.biSizeImage = cbRowDst*height;

  fwrite ("B", 1, 1, stdout);
  fwrite ("M", 1, 1, stdout);
  fwrite (&hdr_file, 1, sizeof (hdr_file), stdout);
  fwrite (&hdr, 1, sizeof (hdr), stdout);

  LDither dither;
  dither.init ();

  fwrite (dither.palette (), dither.colors (), sizeof (RGBQUAD), stdout);
  RGBQUAD rgb;
  memset (&rgb, 0, sizeof (RGBQUAD));
  for (int i = 256 - dither.colors (); i--; )
    fwrite (&rgb, 1, sizeof (RGBQUAD), stdout);

  unsigned8* pbDst = (unsigned8*) malloc (height*cbRowDst);
  memset (pbDst, 0, height*cbRowDst);

  dither.dither (width, height, pbSrc, pbDst);

  fwrite (pbDst, height, cbRowDst, stdout);

  fflush (stdout);

  exit (0);
}

#endif
