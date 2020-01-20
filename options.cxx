/* options.cxx
     $Id: options.cxx,v 1.4 1998/10/14 00:40:07 elf Exp $

   written by Marc Singer
   20 April 1996

   This file is part of the project Buici.  See the file README for
   more information.

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

   ---------------------
   ABBREVIATED CHANGELOG
   ---------------------

   Option parsing and value storage.

   Version 0.4 (13 October 1997)
     Allow long options to be prefixed by a single dash.  This is for
     X-Windows compatibility.  This entails some fancy footwork for
     option parsing and the addition of an option flag to permit
     recognition of long options when looking for short ones.
   Version 0.3 (14 November 1996)
     Permit option arguments to be prefixed by '='.
   Version 0.2 (14 May 1996)
     Long option names prefixed by a period are interpreted without a
     dash.  This allows command parsing as well as option parsing.
   Version 0.1
     Short and long options.  Default and error option codes.  Long
     option prefix matching.

   -----
   NOTES
   -----

   - We should detect ambiguous partials.  We need to change
     fetch_option to return the error code and not the OPTION pointer.

*/

  /* ----- Includes */

#include "standard.h"
#include "options.h"
#include <strings.h>
#include <string.h>


#define USE_PARTIALS		// Permit partial matches for long options
//#define TEST_OPTIONS


  /* ----- Class Globals/Statics */

typedef enum {
  STATE_0			= 0,
  STATE_FOUND_DASH		= 1,
  STATE_FOUND_DASHDASH		= 2,
} STATE_PARSE;

typedef enum {
  FETCH_DEFAULT			= 0,
  FETCH_SHORT			= 1,
  FETCH_LONG			= 2,
  FETCH_COMMAND			= 3,
} FETCH_MODE;


static int eval_option (char* pch, int cch, char* pchArgument,
			OPTION* pOptions);
static OPTION* fetch_option (char* pch, FETCH_MODE mode, OPTION* rgOptions);
int fetch_argument (OPTION* pOption, char* pch, int argc, char** argv,
		    char** ppchArgument);


  /* ----- Methods */

char* parse_application (char* pch)
{
  char* pchSep    = rindex (pch, '\\');
  char* pchSepAlt = rindex (pch, '/');
  char* pchColon  = rindex (pch, ':');
  char* pchDot    = rindex (pch, '.');

  if (pchSepAlt > pchSep)
    pchSep = pchSepAlt;
  if (pchColon > pchSep)
    pchSep = pchColon;
  pch = pchSep ? pchSep + 1 : pch;

  // This line code, while OK on Windows, may cause the process name
  // to disappear from the process list.
//  if (pchDot && strcasecmp (pch, ".exe"))
//    *pchDot = 0;

  return pch;
}  /* parse_applications */


/* parse_options

   accepts the argument vector for the application and an option
   description array and parses the command line arguments.  The
   return value is zero if the parse succeeds or non-zero if there
   was an error.

*/

