/* lwindow.cxx
     $Id: lwindow.cxx,v 1.20 1998/10/15 04:11:51 elf Exp $

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

/* LWindow::create

   creates the X server window object.  As a side effect this LWindow
   structure is linked into the hierarchy of windows.

*/

bool LWindow::create (LWindow* pWindowParent)
{
  Window wndParent = (pWindowParent
		      ? pWindowParent->window ()
		      : XDefaultRootWindow (m_pDisplay->display ()));
  XSetWindowAttributes attr = m_attr;
  unsigned long mask = m_maskAttr;

  //  if (!(mask & CWBackPixel)) {
  //    mask |= CWBackPixel;
  //    attr.background_pixel = 19;
  //  }
  //  if (!(mask & CWBorderPixel)) {
  //    mask |= CWBorderPixel;
  //    attributes.border_pixel = 1;
  //  }
  //  if (!(mask & CWEventMask) || event_mask) {
  //    mask |= CWEventMask;
  //    attributes.event_mask = event_mask;
  //  }
  //  m_pEventHandlerMap = pEventHandlerMap;
  mask |= CWColormap;
  attr.colormap = m_pDisplay->colormap ();

  mask |= CWEventMask;

  m_window = XCreateWindow (xdisplay (),
			    wndParent,
			    m_x, m_y, m_width, m_height, 0,
			    m_pDisplay->depth (),
			    InputOutput, m_pDisplay->visual (),
			    mask, &attr);
  m_pDisplay->flush ();		// Helps notify semantics, really? **FIXME**

  //  m_event_mask = attr.event_mask;

  if (m_window) {
				// Properties
    if (m_szXName) {
      XChangeProperty (xdisplay (), m_window, XA_WM_NAME, XA_STRING,
		       sizeof (char)*8, PropModeReplace,
		       (unsigned char*) m_szXName, strlen (m_szXName));
      XChangeProperty (xdisplay (), m_window, XA_WM_ICON_NAME, XA_STRING,
		       sizeof (char)*8, PropModeReplace,
		       (unsigned char*) m_szXName, strlen (m_szXName));
    }

    if (m_szXClass) {
      int cch;
      for (cch = 0; m_szXClass[cch];
	   cch += 1 + strlen (m_szXClass + cch));
      XChangeProperty (xdisplay (), m_window, XA_WM_CLASS, XA_STRING,
		       sizeof (char)*8, PropModeReplace,
		       (unsigned char*) m_szXClass, cch);
    }
    {
      XSizeHints hints;
      memset (&hints, 0, sizeof (hints));
      hints.flags |= (m_fOrigin ? USPosition : PPosition);
      hints.flags |= (m_fExtent ? USSize : PSize);
//      hints.flags |= PMinSize | PMaxSize | PAspect;
      hints.x = m_x;
      hints.y = m_y;
      hints.width = m_width;
      hints.height = m_height;
      hints.min_width = 32;
      hints.min_height = 32;
      hints.max_width = 1024;
      hints.max_height = 1024;
      hints.min_aspect.x = 1;
      hints.min_aspect.y = 1;
      hints.max_aspect.x = 1;
      hints.max_aspect.y = 1;
      XSetWMNormalHints (xdisplay (), m_window, &hints);
    }
    {
      XWMHints hints;
      memset (&hints, 0, sizeof (hints));
      hints.flags = InputHint | StateHint;
      hints.input = True;	// We default to a Passive input model
      hints.initial_state = NormalState;
      XSetWMHints (xdisplay (), m_window, &hints);
    }
    {
      char sz[256];
      gethostname (sz, sizeof (sz));
      XChangeProperty (xdisplay (), m_window, XA_WM_CLIENT_MACHINE, XA_STRING,
		       sizeof (char)*8, PropModeReplace,
		       (unsigned char*) sz, strlen (sz));
    }

    m_pDisplay->hash_window (this);
    m_pWindowParent = pWindowParent;

				// Link
    LWindow** ppWindowHere = (pWindowParent
			      ? &pWindowParent->m_pWindowChild
			      : m_pDisplay->root_windows ());
    m_pWindowSibling = *ppWindowHere;
    *ppWindowHere = this;

				// Give due notice
    XoEvent _event;
    memset (&_event, 0, sizeof (_event));
    XoCreateSelfWindowEvent& event = _event.xcreatewindow;
    event.type = CreateSelfNotify;
    event.display = m_pDisplay->display ();
    event.window = m_window;
    event.parent = wndParent;
    event.x      = m_x;
    event.y      = m_y;
    event.width  = m_width;
    event.height = m_height;
    m_pDisplay->dispatch ((XEvent*) &_event);
  }

  return m_window != 0;
}


