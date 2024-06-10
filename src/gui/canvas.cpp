#include "canvas.hpp"

#include <cairo.h>
#include <cstdio>
#include <gtk/gtk.h>
#include <vector>

using namespace std;

const float CANVAS_FAC = 0.3;

GtkWidget *canvas;
void (*on_canvas_updated)(CanvasState);

/* Wrapper to fully redraw a widget */
void queue_draw_area(GtkWidget *widget) {
  int width = gtk_widget_get_allocated_width(widget);
  int height = gtk_widget_get_allocated_height(widget);
  gtk_widget_queue_draw_area(widget, 0, 0, width, height);
}

void redraw_canvas() {
  queue_draw_area(canvas);
}

void attach_canvas_updated_callback(void (*func)(CanvasState)) {
  on_canvas_updated = func;
}

gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
  struct CanvasState *state = (struct CanvasState *)data;

  GdkRGBA color;
  GtkStyleContext *context = gtk_widget_get_style_context(widget);
  guint width = gtk_widget_get_allocated_width(widget);
  guint height = gtk_widget_get_allocated_height(widget);

  // Render the background
  gtk_render_background(context, cr, 0, 0, width, height);

  // Get the current color from the style context and set it as the source color
  gtk_style_context_get_color(context, gtk_style_context_get_state(context), &color);
  gdk_cairo_set_source_rgba(cr, &color);

  // Draw boxes with outline
  for (int i = 0; i < state->boxes.size(); i++) {
    Box *box = &state->boxes.at(i);
    float x = box->x;
    float y = box->y;

    cairo_rectangle(cr, x * CANVAS_FAC, y * CANVAS_FAC, box->width * CANVAS_FAC, box->height * CANVAS_FAC);
    cairo_stroke_preserve(cr);
  }

  // Draw bounds of canvas for debugging
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_stroke_preserve(cr);

  // Draw a filled dot for debugging
  cairo_new_path(cr);
  float x = state->drag_start_x + state->drag_delta_x;
  float y = state->drag_start_y + state->drag_delta_y;
  cairo_arc(cr, x, y, 5, 0, 2 * G_PI); // 5 is the radius of the dot
  cairo_fill(cr);

  return FALSE;
}


void on_drag_start(GtkGestureDrag *drag_, gdouble start_x, gdouble start_y, gpointer data) {
  // g_print("\nStart: (%f, %f)\n", start_x, start_y);
  struct CanvasState *state = (struct CanvasState *)data;

  state->drag_start_x = start_x;
  state->drag_start_y = start_y;
  state->drag_delta_x = 0;
  state->drag_delta_y = 0;

  for (int i = 0; i < state->boxes.size(); i++) {
    Box box = state->boxes.at(i);
    if (box.within(start_x / CANVAS_FAC, start_y / CANVAS_FAC)) {
      state->selected_box = i;
      state->box_start_x = box.x;
      state->box_start_y = box.y;
      break;
    }
  }
  on_canvas_updated(*state);

  queue_draw_area(canvas);
}

void on_drag_update(GtkGestureDrag *drag_, gdouble delta_x, gdouble delta_y, gpointer data) {
  // g_print("Update: (%f, %f)\n", delta_x, delta_y);
  struct CanvasState *state = (struct CanvasState *)data;

  state->drag_delta_x = delta_x;
  state->drag_delta_y = delta_y;

  if (state->selected_box != -1) {
    int canvas_width = gtk_widget_get_allocated_width(canvas);
    int canvas_height = gtk_widget_get_allocated_height(canvas);

    // Set new position of held box
    Box *box = &state->boxes.at(state->selected_box);
    float x = state->box_start_x + state->drag_delta_x / CANVAS_FAC;
    float y = state->box_start_y + state->drag_delta_y / CANVAS_FAC;
    // Clamp box to bounds of canvas
    box->x = MIN(MAX(x, 0), canvas_width / CANVAS_FAC - box->width);
    box->y = MIN(MAX(y, 0), canvas_height / CANVAS_FAC - box->height);

    on_canvas_updated(*state);
  }

  queue_draw_area(canvas);
}

void on_drag_end(GtkGestureDrag *drag_, gdouble delta_x, gdouble delta_y, gpointer data) {
  // g_print("End: (%f, %f)\n", delta_x, delta_y);
  struct CanvasState *state = (struct CanvasState *)data;
  state->selected_box = -1;

  queue_draw_area(canvas);
}


GtkWidget *get_canvas(CanvasState *state) {
  canvas = gtk_drawing_area_new();

  g_signal_connect(G_OBJECT(canvas), "draw", G_CALLBACK(draw_callback), state);

  GtkGesture *canvas_drag1_controller = gtk_gesture_drag_new(canvas);
  g_signal_connect(canvas_drag1_controller, "drag-begin", G_CALLBACK(on_drag_start), state);
  g_signal_connect(canvas_drag1_controller, "drag-update", G_CALLBACK(on_drag_update), state);
  g_signal_connect(canvas_drag1_controller, "drag-end", G_CALLBACK(on_drag_end), state);

  return canvas;
}
