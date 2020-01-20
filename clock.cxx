/* clock.cxx
     $Id: clock.cxx,v 1.28 2001/10/31 04:55:54 elf Exp $

   written by Marc Singer
   3 April 1997

   This file is part of the project Buici.  See the file README for
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

   ---------
   Resources
   ---------

   buiciClock.showSecondHand: {true|false|yes|no|0|1} [true]

     Controls the display of the second hand.  Disabling the second
     hand will not prevent an update every second.  Such a feature
     requires a little more work to determine the position of the
     hands and re-render when these values change.


*/

  /* ----- Includes */

#include "standard.h"
#if defined (STDC_HEADERS)
# include <stdarg.h>
#endif
#include <math.h>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "version.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/extensions/shape.h>

#include "ldisplay.h"
#include "lwindow.h"
#include "lres.h"
#include "signal.h"		// Our signal wrapper

#include "buici.xbm"		// Our name as a bitmap

// FACE_BACKING uses a pixmap to construct the clock face before
//   sending the image to the display.  It is the only method that
//   will show the Buici name on the clock.  It is considered to be
//   the best.
// BATCH_LINES sends the clock face markings to the server in one
//   protocol dispatch.  This is done for efficiency and is probably the
//   most frugal use of bandwidth.  It doesn't display the Buici name.
// <default> sends the face markings one at a time to the server.
//   Slow, and simple.

#define USE_FACE_BACKING
//#define USE_BATCH_LINES

//#define USE_LOCAL_GC		// Cache our own GC

extern char* g_szApplication;		// Name of application
extern char* g_szTimeZone;
bool g_fQuit;
extern int g_fAsDesktop;
extern int g_fAsToolbar;
extern int g_fAsDock;
extern int g_fOverrideRedirect;
extern int g_showSecondHand;

int do_clock (void);
void signal_alarm (LSignalHandle handle, void* pv);
int x_after (Display*);
time_t calc_delta (time_t);

void draw_dial (Display* display, Visual* visual,
		Pixmap pixmap, int dx, int dy);
void draw_hands (Display* display, Visual* visual,
		Pixmap pixmap, int dx, int dy, int seconds);
void draw_dial_shape (Display* display, Pixmap pixmap, int dx, int dy);

class WTopLevel : public LWindow {
protected:
  //  bool m_fSharedMemory;
  //  XShmSegmentInfo m_shmInfo;
  Pixmap m_pixmap;		// X drawable for rendering clock face
#if defined (USE_FACE_BACKING)
  Pixmap m_pixmapFace;		// X drawable for backing clock face
#endif
  time_t m_seconds;		// Last seconds value rendered
  bool m_fExpose;		// Exposure event forces redraw

  int m_width;			// Width we report
  int m_height;			// Width we report

  unsigned32 m_pixelGray;	// Gray pixel used for anti-aliasing

#if defined (USE_LOCAL_GC)
  GC m_gc;			// We cache our own GC to speed drawing
#endif

  GC gc (void) {
#if defined (USE_LOCAL_GC)
    if (m_gc)
      return m_gc;
    XGCValues xgcv;
    return m_gc = XCreateGC (xdisplay (), window (), 0, &xgcv);
#else
    return display ()->gc ();
#endif
  }

public:
  bool m_fSecondHand;		// true: display second hand

  WTopLevel (LDisplay* pDisplay) : LWindow (pDisplay) { zero (); }
  WTopLevel (LWindow* pWindow)   : LWindow (pWindow) { zero (); }
  ~WTopLevel () {
    release_this (); }
  void release_this (void);
  void zero (void) {
    memset (((LWindow*) this) + 1, 0, sizeof (*this) - sizeof (LWindow)); }

  void allocate_colors (void);
  void buttondown (XButtonEvent* pEvent);
  int compute_segment (XSegment* pseg, double theta,
		       float outside, float inside, float width);
  void draw (void);
  void expose (XExposeEvent* pEvent);
  bool is_expose (void) {
    return m_fExpose; }
  bool render (void);
  void render_buici (GC gc, Pixmap pixmap);
  void render_circle (GC gc, Pixmap pixmap, double theta,
		      float offset, float diameter);
  void render_line (GC gc, Pixmap pixmap, double theta,
		    float outside, float inside, float width);
  void setup_time (void);
  void shape (void);

