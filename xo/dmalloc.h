/* dmalloc.h
   $Id: dmalloc.h,v 1.4 1997/10/18 04:57:33 elf Exp $
   
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

   Debugging memory allocator.

*/

#if !defined (__DMALLOC_H__)
#    define   __DMALLOC_H__

/* ----- Includes */

#include "memory.h"

/* ----- Globals */

#ifdef __cplusplus
extern "C" {
#endif

void* _malloc_ (size_t cb, const char*, int);
void* _calloc_ (size_t c, size_t cb, const char*, int);
void* _realloc_ (void* pv, size_t cb, const char*, int);
void  _free_ (void* pv);

void dmalloc_exit (void);
void dmalloc_validate (void);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
void* operator new (size_t, const char*, int);
void  operator delete (void*);
#endif

#if !defined (BY_DMALLOC)
# define new		new(__FILE__,__LINE__)
# define calloc(c,cb)	_calloc_((c),(cb),__FILE__,__LINE__)
# define malloc(cb)	_malloc_((cb),__FILE__,__LINE__)
# define realloc(pv,cb)	_realloc_((pv),(cb),__FILE__,__LINE__)
# define free		_free_
#endif


#endif  /* __DMALLOC_H__ */
