/* main.cxx
     $Id: main.cxx,v 1.5 2000/01/13 07:04:59 elf Exp $

   written by Marc Singer
   3 April 1997

   This file is part of the project Buici.  See the file README for
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

  /* ----- Includes */

#include "standard.h"

#include <sys/types.h>
#include <fcntl.h>

#include "version.h"


#if !defined (O_BINARY)
#define O_BINARY 0
#endif

char* g_szApplication;		// Name of application

int do_unrecognized (OPTION*, const char*);
int do_usage (OPTION*, const char*);
int do_version (OPTION*, const char*);
int do_clock (void);

int g_fAsDesktop;
int g_fAsToolbar;
int g_fAsDock;
int g_fOverrideRedirect;
int g_showSecondHand = -1;	// -1 means use the resource or default to true
char* g_szGeometry;
char* g_szTimeZone = NULL;

OPTION rgOptions[] =
{
  { "geometry",		OptArg | OptSetString | OptAllDash, &g_szGeometry },
  { "as-dock",		OptSetInt, &g_fAsDock				  },
  { "as-desktop",	OptSetInt, &g_fAsDesktop			  },
  { "as-toolbar",	OptSetInt, &g_fAsToolbar			  },
  { "override-redirect", OptSetInt, &g_fOverrideRedirect		  },
  { "show-second-hand",	OptArg | OptSetInt, &g_showSecondHand		  },
  { "tz",		    OptArg | OptSetString | OptAllDash, &g_szTimeZone },
  { "help",		0, NULL, do_usage				},
  { "h",		0, NULL, do_usage				},

  { "version",		0, NULL, do_version				},
  { "V",		0, NULL, do_version				},

  { "",			OptDefault, NULL, do_unrecognized		},
  { NULL								},
};


  /* ----- Methods */


int do_unrecognized (OPTION* /* pOption */, const char* pch)
{
  printf ("%s: unrecognized option '%s'\n",
	  g_szApplication, pch);
  printf ("Try '%s --help' for usage information.\n",
	  g_szApplication);
   return 1;
}  /* do_unrecognized */


int do_usage (OPTION*, const char*)
{
  printf (
"usage: %s [options]\n"
"  -geometry =WIDTHxHEIGHT+OFFSETX+OFFSETY\n"
"                       Toolkit option to set size and position of window\n"
"  --as-desktop		Set window type to DESKTOP via EWMH\n"
"  --as-toolbar		Set window type to TOOLBAR via EWMH\n"
"  --as-dock		Set window type to DOCK via EWMH\n"
"  --no-override	Suppress window manager control by\n"
"                       inhibiting override-redirect\n"
"  --show-second-hand=0|1 Hide or show the second hand.  Overrides resource.\n"
"  --tz             Set the selected time zone\n"
"  --version, -V        Display version and copyright\n"
"  --help, -h           Usage message\n"
/* "                   (*) Default option\n" */
,
	  g_szApplication);
  return 1;
}  /* do_usage */


int do_version (OPTION*, const char*)
{
  printf ("%s version %s -- %s\n", g_szApplication, g_szVersion,
	  g_szCopyright);
  return 1;
}  /* do_version */


int main (int argc, char** argv)
{
  g_szApplication = parse_application (*argv);

  int argc_used;
  int result = parse_options (argc - 1, argv + 1, rgOptions, &argc_used);

  if (result) {
    switch (result) {
      case 1:
	break;
      default:
	printf ("parse error %d at word %d\n", result, argc_used + 1);
	break;
    }  /* switch */
    return 1;
  }  /* if */

  do_clock ();

  return 0;
}  /* main */