  Pixmap get_pixmap ();
#if defined (USE_FACE_BACKING)
  Pixmap get_pixmap_face ();
#endif
  int line_width (double r) {
    int line_width = 1;
    if (r)
      line_width = int ((width () - 1.0 + height () - 1.0)/2.0*r);
    if (!line_width)
      line_width = 1;
    return line_width; }

  int width () {
    return m_width  ? m_width
      : (m_width  = ((LWindow::width ()  + 1)&~1) - 1); }
  int height () {
    return m_height ? m_height
      : (m_height = ((LWindow::height () + 1)&~1) - 1); }
};


typedef enum {
  markLine = 1,
  markCircle = 2,
} eMarkShape;

typedef struct {
  int count;			// Number of strokes, hands are zero
  int shape;			// Shape of the mark
  float rInside;		// Inside,  outside, and stroke width as
  float rOutside;		//    fractions of the radius
  float rWidth;			// Zero widths are one pixel
  int modSeconds;		// Modulus for computing hand angle
  unsigned32 color;		// Color (not presently implemented)
  unsigned32 pixel;		// Pixel value returned by server
} Mark;

#define WIDTH_BORDER	0.05
#define WIDTH_BUICI	0.22
//#define THRESHOLD_BLACK	40
//#define THRESHOLD_GRAY	10
#define THRESHOLD_BLACK	(40)
#define THRESHOLD_GRAY	(20)

Mark g_rgMarks[] = {
  { 60, markLine,   0.82,  0.76,  0,         0, 0x000000 },	// Second marks
  { 12, markLine,   0.82,  0.62,  0.040,     0, 0x000000 },	// Hour marks
  {  0, markLine,   0.48, -0.12,  0.030, 43200, 0x000000 },	// Hour hand
  {  0, markLine,   0.70, -0.12,  0.030,  3600, 0x000000 },	// Minute hand
  {  0, markLine,   0.58, -0.2,   0,        60, 0xff0000 },	// Second hand
  {  0, markCircle, 0.58,  0.058, 0,        60, 0xff0000 },	// Second hand
  { -1 }
};

EventMap g_rgEMTop[] = {
  { ButtonPress,	(PFNEvent) &WTopLevel::buttondown	},
  { Expose,		(PFNEvent) &WTopLevel::expose		},
  { 0, NULL },
};


  /* ----- Methods */

/* calc_delta

   calculates/caches the timezone delta.  It performs some caching
   checks to make sure it isn't calculated needlessly.

   It computes the delta when timeNow

*/

time_t calc_delta (time_t timeNow)
{
  static time_t timeLastCalc;	// Time delta was last computed
  static time_t timeDelta;	// Last computed delta
  const int timeRecalc = 60*60;	// Interval for recalc, aligned to 12:00:00am

  if (timeNow == 0)
    time (&timeNow);
  if (timeNow != timeLastCalc
      && (   timeNow - timeLastCalc > timeRecalc
	  || (timeNow%timeRecalc) == 0)) {
    //    fprintf (stderr, "recalc delta\n");

				// -- Determine timezone correction
    struct tm tmGMT   = *gmtime (&timeNow);
    struct tm tmLocal = *localtime (&timeNow);
    tmGMT.tm_isdst = 0;		// We want the absolute difference
    tmLocal.tm_isdst = 0;
    timeDelta = mktime (&tmLocal) - mktime (&tmGMT);
  //  fprintf (stderr, "deltaZone is %ld seconds or %d hours\n",
  //	   timeDelta, timeDelta/3600);
    timeLastCalc = timeNow;
  }

  return timeDelta;
}


int round_at (double r, double c, bool fInvert = false)
{
#if 1
  if ((!fInvert && r < 0) || (fInvert && r > 0))
    r -= 0.5;
  else
    r += 0.5;
  return int (r + c);
#else
  return int (r + 0.5 + c);
#endif
}