int parse_options (int argc, char** argv, OPTION* rgOptions, int* pargc_used)
{
  int argc_used;
  if (!pargc_used)
    pargc_used = &argc_used;
  *pargc_used = 0;

  int result = 0;

  int state = STATE_0;

  for (; argc; --argc, ++argv, ++*pargc_used) {
    char* pch;
    for (pch = *argv; *pch; ++pch) {
      OPTION* pOption;
      char* pchArgument = NULL;
      int cch;			// General purpose length storage

      switch (state) {

	case STATE_0:
	  if (*pch == '-') {
	    state = STATE_FOUND_DASH;
	    continue;
	  }  /* if */

	  if ((pOption = fetch_option (pch, FETCH_COMMAND, rgOptions))) {
	    if ((result = eval_option (pch, strlen (pch), NULL, pOption)))
	      return result;
	  }  /* if */
	  else if ((pOption = fetch_option (NULL, FETCH_DEFAULT, rgOptions))) {
	    if ((result = eval_option (NULL, 0, pch, pOption)))
	      return result;
	  }  /* else-if */
	  else
	    return OptErrOk;
	  pch += strlen (pch) - 1;
	  break;

	case STATE_FOUND_DASH:
	  if (*pch == '-') {
	    state = STATE_FOUND_DASHDASH;
	    continue;
	  }  /* if */

	  pOption = fetch_option (pch, FETCH_SHORT, rgOptions);
	  if (!pOption)
	    return OptErrUnrecognized;

	  if (result
	      = fetch_argument (pOption, pch, argc, argv, &pchArgument))
	    return result;
	  cch = (pOption->sz && pOption->sz[1] ? strlen (pch) : 1);

	  if ((result = eval_option (pch, cch, pchArgument, pOption)))
	    return result;

	  if (pchArgument) {
	    if (argc && argv[1] == pchArgument) {
	      --argc;
	      ++argv;
	      ++*pargc_used;
	    }
	    else
	      cch = strlen (pch);
	    state = STATE_0;
	  }
	  pch += cch - 1;
	  break;

	case STATE_FOUND_DASHDASH:
	  pOption = fetch_option (pch, FETCH_LONG, rgOptions);
	  if (!pOption)
	    return OptErrUnrecognized;

	  if (result
	      = fetch_argument (pOption, pch, argc, argv, &pchArgument))
	    return result;
	  cch = (pOption->sz && pOption->sz[1] ? strlen (pch) : 1);

	  if ((result = eval_option (pch, cch, pchArgument, pOption)))
	    return result;

	  if (pchArgument && (argc && argv[1] == pchArgument)) {
	    --argc;
	    ++argv;
	    ++*pargc_used;
	  }
	  pch += cch - 1;
	  state = STATE_0;
	  break;
      }  /* switch */
    }  /* for */
  }  /* for */
  return OptErrOk;
}  /* parse_options */


/* fetch_argument

   handle the complexity of argument fetching.  It accepts the parsed
   option and the argument pointers, argc and argv, and returns a
   pointer to the argument, if there is one.

*/

int fetch_argument (OPTION* pOption, char* pch, int argc, char** argv,
		    char** ppchArgument)
{
  *ppchArgument = NULL;
  if (!(pOption->flags & OptArg))
    return OptErrOk;

  if (!(pOption->sz && pOption->sz[0] && pOption->sz[1])) {
    if (pch[1]) {
      *ppchArgument = pch + 1;
      if (**ppchArgument == '=')
	++*ppchArgument;
    }
    else if (argc > 1)
      *ppchArgument = argv[1];
  }
  else {
    int cch = strcspn (pch, "=");
    if (pch[cch] == '=') {
      *ppchArgument = pch + cch + 1;
      pch[cch] = 0;		// Important for OptDefault?  Not really.
    }
    else if (argc > 1)
      *ppchArgument = argv[1];
  }
  if (!*ppchArgument || !**ppchArgument)
    return OptErrNoArgument;
  return OptErrOk;
}


/* fetch_option

   accepts a pointer into a word within the command line, usually
   after either a single or double dash, and a mode specifier that
   determines which types of options it can match.  It then tries to
   match that option string in the option data.  Long options and
   command options will match partials if the USE_PARTIALS macro is
   defined.

 */

