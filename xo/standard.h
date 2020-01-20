/* standard.h
   $Id: standard.h,v 1.7 1997/10/23 06:00:47 elf Exp $

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

#if !defined (__STANDARD_H__)
#    define   __STANDARD_H__

/* ----- Includes */

#include "config.h"		// autoconf configuration header

#if defined (STDC_HEADERS)
# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
#endif
#if defined (HAVE_UNISTD_H)
# include <unistd.h>
#endif

#if defined (HAVE_LIMITS_H)
# include "limits.h"
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>

//#include "dmalloc.h"

// ----- Typedefs

typedef signed char int8;
typedef unsigned char unsigned8;
typedef short int16;
typedef unsigned short unsigned16;
typedef signed long int32;
typedef signed long long int64;
typedef unsigned long unsigned32;
typedef unsigned long long unsigned64;


#endif  /* __STANDARD_H__ */