double round_up (double r)
{
  if (r < 0)
    return double (int (r*1e8 - 5)/10)/1e7;
  else
    return double (int (r*1e8 + 5)/10)/1e7;
}


inline int point_on_circle (double v, int r)
{
  if (v < 0)
    v -= 0.5;
  else
    v += 0.5;
  return int (v + r/2.0);
}


void exit_error (const char* sz, ...)
{
  fprintf (stderr, "%s: ", g_szApplication);

  va_list ap;
  va_start (ap, sz);
  vfprintf (stderr, sz, ap);
  va_end (ap);

  fprintf (stderr, "\n");

  exit (1);
}


int x_after (Display* /* pDisplay */)
{
  static time_t timeStart;

  if (!timeStart)
    time (&timeStart);

  //  fprintf (stderr, "protocol request %ld\n", time (NULL) - timeStart);
  return 0;
}


void WTopLevel::allocate_colors (void)
{
  Colormap colormap = XDefaultColormap (xdisplay (), 0);

  XColor xcolor;
  memset (&xcolor, 0, sizeof (xcolor));

  for (Mark* pMark = g_rgMarks; pMark->count != -1; ++pMark) {
    xcolor.red   = (((pMark->color >> 16) & 0xff) << 8);
    xcolor.green = (((pMark->color >>  8) & 0xff) << 8);
    xcolor.blue  = (((pMark->color >>  0) & 0xff) << 8);
    if (XAllocColor (xdisplay (), colormap, &xcolor))
      pMark->pixel = xcolor.pixel;
    else
      pMark->pixel = XBlackPixel (xdisplay (), 0);
  }

  xcolor.red = xcolor.green = xcolor.blue = 0x8000;
  if (XAllocColor (xdisplay (), colormap, &xcolor))
    m_pixelGray = xcolor.pixel;
}


void WTopLevel::buttondown (XButtonEvent* /* pEvent */)
{
  //  fprintf (stderr, "click\n");
  //  XUnmapWindow (pEvent->display, pEvent->window);
  //  g_fQuit = true;
}


/* WTopLevel::compute_segment

   computes a line segment for clock marks or hands.  It is a
   duplicate of the render_line code that writes the segment
   information to an array so that the segments may be submitted to
   the server in one command.

   The outside and inside parameters are real number representing the
   proportion of the outside and inside points of the line with
   respect to the radius of the clock face.  A value of 0 is the
   center and a value of 1.0 is the outside edge of the face.

   The theta parameter a number from 0 to 1.0 that describes the
   angle of the line as a fraction of the circle.  Zero is noon or
   midnight.  0.25 is three o'clock.  This routine converts the number
   of radians for calculation--unless we choose to use a lookup table
   for angles.

*/

int WTopLevel::compute_segment (XSegment* pseg, double theta,
				float outside, float inside,
				float /*line_width_fraction */)
{
  theta = M_PI_2 - theta*2.0*M_PI;
  double sin_t = -sin (theta);
  double cos_t = cos (theta);
  double rx = (width () - 0)/2.0;		// radius along X axis
  double ry = (height () - 0)/2.0;		// radius along Y axis

  if (outside*inside < 0) {
    pseg->x1 = round_at (outside*rx*cos_t, (width () + 1)/2.0);
    pseg->y1 = round_at (outside*ry*sin_t, (height () + 1)/2.0);
    pseg->x2 = (width ()  + 1)/2;
    pseg->y2 = (height () + 1)/2;
    pseg[1].x1 = pseg->x2;
    pseg[1].y1 = pseg->y2;
    ++pseg;
    pseg->x2 = round_at (inside*rx*cos_t,  (width () + 1)/2.0, true);
    pseg->y2 = round_at (inside*ry*sin_t,  (height () + 1)/2.0, true);
    return 2;
  }
  else {
    pseg->x1 = round_at (outside*rx*cos_t, (width () + 1)/2.0);
    pseg->y1 = round_at (outside*ry*sin_t, (height () + 1)/2.0);
    pseg->x2 = round_at (inside*rx*cos_t,  (width () + 1)/2.0);
    pseg->y2 = round_at (inside*ry*sin_t,  (height () + 1)/2.0);
    return 1;
  }
}


