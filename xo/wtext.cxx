/* wtext.cxx
     $Id: wtext.cxx,v 1.9 1998/10/15 04:11:51 elf Exp $

   written by Marc Singer
   22 May 1997

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

   Text edit and display control.

*/


#include "standard.h"

#include "ldisplay.h"
#include "lwindow.h"
#include "wtext.h"

#include <X11/Xutil.h>

//#include "wtext.h"
#define PIX_MARGIN_LEFT		5
#define WIDTH_CARET		3
#define PIX_OFFSET_CARET	1



#define EVENT_MASK (  ButtonPressMask \
		    | ButtonReleaseMask \
		    | ExposureMask \
		    | KeyPressMask \
		    | KeyReleaseMask \
		    | EnterWindowMask \
		    | LeaveWindowMask \
		    | FocusChangeMask )

EventMap g_rgEMText[] = {
  { ButtonPress,	(PFNEvent) &WTextEdit::buttondown	},
  { ButtonRelease,	(PFNEvent) &WTextEdit::buttonup		},
  { Expose,		(PFNEvent) &WTextEdit::expose		},
  { CreateSelfNotify,	(PFNEvent) &WTextEdit::createself	},
  { EnterNotify,	(PFNEvent) &WTextEdit::enter		},
  { LeaveNotify,	(PFNEvent) &WTextEdit::leave		},
  { KeyPress,		(PFNEvent) &WTextEdit::keypress		},
  { KeyRelease,		(PFNEvent) &WTextEdit::keyrelease	},
  { FocusIn,		(PFNEvent) &WTextEdit::focusin		},
  { FocusOut,		(PFNEvent) &WTextEdit::focusout		},
  { 0, NULL },
};

void WTextEdit::buttondown (XButtonEvent* pEvent)
{
}

void WTextEdit::buttonup (XButtonEvent* pEvent)
{
}

void WTextEdit::createself (XoCreateSelfWindowEvent* /* pEvent */)
{
  m_sz = (char*) malloc (m_cchMax = 1024);
  m_cchLim = 0;
  *m_sz = 0;
}

void WTextEdit::enter (XEnterWindowEvent* pEvent)
{
  XFontStruct* pFont = m_pDisplay->find_font (m_fid);
  if (!pFont)
    return;
  
  m_fFocus = true;
  XClearArea (pEvent->display, pEvent->window, 
	      PIX_MARGIN_LEFT + m_widthString + PIX_OFFSET_CARET, 
	      (height () - pFont->ascent - pFont->descent)/2,
	      WIDTH_CARET, pFont->ascent + pFont->descent, True);
  XGrabKeyboard (pEvent->display, pEvent->window, True, 
		 GrabModeAsync, GrabModeAsync, CurrentTime);
}

void WTextEdit::leave (XLeaveWindowEvent* pEvent)
{
  XFontStruct* pFont = m_pDisplay->find_font (m_fid);
  if (!pFont)
    return;
  
  m_fFocus = false;
  XClearArea (pEvent->display, pEvent->window, 
	      PIX_MARGIN_LEFT + m_widthString + PIX_OFFSET_CARET, 
	      (height () - pFont->ascent - pFont->descent)/2,
	      WIDTH_CARET, pFont->ascent + pFont->descent, True);
  XUngrabKeyboard (pEvent->display, CurrentTime);
}

void WTextEdit::focusin (XFocusInEvent* pEvent)
{
}

void WTextEdit::focusout (XFocusOutEvent* pEvent)
{
}

void WTextEdit::expose (XExposeEvent* pEvent)
{
  GC gc = m_pDisplay->gc ();

  XSetState (pEvent->display, gc, m_foreground, m_background,
	     GXcopy, AllPlanes);

  XRectangle rect = { static_cast<short int> (pEvent->x),
                      static_cast<short int> (pEvent->y),
                      static_cast<short unsigned> (pEvent->width),
                      static_cast<short unsigned> (pEvent->height) };
  XSetClipRectangles (pEvent->display, gc, 0, 0, &rect, 1, Unsorted);

  XDrawRectangle (pEvent->display, pEvent->window, gc, 
		  0, 0,
		  width () - 1, height () - 1
		  );

  XFontStruct* pFont = m_pDisplay->find_font (m_fid);
  if (pFont) {
    int direction;
    int ascent;
    int descent;
    XCharStruct xchar;
    XSetFont (pEvent->display, gc, pFont->fid);
    XTextExtents (pFont, m_sz, m_cchLim, 
		  &direction, &ascent, &descent, &xchar);
    XDrawString (pEvent->display, pEvent->window, gc, 
		 PIX_MARGIN_LEFT,
		 (height () - ascent - descent)/2 + ascent,
		 m_sz, m_cchLim);

    if (m_fFocus)
      XCopyPlane (pEvent->display, m_pixmapCaret, pEvent->window, gc, 
		  0, 0, WIDTH_CARET, ascent + descent,
		  PIX_MARGIN_LEFT + m_widthString + PIX_OFFSET_CARET, 
		  (height () - ascent - descent)/2, 
		  0x1);
  }
}


