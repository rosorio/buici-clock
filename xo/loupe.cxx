/* loupe.cxx
     $Id: loupe.cxx,v 1.7 1998/10/15 07:12:41 elf Exp $

   written by Marc Singer
   30 May 1997

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

#include "ldisplay.h"
#include "lwindow.h"
#include "wdialog.h"
#include "stats.h"
#include "dither.h"
#include "lpicture.h"

#include <sys/shm.h>
#include <sys/ipc.h>
#include <X11/extensions/XShm.h>

#define ROWWIDTH(width,depth) ((((width)*(depth))/8 + 3)& ~0x3)
#define SHM(v,s,m) ((s < 8) ? (((v) << (8 - (s)))&(m)) \
		    : (((v) >> ((s) - 8))&(m)))

#define USE_SHM			// Enable XShmImage's
#define USE_DITHER

char** g_argv;
bool g_fQuit;

void exit_error (const char* sz, ...)
{
  fprintf (stderr, "%s: ", g_argv[0]);

  va_list ap;
  va_start (ap, sz);
  vfprintf (stderr, sz, ap);
  va_end (ap);

  fprintf (stderr, "\n");

  exit (1);
}


class WTopLevel : public LWindow {
protected:
  bool m_fSharedMemory;
  XShmSegmentInfo m_shmInfo;

public:
  XImage* m_pImage;

public:
  WTopLevel (LDisplay* pDisplay) : LWindow (pDisplay) { zero (); }
  WTopLevel (LWindow* pWindow)   : LWindow (pWindow) { zero (); }
  void zero (void) {
    memset (((LWindow*) this) + 1, 0, sizeof (*this) - sizeof (LWindow)); }
  void buttondown (XButtonEvent* pEvent);
  void expose (XExposeEvent* pEvent);

  void load_image (const char* szFile);

};


EventMap g_rgEMTop[] = {
  { ButtonPress,	(PFNEvent) &WTopLevel::buttondown	},
  { Expose,		(PFNEvent) &WTopLevel::expose		},
  { 0, NULL },
};


void WTopLevel::buttondown (XButtonEvent* pEvent)
{
  XUnmapWindow (pEvent->display, pEvent->window);
  g_fQuit = true;
}


void WTopLevel::expose (XExposeEvent* pEvent)
{
  GC gc = display ()->gc ();

  LTime time;
  time.reset ();
  long overhead = time.delta ();
  {
    double r = (overhead*1e-6)/1;
    printf ("(overhead %d.%07d s/fetch) ", int (r), int ((r - int (r))*1e7));
  }
  time.reset ();

  int cBlt = 1;

  if (m_fSharedMemory) {
    for (int i = cBlt; i--;) {
      XShmPutImage (pEvent->display, pEvent->window, gc, m_pImage,
		 0, 0, 0, 0, m_pImage->width, m_pImage->height, False);
      m_pDisplay->flush ();
    }
  }
  else {
    for (int i = cBlt; i--;) {
      XPutImage (pEvent->display, pEvent->window, gc, m_pImage,
		 0, 0, 0, 0, m_pImage->width, m_pImage->height);
      m_pDisplay->flush ();
    }
  }
  {
    long delta = time.delta ();
    double r = ((delta - overhead)*1e-6)/cBlt;
    printf ("took %d.%07d s/blt\n", int (r), int ((r - int (r))*1e7));
  }
}


void register_base_classes (LDisplay* pDisplay)
{
  LWindow* pWindow = new LWindow (pDisplay);
  pWindow->event_map (g_rgEMTop);
  pWindow->select_events (ButtonPressMask | ExposureMask);
  pWindow->set_background_pixel (XBlackPixel (pDisplay->display (), 0));
  pWindow->set_bit_gravity (NorthWestGravity);
  //  pWindow->notify ((PFNEvent) WTopLevel::child);
  if (!pDisplay->hash_template ("top-level", pWindow))
    return;

  //  WButton::register_template (pDisplay);
  //  WTextEdit::register_template (pDisplay);
  //  WDialog::register_template (pDisplay);
}



void WTopLevel::load_image (const char* szFile)
{
  LPicture picture;
  if (!picture.read (szFile))
    return;

  int width = picture.m_width;
  int height = picture.m_height;
  int cbRowSrc = picture.m_cbStride;

				// --- Create the XImage
  Visual* pVisual = m_pDisplay->visual ();
  int shiftRed   = m_pDisplay->shift_red ();
  int shiftGreen = m_pDisplay->shift_green ();
  int shiftBlue  = m_pDisplay->shift_blue ();
  int depth = m_pDisplay->depth ();
  int cbRowDst = ROWWIDTH (width, depth);
  unsigned8* pbDst;

  if (
#if !defined (USE_SHM)
      0 &&
#endif
      XShmQueryExtension (m_pDisplay->display ())) {

    m_pImage = XShmCreateImage (m_pDisplay->display (),
				pVisual,
				depth, ZPixmap, NULL, &m_shmInfo,
				width, height);
    m_shmInfo.shmid = shmget (IPC_PRIVATE, cbRowDst*height,
			      IPC_CREAT | 0777);
    m_shmInfo.shmaddr = m_pImage->data
      = (char*) (pbDst = (unsigned8*) shmat (m_shmInfo.shmid, 0, 0));
    m_shmInfo.readOnly = false;
    XShmAttach (m_pDisplay->display (), &m_shmInfo);
    m_fSharedMemory = true;
  }
  else {
    pbDst = (unsigned8*) malloc (height*cbRowDst);
    m_pImage = XCreateImage (m_pDisplay->display (),
			     pVisual,
			     depth, ZPixmap, 0, (char*) pbDst, width, height,
			     m_pDisplay->pad (), cbRowDst);

  }

  if (picture.m_depth == 8) {
    int cbDeltaSrc = width - cbRowSrc;
    int cbDeltaDst = width*(depth/8) - cbRowDst;
    unsigned8* pbS = (unsigned8*) picture.m_pv;
    unsigned8* pbD = pbDst;
    if (picture.m_fNonNormal) {
      pbS += (height - 1)*cbRowSrc;
      cbDeltaSrc -= 2*cbRowSrc;
    }
    for (int row = 0; row < height;
	 ++row, pbS += cbDeltaSrc, pbD += cbDeltaDst) {
      for (int col = 0; col < width; ++col, ++pbS, pbD += depth/8) {
	RGBQUAD& rgb = picture.m_rgRGB[*pbS];
	unsigned16 pel
	  = SHM (rgb.rgbRed,   shiftRed,   pVisual->red_mask)
	  | SHM (rgb.rgbGreen, shiftGreen, pVisual->green_mask)
	  | SHM (rgb.rgbBlue,  shiftBlue,  pVisual->blue_mask);
	if (depth == 16)
	  *(unsigned16*)pbD = pel;
	else
	  *pbD = (unsigned8) pel;
      }
    }
  }
#if defined (USE_DITHER)
  if (picture.m_depth == 24) {
    unsigned8* pbSrc = (unsigned8*) picture.m_pv;
    LDither dither;
    dither.init ();
    dither.dither (width, height, pbSrc, pbDst);
    cbRowSrc = ROWWIDTH (width, 8);
    pbSrc = (unsigned8*) malloc (height*cbRowSrc);
    memcpy (pbSrc, pbDst, height*cbRowSrc);
    RGBQUAD* rgRGB = (RGBQUAD*) dither.palette ();

    int cbDeltaSrc = width - cbRowSrc;
    int cbDeltaDst = width*(depth/8) - cbRowDst;
    unsigned8* pbS = pbSrc;
    unsigned8* pbD = pbDst;
    if (picture.m_fNonNormal) {
      pbS += (height - 1)*cbRowSrc;
      cbDeltaSrc -= 2*cbRowSrc;
    }
    for (int row = 0; row < height;
	 ++row, pbS += cbDeltaSrc, pbD += cbDeltaDst) {
      for (int col = 0; col < width; ++col, ++pbS, pbD += depth/8) {
	RGBQUAD& rgb = rgRGB[*pbS];
	unsigned16 pel
	  = SHM (rgb.rgbRed,   shiftRed,   pVisual->red_mask)
	  | SHM (rgb.rgbGreen, shiftGreen, pVisual->green_mask)
	  | SHM (rgb.rgbBlue,  shiftBlue,  pVisual->blue_mask);
	if (depth == 16)
	  *(unsigned16*)pbD = pel;
	else
	  *pbD = (unsigned8) pel;
      }
    }
    free (pbSrc);
  }

#else
  if (picture.m_depth == 24) {
    int cbDeltaSrc = width*(24/8)    - cbRowSrc;
    int cbDeltaDst = width*(depth/8) - cbRowDst;
    unsigned8* pbS = (unsigned8*) picture.m_pv;
    unsigned8* pbD = pbDst;
    if (picture.m_fNonNormal) {
      pbS += (height - 1)*cbRowSrc;
      cbDeltaSrc -= 2*cbRowSrc;
    }
    for (int row = 0; row < height;
	 ++row, pbS += cbDeltaSrc, pbD += cbDeltaDst) {
      for (int col = 0; col < width; ++col, pbS += 3, pbD += depth/8) {
	unsigned16 pel
	  = SHM (pbS[2], shiftRed,   pVisual->red_mask)
	  | SHM (pbS[1], shiftGreen, pVisual->green_mask)
	  | SHM (pbS[0], shiftBlue,  pVisual->blue_mask);
	if (depth == 16)
	  *(unsigned16*)pbD = pel;
	else
	  *pbD = (unsigned8) pel;
      }
    }
  }
#endif

}


int main (int argc, char** argv)
{
  if (argc != 2) {
    fprintf (stderr, "usage: %s <bitmapfilename>\n", argv[0]);
  exit (1);
  }

  {
    LDisplay display;
    if (!display.open ())
      exit_error ("unable to open display");

    register_base_classes (&display);

    WTopLevel* pWindow = new WTopLevel (display.find_template ("top-level"));
    pWindow->load_image (argv[1]);
    pWindow->position (100, 100,
		       pWindow->m_pImage->width, pWindow->m_pImage->height);
    if (!pWindow->create (0))
      exit_error ("unable to open window");
    pWindow->map ();

    while (!g_fQuit) {
      display.dispatch_next_event ();
    }

    pWindow->unmap ();		// Make a clean (looking) exit
    display.flush ();

    delete pWindow;
  }
//  dmalloc_exit ();		// See what is left allocated
}
