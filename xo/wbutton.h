/* wbutton.h						-*- C++ -*-
   $Id: wbutton.h,v 1.3 1997/10/18 04:57:44 elf Exp $
   
   written by Marc Singer
   12 May 1997

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

#if !defined (__WBUTTON_H__)
#    define   __WBUTTON_H__

/* ----- Includes */

/* ----- Classes */


class WButton : public LWindow {

protected:
  Font m_fid;
  unsigned long m_foreground;	// Color values for foreground and
  unsigned long m_background;	//   background pixels
  unsigned long m_highlight;	// Highlight for pressed button
  bool m_fPress;
  bool m_fOutOfWindow;

public:
  void buttondown (XButtonEvent* pEvent);
  void buttonup	  (XButtonEvent* pEvent);
  void expose	  (XExposeEvent* pEvent);
  void createself (XoCreateSelfWindowEvent* pEvent);
  void enter	  (XEnterWindowEvent* pEvent);
  void leave	  (XLeaveWindowEvent* pEvent);

public:
  WButton (LDisplay* pDisplay) : LWindow (pDisplay) {
    zero (); m_cbInstance = sizeof (*this) - sizeof (LWindow); }
  WButton (LWindow* pWindow) : LWindow (pWindow) {}
  ~WButton () {
    release_this (); }
  void release_this (void) {}
  void zero (void) {
    memset (((LWindow*) this) + 1, 0, sizeof (*this) - sizeof (LWindow)); }

  static void register_template (LDisplay* pDisplay);
};


#define ButtonPressNotify	1


#endif  /* __WBUTTON_H__ */
