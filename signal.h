/* signal.h				-*- C++ -*-
   $Id: signal.h,v 1.1 1997/10/12 19:57:37 elf Exp $
   
   written by Marc Singer
   19 Oct 1996

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

#if !defined (__SIGNAL_H__)
#    define   __SIGNAL_H__

/* ----- Includes */

#include <signal.h>

/* ----- Globals */

typedef void* LSignalHandle;		// Encapsulated signal handle
typedef void (*LSignalHandler) (LSignalHandle,
				void*);	// Encapsulated signal handler

typedef struct _LSignalHandlerInstance {
  struct _LSignalHandlerInstance* pNext; // Link pointer (must be first)
  LSignalHandler pfn;			// Caller's function pointer
  void* pv;				// Caller's context pointer
  int rank;				// Handler's rank among like handlers
  int flags;				// Some flags, if we need them
} LSignalHandlerInstance;


class LSignal {
protected:
  static LSignalHandlerInstance* g_rgpHandler[NSIG];
  static struct sigaction g_rgAction[NSIG];
  static sigset_t g_setEnable;		// Set when our handler is enabled

  static void handler (int signal);	// Reentrant, OS signal handler

  static void disable (int signal);
  static void enable (int signal);

  static void block (int signal);
  static void unblock (int signal);

public:  
  static int accept (int signal, LSignalHandler pfn, void* pv,
		     int rank, int flags); 
  static int release (int signal, LSignalHandler pfn, void* pv);
  static int signal_of (LSignalHandle handle);
};


#endif  /* __SIGNAL_H__ */
