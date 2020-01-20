/* larray.cxx
     $Id: larray.cxx,v 1.2 1997/10/18 04:57:33 elf Exp $

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

   -- IMPLEMENTATION NOTE --
   The m_cStep member is set to -1 if a request was made to allocate
   the array larger than 64 AND we are not using GCC or Win32.  Once
   allocated, we do not permit reallocation.

*/

  /* ----- Includes */

#include "standard.h"
#include "larray.h"

#include <memory.h>
#include <assert.h>

#define max(a,b) (((a) >= (b)) ? (a) : (b))
#define min(a,b) (((a) <= (b)) ? (a) : (b))


  /* ----- Class Globals/Statics */

  /* ----- Methods */

/* LArray::add

   adds an item to the end of the array.

*/

bool LArray::add (const void* pv)
{
  if (!alloc (m_cItems + 1))
    return false;

  memcpy (_at (m_cItems++), pv, m_cbEach);
  return true;
}  /* LArray::add */


/* LArray::add_new

   adds an item to the end of the array and returns pointer to it.

*/

void* LArray::add_new (void)
{
  if (!alloc (m_cItems + 1))
    return NULL;

  return _at (m_cItems++);
}  /* LArray::add_new */


/* LArray::alloc

   tries to make space for cItems in the array.  If there is already
   enough space, nothing is done.  The return value is true if there
   is enough room for cItems.

*/

bool LArray::alloc (long cItems)
{
  assert (m_cbEach);

  if (m_cStep < 0)
    return false;

  if (m_cStep == 0)
    m_cStep = 1;
  cItems = ((cItems + m_cStep - 1)/m_cStep)*m_cStep;
  if (m_cItemMax >= cItems) {
    assert (m_cItemMax == 0 || m_rgItems);
    return true;
  }  /* if */

  if (m_rgItems) {
    void* pItems = realloc (m_rgItems, size_t (m_cbEach*cItems));
    if (!pItems)
      return false;
    m_rgItems = pItems;
    memset (_at (m_cItemMax), 0, size_t (m_cbEach*(cItems - m_cItemMax)));
  }  /* if */
  else
    m_rgItems = calloc ((size_t) cItems, m_cbEach);
  if (m_rgItems)
    m_cItemMax = cItems;
  return m_rgItems != NULL;
}  /* LArray::alloc */


/* LArray::alloc_to

   allocates the array to include at least cItems and it also sets
   the current number of filled items to cItems.  The effect is the
   same as adding cItems worth of empty cells, but this method is
   faster.

*/

bool LArray::alloc_to (long cItems)
{
  if (!alloc (cItems) && m_cItemMax < cItems)
    return false;
  m_cItems = max (m_cItems, cItems);
  return true;
}  /* LArray::alloc_to */
    

/* LArray::at

   routines to convert indicies and pointers into pointers and
   indicies.  Note that these are NOT inline so we can use dynamic
   linking to change them.

*/

void* LArray::at (long iItem) const
{
  return _at (iItem);
}  /* LArray::at */

long LArray::at (const void* pv) const
{
  if (pv < m_rgItems || pv >= _at (m_cItems))
    return -1;
  return _at (pv);
}  /* LArray::at */

/* LArray::attach

   uses an external data pointer for the array storage.  This
   automatically sets the m_cbStep to -2 which indicates that the
   array does not own the storage.  LArray uses the cb parameter to
   compute the number of available slots for the array.

*/

void LArray::attach (void* pv, long cb)
{
  release ();
  m_rgItems = pv;
  m_cStep = -2;
  m_cItemMax = cb/m_cbEach;
}  /* LArray::attach */

/* LArray::contains

   returns true if the array contains en element matching the data
   pointed to by *pv.  Note that this is a SLOW operation requiring a
   comparison with every element in the array.

*/

bool LArray::contains (const void* pv)
{
  for (void* pvElement = NULL; pvElement = enumerate (pvElement); )
    if (memcmp (pvElement, pv, m_cbEach) == 0)
      return true;
  return false;
}  /* LArray::contains */


/* LArray::enumerate and enumerate_reverse

   enumerates a reference pointer to the items in the list.  The
   return value points to the next item in the array.  To enumerate
   all items in the array, pass NULL the first time and the last
   returned pointer on successive calls.  Enumerate is complete when
   the function returns NULL.

   The variant enumerate_reverse enumerates from the end of the list.
   It is useful when deleting items from the array because the
   enumeration pointer will be properly decremented on each iteration.

   These functions enumerate pointers to the elements of the array.
   This means that in order to enumerate the elements of an array of
   character pointers, you need to construct the loop this way:

  for (char** ppch = NULL; ppch = (char**) array.enumerate ((void*) ppch);) {
    printf ("Got the string '%s'\n", *ppch);

*/

