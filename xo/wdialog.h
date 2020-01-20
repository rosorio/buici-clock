/* wdialog.h							-*- C++ -*-
   $Id: wdialog.h,v 1.3 1997/10/18 04:57:45 elf Exp $
   
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

#if !defined (__WDIALOG_H__)
#    define   __WDIALOG_H__

/* ----- Includes */

#include "lres.h"

/* ----- Classes */

class WDialog : public LWindow {

protected:
  Font m_fid;			// Default dialog font, used for dlg units
  int m_unitWidth;		// Width of a dialog character
  int m_unitHeight;		// Height of a dialog character
  LResNode* m_pNodeResource;	// Resource node

  void create_pushbutton (XoCreateSelfWindowEvent* pEvent, LResNode* pNode);
  void create_textedit   (XoCreateSelfWindowEvent* pEvent, LResNode* pNode);

public:
  void createself (XoCreateSelfWindowEvent* pEvent);

public:
  WDialog (LDisplay* pDisplay) : LWindow (pDisplay) {
    zero (); m_cbInstance = sizeof (*this) - sizeof (LWindow); }
  WDialog (LWindow* pWindow) : LWindow (pWindow) {}
  ~WDialog () {
    release_this (); }
  void release_this (void) {}
  void zero (void) {
    memset (((LWindow*) this) + 1, 0, sizeof (*this) - sizeof (LWindow)); }

  static void register_template (LDisplay* pDisplay);

  static WDialog* create (LWindow* pWindowOwner, char* szFileResource, int id);
};


#endif  /* __WDIALOG_H__ */
