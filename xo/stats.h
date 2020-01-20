/* stats.h		(* Microsoft C++ 8.0 *)
     $Id: stats.h,v 1.3 1997/10/18 04:57:43 elf Exp $

   written by Marc Singer
   3 October 1996
   
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

   Support for (simple) statistical analysis of program performance
   data.

*/

#if !defined (__STATS_H__)
#    define   __STATS_H__

  /* ----- Includes */

#include <sys/times.h>
#include <sys/resource.h>
#if defined (HAVE_MEMORY_H)
# include <memory.h>
#endif

  /* ----- Class Globals/Statics */

#define C_STAT_SAMPLES	200	// Samples saved for rolling statistics

  /* ----- Methods */



class LStatistics {
protected:
  unsigned long m_cSamples;		// Number of samples collected
  long m_rgSamples[C_STAT_SAMPLES]; // Sample data

public:

  LStatistics () {
    zero (); }
  ~LStatistics () { }
  void zero () {
    memset (this, 0, sizeof (*this)); }

  void sample (long l);

  long samples (void) {
    return m_cSamples; }
  int count (void) {
    return (m_cSamples > C_STAT_SAMPLES) ? C_STAT_SAMPLES : (int) m_cSamples; }
  float mean (void);
  float stddev (void);
  void calc (float& rMean, float& rStddev, long& min, long& max);
};

class LTime {
protected:
  struct tms m_times;
  struct rusage m_usage;
  struct timeval m_timeval;

public:
  LTime () {
    zero (); }
  ~LTime () {}
  void zero () {
    memset (this, 0, sizeof (*this)); }

  void reset (void);
  long delta (void);

};

#endif  /* __STATS_H__ */