void WTopLevel::draw (void)
{
  GC _gc = gc ();
  Pixmap pixmap = get_pixmap ();

  XCopyArea (xdisplay (), pixmap, window (),
	     _gc, 0, 0, width (), height (), 0, 0);
  m_fExpose = false;
}


void WTopLevel::expose (XExposeEvent* /* pEvent */)
{
  m_fExpose = true;		// Force redraw next time.
}


Pixmap WTopLevel::get_pixmap (void)
{
  if (m_pixmap)
    return m_pixmap;
  if (!window ())
    return 0;

  m_pixmap = XCreatePixmap (xdisplay (), window (),
			    width (), height (), display ()->depth ());

  return m_pixmap;
}

#if defined (USE_FACE_BACKING)
Pixmap WTopLevel::get_pixmap_face (void)
{
  if (m_pixmapFace)
    return m_pixmapFace;
  if (!window ())
    return 0;

  m_pixmapFace = XCreatePixmap (xdisplay (), window (),
				width (), height (), display ()->depth ());

  return m_pixmapFace;
}
#endif


bool WTopLevel::render (void)
{
				// -- Check time
#if 0
  struct timeval tv;
  struct timezone tz;
  memset (&tv, 0, sizeof (tv));
  memset (&tz, 0, sizeof (tz));
  gettimeofday (&tv, &tz);
  time_t seconds = tv.tv_sec - tz.tz_minuteswest*60;
  if (tz.tz_dsttime == DST_USA)
    seconds += 3600;
#else
  time_t seconds;
  time (&seconds);
  seconds += calc_delta (seconds);
#endif

				// Return immediately if there is
				// nothing new to render.
  if (seconds == m_seconds)
    return false;
  m_seconds = seconds;

  //  fprintf (stderr, "rendering\n");

				// -- Do the rendering
  GC _gc = gc ();

  if (0) {
    XGCValues values;
    values.graphics_exposures = False;
    XChangeGC (display ()->display (), _gc, GCGraphicsExposures, &values);
  }

#if defined (USE_FACE_BACKING)
  if (!m_pixmapFace) {
    Pixmap pixmap = get_pixmap_face ();

//    printf ("drawing the backing face\n");
    draw_dial (xdisplay (), xvisual (), pixmap, width (), height ());

#if 0
    XSetForeground (xdisplay (), _gc, XWhitePixel (xdisplay (), 0));
    XFillRectangle (xdisplay (), pixmap, _gc, 0, 0, width (), height ());
    XSetForeground (xdisplay (), _gc, XBlackPixel (xdisplay (), 0));
    XSetBackground (xdisplay (), _gc, XWhitePixel (xdisplay (), 0));

				// -- Draw name 'Buici'
    render_buici (_gc, pixmap);

				// -- Draw perimeter
    int width_border = line_width (WIDTH_BORDER);
    XSetLineAttributes (xdisplay (), _gc, width_border,
			LineSolid, CapButt, JoinMiter);
    --width_border;
    XDrawArc (xdisplay (), pixmap, _gc, width_border, width_border,
	      width () - 1 - width_border*2, height () - 1 - width_border*2,
	      0*64, 360*64);

				// -- Draw marks
    for (Mark* pMark = g_rgMarks; pMark->count != -1; ++pMark) {
      if (!pMark->count)
	continue;
      XSetForeground (xdisplay (), _gc, pMark->pixel);
      for (int i = 0; i < pMark->count; ++i)
	render_line (_gc, pixmap, i/float (pMark->count),
		     pMark->rInside, pMark->rOutside, pMark->rWidth);
    }
#endif
  }
  else {
    XSetForeground (xdisplay (), _gc, XBlackPixel (xdisplay (), 0));
    XSetBackground (xdisplay (), _gc, XWhitePixel (xdisplay (), 0));
  }

  Pixmap pixmap = get_pixmap ();

  XCopyArea (xdisplay (), get_pixmap_face (), pixmap,
	     _gc, 0, 0, width (), height (), 0, 0);


  draw_hands (xdisplay (), xvisual (), pixmap, width (), height (), seconds);

#if 0
				// -- Draw hands
  for (Mark* pMark = g_rgMarks; pMark->count != -1; ++pMark) {
    if (pMark->count)
      continue;
    if (pMark->modSeconds == 60 && !m_fSecondHand)
      continue;
    XSetForeground (xdisplay (), _gc, pMark->pixel);
    switch (pMark->shape) {
    default:
    case markLine:
      render_line (_gc, pixmap, (seconds%pMark->modSeconds)
		   /float (pMark->modSeconds),
		   pMark->rInside, pMark->rOutside, pMark->rWidth);
      break;
    case markCircle:
      render_circle (_gc, pixmap, (seconds%pMark->modSeconds)
		     /float (pMark->modSeconds),
		     pMark->rInside, pMark->rOutside);
      break;
    }
  }
#endif

#else

  Pixmap pixmap = get_pixmap ();

  XSetForeground (xdisplay (), _gc, XWhitePixel (xdisplay (), 0));
  XFillRectangle (xdisplay (), pixmap, _gc, 0, 0, width (), height ());
  XSetForeground (xdisplay (), _gc, XBlackPixel (xdisplay (), 0));
  XSetBackground (xdisplay (), _gc, XWhitePixel (xdisplay (), 0));

				// -- Draw perimeter
  int width_border = line_width (WIDTH_BORDER);
  XSetLineAttributes (xdisplay (), _gc, width_border,
		      LineSolid, CapButt, JoinMiter);
  --width_border;
  XDrawArc (xdisplay (), pixmap, _gc, width_border, width_border,
	    width () - 1 - width_border*2, height () - 1 - width_border*2,
	    0*64, 360*64);

# if defined (USE_BATCH_LINES)
  XSegment rgseg[100];		// FIXME: bad hard limit
  int iSegment = 0;

  printf ("redrawing the marks batch\n");

				// -- Draw marks
  for (Mark* pMark = g_rgMarks; pMark->count != -1; ++pMark, iSegment = 0) {
    if (pMark->count)
      for (int i = 0; i < pMark->count; ++i)
	iSegment += compute_segment (&rgseg[iSegment], i/float (pMark->count),
				     pMark->rInside, pMark->rOutside,
				     pMark->rWidth);
    else {
      if (pMark->modSeconds == 60 && !m_fSecondHand)
	continue;
      iSegment += compute_segment (&rgseg[iSegment],
				   (seconds%pMark->modSeconds)
				   /float (pMark->modSeconds),
				   pMark->rInside, pMark->rOutside,
				   pMark->rWidth);
    }

    XSetLineAttributes (xdisplay (), _gc, line_width (pMark->rWidth),
			LineSolid, CapButt, JoinMiter);
    XDrawSegments (xdisplay (), pixmap, _gc, rgseg, iSegment);
  }

# else

  //  printf ("redrawing the marks\n");

				// -- Draw marks
  for (Mark* pMark = g_rgMarks; pMark->count != -1; ++pMark) {
    if (pMark->count)
      for (int i = 0; i < pMark->count; ++i)
	render_line (_gc, pixmap, i/float (pMark->count),
		     pMark->rInside, pMark->rOutside, pMark->rWidth);
    else {
      if (pMark->modSeconds == 60 && !m_fSecondHand)
	continue;
      render_line (_gc, pixmap, (seconds%pMark->modSeconds)
		   /float (pMark->modSeconds),
		   pMark->rInside, pMark->rOutside, pMark->rWidth);
    }
  }
# endif
#endif

  return true;
}

