/* lhash.cxx
     $Id: lhash.cxx,v 1.4 1997/10/18 04:57:35 elf Exp $

   written by Marc Singer
   15 October 1997
   
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

  /* ----- Includes */

#include "standard.h"

#include <ctype.h>

#include "lhash.h"

  /* ----- Class Globals/Statics */

  /* ----- Methods */


bool LHashBucket::add (HashKey key, const void* pv)
{
  int i = _find (key);

	// -- Add new entry
  if (i == -1)
    return m_pArrayKey->add (&key)
        && m_pArrayItem->add (pv);

	// -- Replace old entry
  void* pvItem = m_pArrayItem->reference (i);
  if (pvItem)
    memcpy (pvItem, pv, m_cbEach);
  return pvItem != NULL;
}  /* LHashBucket::add */


/* LHashBucket::_find

   performs the internal lookup within a bucket.  We do a simple
   linear search of the items in the list.  The return value is the
   index of the item or -1 if the key cannot be found.

*/

int LHashBucket::_find (HashKey key)
{
  if (!is_init ())
    return -1;
  
  int i = 0;
  for (HashKey* pKey = NULL;
	 pKey = (HashKey*) m_pArrayKey->enumerate ((void*) pKey); ++i)
    if (*pKey == key)
      return i;
  return -1;
}  /* LHashBucket::_find */

bool LHashBucket::find (HashKey key, void* pv)
{
  int i = _find (key);
  if (i == -1)
    return false;
  return m_pArrayItem->get (i, pv);
}  /* LHashBucket::find */


void* LHashBucket::find (HashKey key)
{
  int i = _find (key);
  if (i == -1)
    return NULL;
  return m_pArrayItem->reference (i);
}  /* LHashBucket::find */


void LHashBucket::init (int cbEach)
{
  release_all ();
  m_cbEach = cbEach;
  m_pArrayItem = new LArray (cbEach);
  m_pArrayKey  = new LArray (sizeof (HashKey));
}  /* LHashBucket::init */

void LHashBucket::release_all (void)
{
  if (m_pArrayKey)
    delete m_pArrayKey;
  m_pArrayKey = NULL;
  if (m_pArrayItem)
    delete m_pArrayItem;
  m_pArrayItem = NULL;
  m_cbEach = 0;
}  /* LHashBucket::release_all */

bool LHashBucket::remove (HashKey key)
{
  int i = _find (key);

  if (i == -1)
    return false;

  m_pArrayKey->remove (i);
  m_pArrayItem->remove (i);
  return true;
}  /* LHashBucket::remove */


void LHashBucket::zero (void)
{
  memset (this, 0, sizeof (*this));
}  /* LHashBucket::zero */


bool LHashTable::add (HashKey key, const void* pv)
{
  LHashBucket* pBucket
      = (LHashBucket*) m_pArrayBucket->reference (bucket_of (key));
  if (!pBucket)
    return false;
  if (!pBucket->is_init ())
    pBucket->init (m_cbEach);
  return pBucket->add (key, pv);
}  /* LHashTable::add */


/* LHashTable::enumerate_reverse

   enumerates all keys in the hash table.  It is called _reverse
   because it uses to the same technique as LArray::enumerate_reverse
   that guarantees that deleted items during the enumeration will not
   affect the completeness of the coverage.

   The unsigned32 index controls the enumeration.  For the first call,
   pass index as zero.  On subsequent calls, pass the index as
   returned from the previous call.  The return value is zero when
   enumeration is complete.

*/

unsigned32 LHashTable::enumerate_reverse (unsigned32 index, HashKey* pKey, 
				    const void** ppv)
{
  LHashBucket* pBucket;
  unsigned iBucket = HIWORD (index);
  unsigned long iKey = LOWORD (index) - 1;

	// -- Initial case
  if (index == 0) {
    iBucket = m_cBuckets;
    iKey = 0;
  }  /* if */

	// -- Iteration across bucket boundary
  if (!iKey) {
    while (iBucket--) {
      pBucket = (LHashBucket*) m_pArrayBucket->reference (iBucket);
      if (!pBucket || !pBucket->is_init ())
	continue;
      iKey = pBucket->get_count ();
      break;
    }  /* while */
    if (!iKey)
      return 0;
  }  /* if */

  --iKey;
  pBucket = (LHashBucket*) m_pArrayBucket->at (iBucket);

  if (pKey)
    *pKey = pBucket->get_key (iKey);
  if (ppv)
    *ppv  = pBucket->reference_item (iKey);

  return MAKELONG (iKey + 1, iBucket);
}  /* LHashTable::enumerate_reverse */


void* LHashTable::find (HashKey key)
{
  LHashBucket* pBucket
      = (LHashBucket*) m_pArrayBucket->reference (bucket_of (key));
  return pBucket ? pBucket->find (key) : NULL;
}  /* LHashTable::find */


bool LHashTable::find (HashKey key, void* pv)
{
  LHashBucket* pBucket
      = (LHashBucket*) m_pArrayBucket->reference (bucket_of (key));
  return pBucket ? pBucket->find (key, pv) : false;
}  /* LHashTable::find */


void LHashTable::init (int cbEach, int cBuckets)
{
  release_all ();
  m_cbEach = cbEach;
  m_cBuckets = cBuckets;
  if (!cBuckets)
    return;
  m_pArrayBucket = new LArray (sizeof (LHashBucket));
  m_pArrayBucket->alloc_to (cBuckets);
}

HashKey LHashTable::make_string_key (const char* sz)
{
  return LHashTable::make_string_key (sz, strlen (sz));
}


HashKey LHashTable::make_string_key (const char* sz, int cch)
{
  unsigned long l = 0;

  while (cch--) {
    l = (l << 4) + (unsigned char)*(sz++);
    unsigned long m = l & 0xf0000000;
    if (m) {
      l = l ^ (m >> 24);
      l = l ^ m;
    }
  }
  return l;
}


HashKey LHashTable::make_case_string_key (const char* sz)
{
  return LHashTable::make_case_string_key (sz, strlen (sz));
}


HashKey LHashTable::make_case_string_key (const char* sz, int cch)
{
  unsigned long l = 0;

  while (cch--) {
    l = (l << 4) + tolower (*(sz++));
    unsigned long m = l & 0xf0000000;
    if (m) {
      l = l ^ (m >> 24);
      l = l ^ m;
    }
  }
  return l;
}


void LHashTable::release_all (void)
{
  if (m_pArrayBucket) {
    for (LHashBucket* pBucket = NULL;
	 pBucket = (LHashBucket*) m_pArrayBucket
	 ->enumerate ((void*) pBucket); )
      pBucket->release_all ();
    delete m_pArrayBucket;
    m_pArrayBucket = NULL;
  }  /* if */
}  /* LHashTable::release_all */


bool LHashTable::remove (HashKey key)
{
  LHashBucket* pBucket
      = (LHashBucket*) m_pArrayBucket->reference (bucket_of (key));
  return pBucket ? pBucket->remove (key) : false;
}  /* LHashTable::remove */


void LHashTable::zero (void)
{
  memset (this, 0, sizeof (*this));
}  /* LHashTable::zero */
