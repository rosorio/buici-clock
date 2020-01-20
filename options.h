/* options.h
     $Id: options.h,v 1.3 1997/10/14 09:11:26 elf Exp $

   written by Marc Singer
   20 April 1996

   This file is part of the project CurVeS.  See the file README for
   more information.

   Copyright (C) 1996 Marc Singer

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

*/

#ifndef __OPTIONS_H__
#define __OPTIONS_H__

// ----- Inclusions

// ----- Constants

// ----- Typedefs

typedef enum {
  OptNonoption	= 0x0001,	// Option to use for non-options
  OptDefault	= 0x0002,	// Option to use when no other found
  OptArg	= 0x0004,	// Option has an argument
  OptCommand	= 0x0008,	// Option is a command, no dash prefix
  OptAllDash	= 0x0010,	// Long option recognized after one dash or two
  OptSetType	= 0x0f00,
  OptSetMask	= 0xff00,
  OptSetString	= 0x0100,
  OptSetInt	= 0x0200,
  OptSetShort	= 0x0400,
  OptSetLong	= 0x0800,
  OptClear	= 0x8000,
} E_Opt;

typedef enum {
  OptErrOk		= 0,
  OptErrFail		= 1,	// Option function failed
  OptErrUnrecognized	= 2,	// Unrecognized option
  OptErrNoArgument	= 3,	// Argument missing
  OptErrBadOption	= 4,	// Error in option descriptions
  OptErrAmbiguous	= 5,	// Ambiguous partial
  OptErrExit		= 9,	// Used internally for quick exit
} E_OptErr;

struct _OPTION;

typedef int (*PFN_OPTION) (struct _OPTION* pOption, const char* pch);

typedef struct _OPTION {
  const char* sz;		// Text of option
  unsigned flags;
  void* pv;			// Pointer to option result
  PFN_OPTION pfn;		// Callback function
} OPTION;

// ----- Classes

// ----- Macros

// ----- Globals / Externals

// ----- Prototypes

char* parse_application (char* pch);
int parse_options (int argc, char** argv, OPTION* rgOptions, int* argc_used);


// ----- Inline

#endif		// __OPTIONS_H__
