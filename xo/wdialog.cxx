/* wdialog.cxx
     $Id: wdialog.cxx,v 1.12 1998/10/15 04:17:25 elf Exp $

   written by Marc Singer
   16 May 1997

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
//#include "lwindowclass.h"
#include "lwindow.h"
#include "wdialog.h"
#include "wbutton.h"
#include "wtext.h"


typedef struct {
  Font fid;			// Default dialog font, used for dlg units
  int id;			// ID for identifying 
  int unitWidth;		// Width of a dialog character
  int unitHeight;		// Height of a dialog character
  LResNode* pNodeResource;	// Resource node
} DIALOG_INFO;

#define EVENT_MASK (  ButtonPressMask \
		    | ButtonReleaseMask \
		    | ExposureMask \
		    | EnterWindowMask \
		    | LeaveWindowMask)
EventMap g_rgEHDialog[] = {
  { CreateSelfNotify,	(PFNEvent) &WDialog::createself	},
  { 0, NULL },
};


WDialog* WDialog::create (LWindow* pWindowOwner, char* szFileResource, int id)
{
				// Parse the resource file, **FIXME**
				// this should be done in LResNode

  FILE* fp = fopen (szFileResource, "r");
  yyin = fp;
  yyparse ();
  fclose (fp);

				// Extract dialog specific information
  int rgi[4] = { 0, 0, 0, 0 };

  LResNode* pNode = LResNode::locate ("dialog", id);
  if (!pNode)
    return NULL;

  WDialog* pWindow = new WDialog (pWindowOwner->display ()
				  ->find_template ("dialog"));
  pWindow->m_pNodeResource = pNode;

  //  {
  //    LResNode* pNodeArg = pNode->enum_args ();
  //    if (pNodeArg)
  //      pNodeArg->integer (&id);
  //  }
  pWindow->id (id);

  for (pNode = pNode->find_resources (); pNode; pNode = pNode->next ()) {
    char* sz = pNode->keyword ();
    if (!sz)
      continue;

    int* pi;
    if (!strcasecmp (sz, "origin"))
      pi = &rgi[0];
    else if (!strcasecmp (sz, "extent"))
      pi = &rgi[2];
    else
      continue;

    int i = 0;
    for (LResNode* pNodeArg = pNode; 
	 i < 2 && (pNodeArg = pNodeArg->enum_args ());) {
      if (pNodeArg->integer (pi)) {
	*pi++ *= (i ? pWindow->m_unitHeight : pWindow->m_unitWidth);
	++i;
      }
      float r;
      if (pNodeArg->real (&r)) {
	*pi++ = int (r* (i ? pWindow->m_unitHeight : pWindow->m_unitWidth));
	++i;
      }
    }
  }

  pWindow->owner (pWindowOwner, pWindow->id ());
  pWindow->position (rgi[0], rgi[1], rgi[2], rgi[3]);
  pWindow->LWindow::create (NULL);
  pWindow->map ();

  return pWindow;
}


void WDialog::createself (XoCreateSelfWindowEvent* pEvent)
{
  fprintf (stderr, "create dialog\n");
  LResNode* pNode = m_pNodeResource;

				// Create the child windows
  for (pNode = pNode->find_resources (); pNode; pNode = pNode->next ()) {
    char* sz = pNode->keyword ();
    if (!sz)
      continue;
    if (!strcasecmp (sz, "pushbutton"))
      create_pushbutton (pEvent, pNode);
    if (!strcasecmp (sz, "textedit"))
      create_textedit (pEvent, pNode);
  }
  m_pNodeResource = NULL;
}


void WDialog::create_pushbutton (XoCreateSelfWindowEvent* /* pEvent */,
				 LResNode* pNode)
{
  char* szTitle = NULL;
  int rgi[4] = { 0, 0, 0, 0 };

  WButton* pWindow = new WButton (m_pDisplay->find_template ("button"));

  int id = 0;
  {
    LResNode* pNodeArg = pNode->enum_args ();
    if (pNodeArg)
      pNodeArg->integer (&id);
  }

  pWindow->id (id);

  for (pNode = pNode->find_resources (); pNode; pNode = pNode->next ()) {
    char* sz = pNode->keyword ();
    if (!sz)
      continue;

    int* pi;
    if (!strcasecmp (sz, "origin"))
      pi = &rgi[0];
    else if (!strcasecmp (sz, "extent"))
      pi = &rgi[2];
    else if (!strcasecmp (sz, "title")) {
      LResNode* pNodeArg = pNode->enum_args ();
      pNodeArg->string (&szTitle);
      continue;
    }
    else
      continue;

    int i = 0;
    for (LResNode* pNodeArg = pNode; 
	 i < 2 && (pNodeArg = pNodeArg->enum_args ());) {
      if (pNodeArg->integer (pi)) {
	*pi++ *= (i ? m_unitHeight : m_unitWidth);
	++i;
      }
      float r;
      if (pNodeArg->real (&r)) {
	*pi++ = int (r* (i ? m_unitHeight : m_unitWidth));
	++i;
      }
    }
  }

  pWindow->text (szTitle);
  pWindow->owner (owner (), ((this->id () << 16) | id));
  pWindow->position (rgi[0], rgi[1], rgi[2], rgi[3]);
  pWindow->create (this);
  pWindow->map ();
}


