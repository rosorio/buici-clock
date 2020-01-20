/* lhash.h				Emacs, this file is -*- C++ -*-
     $Id: lhash.h,v 1.4 1997/10/18 04:57:36 elf Exp $

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

#ifndef __LHASH_H__
#define __LHASH_H__

/* ----- Inclusions */

#include "larray.h"

/* ----- Constants */

#define LOWORD(v) ((v)&0xffff)
#define HIWORD(v) (((v)>>16)&0xffff)
#define MAKELONG(l,h) ((((h) & 0xffff)<<16) | ((l) & 0xffff))

/* ----- Typedefs */

typedef unsigned long HashKey;

/* ----- Classes */

class LHashBucket {
private:
  LArray* m_pArrayItem;		// Array of data items (user specified lengths)
  LArray* m_pArrayKey;		// Array of keys
  int m_cbEach;			// Size of user items

  int _find (HashKey);

public:
  LHashBucket (int cbEach) {
    zero (); init (cbEach); }
  ~LHashBucket () {
    release_all (); }
  void init (int);
  void release_all (void);
  void zero (void);

  bool add (HashKey, const void*);
  void* find (HashKey);
  bool find (HashKey, void*);
  bool is_init (void) {
    return m_cbEach != 0; }
  long get_count (void) {
    return m_pArrayKey->get_count (); }
  HashKey get_key (long i) {
    HashKey* pKey = (HashKey*) m_pArrayKey->at (i);
    return pKey ? *pKey : 0; }
  void* reference_item (long i) {
    return m_pArrayItem->reference (i); }
  bool remove (HashKey);
};

class LHashTable {
private:
  LArray* m_pArrayBucket;	// Array of hash buckets

  int m_cBuckets;		// Number of buckets in table
  int m_cbEach;			// Size of user's data item

public:
  LHashTable () {
    zero (); init (0, 0); }
  LHashTable (int cbEach, int cBuckets) {
    zero (); init (cbEach, cBuckets); }
  ~LHashTable () {
    release_all (); }
  bool is_init (void) {
    return m_cbEach != 0 && m_cBuckets != 0; }
  void init (int, int);
  void release_all (void);
  void zero (void);

  bool add (HashKey, const void*);
  int bucket_of (HashKey key) {
    return (int) (m_cBuckets
		  ? ((LOWORD (key) + HIWORD (key)) % m_cBuckets)
		  : 0); }
  unsigned32 enumerate_reverse (	// Enumerate ALL keys in table
				unsigned32 index, 
				HashKey* pKey, 
				const void** ppv);
  void* find (HashKey);
  bool find (HashKey, void*);
  static HashKey make_string_key (const char*);
  static HashKey make_string_key (const char*, int);
  static HashKey make_case_string_key (const char*);
  static HashKey make_case_string_key (const char*, int);
  bool remove (HashKey);
};

#endif		/* __LHASH_H__ */
