/* messages.h						-*- C++ -*-
   $Id: messages.h,v 1.2 1997/10/18 04:57:40 elf Exp $
   
   written by Marc Singer
   12 May 1997

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

   We have some of our own messages to piggyback onto the core message
   sending code.  We do this in a simple way.  We define message
   identifiers that cannot practically collide with the protocol
   messages.  We choose to avoid the X client message as it is best
   suited to inter-application messaging which does not suit us.  Ours
   is a more efficient mechanism.

*/

#if !defined (__MESSAGES_H__)
#    define   __MESSAGES_H__

/* ----- Includes */

/* ----- Globals */

#define FIRSTXoEventBase	0x8000
#define CreateSelfNotify	0x8001 // Notify to window that it is created
#define ChildNotify		0x8002 // Button notifying parent of event

typedef struct {
  int type;
  unsigned long serial;		// We don't use this
  Bool send_event;		// We always set this, only because it is true
  Display* display;		// Display where child and this window reside
  Window window;		// Window receiving the event
} XoAnyEvent;

typedef struct {
  int type;
  unsigned long serial;		// We don't use this
  Bool send_event;		// We always set this, only because it is true
  Display* display;		// Display where child and this window reside
  Window window;		// Window receiving the event
			// -- Event specific portion
  Window parent;		// Parent that owns this window
  void* pvParentData;		// Context pointer returned to parent on notify
  int x, y;			// Window origin relative to parent
  int width, height;		// Window extent
  int border_width;		// I'm not sure if this is important
} XoCreateSelfWindowEvent;

typedef struct {
  int type;
  unsigned long serial;		// We don't use this
  Bool send_event;		// We always set this, only because it is true
  Display* display;		// Display where child and this window reside
  Window window;		// Window receiving the event
			// -- Event specific portion
  LWindow* pWindowChild;	// Child window sending the event
  void* pvParentData;		// Context pointer, a type of ID
  int child_type;		// Child's event type, class specific
  int rgData[4];		// Some data from the child
} XoChildNotifyEvent;

typedef union {
  int type;
  XoAnyEvent xany;
  XoCreateSelfWindowEvent xcreatewindow;
  XoChildNotifyEvent xchildnotify;
} XoEvent;

#endif  /* __MESSAGES_H__ */