void WTopLevel::release_this (void)
{
  if (m_pixmap) {
    XFreePixmap (xdisplay (), m_pixmap);
    m_pixmap = 0;
  }
#if defined (USE_FACE_BACKING)
  if (m_pixmapFace) {
    XFreePixmap (xdisplay (), m_pixmapFace);
    m_pixmapFace = 0;
  }
#endif
}


void WTopLevel::render_buici (GC gc, Pixmap pixmap)
{
  Pixmap pixmapBuici = 0;
  int dx = line_width (WIDTH_BUICI);
  int dy = (buici_height*dx + buici_width/2)/buici_width;
  int x = width ()/2 - dx/2;
  int y = height ()/3 - dy/2;
  int cbRowSrc = (buici_width + 7)/8;
  int cbRowDst = (dx + 7)/8;

  unsigned8* rgb = (unsigned8*) malloc (cbRowDst*dy);
  unsigned8* rgbGray = (unsigned8*) malloc (cbRowDst*dy);
  memset (rgb, 0, cbRowDst*dy);
  memset (rgbGray, 0, cbRowDst*dy);
  int rowOutput = 0;
  unsigned8 rgCount[800];	// FIXME: duh
  int countMax = (buici_width*buici_height)/(dx*dy);
  //  fprintf (stderr, "countMax %d %dx%d -> %dx%d\n", countMax,
  //	   buici_width, buici_height, dx, dy);
  memset (rgCount, 0, sizeof (rgCount));
  for (int j = 0; j < buici_height; ++j) {
    for (int i = 0; i < buici_width; ++i) {
      if (!(buici_bits[j*cbRowSrc + i/8] & (1 << (i % 8))))
	continue;
      int x = (i*dx)/buici_width;
      ++rgCount[x];
    }
    int y = ((j + 1)*dy)/buici_height;
    if (y != rowOutput) {
      for (int i = 0; i < dx; ++i) {
	if (rgCount[i] < countMax*THRESHOLD_GRAY/100)
	  continue;
	rgbGray[rowOutput*cbRowDst + i/8] |= (1 << (i % 8));
	if (rgCount[i] < countMax*THRESHOLD_BLACK/100)
	  continue;
	rgb[rowOutput*cbRowDst + i/8] |= (1 << (i % 8));
      }
      memset (rgCount, 0, sizeof (rgCount));
      rowOutput = y;
    }
  }
//  dmalloc_validate ();		// Paranoid check

  pixmapBuici = XCreateBitmapFromData (xdisplay (), window (),
				       (const char*) rgbGray, dx, dy);
  XSetState (xdisplay (), gc, m_pixelGray, XWhitePixel (xdisplay (), 0),
	     GXcopy, AllPlanes);
  XCopyPlane (xdisplay (), pixmapBuici, pixmap, gc,
	      0, 0, dx, dy, x, y, 0x1);
  XFreePixmap (xdisplay (), pixmapBuici);

  pixmapBuici = XCreateBitmapFromData (xdisplay (), window (),
				       (const char*) rgb, dx, dy);
  XSetState (xdisplay (), gc, ~0, 0, GXandInverted, AllPlanes);
  XCopyPlane (xdisplay (),pixmapBuici, pixmap, gc,
	      0, 0, dx, dy, x, y, 0x1);
  XSetState (xdisplay (), gc,
	     XBlackPixel (xdisplay (), 0),
	     XWhitePixel (xdisplay (), 0), GXor, AllPlanes);
  XCopyPlane (xdisplay (),pixmapBuici, pixmap, gc,
	      0, 0, dx, dy, x, y, 0x1);
  XFreePixmap (xdisplay (), pixmapBuici);

  XSetState (xdisplay (), gc,
	     XBlackPixel (xdisplay (), 0),
	     XWhitePixel (xdisplay (), 0), GXcopy, AllPlanes);

  free (rgb);
  free (rgbGray);
}


