/* standard.h						-*- C++ -*-
     $Id: standard.h,v 1.3 1998/10/15 07:18:04 elf Exp $

   written by Marc Singer
   3 April 1997

   This file is part of the project DeltaCine.  See the file README for
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

   --- Introduction

   Standard header for project.
*/

#ifndef __STANDARD_H__
#define __STANDARD_H__

// ----- Inclusions

#include "config.h"		// autoconf configuration header

#if defined (HAVE_UNISTD_H)
# include <unistd.h>
#endif
#if defined (STDC_HEADERS)
# include <stdio.h>
# include <stdlib.h>
#endif

#include "assert.h"
//#include "dmalloc.h"

#include "options.h"

// ----- Constants

// ----- Typedefs

typedef signed char int8;
typedef unsigned char unsigned8;
typedef short int16;
typedef unsigned short unsigned16;
typedef signed long int32;
typedef signed long long int64;
typedef unsigned long unsigned32;
typedef unsigned long long unsigned64;

// ----- Classes

// ----- Macros

#define _memmove(d,s,c,i) { fprintf (stdout,"<%d", i); fflush (stdout); \
			    memmove ((d),(s),(c)); \
			    fprintf (stdout,">"); fflush (stdout); }

// ----- Globals / Externals

extern const char* g_szRelease;

// ----- Prototypes

// ----- Inline

#endif		// __STANDARD_H__
