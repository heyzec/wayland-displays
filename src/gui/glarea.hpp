#include "gui/box.hpp"

#include <gtk/gtk.h>
#include <string>
#include <vector>

void update_glarea(std::vector<Box> new_boxes, std::vector<std::string> names);

void setup_glarea(GtkWidget *gl_area);
