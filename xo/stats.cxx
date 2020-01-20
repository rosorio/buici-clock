/* stats.cxx
     $Id: stats.cxx,v 1.4 2000/01/13 07:04:46 elf Exp $

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


   Support for (simple) statistical analysis of program performance
   data.

*/

  /* ----- Includes */

#include "standard.h"
#include "stats.h"
#include "math.h"
#include "sys/time.h"
#include <assert.h>


  /* ----- Class Globals/Statics */

  /* ----- Methods */


void LStatistics::calc (float& rMean, float& rStddev, long& min, long& max)
{
  double sum_xx = 0.0;
  double sum_x = 0.0;
  min = LONG_MAX;
  max = LONG_MIN;

  //  TraceMsg (NULL, "stddev %d samples", count ());

  for (int c = count (); c--; ) {
    sum_xx += (double) m_rgSamples[c]*(double) m_rgSamples[c];
    sum_x  += (double) m_rgSamples[c];
    if (m_rgSamples[c] > max)
      max = m_rgSamples[c];
    if (m_rgSamples[c] < min)
      min = m_rgSamples[c];
  }

  double n = (double) count ();
  rMean   = float (sum_x/n);
  rStddev = float (sqrt ((sum_xx - (sum_x*sum_x)/n)/(n - 1.0)));
}



float LStatistics::mean (void) 
{
  //  TraceMsg (NULL, "mean has %d samples", count ());
  double value = 0.0;
  for (int c = count (); c--; )
    value += (double) m_rgSamples[c];
  return float (value/(double) count ());
}


/* LStatistics::sample

   collects a sample.  It save the last 
     sizeof (m_rgSamples)/sizeof (long)
   samples.

*/

void LStatistics::sample (long sample)
{
  int i = int (((m_cSamples++) % (sizeof (m_rgSamples)/sizeof (long))));
  assert (i < C_STAT_SAMPLES);
  m_rgSamples[i] = sample;
}


/* LStatistics::stddev

   calculates the sample standard deviation where the samples
   represent a subset of a larger population.  The compuation has been
   simplified algebraicly from the definition of the standard
   deviation.

*/

float LStatistics::stddev (void)
{
  double sum_xx = 0.0;
  double sum_x = 0.0;
  double n = (double) count ();

  //  TraceMsg (NULL, "stddev %d samples", count ());

  for (int c = count (); c--; ) {
    sum_xx += (double) m_rgSamples[c]*(double) m_rgSamples[c];
    sum_x  += (double) m_rgSamples[c];
  }
  return float (sqrt ((sum_xx - (sum_x*sum_x)/n)/(n - 1.0)));
}

long LTime::delta (void)
{
  //  struct tms l_times;
  //  struct rusage l_usage;
  struct timeval l_timeval;

  //  times (&l_times);

  //  getrusage (RUSAGE_SELF, &l_usage);
  //  return (l_usage.ru_utime.tv_sec - m_usage.ru_utime.tv_sec)*1000000L
  //    + (l_usage.ru_utime.tv_usec - m_usage.ru_utime.tv_usec);

  gettimeofday (&l_timeval, NULL);
  return (l_timeval.tv_sec - m_timeval.tv_sec)*1000000L
    + (l_timeval.tv_usec - m_timeval.tv_usec);
}


void LTime::reset (void)
{
  //  times (&m_times);
  //  getrusage (RUSAGE_SELF, &m_usage);
  gettimeofday (&m_timeval, NULL);
}
