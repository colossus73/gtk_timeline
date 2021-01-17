#include "gtk_timeline.h"

/**
 * ImgTimeline is a GTK+3 custom widget based on GtkLayout 
    which aims to give
 *
 */

#define GTK_TIMELINE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), GTK_TIMELINE_TYPE, ImgTimelinePrivate))

typedef struct _ImgTimelinePrivate ImgTimelinePrivate;

struct _ImgTimelinePrivate
{
  //colors
  gchar *video_background_string;
  gchar *audio_background_string;
  gdouble video_background[4];
  gdouble audio_background[4];
  
  gint last_slide_posX;
  gint zoom;
  gint seconds;
  gint minutes;
  gint hours;
  gint total_time;
  gint time_marker_xpos;
  gboolean button_pressed;

  cairo_surface_t *surface;
  GtkWidget *slide_selected;
};

//Private functions.
static void gtk_timeline_class_init(ImgTimelineClass *klass);
static void gtk_timeline_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void gtk_timeline_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void gtk_timeline_init(ImgTimeline *da);
static gboolean gtk_timeline_draw(GtkWidget *widget, cairo_t *cr);
static void gtk_timeline_draw_time_ticks(GtkWidget *widget, cairo_t *cr, gint width);
static void gtk_timeline_finalize(GObject *object);
static void gtk_timeline_drag_data_get(GtkWidget *widget, GdkDragContext *drag_context, GtkSelectionData *data, guint info, guint time,
gpointer user_data);

G_DEFINE_TYPE_WITH_CODE (ImgTimeline, gtk_timeline, GTK_TYPE_LAYOUT, G_ADD_PRIVATE (ImgTimeline))

