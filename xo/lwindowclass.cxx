/* lwindowclass.cxx
     $Id: lwindowclass.cxx,v 1.2 1997/10/18 04:57:39 elf Exp $

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

#include "standard.h"
#include "lwindowclass.h"

LHashTable LWindowClass::g_hashClass; // szClass->LWindowClass*

LWindowClass* LWindowClass::find_class (char* szName)
{
  HashKey key = LHashTable::make_case_string_key (szName);
  LWindowClass* pClass = NULL;
  g_hashClass.find (key, &pClass);
  return pClass;
}


bool LWindowClass::register_class (char* szName, long event_mask,
				   EventHandlerMap* pEventHandlerMap)
{
  if (!g_hashClass.is_init ())
    g_hashClass.init (sizeof (LWindowClass*), 101);

  HashKey key = LHashTable::make_case_string_key (szName);
  LWindowClass* pClass = NULL;
  g_hashClass.find (key, &pClass);
  
  if (pClass)
    return false;

  m_szName = (char*) malloc (strlen (szName) + 1);
  strcpy (m_szName, szName);
  m_pEventHandlerMap = pEventHandlerMap;
  m_attribute.event_mask = event_mask;
  m_attribute_mask |= CWEventMask;

  pClass = this;
  g_hashClass.add (key, &pClass);

  return true;
}