void* LArray::enumerate (const void* pv)
{
  if (!pv)
    return m_cItems ? m_rgItems : NULL;

  if (pv < m_rgItems)		// Error case
    return NULL;

  long iItem = _at (pv);
  if (iItem < 0 || iItem >= m_cItems)
    return NULL;
  ++iItem;
  return (iItem < m_cItems) ? _at (iItem) : NULL;
}  /* LArray::enumerate */

void* LArray::enumerate_reverse (const void* pv)
{
  if (!pv)
    return m_cItems ? _at (m_cItems - 1) : NULL;

  if (pv < m_rgItems)		// Error case
    return NULL;

  long iItem = _at (pv) - 1;
  return (iItem >= 0 && iItem < m_cItems) ? _at (iItem) : NULL;
}  /* LArray::enumerate_reverse */


/* LArray::get

   gets an item in the array.  m_cbEach bytes are copied from index
   iItem.  The return value is true if the item was copied.

*/

bool LArray::get (long iItem, void* pv)
{
  if (iItem < 0 || iItem >= m_cItems || !pv)
    return false;
  memcpy (pv, _at (iItem), m_cbEach);
  return true;
}  /* LArray::get */


/* LArray::init

   initializes the array.  cbEach is the size of each item and cStep
   is the number of items to allocate when the array is full.

*/

void LArray::init (int cbEach, int cStep)
{
  release ();

  m_cbEach = cbEach;
  m_cStep = cStep;
}  /* LArray::init */

 
/* LArray::insert

   makes space before the indicated item for a new item in the array.
   Note that this function has the side-effect of changing the
   addresses of the items that follow and potentially of all items if
   the array is realloc'd in a new memory location.  The return value
   is the pointer to the new, blank item or NULL if the memory cannot
   be allocated for it.

*/

void* LArray::insert (const void* pv)
{
  assert (m_cbEach);

  long iInsert = at (pv);
  if (iInsert == -1)
    return NULL;

  if (!alloc_to (m_cItems + 1))
    return NULL;
	// (m_cItems has been incremented)

	// -- Move data after iInsert to accomodate new entry
  pv = _at (iInsert);
  memmove (((char*) pv) + m_cbEach,
           ((char*) pv),
           size_t ((m_cItems - 1 - iInsert)*m_cbEach));
//  ++m_cItems;

  return (void*) pv;
}  /* LArray::insert */


/* LArray::reference

   returns a pointer to the specified item.  If the index is out of
   range, the return value is NULL.

*/

void* LArray::reference (long iItem)
{
  if (iItem < 0 || iItem >= m_cItems)
    return NULL;
  return _at (iItem);
}  /* LArray::reference */


/* LArray::release

   releases memory allocated for the array.  Note that if the array
   stores pointers, those pointers are not freed.

*/

void LArray::release (void)
{
  if (m_rgItems) {
    if (m_cStep >= 0)
      free (m_rgItems);
  }  /* if */

  m_rgItems = NULL;
  m_cItems = m_cItemMax = 0;
}  /* LArray::release */


/* LArray::remove with variants

   removes cItems from the list at offset iItem.  The size of the
   allocation does not change.  The variant that takes a void*
   pointer removes only that item from the list.
   
*/

void LArray::remove (long iItem, long cItems)
{
  iItem = min (m_cItems - 1, iItem);
  if (iItem < 0)
    return;
  cItems = min (m_cItemMax - iItem, cItems);

  memmove (_at (iItem), _at (iItem + cItems),
           size_t ((m_cItems - iItem - cItems)*m_cbEach));
  m_cItems -= cItems;
}  /* LArray::remove */


void LArray::remove (const void* pv)
{
  if (!m_cItems)
    return;
  
  long iItem = at (pv);
  if (iItem == -1)
    return;
  
  long cItems = 1;

  memmove (_at (iItem), _at (iItem + cItems),
           size_t ((m_cItems - iItem - cItems)*m_cbEach));
  m_cItems -= cItems;
}  /* LArray::remove */


/* LArray::set

   sets the data for an item in the array.

*/

bool LArray::set (long iItem, const void* pv)
{
  if (!alloc_to (iItem))
    return false;

  memcpy (_at (iItem), pv, m_cbEach);
  return true;
}  /* LArray::set */


/* LArray::truncate

   truncates the array at cItems items.  This is used to effectively
   shrink array usage when the array might grow again.

*/

void LArray::truncate (long cItems)
{
  cItems = max (0, cItems);
  m_cItems = min (m_cItems, cItems);
}  /* LArray::truncate */
