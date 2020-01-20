/* ldisplay.cxx
     $Id: ldisplay.cxx,v 1.19 1997/10/23 02:28:57 elf Exp $

   written by Marc Singer
   8 May 1997

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
#include "lwindow.h"

unsigned long LDisplay::color_distance (XColor& color, XColor& colorMatch)
{
  unsigned long sh = 16 - m_pVisual->bits_per_rgb;
//unsigned long mask = (0xffff ^ ((1 << (16 - m_pVisual->bits_per_rgb)) - 1));
  unsigned long distRed   = (color.red   >> sh) - (colorMatch.red   >> sh);
  unsigned long distGreen = (color.green >> sh) - (colorMatch.green >> sh);
  unsigned long distBlue  = (color.blue  >> sh) - (colorMatch.blue  >> sh);
  return distRed*distRed + distGreen*distGreen + distBlue*distBlue;
}


void LDisplay::dispatch (XEvent* pEvent)
{
  LWindow* pWindow = find_window (pEvent->xany.window);
  if (!pWindow)
    return;
  PFNEvent pfn = pWindow->find_event (pEvent->type, pEvent);
  if (!pfn)
    return;
  (pWindow->*pfn) (pEvent);
}

void LDisplay::dispatch_next_event (void)
{
  XEvent event;
  next_event (&event);

				// Update keyboard mapping
  if (event.type == MappingKeyboard) {
    XRefreshKeyboardMapping ((XMappingEvent*) &event);
    return;
  }

//fprintf (stderr, "event %d  window 0x%lx\n", event.type, event.xany.window);
  LWindow* pWindow = find_window (event.xany.window);
  if (!pWindow)
    return;
  PFNEvent pfn = pWindow->find_event (event.type, &event);
  if (!pfn)
    return;
  (pWindow->*pfn) (&event);
}


LWindow* LDisplay::find_child (int id)
{
  return m_pWindowRoot ? m_pWindowRoot->find_sibling (id) : (LWindow*) NULL;
}


bool LDisplay::open (char* szDisplay)
{
  m_pDisplay = XOpenDisplay (szDisplay);

  if (!m_pDisplay)
    return false;

  m_cbitUnit = XBitmapUnit (m_pDisplay);
  m_cbitPadding  = XBitmapPad (m_pDisplay);
  m_depth = XDefaultDepth (m_pDisplay, 0);

  m_hashWindow.init (sizeof (LWindow*), 301);
  m_font_cache.init (this);
  m_hashTemplate.init (sizeof (LWindow*), 301);
//  dmalloc_validate ();

				// -- Get the resource database
  {
    char* szDatabase = XResourceManagerString (m_pDisplay);
    // ** FIXME ** Seems to me that this is a really bad problem if this fails
    if (szDatabase)
      m_database = XrmGetStringDatabase (szDatabase);
  }

  if (!m_visual_class)
    m_visual_class = TrueColor;
  XMatchVisualInfo (m_pDisplay, 0, m_depth, m_visual_class, &m_visualInfo);
  m_pVisual = (m_visualInfo.visual
	       ? m_visualInfo.visual
	       : XDefaultVisual (m_pDisplay, 0));
  m_colormap = ((m_visualInfo.c_class == TrueColor)
		? XCreateColormap (m_pDisplay,
				   XDefaultRootWindow (m_pDisplay),
				   m_pVisual, AllocNone)
		: CopyFromParent);
#if defined (TALK)
  printf ("using visualID 0x%lx\n",
	  XVisualIDFromVisual (m_pVisual));
#endif

  if (m_pVisual->red_mask) {
    unsigned mask;
    for (m_shiftRed = 0, mask = m_pVisual->red_mask;
	 !(mask & 0x8000) && mask; ++m_shiftRed, (mask <<= 1))
      ;
    for (m_shiftGreen = 0, mask = m_pVisual->green_mask;
	 !(mask & 0x8000) && mask; ++m_shiftGreen, (mask <<= 1))
      ;
    for (m_shiftBlue = 0, mask = m_pVisual->blue_mask;
	 !(mask & 0x8000) && mask; ++m_shiftBlue, (mask <<= 1))
      ;
  }

				// Cache some GCs
  XGCValues values;
  unsigned long value_mask = 0;

  values.foreground = XWhitePixel (m_pDisplay, 0);
  values.background = XBlackPixel (m_pDisplay, 0);
  value_mask |= GCForeground | GCBackground;
  m_gcAttribute = XCreateGC (m_pDisplay, XDefaultRootWindow (m_pDisplay),
			     value_mask, &values);
  m_gcDraw      = XCreateGC (m_pDisplay, XDefaultRootWindow (m_pDisplay),
			     0, 0);

				// Evaluate pixmap format
#if defined (TALK)
  {
    int c;
    XPixmapFormatValues* pFormat
      = XListPixmapFormats (m_pDisplay, &c);
    for (int i = 0; i < c; ++i)
      printf ("pixmap depth %d  bits_per_pixel %d  scanline_pad %d\n",
	      pFormat[i].depth,
	      pFormat[i].bits_per_pixel,
	      pFormat[i].scanline_pad);
    XFree (pFormat);

    printf ("bitmap unit %d  bitmap pad %d\n",
	    XBitmapUnit (m_pDisplay), XBitmapPad (m_pDisplay));

    printf ("image byte order %s  bit order %s\n",
	    XImageByteOrder (m_pDisplay) ? "msb" : "lsb",
	    XBitmapBitOrder (m_pDisplay) ? "msb" : "lsb");

  }
#endif

//  dmalloc_validate ();

  return true;
}


bool LDisplay::find_pixel (char* szColorName, unsigned long* pPixel)
{
  Colormap colormap = XDefaultColormap (m_pDisplay, 0);
  XColor color;
  XParseColor (m_pDisplay, colormap, szColorName, &color);
  bool fReturn = false;

  if (   m_pVisual->c_class == PseudoColor
      || m_pVisual->c_class == StaticColor) {
    int cColors = m_pVisual->map_entries;
    XColor* rgColor = (XColor*) malloc (sizeof (XColor)*cColors);
    memset (rgColor, 0, sizeof (XColor)*cColors);
    for (int i = cColors; i--;) {
      rgColor[i].pixel = i;
      rgColor[i].flags = DoRed | DoGreen | DoBlue;
    }
    XQueryColors (m_pDisplay, colormap, rgColor, cColors);
    unsigned long distanceLast = 0xfffffff;
    int pixel = 0;		// ** FIXME: shouldn't we detect error?
    for (int i = 0; i < cColors; ++i) {
      unsigned long distance = color_distance (color, rgColor[i]);
      if (distance >= distanceLast)
	continue;
      pixel = i;
      distanceLast = distance;
      if (distance == 0)
	break;
    }
    *pPixel = pixel;
    fReturn = true;
    free (rgColor);
  }
  else if (   m_pVisual->c_class == DirectColor
	   || m_pVisual->c_class == TrueColor) {
    *pPixel = 0;
    *pPixel |= (color.red   >> m_shiftRed)   & m_pVisual->red_mask;
    *pPixel |= (color.green >> m_shiftGreen) & m_pVisual->green_mask;
    *pPixel |= (color.blue  >> m_shiftBlue)  & m_pVisual->blue_mask;
    fReturn = true;
  }
  return fReturn;
}


LWindow* LDisplay::find_template (const char* szTemplate)
{
  LWindow* pWindow = NULL;
  HashKey key = LHashTable::make_string_key (szTemplate);

  m_hashTemplate.find (key, &pWindow);
  return pWindow;
}


LWindow* LDisplay::find_window (Window window)
{
  LWindow* pWindow = NULL;

  m_hashWindow.find ((HashKey) window, &pWindow);
  return pWindow;
}


GC LDisplay::gc (void)
{
  XCopyGC (m_pDisplay, m_gcAttribute, (1 << (GCLastBit + 1)) - 1, m_gcDraw);
  return m_gcDraw;
}


bool LDisplay::hash_template (const char* szTemplate, LWindow* pWindow)
{
  HashKey key = LHashTable::make_string_key (szTemplate);
  if (m_hashTemplate.find (key))
    return false;

  m_hashTemplate.add (key, &pWindow);
  return true;
}

void LDisplay::hash_window (LWindow* pWindow)
{
  Window window = pWindow->window ();

  if (window)
    m_hashWindow.add ((HashKey) window, &pWindow);
}


void LDisplay::release_this (void)
{
				// FIXME we cannot *really* release
				// the templates because they may
				// allocate memory.  This works for
				// those that do nothing of the sort.
  HashKey key;
  LWindow** ppWindow;
  for (unsigned32 index = 0;
       index = m_hashTemplate.enumerate_reverse (index, &key,
						 (const void**) &ppWindow); ) {
    delete *ppWindow;
    m_hashTemplate.remove (key);
  }

				// -- Do the rest of the cleanup

  m_font_cache.release_this ();

  if (m_gcAttribute) {
    XFreeGC (m_pDisplay, m_gcAttribute);
    XFreeGC (m_pDisplay, m_gcDraw);
    m_gcAttribute = m_gcDraw = 0;
  }

  if (m_pDisplay) {
    XCloseDisplay (m_pDisplay);
    m_pDisplay = 0;
  }
}


void LDisplay::unhash_window (LWindow* pWindow)
{
  Window window = pWindow->window ();

  if (window)
    m_hashWindow.remove ((HashKey) window);
}