/* WTopLevel::render_circle

   renders a circle for clock marks or hands.  The offset and diameter
   parameters are real numbers representing the proportion of the
   circle center and diameter with respect to the radius of the clock
   face.  An offset value of 0 is the center and a value of 1.0 is the
   outside edge of the face.

   The theta parameter a number from 0 to 1.0 that describes the
   angle of the line as a fraction of the circle.  Zero is noon or
   midnight.  0.25 is three o'clock.  This routine converts the number
   of radians for calculation--unless we choose to use a lookup table
   for angles.

*/

void WTopLevel::render_circle (GC gc, Pixmap pixmap, double theta,
			       float offset, float diameter)
{
  theta = M_PI_2 - theta*2.0*M_PI;
  double sin_t = -sin (theta);
  double cos_t = cos (theta);
  double rx = (width () - 0)/2.0;		// radius along X axis
  double ry = (height () - 0)/2.0;		// radius along Y axis
  double cx = offset*rx*cos_t;
  double cy = offset*ry*sin_t;
  double dx = diameter*width ();
  double dy = diameter*height ();
  int x = round_at (cx, (width  () + 1)/2.0 - (dx + 0.5)/2.0);
  int y = round_at (cy, (height () + 1)/2.0 - (dy + 0.5)/2.0);

  XFillArc (xdisplay (), pixmap, gc, x, y,
	    int (dx + 0.5), int (dy + 0.5),
	    0*64, 360*64);
}


