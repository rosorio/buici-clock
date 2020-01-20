/* wbutton.cxx
     $Id: wbutton.cxx,v 1.14 1998/10/15 04:11:51 elf Exp $

   written by Marc Singer
   9 May 1997

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

   Implementation of a button window class.

*/

#include "standard.h"


#include "ldisplay.h"
#include "lwindow.h"
//#include "lwindowclass.h"

#include "wbutton.h"


#define EVENT_MASK (  ButtonPressMask \
		    | ButtonReleaseMask \
		    | ExposureMask \
		    | EnterWindowMask \
		    | LeaveWindowMask)

EventMap g_rgEMButton[] = {
  { ButtonPress,	(PFNEvent) &WButton::buttondown		},
  { ButtonRelease,	(PFNEvent) &WButton::buttonup		},
  { Expose,		(PFNEvent) &WButton::expose		},
//  { CreateSelfNotify,	(PFNEvent) &WButton::createself		},
  { EnterNotify,	(PFNEvent) &WButton::enter		},
  { LeaveNotify,	(PFNEvent) &WButton::leave		},
  { 0, NULL },
};

void WButton::buttondown (XButtonEvent* pEvent)
{
  m_fPress = true;
  m_fOutOfWindow = false;

  XClearArea (pEvent->display, pEvent->window, 0, 0, 0, 0, True);
}

void WButton::buttonup (XButtonEvent* pEvent)
{
  m_fPress = false;
  if (!m_fOutOfWindow) {
    XClearArea (pEvent->display, pEvent->window, 0, 0, 0, 0, True);
    notify (ButtonPressNotify);
  }
}

void WButton::createself (XoCreateSelfWindowEvent* /* pEvent */)
{
  //  fprintf (stderr, "create button\n");
}

void WButton::enter (XEnterWindowEvent* pEvent)
{
  if (!m_fPress)
    return;
  m_fOutOfWindow = false;

  XClearArea (pEvent->display, pEvent->window, 0, 0, 0, 0, True);
}

void WButton::leave (XLeaveWindowEvent* pEvent)
{
  if (!m_fPress)
    return;
  m_fOutOfWindow = true;

  XClearArea (pEvent->display, pEvent->window, 0, 0, 0, 0, True);
}

void WButton::expose (XExposeEvent* pEvent)
{
  GC gc = m_pDisplay->gc ();

  bool fDown = m_fPress && !m_fOutOfWindow;
  int colorFG = fDown ? m_highlight  : m_foreground;
  int colorBG = fDown ? m_foreground : m_highlight;

  XSetState (pEvent->display, gc, colorFG, colorBG, GXcopy, AllPlanes);

  XDrawRectangle (pEvent->display, pEvent->window, gc, 
		  0, 0,
		  width () - 1, height () - 1
		  );
  
  XFontStruct* pFont = display ()->find_font (m_fid);
  if (pFont) {
    const char* szTitle = text ();
    if (!szTitle) 
      szTitle = "Pressing";
    int cchTitle = strlen (szTitle);
    int direction;
    int ascent;
    int descent;
    XCharStruct xchar;
    XSetFont (pEvent->display, gc, pFont->fid);
    //  fprintf (stderr, "fontID 0x%x\n", pFont->fid);
    XTextExtents (pFont, szTitle, cchTitle, 
		  &direction, &ascent, &descent, &xchar);
    XDrawString (pEvent->display, pEvent->window, gc, 
		 (width () - xchar.width)/2,
		 (height () - ascent - descent)/2 + ascent,
		 szTitle, cchTitle);
  }
}


void WButton::register_template (LDisplay* pDisplay)
{
  WButton* pWindow = new WButton (pDisplay);
  pWindow->event_map (g_rgEMButton);
  pWindow->select_events (EVENT_MASK);

				// -- Prepare instance data
  XrmValue value;
  pDisplay->find_resource ("xo.button.font", "Toolkit.Button.Font", 
			   NULL, &value);
  XFontStruct* pFont = pDisplay->find_font (value.addr);
  if (pFont)
    pWindow->m_fid = pFont->fid;
  pDisplay->find_resource ("xo.button.background", "Toolkit.Button.Background",
			   NULL, &value);
  pDisplay->find_pixel (value.addr, &pWindow->m_background);
  
  pDisplay->find_resource ("xo.button.foreground", "Toolkit.Button.Foreground",
			   NULL, &value);
  pDisplay->find_pixel (value.addr, &pWindow->m_foreground);
  
  pDisplay->find_resource ("xo.button.highlight", "Toolkit.Button.Highlight",
			   NULL, &value);
  pDisplay->find_pixel (value.addr, &pWindow->m_highlight);
  
  pWindow->set_background_pixel (pWindow->m_background);

  pDisplay->hash_template ("button", pWindow);
}



