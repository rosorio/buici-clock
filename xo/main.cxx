/* main.cxx
     $Id: main.cxx,v 1.11 1997/10/18 04:57:40 elf Exp $

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
#include "wdialog.h"

void register_base_classes (LDisplay* pDisplay);

int g_argc;
char** g_argv;
bool g_fQuit;

void exit_error (char* sz, ...)
{
  fprintf (stderr, "%s: ", g_argv[0]);

  va_list ap;
  va_start (ap, sz);
  vfprintf (stderr, sz, ap);
  va_end (ap);

  fprintf (stderr, "\n");

  exit (1);
}


int main (int argc, char** argv)
{
  g_argc = argc;
  g_argv = argv;

  for (int i = 1; i < argc; ++i)
    if (strcasecmp (argv[i], "--no-ornaments") == 0)
      g_fNoOrnaments = true;

  LDisplay display;
  if (!display.open ())
    exit_error ("unable to open display");

  register_base_classes (&display);

  LWindow* pWindow = new LWindow (display.find_template ("top-level"));
#if 0
  pWindow->position (100, 100, 200, 200);
  if (!pWindow->create (0))
    exit_error ("unable to open window");
  pWindow->map ();
#endif

  LWindow* pWindowDialog = WDialog::create (pWindow, "sample.dlg", 1);

  //  XEvent event;
  while (!g_fQuit) {
    display.dispatch_next_event ();
  }

  pWindowDialog->unmap ();	// Make a clean exit
  display.flush ();
}
