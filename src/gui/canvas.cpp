#include "canvas.hpp"

#include <cairo.h>
#include <gtk/gtk.h>
#include <vector>

/* A number larger than canvas size */
#define INF 1500
/* Minimum distance between sides of boxes for snapping to occur */
#define SNAP_DIST 50
/* Scaling factor between compositor pixels and canvas pixels */
const float CANVAS_FAC = 0.15;

GtkWidget *canvas;
void (*on_canvas_updated)(CanvasState);

// ============================================================
// Helpers
// ============================================================

/* Snap selected box to other boxes.
 * x, y: coordinates of the selected box, as determined by dragging, to be modified
 * boxes: all the boxes on the canvas
 * selected: index of the selected box in boxes
 */
static void snap(float *x, float *y, std::vector<Box> boxes, int selected) {
  Box *box = &boxes.at(selected);

  // When b == 0, snap on y, and when b == 1, snap on x
  for (int b = 0; b < 2; b++) {
    float closest = -1;
    float closest_dist = INF;
    for (int i = 0; i < boxes.size(); i++) {
      // Don't snap selected box to itself!
      if (i == selected) {
        continue;
      }

      const Box target = boxes.at(i);
      float dist;
      // 1) Try snap left/top of dragged box to left/top of target box
      dist = b ? ABS(*x - target.x) : ABS(*y - target.y);
      if (dist < closest_dist) {
        closest = b ? target.x : target.y;
        closest_dist = dist;
      }
      // 2) Try snap left/top of dragged box to right/bottom of target box
      dist = b ? ABS(*x - (target.x + target.width)) : ABS(*y - (target.y + target.height));
      if (dist < closest_dist) {
        closest = b ? target.x + target.width : target.y + target.height;
        closest_dist = dist;
      }
      // 3) Try snap right/bottom of dragged box to left/top of target box
      dist = b ? ABS((*x + box->width) - target.x) : ABS((*y + box->height) - target.y);
      if (dist < closest_dist) {
        closest = b ? target.x - box->width : target.y - box->height;
        closest_dist = dist;
      }
      // 4) Try snap right/bottom of dragged box to right/bottom of target box
      dist = b ? ABS((*x + box->width) - (target.x + target.width))
               : ABS((*y + box->height) - (target.y + target.height));
      if (dist < closest_dist) {
        closest = b ? target.x + target.width - box->width : target.y + target.height - box->height;
        closest_dist = dist;
      }
    }
    if (closest_dist < SNAP_DIST) {
      if (b) {
        *x = closest;
      } else {
        *y = closest;
      }
    }
  }
}

/* Wrapper to fully redraw a widget */
void queue_draw_area(GtkWidget *widget) {
  int width = gtk_widget_get_allocated_width(widget);
  int height = gtk_widget_get_allocated_height(widget);
  gtk_widget_queue_draw_area(widget, 0, 0, width, height);
}

void set_stroke_thin(cairo_t *cr) {
  cairo_set_line_width(cr, 0.5);
  cairo_set_source_rgba(cr, 0, 0, 0, 0.5);
}

void set_stroke_normal(cairo_t *cr, GtkWidget *widget) {
  GdkRGBA color;
  cairo_set_line_width(cr, 2);
  // Get the current color from the style context and set it as the source color
  GtkStyleContext *context = gtk_widget_get_style_context(widget);
  gtk_style_context_get_color(context, gtk_style_context_get_state(context), &color);
  gdk_cairo_set_source_rgba(cr, &color);
}

gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
  struct CanvasState *state = (struct CanvasState *)data;

  guint width = gtk_widget_get_allocated_width(widget);
  guint height = gtk_widget_get_allocated_height(widget);

  // Draw outline of boxes
  set_stroke_normal(cr, widget);
  for (const Box box : state->boxes) {
    cairo_rectangle(cr, box.x * CANVAS_FAC, box.y * CANVAS_FAC, box.width * CANVAS_FAC,
                    box.height * CANVAS_FAC);
  }
  cairo_stroke(cr);

  if (state->selected_box == -1) {
    return FALSE;
  }

  set_stroke_thin(cr);
  for (const Box box : state->boxes) {
    // Draw vertical guidelines
    for (float x : std::vector<float>{box.x, box.x + box.width}) {
      cairo_move_to(cr, x * CANVAS_FAC, 0);
      cairo_line_to(cr, x * CANVAS_FAC, INF);
    }

    // Draw horizontal guidelines
    for (float y : std::vector<float>{box.y, box.y + box.height}) {
      cairo_move_to(cr, 0, y * CANVAS_FAC);
      cairo_line_to(cr, INF, y * CANVAS_FAC);
    }
  }
  cairo_stroke(cr);

  // Draw top and left bounds of canvas
  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, 0, INF);
  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, INF, 0);
  cairo_stroke(cr);

  // Draw a filled dot for debugging
  cairo_new_path(cr);
  float x = state->drag_start_x + state->drag_delta_x;
  float y = state->drag_start_y + state->drag_delta_y;
  cairo_arc(cr, x, y, 5, 0, 2 * G_PI); // 5 is the radius of the dot
  cairo_fill(cr);

  return FALSE;
}

// ============================================================
// Drag callbacks
// ============================================================

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

  if (state->selected_box == -1) {
    return;
  }

  int canvas_width = gtk_widget_get_allocated_width(canvas);
  int canvas_height = gtk_widget_get_allocated_height(canvas);

  // Set new position of held box
  Box *box = &state->boxes.at(state->selected_box);
  float x = state->box_start_x + delta_x / CANVAS_FAC;
  float y = state->box_start_y + delta_y / CANVAS_FAC;

  // Modify x and y using the snapping algorithm
  snap(&x, &y, state->boxes, state->selected_box);

  // Clamp box to bounds of canvas
  box->x = MIN(MAX(x, 0), canvas_width / CANVAS_FAC - box->width);
  box->y = MIN(MAX(y, 0), canvas_height / CANVAS_FAC - box->height);

  on_canvas_updated(*state);

  queue_draw_area(canvas);
}

void on_drag_end(GtkGestureDrag *drag_, gdouble delta_x, gdouble delta_y, gpointer data) {
  // g_print("End: (%f, %f)\n", delta_x, delta_y);
  struct CanvasState *state = (struct CanvasState *)data;
  state->selected_box = -1;

  queue_draw_area(canvas);
}

// ============================================================
// Entry point
// ============================================================

void setup_canvas(GtkDrawingArea *drawing_area, CanvasState *state) {
  canvas = GTK_WIDGET(drawing_area);

  g_signal_connect(G_OBJECT(canvas), "draw", G_CALLBACK(draw_callback), state);

  GtkGesture *canvas_drag1_controller = gtk_gesture_drag_new(canvas);
  g_signal_connect(canvas_drag1_controller, "drag-begin", G_CALLBACK(on_drag_start), state);
  g_signal_connect(canvas_drag1_controller, "drag-update", G_CALLBACK(on_drag_update), state);
  g_signal_connect(canvas_drag1_controller, "drag-end", G_CALLBACK(on_drag_end), state);
}

void attach_canvas_updated_callback(void (*func)(CanvasState)) {
  on_canvas_updated = func;
}

void redraw_canvas() {
  queue_draw_area(canvas);
}
