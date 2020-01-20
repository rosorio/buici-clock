/* draw.cc

   written by Marc Singer
   25 Dec 2006

   Copyright (C) 2006 Marc Singer

   -----------
   DESCRIPTION
   -----------

   Drawing of clock mechanism with cairo.

   g++ `pkg-config --cflags --libs cairo` -o draw draw.cc

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include <math.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>

#define DX	100.0
#define DY	100.0
#define WIDTH_BORDER (DX*0.04)
#define WIDTH_THICK  (DX*0.026)
#define WIDTH_THIN 1.0
#define WIDTH_HAND (WIDTH_THICK*0.8)
#define FONT "serif"

void draw (void)
{
  cairo_surface_t* s = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
						   (int) DX, (int) DY);
  cairo_t* cr = cairo_create (s);
	// -- Setup initial matrix
  cairo_translate (cr, DX/2.0, DY/2.0);
//  cairo_scale (cr, 0.3, 0.3);
  //  cairo_rotate (cr, M_PI/8);

//  cairo_rectangle (cr, 0, 0, DX, DY);
//  cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
//  cairo_fill (cr);

	// -- Draw border

  cairo_arc (cr, 0, 0, DX/2 - WIDTH_BORDER/2, 0, 2*M_PI);
  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_fill_preserve (cr);
//  cairo_arc (cr, DX/2.0, DY/2.0, DX/2 - WIDTH_THICK, 0, 2*M_PI);
  cairo_set_line_width (cr, WIDTH_BORDER);
  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_stroke (cr);

	// -- Draw ticks

  for (int i = 0; i < 60; ++i) {
    cairo_save (cr);
    cairo_rotate (cr, (2*M_PI)*i/60.0);
    cairo_set_line_width (cr, (i%5 == 0) ? WIDTH_THICK : WIDTH_THIN);
    cairo_move_to (cr, (DX/2.0)*0.90, 0);
    cairo_line_to (cr, (i%5 == 0) ? (DX/2.0)*0.70 : (DX/2.0)*0.83, 0);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_stroke (cr);
    cairo_restore (cr);
  }

	// -- Draw brand
  {
    const char* sz = "Buici";
    cairo_save (cr);
    cairo_select_font_face (cr, FONT,
			    CAIRO_FONT_SLANT_ITALIC,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_set_font_size (cr, DX*0.08);
    cairo_text_extents_t ex;
    cairo_text_extents (cr, sz, &ex);
    cairo_translate (cr, -ex.width/2 + ex.x_bearing,
		     -ex.y_bearing/2 - (DX/2)*0.35);
    cairo_show_text (cr, sz);

    cairo_restore (cr);
  }


	// -- Draw hands
  {
    time_t t = time (NULL);
    struct tm tm;
    localtime_r (&t, &tm);

    int second = tm.tm_sec; // t%60;
    float minute = tm.tm_min + second/60.0; //(t/60)%60;
    float hour = tm.tm_hour + minute/60.0; //(t/(60*60))%12;

    cairo_save (cr);
    cairo_rotate (cr, ((2.0*M_PI)*hour)/12.0);
    cairo_set_line_width (cr, WIDTH_HAND);
    cairo_move_to (cr, 0, (DY/2.0)*0.15);
    cairo_line_to (cr, 0, -(DY/2.0)*0.58);
    cairo_stroke (cr);
    cairo_restore (cr);

    cairo_save (cr);
    cairo_rotate (cr, ((2.0*M_PI)*minute)/60.0);
    cairo_set_line_width (cr, WIDTH_HAND);
    cairo_move_to (cr, 0,  (DY/2.0)*0.15);
    cairo_line_to (cr, 0, -(DY/2.0)*0.75);
    cairo_path_t* path = cairo_copy_path (cr);
    //    cairo_stroke (cr);
    cairo_translate (cr, WIDTH_THIN, WIDTH_THIN);
    cairo_append_path (cr, path);
    cairo_path_destroy (path);
    cairo_set_source_rgb (cr, 0, 1.0, 0);
    //    cairo_stroke (cr);
    cairo_restore (cr);

    cairo_save (cr);
    cairo_rotate (cr, ((2.0*M_PI)*second)/60.0);
    cairo_set_line_width (cr, WIDTH_THIN);
    cairo_move_to (cr, 0,  (DY/2.0)*0.20);
    cairo_line_to (cr, 0, -(DY/2.0)*0.64);
    cairo_set_source_rgb (cr, 1.0, 0, 0);
    cairo_stroke (cr);
    cairo_arc (cr, 0, -(DY/2.0)*0.64, DX*0.03, 0, 2*M_PI);
    cairo_fill (cr);
    cairo_restore (cr);
  }


  cairo_surface_write_to_png (s, "foo.png");

  cairo_destroy (cr);
  cairo_surface_destroy (s);
}


void draw_hands (Display* display, Visual* visual,
		 Pixmap pixmap, int dx, int dy, int seconds)
{
  cairo_surface_t* s = cairo_xlib_surface_create (display, pixmap, visual,
						  dx, dy);
  cairo_t* cr = cairo_create (s);
  cairo_surface_destroy (s);

	// -- Setup initial matrix
  cairo_translate (cr, (double) dx/2.0, (double) dy/2.0);
  cairo_scale (cr, (double) dx/DX, (double) dy/DY);

	// -- Draw hands
  {
    seconds %= 60*60*12;
    float minute = ((double) (seconds%(60*60)))/60.0;
    float hour = ((double) seconds)/(60.0*60.0);
//    printf ("time %d %.2f %.2f\n", seconds, minute, hour);

	// Hour hand
    cairo_save (cr);
    cairo_rotate (cr, ((2.0*M_PI)*hour)/12.0);
    cairo_set_line_width (cr, WIDTH_HAND);
    cairo_move_to (cr, 0, (DY/2.0)*0.15);
    cairo_line_to (cr, 0, -(DY/2.0)*0.58);
    cairo_stroke (cr);
    cairo_restore (cr);

	// Minute hand
    cairo_move_to (cr, 0,  (DY/2.0)*0.15);
    cairo_line_to (cr, 0, -(DY/2.0)*0.75);
    cairo_path_t* path = cairo_copy_path (cr);
    cairo_set_line_width (cr, WIDTH_HAND);
    cairo_new_path (cr);

#if 0
    cairo_save (cr);
    cairo_translate (cr, WIDTH_THIN*2, WIDTH_THIN*2);
    cairo_rotate (cr, ((2.0*M_PI)*minute)/60.0);
    cairo_append_path (cr, path);
    cairo_set_source_rgb (cr, 0.7, 0.7, 0.7);
    cairo_stroke (cr);
    cairo_restore (cr);
#endif

    cairo_save (cr);
    cairo_rotate (cr, ((2.0*M_PI)*minute)/60.0);
    cairo_append_path (cr, path);
    cairo_stroke (cr);
    cairo_restore (cr);

    cairo_path_destroy (path);

	// Second hand
    cairo_save (cr);
    cairo_rotate (cr, ((2.0*M_PI)*seconds)/60.0);
    cairo_set_line_width (cr, WIDTH_THIN);
    cairo_move_to (cr, 0,  (DY/2.0)*0.20);
    cairo_line_to (cr, 0, -(DY/2.0)*0.64);
    cairo_set_source_rgb (cr, 1.0, 0, 0);
    cairo_stroke (cr);
    cairo_arc (cr, 0, -(DY/2.0)*0.64, DX*0.03, 0, 2*M_PI);
    cairo_fill (cr);
    cairo_restore (cr);
  }

  cairo_destroy (cr);
}

void draw_dial (Display* display, Visual* visual,
		Pixmap pixmap, int dx, int dy)
{
  cairo_surface_t* s = cairo_xlib_surface_create (display, pixmap, visual,
						  dx, dy);
//  cairo_surface_t* s = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
//						   DX, DY);
  cairo_t* cr = cairo_create (s);
  cairo_surface_destroy (s);

	// -- Setup initial matrix
  cairo_translate (cr, (double) dx/2.0, (double) dy/2.0);
  cairo_scale (cr, (double) dx/DX, (double) dy/DY);
  //  cairo_rotate (cr, M_PI/8);

	// -- Erase
  cairo_save (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
  cairo_paint (cr);
  cairo_restore (cr);
//  cairo_rectangle (cr, -DX/2.0, -DY/2.0, DX, DY);
//  cairo_set_source_rgb (cr, 0.9, 0, 0);
//  cairo_fill (cr);

	// -- Draw border

  cairo_arc (cr, 0, 0, DX/2 - WIDTH_BORDER/2, 0, 2*M_PI);
  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_fill_preserve (cr);
//  cairo_arc (cr, DX/2.0, DY/2.0, DX/2 - WIDTH_THICK, 0, 2*M_PI);
  cairo_set_line_width (cr, WIDTH_BORDER);
  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_stroke (cr);

	// -- Draw ticks

  for (int i = 0; i < 60; ++i) {
    cairo_save (cr);
    cairo_rotate (cr, (2*M_PI)*i/60.0);
    cairo_set_line_width (cr, (i%5 == 0) ? WIDTH_THICK : WIDTH_THIN);
    cairo_move_to (cr, (DX/2.0)*0.88, 0);
    cairo_line_to (cr, (i%5 == 0) ? (DX/2.0)*0.70 : (DX/2.0)*0.81, 0);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_stroke (cr);
    cairo_restore (cr);
  }

	// -- Draw brand
  {
    const char* sz = "Buici";
    cairo_save (cr);
    cairo_select_font_face (cr, FONT,
			    CAIRO_FONT_SLANT_ITALIC,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_set_font_size (cr, DX*0.08);
    cairo_text_extents_t ex;
    cairo_text_extents (cr, sz, &ex);
    cairo_translate (cr, -ex.width/2 + ex.x_bearing,
		     -ex.y_bearing/2 - (DX/2)*0.35);
    cairo_show_text (cr, sz);

    cairo_restore (cr);
  }

#if 0

	// -- Draw hands
  {
    time_t t = time (NULL);
    struct tm tm;
    localtime_r (&t, &tm);

    int second = tm.tm_sec; // t%60;
    float minute = tm.tm_min + second/60.0; //(t/60)%60;
    float hour = tm.tm_hour + minute/60.0; //(t/(60*60))%12;

    cairo_save (cr);
    cairo_rotate (cr, ((2.0*M_PI)*hour)/12.0);
    cairo_set_line_width (cr, WIDTH_HAND);
    cairo_move_to (cr, 0, (DY/2.0)*0.15);
    cairo_line_to (cr, 0, -(DY/2.0)*0.58);
    cairo_stroke (cr);
    cairo_restore (cr);

    cairo_save (cr);
    cairo_rotate (cr, ((2.0*M_PI)*minute)/60.0);
    cairo_set_line_width (cr, WIDTH_HAND);
    cairo_move_to (cr, 0,  (DY/2.0)*0.15);
    cairo_line_to (cr, 0, -(DY/2.0)*0.75);
    cairo_stroke (cr);
    cairo_restore (cr);

    cairo_save (cr);
    cairo_rotate (cr, ((2.0*M_PI)*second)/60.0);
    cairo_set_line_width (cr, WIDTH_THIN);
    cairo_move_to (cr, 0,  (DY/2.0)*0.20);
    cairo_line_to (cr, 0, -(DY/2.0)*0.64);
    cairo_set_source_rgb (cr, 1.0, 0, 0);
    cairo_stroke (cr);
    cairo_arc (cr, 0, -(DY/2.0)*0.64, DX*0.03, 0, 2*M_PI);
    cairo_fill (cr);
    cairo_restore (cr);
  }


  cairo_surface_write_to_png (s, "foo.png");
#endif

  cairo_destroy (cr);
}


void draw_dial_shape (Display* display, Pixmap pixmap, int dx, int dy)
{
  cairo_surface_t* s
    = cairo_xlib_surface_create_for_bitmap (display, pixmap,
					    XDefaultScreenOfDisplay (display),
					    dx, dy);
  cairo_t* cr = cairo_create (s);
  cairo_surface_destroy (s);

	// -- Setup initial matrix
  cairo_translate (cr, (double) dx/2.0, (double) dy/2.0);
  cairo_scale (cr, (double) dx/DX, (double) dy/DY);

	// -- Erase
  cairo_save (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
  cairo_paint (cr);
  cairo_restore (cr);

	// -- Draw border and fill
  cairo_arc (cr, 0, 0, DX/2 - WIDTH_BORDER/2, 0, 2*M_PI);
  cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 1.0);
  cairo_fill_preserve (cr);
  cairo_set_line_width (cr, WIDTH_BORDER);
  cairo_stroke (cr);

  cairo_destroy (cr);
}


#if defined (TEST)
int main (int, char**)
{
  draw ();

  return 0;
}
#endif
