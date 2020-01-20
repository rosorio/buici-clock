/* dither.h			-*- C++ -*-
   $Id: dither.h,v 1.2 1997/10/18 04:57:32 elf Exp $

   written by Marc Singer
   24 Mar 1997

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

*/

#if !defined (__DITHER_H__)
#    define   __DITHER_H__

/* ----- Includes */

#include <memory.h>

#define C_COLOR_MAX 256		/* Total number of colors		*/
#define C_SHADES_MAX 256	/* Max number of shades in primary	*/

typedef unsigned32 PEL;
#define COLOROF(r,g,b) (((r)<<16) + ((g)<<8) + (b))

class LDither {
protected:
  int m_cRed;			// Count of red shades
  int m_cGreen;			// Count of green shades
  int m_cBlue;			// Count of blue shades
  int m_cColors;		// Count of all combinations, cRed*cGreen*cBlue

  void* m_rgRGB;		// Dithering palette RGBQUAD
  int **m_pMatrix;		// Dithering matrix
  int m_dim;			// Dimension of dither matrix
  int m_dim2;			// Dither matrix dimension squared

  int m_level_red;
  int m_level_green;
  int m_level_blue;
  int m_dither_divisor;

#define LEVELS(s) (m_dim2*((shades) - 1) + 1)
#define LEVEL5MULT(p) (((p) << 10) + (p))
#define LEVEL9MULT(p) (((p) << 11) + (p))
  unsigned8 dither (PEL pel, int d, int shades) {
    return unsigned8 ((LEVELS (shades)*(pel) + (d))/(m_dim2*C_SHADES_MAX)); }
  unsigned8 dither_red (PEL pel, int d) {
    return unsigned8 ((LEVEL5MULT(pel) + (d)) >> 16); }
  unsigned8 dither_green (PEL pel, int d) {
    return unsigned8 ((LEVEL9MULT(pel) + (d)) >> 16); }
  unsigned8 dither_blue (PEL pel, int d) {
    return unsigned8 ((LEVEL5MULT(pel) + (d)) >> 16); }
  int levels (int shades) {
    return m_dim2*((shades) - 1) + 1; }
  int matrix_value (int y, int x, int size);

public:
  LDither () {
    zero (); }
  ~LDither () {
    release_this (); }
  void zero (void) {
    memset (this, 0, sizeof (*this)); }
  void init (void) {
    init (4, 5, 9, 5); }
  void release_this (void);

  int color_index (int r, int g, int b) {
    return  ((r)*m_cGreen + (g))*m_cBlue + (b); }
  void create_matrix (int dim);
  void dither (int x, int y, int cx, int cy,
	       unsigned8* pbSrc, unsigned8* pbDst);
  void dither (int cx, int cy, unsigned8* pbSrc, unsigned8* pbDst) {
    dither (0, 0, cx, cy, pbSrc, pbDst); }
  void init (int dim, int cRed, int cGreen, int cBlue);
  int closest_match (float r, float g, float b) {
    return color_index (int (r*(m_cRed   - 1)),
			int (g*(m_cGreen - 1)),
			int (b*(m_cBlue  - 1))); }

  int colors (void) {
    return m_cColors; }
  void* palette (void) {
    return m_rgRGB; }

};


#endif  /* __DITHER_H__ */