OPTION* fetch_option (char* pch, FETCH_MODE mode, OPTION* rgOptions)
{
  //  int cch = ((mode == FETCH_SHORT) ? 1 : (pch ? strlen (pch) : 0));
  int cch = pch ? strlen (pch) : 0;
  if (cch) {
    int cchOption = strcspn (pch, "=");
    if (cchOption < cch)
      cch = cchOption;
  }

  OPTION* pOptionDefault = NULL;
  OPTION* pOptionPartial = NULL;

  for (OPTION* pOption = rgOptions; pOption && pOption->sz; ++pOption) {
    if (   (pOption->flags & OptDefault)
	&& !pOptionDefault)
      pOptionDefault = pOption;

    if (!pch) {
      if (pOption->flags & OptNonoption)
	return pOption;
      continue;
    }  /* if */

    if (*pOption->sz != *pch)
      continue;

    bool fLongOption = pOption->sz && pOption->sz[0] && pOption->sz[1];

    if (   (mode == FETCH_COMMAND && !(pOption->flags & OptCommand))
	|| (mode != FETCH_COMMAND &&  (pOption->flags & OptCommand)))
      continue;

    if (mode == FETCH_SHORT) {
      if (!fLongOption)
	return pOption;
      if (!(pOption->flags & OptAllDash))
	continue;
    }
    if (strncmp (pch, pOption->sz, cch))
      continue;
    if (!pOption->sz[cch])
      return pOption;
    if (mode == FETCH_SHORT && fLongOption) // No partials for OptAllDash
      continue;
#if defined USE_PARTIALS
    pOptionPartial = pOptionPartial ? (OPTION*) -1 : pOption;
#endif
  }  /* for */

  return (pOptionPartial && pOptionPartial != (OPTION*) -1)
      ? pOptionPartial
      : pOptionDefault;
}  /* fetch_option */


/* eval_option

   accepts a pointer to the user's option string, a pointer to the
   option argument, and the fetched option pointer.

 */

int eval_option (char* pch, int cch, char* pchArgument, OPTION* pOption)
{
  if (pOption->pfn) {
    char sz[2];
    if (pOption->flags & OptDefault) {
      if (cch == 1) {
	sz[0] = *pch;
	sz[1] = 0;
	pchArgument = sz;
      }  /* if */
      else
	pchArgument = pch;
    }  /* if */
    return pOption->pfn (pOption, pchArgument)
      ? OptErrFail
      : OptErrOk;
  }  /* if */

  if (pOption->flags & OptDefault) {
    printf ("Unrecognized option '%.*s'\n", cch, pch);
    return OptErrUnrecognized;
  }

  if (pOption->flags & OptNonoption)
    return OptErrExit;

  if ((pOption->flags & OptSetMask) && pOption->pv) {
    long value = 1;
    if (pchArgument && !(pOption->flags & OptSetString))
      value = strtol (pchArgument, NULL, 0);
    else if (pOption->flags & OptClear)
      value = 0;

    switch (pOption->flags & OptSetType) {
      default:
	return OptErrBadOption;
      case OptSetInt:
	*(int*)   pOption->pv = int   (value);
	break;
      case OptSetShort:
	*(short*) pOption->pv = short (value);
	break;
      case OptSetLong:
	*(long*)  pOption->pv = long  (value);
	break;
      case OptSetString:
	*(char**) pOption->pv = pchArgument;
	break;
    }  /* switch */

    return OptErrOk;
  }  /* if */

    return OptErrBadOption;
}  /* eval_option */

#if defined (TEST_OPTIONS)

char* g_szOptS;

int do_a (OPTION*, char*)
{
  printf ("option a\n");
  return 0;
}

int do_unrecognized (OPTION*, char* sz)
{
  printf ("unrecognized option '%s'\n", sz);
}

OPTION rgOptions[] =
{
  { "s",	OptArg | OptSetString, &g_szOptS			},
  { "string",	OptArg | OptSetString, &g_szOptS			},
  { "geom",	OptArg | OptSetString | OptAllDash, &g_szOptS		},
  { "garage",	OptArg | OptSetString | OptAllDash, &g_szOptS		},
  { "a",	0, NULL, do_a						},
  { "",		OptDefault, NULL, do_unrecognized			},
  { NULL								},
};

int main (int argc, char** argv)
{
  int argc_used;
  int result = parse_options (argc - 1, argv + 1, rgOptions, &argc_used);

  if (g_szOptS)
    printf ("option s '%s'\n", g_szOptS);
  printf ("result %d  argc_used %d\n", result, argc_used);

  return 0;
}

#endif
