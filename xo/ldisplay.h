/* ldisplay.h							-*- C++ -*-
     $Id: ldisplay.h,v 1.17 1998/10/26 18:59:11 elf Exp $

   written by Marc Singer
   8 May 1997

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

   NOTES
   -----

   - GC

     It is *very* desirable to cache graphics contexts.  It is also
     desirable to avoid creating loads of GCs when a program starts
     up.  So, what we do is create a default GC per VISUAL.
     Presently, we do nothing with visuals/screens, so this translates
     to one GC per LDisplay.  Eventually, we need to create one per
     VISUAL.  The idea is that most drawing can be done by fetching
     this default GC, making the necessary changes to the graphics
     state, and then performing drawing.  But, we do something sneaky
     here.  We really create two GCs.  One stores the default drawing
     state, and the other is given to drawables to use.  Before
     handing a drawable the GC, we copy the attributes from the
     default settings one.  This way, we can simulate attribute
     save/restore.

*/

#if !defined (__LDISPLAY_H__)
#    define   __LDISPLAY_H__

/* ----- Includes */

#include "lhash.h"
#include "lfont.h"
#include <memory.h>

/* ----- Globals */


class LWindow;

typedef struct {
  LWindow* pWindow;		// Child window sending the notification
  int id;			// Child window ID, if there is one
  int event;			// What the child has done
  unsigned long data[4];	// Extra data the child may want to send
} ChildNotifyInfo;


class LDisplay {
protected:
  Display* m_pDisplay;

			// -- Display Attributes
  int m_cbitPadding;		// Scanlines padded to this number of bits
  int m_cbitUnit;		// Bitmap row unit quant
  int (*m_pfnAfter) (Display*);	// User's after function

			// -- Visual Attributes
  Visual* m_pVisual;		// We only support one visual right now
  int m_visual_class;		// Requested class
  XVisualInfo m_visualInfo;
  int m_shiftRed;
  int m_shiftGreen;
  int m_shiftBlue;
  int m_depth;			// Visual's bit depth
  Colormap m_colormap;

			// -- Drawing Contexts
  GC m_gcAttribute;		// Cached attribute GC (see NOTES)
  GC m_gcDraw;			// Cached drawing GC (see NOTES)

			// -- X Resources
  XrmDatabase m_database;
  LFontCache m_font_cache;	// Cache of font names to IDs

			// -- Windows
  LWindow* m_pWindowRoot;	// Link to windows created at the root
  LHashTable m_hashWindow;	// Window->LWindow*
  LHashTable m_hashTemplate;	// "windowclass"->LWindow*

public:
  LDisplay () {
    zero (); }
  ~LDisplay () {
    release_this (); }
  void zero (void) {
    memset (this, 0, sizeof (*this)); }

  void release_this (void);

  unsigned long color_distance (XColor& color, XColor& colorMatch);
  void dispatch_next_event (void);
  void dispatch (XEvent* pEvent);
  Colormap colormap (void) {
    return m_colormap; }
  int height (void) {
    return XDisplayHeight (m_pDisplay, 0); }
  int width (void) {
    return XDisplayWidth (m_pDisplay, 0); }
  int depth (void) {
    return m_depth; }
  Display* display (void) {
    return m_pDisplay; }
  bool is_pending_event (void) {
    return m_pDisplay ? (XPending (m_pDisplay) != 0) : false; }
  int pad (void) {
    return m_cbitPadding; }
  Visual* visual (void) {
    return m_pVisual; }
  XFontStruct* find_font (char* szFont) {
    return m_font_cache.find (szFont); }
  XFontStruct* find_font (Font fid) {
    return m_font_cache.find (fid); }
  LWindow* find_child (int id);
  bool find_resource (const char* szName, const char* szClass, char** pszType,
		      XrmValue* pValue) {
    char* szType = NULL;
    pValue->addr = 0;		// For the callers who don't check return
    return m_database
      ? ( XrmGetResource (m_database, szName, szClass,
			  pszType ? pszType : &szType, pValue) != False)
      : False; }
  bool find_pixel (char* szColorName, unsigned long* pPixel);
  LWindow* find_template (const char* szTemplate);
  LWindow* find_window (Window window);
  void flush (void) {
    m_pDisplay ? XFlush (m_pDisplay) : 0; }
  GC gc (void);
  void hash_window (LWindow* pWindow);
  bool hash_template (const char* szTemplate, LWindow* pWindow);
  void next_event (XEvent* pEvent) {
    m_pDisplay ? XNextEvent (m_pDisplay, pEvent) : 0; }
  bool open (char* szDisplay = NULL);
  void unhash_window (LWindow* pWindow);

  LWindow** root_windows (void) {
    return &m_pWindowRoot; }

  int shift_red (void) {
    return m_shiftRed; }
  int shift_green (void) {
    return m_shiftGreen; }
  int shift_blue (void) {
    return m_shiftBlue; }

  void set_after_function (int (*pfn)(Display*)) {
    m_pfnAfter = pfn; XSetAfterFunction (m_pDisplay, pfn); }
  void set_visual_class (int visual_class) {
    m_visual_class = visual_class; }
};



#endif  /* __LDISPLAY_H__ */