LWindow* LWindow::find_sibling (int id)
{
  for (LWindow* pWindow = this; pWindow; pWindow = pWindow->m_pWindowSibling)
    if (pWindow->id () == id)
      return pWindow;
  return NULL;
}

PFNEvent LWindow::find_event (int event_type, XEvent* event)
{
  for (EventMap* pMap = event_map ();
       pMap && pMap->pfn; ++pMap)
    if (pMap->event_type == event_type)
      return pMap->pfn;

  // This is the new, improved implementation of event handling.  We
  // simply call the virtual function and let the implementor take
  // care of it.  We still return a NULL pointer so that the old code
  // works.

  switch (event_type) {
  case Expose:
    handle_expose ((XExposeEvent*) event);
    break;
  case ButtonPress:
    handle_buttonpress ((XButtonEvent*) event);
    break;
  case ConfigureNotify:
    handle_configure ((XConfigureEvent*) event);
    break;
  case ResizeRequest:
    handle_resizerequest ((XResizeRequestEvent*) event);
    break;
  default:
//    printf ("event %d\n", event_type);
    break;
  }

  return (PFNEvent) NULL;
}


void LWindow::geometry (const char* sz)
{
  if (!sz || !* sz)
    return;

  int x = m_x;
  int y = m_y;
  unsigned int width = m_width;
  unsigned int height = m_height;
  int flags = XParseGeometry (sz, &x, &y, &width, &height);
  if (flags & XNegative)
    x = m_pDisplay->width () - width;
  if (flags & YNegative)
    y = m_pDisplay->height () - height;
  if (flags & XValue) {
    m_x = x;
    m_fOrigin = True;
  }
  if (flags & YValue) {
    m_y = y;
    m_fOrigin = True;
  }
  if (flags & WidthValue) {
    m_width = width;
    m_fExtent = True;
  }
  if (flags & HeightValue) {
    m_height = height;
    m_fExtent = True;
  }
}


/* LWindow::init

   initializes an LWindow instance from an existing LWindow instance.
   This can be though of as a clone operation since everything except
   for the X-Windows window handle is duplicated. (FXIME, really?)

*/

void LWindow::init (LWindow* pWindow)
{
				// Locality
  m_pDisplay		= pWindow->m_pDisplay;
				// Appearance
  m_maskAttr		= pWindow->m_maskAttr;
  m_attr		= pWindow->m_attr;
  m_x			= pWindow->m_x;
  m_y			= pWindow->m_y;
  m_width		= pWindow->m_width;
  m_height		= pWindow->m_height;

				// Personality
  m_cbInstance		= pWindow->m_cbInstance;
  m_pEventMap		= pWindow->m_pEventMap;
  m_pfnNotify		= pWindow->m_pfnNotify;

  memcpy (this + 1, pWindow + 1, m_cbInstance);// Duplicate instance data
}


void LWindow::lower (void)
{
  if (!m_window)
    return;
  XLowerWindow (m_pDisplay->display (), m_window);
}

void LWindow::map (void)
{
  if (!m_window)
    return;
  XMapWindow (m_pDisplay->display (), m_window);
}