/* WTopLevel::render_line

   renders a line for clock marks or hands.  The outside and inside
   parameters are real number representing the proportion of the
   outside and inside points of the line with respect to the radius of
   the clock face.  A value of 0 is the center and a value of 1.0 is
   the outside edge of the face.

   The theta parameter a number from 0 to 1.0 that describes the
   angle of the line as a fraction of the circle.  Zero is noon or
   midnight.  0.25 is three o'clock.  This routine converts the number
   of radians for calculation--unless we choose to use a lookup table
   for angles.

*/

void WTopLevel::render_line (GC gc, Pixmap pixmap, double theta,
			     float outside, float inside,
			     float line_width_fraction)
{
  theta = M_PI_2 - theta*2.0*M_PI;
  double sin_t = -sin (theta);
  double cos_t = cos (theta);
  double rx = (width () - 0)/2.0;		// radius along X axis
  double ry = (height () - 0)/2.0;		// radius along Y axis

  XSetLineAttributes (xdisplay (), gc, line_width (line_width_fraction),
		      LineSolid, CapButt, JoinMiter);
  if (outside*inside < 0) {
    XPoint rgPt[3];
    rgPt[0].x = round_at (outside*rx*cos_t, (width () + 1)/2.0);
    rgPt[0].y = round_at (outside*ry*sin_t, (height () + 1)/2.0);
    rgPt[1].x = (width ()  + 1)/2;
    rgPt[1].y = (height () + 1)/2;
    rgPt[2].x = round_at (inside*rx*cos_t,  (width () + 1)/2.0);
    rgPt[2].y = round_at (inside*ry*sin_t,  (height () + 1)/2.0);
    XDrawLines (xdisplay (), pixmap, gc, rgPt, 3, CoordModeOrigin);
  }
  else
    XDrawLine (xdisplay (), pixmap, gc,
	       round_at (outside*rx*cos_t, (width () + 1)/2.0),
	       round_at (outside*ry*sin_t, (height () + 1)/2.0),
	       round_at (inside*rx*cos_t,  (width () + 1)/2.0),
	       round_at (inside*ry*sin_t,  (height () + 1)/2.0));
}


void WTopLevel::setup_time (void)
{
				// -- Configure interval timer
  LSignal::accept (SIGALRM, signal_alarm, (void*) this, 0, 0);
  {
    struct itimerval value;
    memset (&value, 0, sizeof (value));
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 100000;
    value.it_value.tv_usec = 1;
    //      value.it_value = value.it_interval;
    setitimer (ITIMER_REAL, &value, NULL);
  }
}


