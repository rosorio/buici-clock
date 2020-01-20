/* lwindow.h							-*- C++ -*-
   $Id: lwindow.h,v 1.18 1997/10/23 06:56:16 elf Exp $

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

#if !defined (__LWINDOW_H__)
#    define   __LWINDOW_H__

/* ----- Includes */

#include <memory.h>
#include <stdint.h>
#include "messages.h"

enum {
  windowTypeNormal = 0,
  windowTypeDesktop,
  windowTypeDock,
  windowTypeToolbar,
  windowTypeMenu,
  windowTypeUtility,
  windowTypeSplash,
  windowTypeDialog,
};

class LDisplay;

class LWindow;

typedef void (LWindow::*PFNEvent)(XEvent* pEvent);
typedef struct {
  int event_type;
  PFNEvent pfn;
} EventMap;


class LWindow {
protected:
			// -- X Attributes
  LDisplay* m_pDisplay;		// X display where window is displayed
  Window m_window;		// X window handle
//  long m_event_mask;		// X event mask, cached
  XSetWindowAttributes m_attr;	// X window creation attributes
  unsigned long m_maskAttr;	// X attribute mask for creation
  short m_x, m_y;		// Origin
  short m_width, m_height;	//   and extent
  bool m_fOrigin;		// true: window origin specied
  bool m_fExtent;		// true: window extent specied

			// -- Properties for X Windows Window Managers
  char* m_szXName;
  char* m_szXClass;

			// -- Relationship
  int32_t m_id;			// Generic ID
  LWindow* m_pWindowParent;	// Enclosing window, NULL for top-level
  LWindow* m_pWindowOwner;	// Notification target
  void* m_pvOwner;		// Owner's context pointer
  LWindow* m_pWindowSibling;	// Lateral link
  LWindow* m_pWindowChild;	// Leaf directed link

			// -- Local Attributes
  char* m_szName;		// Window name or content
//  void* m_pvInstance;		// Instance data
  int m_cbInstance;		// Size of window class structure

			// -- Event/Notification Handling
  EventMap* m_pEventMap;
  PFNEvent m_pfnNotify;		// Object-local notification receipt function

public:
			// -- Object creation
  LWindow (LDisplay* pDisplay) {
    zero (); init (pDisplay); }
  LWindow (LWindow* pWindow) {
    zero (); init (pWindow); }
  virtual ~LWindow () {
    release_this (); }
  void init (LDisplay* pDisplay) {
    m_pDisplay = pDisplay; }
  void init (LWindow* pWindow);			// Clone
  void release_this (void);
  void zero (void) {
    memset (this, 0, sizeof (*this)); }

			// -- X-Windows methods
  bool create (LWindow* pWindowParent);
  void map (void);
  void unmap (void) {
    m_window ? XUnmapWindow (m_pDisplay->display (), m_window) : 0; }

			// -- Configuration methods
  void event_map (EventMap* pEventMap) {
    m_pEventMap = pEventMap; }
  void extent (int width, int height) {
    m_width = width; m_height = height; m_fExtent = true; }
  //  void* instance (void* pv) {
  //    void* pvPrev = m_pvInstance; m_pvInstance = pv; return pvPrev; }
  void id (int id) {
    m_id = id; }
  void notify (PFNEvent pfn) {
    m_pfnNotify = pfn; }
  void owner (LWindow* pWindow, void* pv) {
    m_pWindowOwner = pWindow;
    m_pvOwner = pv; }
  void owner (LWindow* pWindow, int32_t v) {
    m_pWindowOwner = pWindow;
    m_pvOwner = (void*) (intptr_t (v)); }
  void geometry (const char* sz);
  void origin (int x, int y) {
    m_x = x; m_y = y; }
  void position (int x, int y, int width, int height) {
    m_x = x; m_y = y; m_width = width; m_height = height; }
  void qualify (const char* szXName, const char* szXClass);
  void text (const char* sz);
  void lower (void);

			// -- Access methods
  LDisplay* display (void) {
    return m_pDisplay; }
  EventMap* event_map (void) {
    return m_pEventMap; }
  LWindow* find_child (int id) {
    return m_pWindowChild
      ? m_pWindowChild->find_sibling (id)
      : (LWindow*) NULL; }
  LWindow* find_sibling (int id);
  int height (void) {
    return m_height; }
  //  void* instance (void) {
  //    return m_pvInstance; }
  int32_t id (void) {
    return m_id; }
  LWindow* owner (void) {
    return m_pWindowOwner; }
  void* owner_data (void) {
    return m_pvOwner; }
  const char* text (void) {
    return m_szName; }
  int width (void) {
    return m_width; }
  Window window (void) {
    return m_window; }
  Display* xdisplay (void) {
    return m_pDisplay->display (); }
  Visual* xvisual (void) {
    return m_pDisplay->visual (); }

			// -- Event control
  void add_events (long event_mask) {
    m_attr.event_mask |= event_mask;
    m_window ? XSelectInput (m_pDisplay->display (),
			     m_window, m_attr.event_mask) : 0; }
  PFNEvent find_event (int event_type, XEvent* event);
  void ignore_events (long event_mask) {
    m_attr.event_mask &= ~event_mask;
    m_window ? XSelectInput (m_pDisplay->display (),
			     m_window, m_attr.event_mask) : 0; }
  void notify (int child_event) {
    notify (child_event, 0, NULL); }
  void notify (int child_event, int cArgs, int* rgArgs);
  void select_events (long event_mask) {
    m_attr.event_mask = event_mask;
    m_window ? XSelectInput (m_pDisplay->display (),
			     m_window, m_attr.event_mask) : 0; }

			// -- Attribute methods
  void set_background_pixel (unsigned long pixel) {
    m_attr.background_pixel = pixel;
    m_maskAttr |= CWBackPixel; }
  void set_bit_gravity (int gravity) {
    m_attr.bit_gravity = gravity;
    m_maskAttr |= CWBitGravity; }
  void set_override_redirect (bool f) {
    m_attr.override_redirect = f;
    m_maskAttr |= CWOverrideRedirect; }
  void transient_for (LWindow* pWindow);
  void net_window_type (int);

  virtual void handle_expose (XExposeEvent* pEvent) {}
  virtual void handle_buttonpress (XButtonEvent* pEvent) {}
  virtual void handle_configure (XConfigureEvent* pEvent) {}
  virtual void handle_resizerequest (XResizeRequestEvent* pEvent) {
    m_width = pEvent->width; m_height = pEvent->height;
    XWindowChanges changes;
    changes.width = m_width;
    changes.height = m_height;
    printf ("configuring to %d %d\n", m_width, m_height);
//    XConfigureWindow (display ()->display (),
//		      window (), CWWidth | CWHeight, &changes);
    XResizeWindow (display ()->display (), window (), m_width, m_height);
  }

};

/* ----- Globals */




#endif  /* __LWINDOW_H__ */
