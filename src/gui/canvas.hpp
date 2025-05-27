#pragma once

#include "gui/box.hpp"

#include <gtk/gtk.h>
#include <vector>

void setup_canvas(GtkDrawingArea *drawing_area, std::vector<Box> boxes);

void refresh_canvas(std::vector<Box> boxes);

void attach_canvas_updated_callback(void (*func)(int, std::vector<Box>));
