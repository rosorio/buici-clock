/* classes.cxx
     $Id: classes.cxx,v 1.16 1998/10/26 18:59:28 elf Exp $

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

*/

#include "standard.h"
#include "ldisplay.h"
#include "lwindow.h"

#include "wbutton.h"
#include "wdialog.h"
#include "wtext.h"

extern bool g_fQuit;

class WTopLevel : public LWindow {
public:
  WTopLevel (LDisplay* pDisplay) : LWindow (pDisplay) {}
  void child (XoChildNotifyEvent* pEvent);
};

//void eh_buttondown (LWindow* pWindow, XButtonEvent* pEvent);
//void eh_expose (LWindow* pWindow, XExposeEvent* pEvent);
//void eh_create (LWindow* pWindow, XoCreateSelfWindowEvent* pEvent);
//void eh_child (LWindow* pWindow, XoChildNotifyEvent* pEvent);

#if 0
EventHandlerMap g_rgEventHandlerMap[] = {
  { ButtonPress,	(PFNEvent) &eh_buttondown	},
  { Expose,		(PFNEvent) &eh_expose		},
  { CreateSelfNotify,	(PFNEvent) &eh_create		},
  { ChildNotify,	(PFNEvent) &eh_child		},
  { 0, NULL },
};
#endif


void WTopLevel::child (XoChildNotifyEvent* pEvent)
{
  printf ("notify from 0x%lx\n", (long) pEvent->pvParentData);
  if (pEvent->pvParentData == (void*) 1
      && pEvent->child_type == ButtonPressNotify) {
    XUnmapWindow (pEvent->display, pEvent->window);
    g_fQuit = true;
  }
  else if (pEvent->pvParentData == (void*) 0x10001
	   && pEvent->child_type == ButtonPressNotify) {
//    XUnmapWindow (pEvent->display, pEvent->window);
    g_fQuit = true;
  }
  else if (pEvent->pvParentData == (void*) 0x10002
	   && pEvent->child_type == ButtonPressNotify) {
    LWindow* pWindow = m_pDisplay->find_child (1);
    pWindow = pWindow->find_child (3);
    WTextEdit* pText = (WTextEdit*) pWindow;
    printf ("Captured '%s'\n", pText->text ());
  }
}

#if 0
void eh_create (LWindow* pWindow, XoCreateSelfWindowEvent* pEvent)
{
  fprintf (stderr, "create self  [%ld] window 0x%lx  parent 0x%lx\n",
	   pEvent->serial,
	   pEvent->window, pEvent->parent);
  
  LWindow* pWindowChild = new LWindow (pWindow->display()
				       ->find_template ("button"));
  pWindowChild->owner (pWindow, (void*) 1);
  pWindowChild->position (10, 10, 70, 40);
  if (pWindowChild->create (pWindow))
    pWindowChild->map ();
  else
    delete pWindowChild;
}


void eh_buttondown (LWindow* /* pWindow */, XButtonEvent* pEvent)
{
  XUnmapWindow (pEvent->display, pEvent->window);
  g_fQuit = true;
}


void eh_expose (LWindow* pWindow, XExposeEvent* pEvent)
{
  fprintf (stderr, "expose serial %ld  count %d  [%d %d %d %d]\n", 
	   pEvent->serial, pEvent->count, 
	   pEvent->x, pEvent->y, pEvent->width, pEvent->height);

  GC gc = pWindow->display ()->gc ();

  //  if (!pEvent->count)
  //    XSetState (pEvent->display, gc, 3, 0, GXcopy, AllPlanes);

  XDrawLine (pEvent->display, pEvent->window, gc, 
	     pEvent->x, pEvent->y, 
	     pEvent->x + pEvent->width, pEvent->y + pEvent->height);
  XDrawLine (pEvent->display, pEvent->window, gc, 
	     pEvent->x + pEvent->width, pEvent->y, 
	     pEvent->x, pEvent->y + pEvent->height);
}
#endif


void register_base_classes (LDisplay* pDisplay)
{
  LWindow* pWindow = new LWindow (pDisplay);
  //  pWindow->event_handler_map (g_rgEventHandlerMap);
  //  pWindow->select_events (ButtonPressMask | ExposureMask);
  //  pWindow->set_background_pixel (XBlackPixel (pDisplay->display (), 0));
  //  pWindow->set_bit_gravity (SouthEastGravity);
  pWindow->notify ((PFNEvent) &WTopLevel::child);
  if (!pDisplay->hash_template ("top-level", pWindow))
    return;

  WButton::register_template (pDisplay);
  WTextEdit::register_template (pDisplay);
  WDialog::register_template (pDisplay);
}
