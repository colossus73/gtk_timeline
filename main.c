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

/*
    gcc -Wall gtk_timeline.c main.c -o timeline `pkg-config gtk+-3.0 --cflags --libs` -lm
*/

#include <gtk/gtk.h>
#include "gtk_timeline.h"
#include <stdlib.h>

static gboolean gtk_timeline_scroll( GtkWidget *widget, GdkEventScroll *event, GtkWidget *scrolledwindow);
static void gtk_timeline_drag_data_received (GtkWidget *timeline, GdkDragContext *context, gint x, gint y, GtkSelectionData *data, guint info, guint time, gpointer pointer);
static gboolean gtk_timeline_slide_drag_motion (GtkWidget *button, GdkDragContext *context, gint x, gint y, guint time, gpointer data);

static gint zoom = 1;

int main(int argc, char *argv[])
{
  static GtkTargetEntry drop_target[] = {
      { "GTK_TIMELINE_SLIDE", GTK_TARGET_SAME_APP, 0 },
      {"text/uri-list", 0, 0}
    };
  gtk_init(&argc, &argv);

  GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Imagination timeline widget");
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 800, 200);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *timeline = gtk_timeline_new();
  gtk_drag_dest_set (timeline, GTK_DEST_DEFAULT_ALL, drop_target, G_N_ELEMENTS(drop_target), GDK_ACTION_MOVE);
  g_object_set(timeline, "total_time",        300, NULL);
  g_object_set(timeline, "video_background", "#0084ff", NULL);
  g_object_set(timeline, "audio_background", "#0084ff", NULL);

  gtk_widget_add_events( timeline, GDK_BUTTON1_MOTION_MASK
                         | GDK_POINTER_MOTION_MASK
                         | GDK_BUTTON_PRESS_MASK
                         | GDK_BUTTON_RELEASE_MASK
                         | GDK_SCROLL_MASK
                         | GDK_KEY_PRESS_MASK );
 // GtkWidget *viewport = gtk_viewport_new(NULL,NULL);
  GtkWidget *scrolledwindow1 = gtk_scrolled_window_new(NULL, NULL);
  g_signal_connect(G_OBJECT(timeline), "scroll-event",      G_CALLBACK(gtk_timeline_scroll), scrolledwindow1);
  g_signal_connect(G_OBJECT(timeline), "drag-data-received", G_CALLBACK(gtk_timeline_drag_data_received), NULL);
  g_signal_connect(G_OBJECT(timeline), "drag-motion",       G_CALLBACK (gtk_timeline_slide_drag_motion), NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_ALWAYS, GTK_POLICY_NEVER);
  
  //gtk_container_add (GTK_CONTAINER(scrolledwindow1), viewport);
  gtk_container_add (GTK_CONTAINER(scrolledwindow1), timeline);
  gtk_container_add (GTK_CONTAINER (window), scrolledwindow1);
  gtk_timeline_add_slide(timeline, "landscape.jpg", 0);
  gtk_timeline_add_slide(timeline, "cappuccetto_rosso.jpg", 0);
  gtk_timeline_add_slide(timeline, "landscape_grass_river.jpg", 0);
  gtk_timeline_add_slide(timeline, "file_not_found.jpg", 0);
  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}

static gboolean gtk_timeline_scroll(GtkWidget *timeline, GdkEventScroll *event, GtkWidget *scrolledwindow)
{
  GdkModifierType accel_mask = gtk_accelerator_get_default_mod_mask();
  GtkAdjustment *scrollX     = gtk_scrollable_get_hadjustment(GTK_SCROLLABLE(timeline));
  gdouble deltax, deltay;

  //g_print("scrollX: %2f\n",gtk_adjustment_get_value(scrollX));
  gdk_event_get_scroll_deltas((GdkEvent *)event, &deltax, &deltay);

  if ( (event->state & accel_mask) == GDK_CONTROL_MASK)
  {
    if (deltay > 0)
      zoom --;
    else if (deltay < 0)
      zoom ++;

    if (zoom < 1)
        zoom = 1;
    gtk_timeline_adjust_zoom(timeline, zoom, deltay);
  }
  else if (deltay > 0)
    gtk_adjustment_set_value(scrollX, gtk_adjustment_get_value(scrollX) + 40);
  else
    gtk_adjustment_set_value(scrollX, gtk_adjustment_get_value(scrollX) - 40);

  return TRUE;
}

void gtk_timeline_drag_data_received (GtkWidget *timeline, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint info, guint time, gpointer pointer)
{
  gchar **images = NULL;
  gchar *filename;
  gint i = 0;

  images = gtk_selection_data_get_uris(selection_data);
  if (images)
  {
    while(images[i])
    {
      filename = g_filename_from_uri (images[i], NULL, NULL);
      gtk_timeline_add_slide(timeline, filename, x);
      g_free(filename);
      i++;
    }
  }
  g_strfreev (images);

  gtk_drag_finish (context, TRUE, FALSE, time);
}

static gboolean gtk_timeline_slide_drag_motion (GtkWidget *timeline, GdkDragContext *context, gint x, gint y, guint time, gpointer data)
{
  GtkWidget *button;

  //button = *(gpointer*)gtk_selection_data_get_data (context);
  button = gtk_drag_get_source_widget(context);

  if (GTK_IS_WIDGET(button))
    gtk_layout_move(GTK_LAYOUT(timeline), button, x - 47.5, 43);

  return TRUE;
}