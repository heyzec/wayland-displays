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

void setup_canvas(GtkDrawingArea *drawing_area, std::vector<Box> boxes);

void refresh_canvas(std::vector<Box> boxes);

void attach_canvas_updated_callback(void (*func)(int, std::vector<Box>));