static void gtk_timeline_class_init(ImgTimelineClass *klass)
{
  GObjectClass *gobject_class;
  GtkWidgetClass *widget_class;

  gobject_class=(GObjectClass*)klass;
  widget_class=(GtkWidgetClass*)klass;

  //Set the property funtions.
  gobject_class->set_property = gtk_timeline_set_property;
  gobject_class->get_property = gtk_timeline_get_property;

  //Draw when first shown.
  widget_class->draw = gtk_timeline_draw;
  gobject_class->finalize = gtk_timeline_finalize;
 
  g_object_class_install_property(gobject_class, VIDEO_BACKGROUND, g_param_spec_string("video_background", "video_background", "video_background", NULL, G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class, AUDIO_BACKGROUND, g_param_spec_string("audio_background", "audio_background", "audio_background", NULL, G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class, TOTAL_TIME,       g_param_spec_int("total_time", "total_time", "total_time", -1, G_MAXINT, 60, G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class, TIME_MARKER_XPOS, g_param_spec_int("time_marker_xpos", "time_marker_xpos", "time_marker_xpos", -1, G_MAXINT, 0, G_PARAM_READWRITE));
}
//Needed for g_object_set().
static void gtk_timeline_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  ImgTimeline *da = GTK_TIMELINE(object);

  switch(prop_id)
  {
      case VIDEO_BACKGROUND:
        gtk_timeline_set_video_background(da, g_value_get_string(value));
      break;
      case AUDIO_BACKGROUND:
        gtk_timeline_set_audio_background(da, g_value_get_string(value));
      break;
      case TOTAL_TIME:
        gtk_timeline_set_total_time(da, g_value_get_int(value));
      break; 
      case TIME_MARKER_XPOS:
        gtk_timeline_set_time_marker(da, g_value_get_int(value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

void gtk_timeline_set_video_background(ImgTimeline *da, const gchar *background_string)
{
  ImgTimelinePrivate *priv = gtk_timeline_get_instance_private(da);
  GdkRGBA rgba;
  
  gdk_rgba_parse(&rgba, background_string);
  priv->video_background[0] = rgba.red;
  priv->video_background[1] = rgba.green;
  priv->video_background[2] = rgba.blue;
  priv->video_background[3] = rgba.alpha;
  
  if(priv->video_background_string)
    g_free(priv->video_background_string);
  
  priv->video_background_string = g_strdup(background_string); 
}

void gtk_timeline_set_audio_background(ImgTimeline *da, const gchar *background_string)
{
  ImgTimelinePrivate *priv = gtk_timeline_get_instance_private(da);
  GdkRGBA rgba;
  
  gdk_rgba_parse(&rgba, background_string);
  priv->audio_background[0] = rgba.red;
  priv->audio_background[1] = rgba.green;
  priv->audio_background[2] = rgba.blue;
  priv->audio_background[3] = rgba.alpha;

  if(priv->audio_background_string)
    g_free(priv->audio_background_string);
  
  priv->audio_background_string = g_strdup(background_string); 
}

void gtk_timeline_set_total_time(ImgTimeline *da, gint total_time)
{
  ImgTimelinePrivate *priv = gtk_timeline_get_instance_private(da);

  priv->total_time = total_time;
}

void gtk_timeline_set_time_marker(ImgTimeline *da, gint posx)
{
  ImgTimelinePrivate *priv = gtk_timeline_get_instance_private(da);

  priv->time_marker_xpos = posx;
}

static void gtk_timeline_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  ImgTimeline *da = GTK_TIMELINE(object);
  ImgTimelinePrivate *priv = gtk_timeline_get_instance_private(da);
  
  switch(prop_id)
  {
    case VIDEO_BACKGROUND:
      g_value_set_string(value, priv->video_background_string);
      break;
    case AUDIO_BACKGROUND:
      g_value_set_string(value, priv->audio_background_string);
      break;
     case TOTAL_TIME:
      g_value_set_int(value, priv->total_time);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}

static void gtk_timeline_init(ImgTimeline *da)
{
  ImgTimelinePrivate *priv = gtk_timeline_get_instance_private(da);
  GtkCssProvider *css;

  css = gtk_css_provider_new();
  gtk_css_provider_load_from_data(css, "button {border-radius:0px}", -1, NULL);
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default(), GTK_STYLE_PROVIDER(css),GTK_STYLE_PROVIDER_PRIORITY_USER);
  g_object_unref (css);

  priv->last_slide_posX = 0;
  priv->zoom = 1;
  priv->seconds = 0;
  priv->minutes = 0;
  priv->hours = 0;
  priv->total_time = 0;
  priv->time_marker_xpos = 0;

  priv->video_background[0]=0.0;
  priv->video_background[1]=0.0;
  priv->video_background[2]=0.0;
  priv->video_background[3]=1.0;
  
  priv->audio_background[0]=0.0;
  priv->audio_background[1]=0.0;
  priv->audio_background[2]=0.0;
  priv->audio_background[3]=1.0;

  priv->video_background_string = g_strdup("rgba(0, 0, 0, 1.0)");
  priv->audio_background_string = g_strdup("rgba(0, 0, 0, 1.0)");
}

GtkWidget* gtk_timeline_new()
{
  return GTK_WIDGET(g_object_new(gtk_timeline_get_type(), NULL));
}

static gboolean gtk_timeline_draw(GtkWidget *da, cairo_t *cr)
{
  ImgTimelinePrivate *priv = gtk_timeline_get_instance_private((ImgTimeline*)da);

  gint width = gtk_widget_get_allocated_width(da);

  cairo_save(cr);
  cairo_translate (cr, 0 , 12);
  gtk_timeline_draw_time_ticks(da, cr, width);

  //Video timeline
  cairo_translate (cr, 0 , 20);
  cairo_set_source_rgba(cr, priv->video_background[0], priv->video_background[1], priv->video_background[2], priv->video_background[3]);
  cairo_set_line_width(cr, 1);
  cairo_rectangle(cr, 0,10, width - 2, 54);
  cairo_fill(cr);
  //separator
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_rectangle(cr, 0,64, width - 2, 3);
  cairo_fill(cr);
  //Audio timeline
  cairo_set_source_rgba(cr, priv->audio_background[0], priv->audio_background[1], priv->audio_background[2], priv->audio_background[3]);
  cairo_rectangle(cr, 0,68, width - 2, 54);
  cairo_fill(cr);
  cairo_restore(cr);

  //This is necessary to draw the slides represented by the GtkButtons
  GTK_WIDGET_CLASS (gtk_timeline_parent_class)->draw (da, cr);
  return TRUE;
}

static void gtk_timeline_finalize(GObject *object)
{ 
  ImgTimeline *da = GTK_TIMELINE(object);
  ImgTimelinePrivate *priv =gtk_timeline_get_instance_private(da);
  
  g_free(priv->video_background_string);
  g_free(priv->audio_background_string);
  
  if(priv->surface != NULL)
    cairo_surface_destroy(priv->surface);

  G_OBJECT_CLASS(gtk_timeline_parent_class)->finalize(object);
}

void gtk_timeline_draw_time_ticks(GtkWidget *da, cairo_t *cr, gint width)
{
  ImgTimelinePrivate *priv = gtk_timeline_get_instance_private((ImgTimeline*)da);
  cairo_text_extents_t extents;
  gchar *time;
  gint i, factor;
  gdouble distanceBetweenTicks, cairo_factor;

  distanceBetweenTicks = 48.5 - priv->zoom;
  factor = 2;

  if (distanceBetweenTicks <= 12)
    distanceBetweenTicks = 12;    
  
  gtk_widget_set_size_request(da, (priv->total_time * distanceBetweenTicks), -1);
  cairo_set_source_rgb(cr, 0,0,0);

  for (i = 0; i < priv->total_time; i++)
  {
    if (i % 2 == 0)
      cairo_factor = 0;
    else
      cairo_factor = 0.5;
    
    //Draw the line markers
    cairo_move_to( cr, i * distanceBetweenTicks + cairo_factor, 4);
    cairo_line_to( cr, i * distanceBetweenTicks + cairo_factor, 24);
    //Draw the sub line markers
    // if (distanceBetweenTicks > 12)
    // {
    //   for (j = 9; j < distanceBetweenTicks; j +=10)
    //   {
    //     cairo_move_to( cr, (i * distanceBetweenTicks + cairo_factor) + j , 12);
    //     cairo_line_to( cr, (i * distanceBetweenTicks + cairo_factor) + j , 24);
    //   }
    // }

    if (priv->zoom >= 8)
        factor = 4;
    else
        factor = 1;

    if (i % factor == 0)
    {  
      time = g_strdup_printf("%02d:%02d:%02d", priv->hours, priv->minutes, priv->seconds);
      cairo_select_font_face(cr, "Ubuntu", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_font_size(cr, 10);
      cairo_text_extents(cr, time, &extents);
      cairo_move_to(cr, (-extents.width/2) + i*distanceBetweenTicks, 0);  
      cairo_show_text(cr, time);
      g_free(time);
    }
    
    if (priv->seconds >= 59)
    {
        priv->seconds = -1;
        priv->minutes++;
    }   
    if (priv->minutes >= 59)
    {
        priv->minutes = 0;
        priv->hours++;
    }
    priv->seconds += priv->zoom;
    if (priv->seconds > 59)
      priv->seconds = 59;

    cairo_stroke(cr);
  }
  priv->seconds = priv->minutes = priv->hours = 0;
}

void gtk_timeline_adjust_zoom(GtkWidget *da, gint zoom, gint direction)
{
  ImgTimelinePrivate *priv = gtk_timeline_get_instance_private((ImgTimeline*)da);

  priv->zoom = zoom;
  if (direction > 0)
    priv->total_time--;
  else
    priv->total_time++;
  //TODO change 300 to the total time of the slideshow
  if (priv->total_time < 300)
      priv->total_time = 300;
g_print("adjust zoom: %d\n",priv->total_time);
  gtk_widget_queue_draw(GTK_WIDGET(da));
}

void gtk_timeline_add_slide(GtkWidget *da, gchar *filename, gint x)
{
  GdkPixbuf *pix;
  GtkWidget *img;
  GtkWidget *button;
  static GtkTargetEntry src_target[] = {
      { "GTK_TIMELINE_SLIDE", GTK_TARGET_SAME_APP, 0 }
    };
  ImgTimelinePrivate *priv = gtk_timeline_get_instance_private((ImgTimeline*)da);

  button = gtk_toggle_button_new();
  gtk_drag_source_set (button, GDK_BUTTON1_MASK, src_target, 1, GDK_ACTION_MOVE);
  g_signal_connect(G_OBJECT(button), "drag-data-get", G_CALLBACK (gtk_timeline_drag_data_get), NULL);
  pix = gdk_pixbuf_new_from_file_at_scale(filename, -1,40, TRUE, NULL);
  if (pix == NULL)
    img = gtk_image_new_from_icon_name("image-missing", GTK_ICON_SIZE_DIALOG);
  else
    img = gtk_image_new_from_pixbuf(pix);
  
  gtk_button_set_image (GTK_BUTTON (button), img);
  if (pix)
    g_object_unref(pix);

  gtk_container_add(GTK_CONTAINER(da), button);
  gtk_widget_show(button);
  if (x > 0)
    gtk_layout_move(GTK_LAYOUT(da), button, x - 47.5, 43);
  else
    gtk_layout_move(GTK_LAYOUT(da), button, priv->last_slide_posX, 43);
  priv->last_slide_posX += 95;
  gtk_widget_set_size_request(button, 95, -1);
}

void gtk_timeline_draw_time_marker(GtkWidget *widget, cairo_t *cr, gint pos_X)
{
  cairo_set_source_rgb(cr, 1,0,0);

}

void gtk_timeline_drag_data_get(GtkWidget *widget, GdkDragContext *drag_context, GtkSelectionData *data, guint info, guint time,
gpointer user_data)
{
  gtk_selection_data_set (data, gdk_atom_intern_static_string ("GTK_TIMELINE_SLIDE"), 32, (const guchar *)&widget, sizeof (gpointer));
}
