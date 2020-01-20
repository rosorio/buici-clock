/* larray.h
     $Id: larray.h,v 1.2 1997/10/18 04:57:34 elf Exp $

   written by Marc Singer
   9 October 1997
   
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

*/

#ifndef __LARRAY_H__
#define __LARRAY_H__

// ----- Inclusions

#include <memory.h>

// ----- Constants

// ----- Typedefs

// ----- Classes

class LArray
{
protected:
  int m_cbEach;				// Size of each item is bytes
  int m_cStep;                          // Allocation increment
  long m_cItems;			// Number of filled items in array
  long m_cItemMax;			// Size of allocation
  void* m_rgItems;                      // Item array

  void* _at (long iItem) const {
    return (void*) ((char*) m_rgItems + iItem*m_cbEach); }
  int _at (const void* pv) const {
    return (((const char*) pv) - ((const char*) m_rgItems))/m_cbEach; }

public:
  LArray () {
    zero (); init (sizeof (void*), 4); }
  LArray (int cbEach) {
    zero (); init (cbEach); }
  LArray (int cbEach, int cStep) {
    zero (); init (cbEach, cStep); }
  ~LArray () { release (); }

  void init (int cbEach, int cStep = 4);
  void release (void);
  void zero (void) { memset (this, 0, sizeof (*this)); }

  bool add (const void* pv);
  void* add_new (void);			// Add and return pointer to it
  bool alloc (long cItems);
  bool alloc_to (long cItems);
  void* at (long iItem) const;
  long at (const void* pv) const;
  void attach (void*, long cb);
  void* base (void) const {
    return m_rgItems; }
  int count_each (void) {
    return m_cbEach; }
  bool contains (const void* pv);
  void* enumerate (const void* pv);
  void* enumerate_reverse (const void* pv);
  bool get (long iItem, void* pv);
  long get_count () const { return m_cItems; }
  void* insert (const void*);
  void* reference (long iItem);
  void remove (const void* pv); 
  void remove (long iItem, long cItems = 1); 
  bool set (long iItem, const void* pv);
  void truncate (long cItems);
};

// ----- Macros

// ----- Globals / Externals

// ----- Prototypes

// ----- Inline

#endif		// __LARRAY_H__