void LWindow::notify (int child_event, int cArgs, int* rgArgs)
{
				// Some windows don't notify
  if (!m_pWindowOwner || !m_pWindowOwner->m_pfnNotify)
    return;

  XoEvent _event;
  memset (&_event, 0, sizeof (_event));

  XoChildNotifyEvent& event = _event.xchildnotify;
  event.type = ChildNotify;
  event.display = m_pDisplay->display ();
  event.window = m_pWindowOwner->window ();
  event.pWindowChild = this;
  event.pvParentData = m_pvOwner;
  event.child_type = child_event;
  for (int i = 0; i < cArgs && rgArgs
	 && i < int (sizeof (event.rgData)/sizeof (int)); ++i)
    event.rgData[i] = rgArgs[i];
  (m_pWindowOwner->*m_pfnNotify) ((XEvent*) &event);
  //  m_pDisplay->dispatch ((XEvent*) &event);
}


void LWindow::release_this (void)
{
  if (m_szName) {
    free ((void*) m_szName);
    m_szName = NULL;
  }
  if (m_szXName) {
    free ((void*) m_szXName);
    m_szXName = NULL;
  }
  if (m_szXClass) {
    free ((void*) m_szXClass);
    m_szXClass = NULL;
  }
  if (m_window) {
    m_pDisplay->unhash_window (this);
    XDestroyWindow (m_pDisplay->display (), m_window);
    m_window = 0;
  }
}


/* LWindow::qualify

   is a first attempt to provide information that Window Managers can
   use to recognize us.

*/

void LWindow::qualify (const char* szXName, const char* szXClass)
{
  if (m_szXName) {
    free ((void*) m_szXName);
    m_szXName = NULL;
  }
  if (szXName && *szXName) {
    m_szXName = (char*) malloc (strlen (szXName) + 1);
    memcpy ((void*) m_szXName, szXName, strlen (szXName) + 1);
  }
  if (m_szXClass) {
    free ((void*) m_szXClass);
    m_szXClass = NULL;
  }
  if (szXClass && *szXClass) {
    int cch = 0;
    while (szXClass[cch])
      cch += 1 + strlen (szXClass + cch);
    //    fprintf (stderr, "class is %d long\n", cch);
    m_szXClass = (char*) malloc (cch + 1);
    memcpy ((void*) m_szXClass, szXClass, cch + 1);
  }
}

void LWindow::text (const char* sz)
{
  if (m_szName) {
    free ((void*) m_szName);
    m_szName = NULL;
  }
  if (!sz || !*sz)
    return;

  m_szName = (char*) malloc (strlen (sz) + 1);
  memcpy ((void*) m_szName, sz, strlen (sz) + 1);
}

void LWindow::transient_for (LWindow* pWindow)
{
  XSetTransientForHint (m_pDisplay->display (),
			pWindow ? pWindow->m_window
			: XDefaultRootWindow (m_pDisplay->display ()),
			m_window);
}

void LWindow::net_window_type (int type)
{
  const char* sz = NULL;
  switch (type) {
  default:
  case windowTypeNormal:
    sz = "_NET_WM_WINDOW_TYPE_NORMAL";
    break;
  case windowTypeDesktop:
    sz = "_NET_WM_WINDOW_TYPE_DESKTOP";
    break;
  case windowTypeDock:
    sz = "_NET_WM_WINDOW_TYPE_DOCK";
    break;
  case windowTypeToolbar:
    sz = "_NET_WM_WINDOW_TYPE_TOOLBAR";
    break;
  case windowTypeMenu:
    sz = "_NET_WM_WINDOW_TYPE_MENU";
    break;
  case windowTypeUtility:
    sz = "_NET_WM_WINDOW_TYPE_UTILITY";
    break;
  case windowTypeSplash:
    sz = "_NET_WM_WINDOW_TYPE_SPLASH";
    break;
  case windowTypeDialog:
    sz = "_NET_WM_WINDOW_TYPE_DIALOG";
    break;
  }

  Atom a = XInternAtom (xdisplay (), "_NET_WM_WINDOW_TYPE", true);
  Atom b = XInternAtom (xdisplay (), sz, true);

  XChangeProperty (xdisplay (), m_window, a, XA_ATOM, sizeof (Atom)*8,
		   PropModeReplace, (unsigned char*) &b, 1);
}
