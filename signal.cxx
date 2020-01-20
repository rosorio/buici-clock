/* signal.cxx
     $Id: signal.cxx,v 1.1 1997/10/12 19:57:37 elf Exp $

   written by Marc Singer
   18 October 1996

   This file is part of the project Buici (taken from CurVeS).  See
   the file README for more information.

   Copyright (C) 1996-1997 Marc Singer

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

   Simple signal wrapper.  This is interesting only because we plan to
   port this application.  Also, it helps to make the interface to
   signals a little more obvious.  For example, we only care about
   certain kinds of signals and we want to be notified in the right
   way for these.  Since we may need to adjust the signal action
   records when the signals are received and services, it is cleaner
   to do this here, in one place, instead of forcing the signal user
   to know the semantics of the signaling.

   This may seem to be overkill for a simple application as we have
   here.  The goal is to understand signals well and to lay the
   foundation for more sophisticated applications that will build on
   this understanding.  The chaining may be very useful when we need
   to specify clean-up handlers for allocated objects and these
   objects may not know about each other.

   -- Chaining signals

   The services provided by the LSignal class is that of chaining
   signal handlers.  The default signal handling for a UN*X process
   links a function pointer to a signal.  However, we want to cast
   signals into an object space which requires a function pointer and
   a context pointer.  The signal code generates lists of handlers for
   each signal and calls them in rank order.  The caller must provide
   an order by specifying a rank when declaring the handler.  These
   ranks have no meaning to the signal code except to allow them to be
   prioritized.  For the most part, signal handlers will be singular
   for each signal, but we provide this inexpensive service with the
   hope that the LSignal interface proves useful.

   ** It appears to be true that signal handlers that modify their
   status while being processed are incompatible with those that do
   not.  I am leaning toward a protocol where handlers, such as the
   suspend handler, that must disable and then raise themselves during
   processessing must be last in their chain.  This allows us to
   modify the handler structures by noting that a handler is the last
   in the list and therefore disregard the handler list after
   processing it.

   -- Signal Types

   I've chosen to start with the native signal types instead of
   abstracting the signal function to another type.  Why?  I figure
   that the signal types will be few and generally supported by OSs
   we're likely to support with the exception of everything written by
   Mxcrxsxft.  If it is necessary to recode the signal types into an
   enumeration it will be an easy task.

   -- Race Conditions

   Since we permit several handlers per signal and since some signals
   are asynchronous and unpredictable, we may choose (not yet decided)
   to limit the number of handlers for some signals to one.  The other
   option is to attempt to block the signal being modified until the
   structures are updated.  Some signals may not be blocked and may,
   therefore, preclude this tactic.

   It turns out that GNU glibc and POSIX guarantee that pointer
   accesses are atomic which means that the race conditions mentioned
   above do not exist.  Since we use a linked list of handlers, there
   is only one store used to link or unlink a LSignalHandlerInstance
   which means that the accept/release operations are safe.

   -- LSignalHandle

   Presently, this is a very cheesey cookie that turns the address of
   the signal number into a LSignalHandle to give to the caller.  The
   user can recoved the signal number, if needed, by calling back to
   the signal class.  However, we may decide to put more information
   in this handle when we determine if we will handle some of the
   signals that sport more than one parameter such as the floating
   point error signal, SIGFPE.

   -- Modifying signal handlers while processing signals

   We need a special version of 

*/

#include "standard.h"
#include "signal.h"
#include <string.h>


LSignalHandlerInstance* LSignal::g_rgpHandler[NSIG];
struct sigaction LSignal::g_rgAction[NSIG];
sigset_t LSignal::g_setEnable;


/* LSignal::accept

   adds a caller's signal handler to the list of handlers for a
   signal.  There are several race conditions that must be addressed,
   but we may finesse this for now.

   The return value is non-zero if there is an error.

*/

int LSignal::accept (int signal, LSignalHandler pfn, void* pv,
		     int rank, int flags)
{
				// -- Stupid checks, but cheap
  if (!pfn || signal < 1 || signal > NSIG)
    return 1;
  
  LSignalHandlerInstance* pSig
    = (LSignalHandlerInstance*) malloc (sizeof (LSignalHandlerInstance));
  assert (pSig);
  memset (pSig, 0, sizeof (*pSig));

  pSig->pfn = pfn;
  pSig->pv = pv;
  pSig->rank = rank;
  pSig->flags = flags;

				// -- Find it's place in the handler list
  LSignalHandlerInstance** ppSig = &g_rgpHandler[signal];
  while (*ppSig && (*ppSig)->rank < pSig->rank)
    ppSig = &(*ppSig)->pNext;

				// -- Link
  pSig->pNext = (*ppSig);
  *ppSig = pSig;

  enable (signal);
  return 0;
}


/* LSignal::block

   this is supposed to do something so that we don't receive this signal
   until it is unblocked.

*/

void LSignal::block (int /* signal */)
{
}


void LSignal::disable (int signal)
{
  if (!sigismember (&g_setEnable, signal))
    return;
  sigdelset (&g_setEnable, signal);

  sigaction (signal, &g_rgAction[signal], NULL);
}


void LSignal::enable (int signal)
{
  if (sigismember (&g_setEnable, signal))
    return;
  sigaddset (&g_setEnable, signal);

  sigaction (signal, NULL, &g_rgAction[signal]); // Save previous action

  struct sigaction action;
  memset (&action, 0, sizeof (action));
  action.sa_handler = handler;
  action.sa_flags  |= SA_RESTART;
  sigemptyset (&action.sa_mask);
  sigaction (signal, &action, NULL);
}


/* LSignal::handler

   single, reentrant handler for all signals.  The check for last'ness
   is necessary to allow the last handler in the chain to manipulate
   the handler's status while it is being processed.

*/

void LSignal::handler (int signal)
{
  bool fLast = false;
  for (LSignalHandlerInstance** ppSig = &g_rgpHandler[signal];
       !fLast && *ppSig; ppSig = &(*ppSig)->pNext) {
    if (!(*ppSig)->pNext)
      fLast = true;
    (*ppSig)->pfn ((LSignalHandle) &signal, (*ppSig)->pv);
  }
}


/* LSignal::release

   removes a signal handler instance.  The return value is non-zero if
   there is an error.

*/

int LSignal::release (int signal, LSignalHandler pfn, void* pv)
{
				// -- Stupid checks, but cheap
  if (!pfn || signal < 1 || signal > NSIG)
    return 1;
  
				// -- Look for the handler of interest
  LSignalHandlerInstance** ppSig = &g_rgpHandler[signal];
  while (*ppSig
	 && (*ppSig)->pfn != pfn
	 && (*ppSig)->pv  != pv)
    ppSig = &(*ppSig)->pNext;
  if (!*ppSig)
    return 1;

				// -- Unlink and release
  LSignalHandlerInstance* pSig = *ppSig;
  *ppSig = pSig->pNext;
  free (pSig);
  if (!*ppSig)
    disable (signal);

  return 0;
}


/* LSignal::signal_of

   recovers the signal number from the signal handle.  

*/

int LSignal::signal_of (LSignalHandle handle)
{
  if (!handle)
    return -1;
  return *((int*) handle);
}


/* LSignal::unblock

   this is supposed to do something so that we don't receive a signal
   while the data structure is being adjusted.

*/

void LSignal::unblock (int /* signal */)
{
}
