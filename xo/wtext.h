/* wtext.h							-*- C++ -*-
   $Id: wtext.h,v 1.4 1997/10/18 04:57:46 elf Exp $
   
   written by Marc Singer
   27 May 1997

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

#if !defined (__WTEXT_H__)
#    define   __WTEXT_H__

/* ----- Includes */


/* ----- Classes */

class WTextEdit : public LWindow {

protected:
  Font m_fid;
  unsigned long m_foreground;	// Color values for foreground and
  unsigned long m_background;	//   background pixels
  unsigned long m_highlight;	// Highlight for pressed button
  char* m_sz;			// String of characters in buffer
  int m_cchLim;			// Length of current string
  int m_cchMax;			// Length of allocated text buffer
  int m_widthString;		// Length of last rendered string
  Pixmap m_pixmapCaret;		// Bitmap representing the caret.
  bool m_fFocus;		// Window has focus

public:
  void buttondown (XButtonEvent* pEvent);
  void buttonup	  (XButtonEvent* pEvent);
  void expose	  (XExposeEvent* pEvent);
  void createself (XoCreateSelfWindowEvent* pEvent);
  void enter	  (XEnterWindowEvent* pEvent);
  void leave	  (XLeaveWindowEvent* pEvent);
  void keypress	  (XKeyPressedEvent* pEvent);
  void keyrelease (XKeyReleasedEvent* pEvent);
  void focusin	  (XFocusInEvent* pEvent);
  void focusout	  (XFocusOutEvent* pEvent);

public:
  WTextEdit (LDisplay* pDisplay) : LWindow (pDisplay) {
    zero (); m_cbInstance = sizeof (*this) - sizeof (LWindow); }
  WTextEdit (LWindow* pWindow) : LWindow (pWindow) {}
  ~WTextEdit () {
    release_this (); }
  void release_this (void) {}
  void zero (void) {
    memset (((LWindow*) this) + 1, 0, sizeof (*this) - sizeof (LWindow)); }

  static void register_template (LDisplay* pDisplay);

				// -- Control Methods
  const char* text (void) {
    return m_sz; }
  void text (const char* sz);

};


/* ----- Globals */



#endif  /* __WTEXT_H__ */
