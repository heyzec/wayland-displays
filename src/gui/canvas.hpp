#include <gtk/gtk.h>
#include <vector>

struct Box {
  float x;
  float y;
  float width;
  float height;

  bool within(float pt_x, float pt_y) {
    return x <= pt_x && pt_x <= x + width && y <= pt_y && pt_y <= y + height;
  }
};

/* Container to store dynamic state of canvas, will be passed around callbacks */
struct CanvasState {
  std::vector<struct Box> boxes;

  // Drag
  float drag_start_x;
  float drag_start_y;
  float drag_delta_x;
  float drag_delta_y;

  int selected_box = -1; // -1 if no box held
  float box_start_x;
  float box_start_y;
};

GtkWidget *get_canvas(CanvasState *);

void attach_canvas_updated_callback(void (*func)(CanvasState));

void redraw_canvas();