void WTextEdit::keypress (XKeyPressedEvent* pEvent)
{
  KeySym keysym = XLookupKeysym (pEvent, 0);
//  printf ("keysym 0x%lx\n", (long) keysym);

  char sz[80];
  memset (sz, 0, sizeof (sz));
  XLookupString (pEvent, sz, sizeof (sz), &keysym, NULL);
//  printf ("string '%s' (0x%x)\n", sz, *sz);

  for (int i = 0; sz[i]; ++i) {
    if (sz[i] >= 0x20 && sz[i] <= 0x7e)
      m_sz[m_cchLim++] = sz[i];
    else if (sz[i] == 0x8 && m_cchLim)
      --m_cchLim;
  }
  m_sz[m_cchLim] = 0;
  //  printf ("'%s'\n", pInfo->sz);

  if (m_cchLim == 0 && m_widthString == 0)
    return;

  XFontStruct* pFont = m_pDisplay->find_font (m_fid);
  if (pFont) {
    int direction;
    int ascent;
    int descent;
    XCharStruct xchar;
    XTextExtents (pFont, m_sz, m_cchLim, 
		  &direction, &ascent, &descent, &xchar);
    if (m_widthString < xchar.width)
      XClearArea (pEvent->display, pEvent->window, 
		  PIX_MARGIN_LEFT + m_widthString, 
		  (height () - ascent - descent)/2,
		  xchar.width - m_widthString
		  + WIDTH_CARET + PIX_OFFSET_CARET,
		  ascent + descent, True);
    else
      XClearArea (pEvent->display, pEvent->window, 
		  PIX_MARGIN_LEFT + xchar.width, 
		  (height () - ascent - descent)/2,
		  m_widthString - xchar.width 
		  + WIDTH_CARET + PIX_OFFSET_CARET,
		  ascent + descent, True);
    m_widthString = xchar.width;
  }

  //  XClearArea (pEvent->display, pEvent->window, 0, 0, 0, 0, True);
}


void WTextEdit::keyrelease (XKeyReleasedEvent* /* pEvent */) 
{

}


void WTextEdit::register_template (LDisplay* pDisplay)
{
  WTextEdit* pWindow = new WTextEdit (pDisplay);
  pWindow->event_map (g_rgEMText);
  pWindow->select_events (EVENT_MASK);

				// -- Prepare instance data
  XrmValue value;
  pDisplay->find_resource ("xo.text.font", "Toolkit.Text.Font", 
			   NULL, &value);
  XFontStruct* pFont = pDisplay->find_font (value.addr);
  if (pFont)
    pWindow->m_fid = pFont->fid;
  pDisplay->find_resource ("xo.text.background", "Toolkit.Text.Background",
			   NULL, &value);
  pDisplay->find_pixel (value.addr, &pWindow->m_background);
  
  pDisplay->find_resource ("xo.text.foreground", "Toolkit.Text.Foreground",
			   NULL, &value);
  pDisplay->find_pixel (value.addr, &pWindow->m_foreground);
  
  pDisplay->find_resource ("xo.text.highlight", "Toolkit.Text.Highlight",
			   NULL, &value);
  pDisplay->find_pixel (value.addr, &pWindow->m_highlight);
  
  pWindow->set_background_pixel (pWindow->m_background);

  {
    int width = WIDTH_CARET;
    int height = pFont->ascent + pFont->descent;
    char rgb[128];
    for (int i = 0; i < height; ++i)
      rgb[i] = (i == 0 || i == height - 1) ? 5 : 2;
//      rgb[i] = (i == 0 || i == height - 1) ? 5 : 0xff;
    pWindow->m_pixmapCaret
      = XCreateBitmapFromData (pDisplay->display (), 
			       XDefaultRootWindow (pDisplay->display ()),
			       rgb, width, height);
  }

  pDisplay->hash_template ("text", pWindow);
}



void WTextEdit::text (const char* sz)
{
  if (!sz) {
    *m_sz = 0;
    return;
  }

  int cb = strlen (sz);
  if (cb > m_cchMax - 1)
    cb = m_cchMax - 1;
  memcpy (m_sz, sz, cb);
  m_sz[cb] = 0;
  m_cchLim = cb;

  XFontStruct* pFont = m_pDisplay->find_font (m_fid);
  if (pFont) {
    int direction;
    int ascent;
    int descent;
    XCharStruct xchar;
    XTextExtents (pFont, m_sz, m_cchLim, 
		  &direction, &ascent, &descent, &xchar);
    m_widthString = xchar.width;
  }
}