void WDialog::create_textedit (XoCreateSelfWindowEvent* /* pEvent */,
			       LResNode* pNode)
{
  char* szText = NULL;
  int rgi[4] = { 0, 0, 0, 0 };

  WTextEdit* pWindow = new WTextEdit (m_pDisplay->find_template ("text"));

  int id = 0;
  {
    LResNode* pNodeArg = pNode->enum_args ();
    if (pNodeArg)
      pNodeArg->integer (&id);
  }

  pWindow->id (id);

  for (pNode = pNode->find_resources (); pNode; pNode = pNode->next ()) {
    char* sz = pNode->keyword ();
    if (!sz)
      continue;

    int* pi;
    if (!strcasecmp (sz, "origin"))
      pi = &rgi[0];
    else if (!strcasecmp (sz, "extent"))
      pi = &rgi[2];
    else if (!strcasecmp (sz, "text")) {
      LResNode* pNodeArg = pNode->enum_args ();
      pNodeArg->string (&szText);
      continue;
    }
    else
      continue;

    int i = 0;
    for (LResNode* pNodeArg = pNode; 
	 i < 2 && (pNodeArg = pNodeArg->enum_args ());) {
      if (pNodeArg->integer (pi)) {
	*pi++ *= (i ? m_unitHeight : m_unitWidth);
	++i;
      }
      float r;
      if (pNodeArg->real (&r)) {
	*pi++ = int (r* (i ? m_unitHeight : m_unitWidth));
	++i;
      }
    }
  }

//  pWindow->owner (pWindowParent->owner (), (void*) ((pInfo->id << 16) | id));
  pWindow->position (rgi[0], rgi[1], rgi[2], rgi[3]);
  pWindow->create (this);
  if (szText)
    pWindow->text (szText);
  pWindow->map ();
}


void WDialog::register_template (LDisplay* pDisplay)
{
  WDialog* pWindow = new WDialog (pDisplay);
  pWindow->event_map (g_rgEHDialog);
  pWindow->select_events (EVENT_MASK);

  XrmValue value;
  pDisplay->find_resource ("xo.dialog.font", "Toolkit.Dialog.Font",
			   NULL, &value);
  XFontStruct* pFont = pDisplay->find_font (value.addr);
  if (pFont) {
    pWindow->m_fid = pFont->fid;
    pWindow->m_unitWidth
      = pFont->per_char['m' - pFont->min_char_or_byte2].width;
//      pInfo->unitWidth
//	= (pFont->min_bounds.width + pFont->max_bounds.width)/2;
    pWindow->m_unitHeight = pFont->max_bounds.ascent
      + pFont->max_bounds.descent;
  }

  pDisplay->hash_template ("dialog", pWindow);

//    pClass->set_background_pixel (XBlackPixel (pDisplay->display (), 0));
//    pClass->set_bit_gravity (SouthEastGravity);
}
