#include <cairo.h>
#include <cstdio>
#include <gtk/gtk.h>
#include <vector>

using namespace std;

struct Box {
  float x;
  float y;
  float width;
  float height;

  bool within(float pt_x, float pt_y) {
    return x <= pt_x && pt_x <= x + width && y <= pt_y && pt_y <= y + height;
  }
};

struct Drag {
  float start_x;
  float start_y;
  float delta_x;
  float delta_y;
};

struct CanvasState {
  GtkWidget *canvas;
  vector<struct Box> *boxes;
  struct Drag *drag;
  int selected_box = -1; // -1 if no box held
};

gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
  struct CanvasState *state = (struct CanvasState *)data;
  struct Drag *drag = state->drag;

  GdkRGBA color;
  GtkStyleContext *context = gtk_widget_get_style_context(widget);
  guint width = gtk_widget_get_allocated_width(widget);
  guint height = gtk_widget_get_allocated_height(widget);

  // Render the background
  gtk_render_background(context, cr, 0, 0, width, height);

  // Get the current color from the style context and set it as the source color
  gtk_style_context_get_color(context, gtk_style_context_get_state(context), &color);
  gdk_cairo_set_source_rgba(cr, &color);

  // Draw all boxes with outline
  for (int i = 0; i < state->boxes->size(); i++) {
    Box *box = &state->boxes->at(i);
    float x = box->x;
    float y = box->y;
    if (i == state->selected_box) {
      x += drag->delta_x;
      y += drag->delta_y;
    }

    cairo_rectangle(cr, x, y, box->width, box->height);
    cairo_stroke_preserve(cr);
  }

  // Draw bounds of canvas for debugging
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_stroke_preserve(cr);

  // Draw a filled dot for debugging
  cairo_new_path(cr);
  float x = drag->start_x + drag->delta_x;
  float y = drag->start_y + drag->delta_y;
  cairo_arc(cr, x, y, 5, 0, 2 * G_PI); // 5 is the radius of the dot
  cairo_fill(cr);

  return FALSE;
}

/* Wrapper to fully redraw a widget */
void queue_draw_area(GtkWidget *widget) {
  int width = gtk_widget_get_allocated_width(widget);
  int height = gtk_widget_get_allocated_height(widget);
  gtk_widget_queue_draw_area(widget, 0, 0, width, height);
}

void on_drag_start(GtkGestureDrag *drag, gdouble start_x, gdouble start_y, gpointer data) {
  g_print("\nStart: (%f, %f)\n", start_x, start_y);
  struct CanvasState *state = (struct CanvasState *)data;

  state->drag->start_x = start_x;
  state->drag->start_y = start_y;
  state->drag->delta_x = 0;
  state->drag->delta_y = 0;

  for (int i = 0; i < state->boxes->size(); i++) {
    Box box = state->boxes->at(i);
    if (box.within(start_x, start_y)) {
      state->selected_box = i;
    }
  }

  queue_draw_area(state->canvas);
}

void on_drag_update(GtkGestureDrag *drag, gdouble delta_x, gdouble delta_y, gpointer data) {
  g_print("Update: (%f, %f)\n", delta_x, delta_y);
  struct CanvasState *state = (struct CanvasState *)data;

  state->drag->delta_x = delta_x;
  state->drag->delta_y = delta_y;

  queue_draw_area(state->canvas);
}

void on_drag_end(GtkGestureDrag *drag, gdouble delta_x, gdouble delta_y, gpointer data) {
  g_print("End: (%f, %f)\n", delta_x, delta_y);
  struct CanvasState *state = (struct CanvasState *)data;

  if (state->selected_box != -1) {
    // Set new position of held box
    Box *box = &state->boxes->at(state->selected_box);
    box->x += state->drag->delta_x;
    box->y += state->drag->delta_y;
    state->selected_box = -1;
  }

  queue_draw_area(state->canvas);
}

GtkWidget *get_canvas(vector<Box> *boxes) {
  GtkWidget *canvas = gtk_drawing_area_new();

  struct CanvasState *state = new struct CanvasState;
  state->drag = new struct Drag;
  state->canvas = canvas;
  state->boxes = boxes;

  g_signal_connect(G_OBJECT(canvas), "draw", G_CALLBACK(draw_callback), state);

  GtkGesture *canvas_drag1_controller = gtk_gesture_drag_new(canvas);
  g_signal_connect(canvas_drag1_controller, "drag-begin", G_CALLBACK(on_drag_start), state);
  g_signal_connect(canvas_drag1_controller, "drag-update", G_CALLBACK(on_drag_update), state);
  g_signal_connect(canvas_drag1_controller, "drag-end", G_CALLBACK(on_drag_end), state);

  return canvas;
}
