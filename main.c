//gcc -Wall gtk_timeline.c main.c -o timeline `pkg-config gtk+-3.0 --cflags --libs` -lm

#include <gtk/gtk.h>
#include "gtk_timeline.h"
#include <stdlib.h>

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
  gtk_window_set_default_size(GTK_WINDOW(window), 800, 160);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *timeline = gtk_timeline_new();
  gtk_drag_dest_set (timeline, GTK_DEST_DEFAULT_ALL, drop_target, G_N_ELEMENTS(drop_target), GDK_ACTION_MOVE);
  g_object_set(timeline, "total_time",        300, NULL);
  g_object_set(timeline, "video_background", "#0084ff", NULL);
  g_object_set(timeline, "audio_background", "#0084ff", NULL);

  gtk_widget_add_events( timeline, 
                           GDK_BUTTON1_MOTION_MASK
                         | GDK_BUTTON_MOTION_MASK
                         | GDK_BUTTON_PRESS_MASK   
                         | GDK_BUTTON_RELEASE_MASK
                         | GDK_SCROLL_MASK
                         | GDK_KEY_PRESS_MASK );
  GtkWidget *viewport = gtk_viewport_new(NULL,NULL);
  GtkWidget *scrolledwindow1 = gtk_scrolled_window_new(NULL, NULL);
  g_signal_connect(G_OBJECT(timeline), "scroll-event",      G_CALLBACK(gtk_timeline_scroll), viewport);
  g_signal_connect(G_OBJECT(timeline), "drag-data-received",G_CALLBACK(gtk_timeline_drag_data_received), NULL);
  g_signal_connect(G_OBJECT(timeline), "button-press-event",G_CALLBACK(gtk_timeline_mouse_button_press), timeline);
  g_signal_connect(G_OBJECT(timeline), "motion-notify-event",G_CALLBACK(gtk_timeline_motion_notify), timeline);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_ALWAYS, GTK_POLICY_AUTOMATIC);
  
  gtk_container_add (GTK_CONTAINER(viewport), timeline);
  gtk_container_add (GTK_CONTAINER(scrolledwindow1), viewport);
  gtk_container_add (GTK_CONTAINER (window), scrolledwindow1);
  gtk_timeline_add_media(timeline, "landscape.jpg", 0);
  gtk_timeline_add_media(timeline, "cappuccetto_rosso.jpg", 0);
  gtk_timeline_add_media(timeline, "landscape_grass_river.jpg", 0);
  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}