void WTopLevel::shape (void)
{
  int i;
  if (!XShapeQueryExtension (xdisplay (), &i, &i))
    return;

  Pixmap pixmap
    = XCreatePixmap (xdisplay (), window (), width (), height (), 1);

#if 0
  XGCValues xgcv;
  GC gc = XCreateGC (xdisplay (), pixmap, 0, &xgcv);
  XSetForeground (xdisplay (), gc, 0);
  XFillRectangle (xdisplay (), pixmap, gc, 0, 0, width (), height ());

# if 0
  XSetForeground (xdisplay (), gc, 1);
  int width_border = line_width (WIDTH_BORDER);
  XSetLineAttributes (xdisplay (), gc, width_border,
		      LineSolid, CapRound, JoinMiter);
  --width_border;
  XFillArc (xdisplay (), pixmap, gc, width_border, width_border,
	    width () - 1 - width_border*2, height () - 1 - width_border*2,
	    0*64, 360*64);
  XDrawArc (xdisplay (), pixmap, gc, width_border, width_border,
	    width () - 1 - width_border*2, height () - 1 - width_border*2,
	    0*64, 360*64);
# endif
  XFreeGC (xdisplay (), gc);
#else
  draw_dial_shape (xdisplay (), pixmap, width (), height ());
#endif

  XShapeCombineMask (xdisplay (), window (), ShapeBounding, 0, 0,
		     pixmap, ShapeSet);
  XShapeCombineMask (xdisplay (), window (), ShapeClip, 0, 0,
		     pixmap, ShapeSet);
  XFreePixmap (xdisplay (), pixmap);
}


void register_base_classes (LDisplay* pDisplay)
{
  LWindow* pWindow = new LWindow (pDisplay);
  pWindow->event_map (g_rgEMTop);
  pWindow->select_events (ButtonPressMask | ExposureMask);
//  pWindow->set_background_pixel (XBlackPixel (pDisplay->display (), 0));
  pWindow->set_bit_gravity (NorthWestGravity);
  if (g_fOverrideRedirect)
    pWindow->set_override_redirect (true);
  //  pWindow->notify ((PFNEvent) WTopLevel::child);
  if (!pDisplay->hash_template ("top-level", pWindow))
    return;

  //  WButton::register_template (pDisplay);
  //  WTextEdit::register_template (pDisplay);
  //  WDialog::register_template (pDisplay);
}


int do_clock (void)
{
  extern char* g_szGeometry;
  int x = 0;
  int y = 0;
  unsigned int dx = 100;
  unsigned int dy = 100;

  if (g_szTimeZone) {
    setenv("TZ",g_szTimeZone , 1);
    tzset();
  }


  {
    LDisplay display;
    display.set_visual_class (PseudoColor);
    if (!display.open ())
      exit_error ("unable to open display");

    register_base_classes (&display);
    WTopLevel* pWindow = new WTopLevel (display.find_template ("top-level"));

    switch (g_showSecondHand) {
    default:
    case -1:
      {
	XrmValue value;
	pWindow->m_fSecondHand =
	  display.find_resource ("buiciClock.showSecondHand",
				 NULL, NULL, &value) ? as_bool (value, true)
	  : true;
      }
      break;
    case 0:
      pWindow->m_fSecondHand = false;
      break;
    case 1:
      pWindow->m_fSecondHand = true;
      break;
    }

    pWindow->position (x, y, dx, dy);
    pWindow->geometry (g_szGeometry);
    pWindow->qualify ("buiciClock", "buiciClock\0BuiciClock\0");
    if (!pWindow->create (0))
      exit_error ("unable to open window");
    pWindow->shape ();

    pWindow->transient_for (pWindow);
    if (g_fAsDesktop)
      pWindow->net_window_type (windowTypeDesktop);
    if (g_fAsToolbar)
      pWindow->net_window_type (windowTypeToolbar);
    if (g_fAsDock)
      pWindow->net_window_type (windowTypeDock);

    pWindow->allocate_colors ();		// First, create the colormap

    pWindow->setup_time ();
    pWindow->render ();
    pWindow->map ();

    //    display.set_after_function (x_after);

    //    fprintf (stderr, "starting dispatch\n");

    while (!g_fQuit) {
      display.dispatch_next_event ();
    }

    fprintf (stderr, "exiting\n");

    pWindow->unmap ();		// Make a clean (looking) exit
    display.flush ();

    delete pWindow;
  }
//  dmalloc_exit ();		// See what is left allocated

  return 0;
}

void signal_alarm (LSignalHandle /* handle */, void* pv)
{
  //  fprintf (stderr, "tick\n");
  WTopLevel* pWindow = (WTopLevel*) pv;
  if (pWindow->render () || pWindow->is_expose ()) {
    pWindow->draw ();
    pWindow->display ()->flush ();
  }
  //  fprintf (stderr, "tock\n");
}
