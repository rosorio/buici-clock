/* lwindowclass.h						-*- C++ -*-
   $Id: lwindowclass.h,v 1.3 1997/10/18 04:57:40 elf Exp $
   
   written by Marc Singer
   9 May 1997

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

#if !defined (__LWINDOWCLASS_H__)
#    define   __LWINDOWCLASS_H__

/* ----- Includes */

#include "lhash.h"

/* ----- Classes */

class LDisplay;
class LWindow;

typedef void (*PFNEvent) (LWindow* pWindow, XEvent* pEvent);
typedef struct {
  int event_type;
  PFNEvent pfn;
} EventHandlerMap;


class LWindowClass {
protected:
  static LHashTable g_hashClass; // szClass->LWindowClass*

  char* m_szName;		// Class name, case insensitive
  EventHandlerMap* m_pEventHandlerMap;

  XSetWindowAttributes m_attribute;
  unsigned long m_attribute_mask;

  void* m_pvUser;		// Associated instance specific data

public:
  LWindowClass () {
    zero (); }
  ~LWindowClass () {
    release_this (); }
  void zero (void) {
    memset (this, 0, sizeof (*this)); }
  bool register_class (char* szName, 
		       long event_mask, EventHandlerMap* pEventHandlerMap);
  void release_this (void) {}

  static LWindowClass* find_class (char* szName);

  unsigned long attributes (XSetWindowAttributes* pAttribute) {
    memcpy (pAttribute, &m_attribute, sizeof (m_attribute));
    return m_attribute_mask; }
  void* data (void) {
    return m_pvUser; }

  EventHandlerMap* event_handler_map (void) {
    return m_pEventHandlerMap; }

  void set_background_pixel (unsigned long pixel) {
    m_attribute.background_pixel = pixel;
    m_attribute_mask |= CWBackPixel; }

  void set_bit_gravity (int gravity) {
    m_attribute.bit_gravity = gravity;
    m_attribute_mask |= CWBitGravity; }

  void* set_data (void* pv) {
    void* pvPrev = m_pvUser; m_pvUser = pv; return pvPrev; }
};


#endif  /* __LWINDOWCLASS_H__ */
