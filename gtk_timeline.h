/*
 *  Copyright (c) 2021 Giuseppe Torelli <colossus73@gmail.com>
 *   *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License,or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not,write to the Free Software
 *  Foundation,Inc.,59 Temple Place - Suite 330,Boston,MA 02111-1307,USA.
 *
 */

#ifndef __GTK_TIMELINE_H__
#define __GTK_TIMELINE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * ImgTimeline:
 *
 * A timeline video/audio widget for use with Imagination.
 *
 */
struct _ImgTimeline
{
  /*< private >*/
  GtkLayout da;
};

enum
{
  PROP_0,
  //color properties
  VIDEO_BACKGROUND,
  AUDIO_BACKGROUND,
  TOTAL_TIME,
  TIME_MARKER_XPOS
};

#define GTK_TIMELINE_TYPE gtk_timeline_get_type()

G_DECLARE_FINAL_TYPE(ImgTimeline, gtk_timeline, GTK, TIMELINE, GtkLayout)

//Public functions.
GtkWidget* gtk_timeline_new();

//Set and get colors.
void gtk_timeline_set_video_background	(ImgTimeline *da, const gchar *background_string);
void gtk_timeline_set_audio_background	(ImgTimeline *da, const gchar *background_string);
void gtk_timeline_adjust_zoom			(GtkWidget *da, gint zoom, gint direction);
void gtk_timeline_set_total_time		(ImgTimeline *da, gint total_time);
void gtk_timeline_add_slide				(GtkWidget *da, gchar *filename, gint posx);
void gtk_timeline_draw_time_marker(GtkWidget *widget, cairo_t *cr, gint pos_X);
void gtk_timeline_set_time_marker(ImgTimeline *widget, gint pos_X);
gboolean gtk_timeline_slide_button_pressed(GtkWidget *widget, GdkEvent *event, ImgTimeline *da);
gboolean gtk_timeline_slide_button_released(GtkWidget *widget, GdkEvent *event, ImgTimeline *da);
gboolean gtk_timeline_slide_motion(GtkWidget *widget, GdkEventCrossing *event, ImgTimeline *da);
GtkWidget *gtk_timeline_private_get_slide_selected(ImgTimeline *da);

G_END_DECLS

#endif 
