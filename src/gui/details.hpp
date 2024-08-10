#include "common/shapes.hpp"
#include <gtk/gtk.h>

// Only allow these globals to be modified from gui.cpp!
extern std::vector<DisplayInfo> displays;
extern int selected_display;

void refresh_details();

void setup_details(GtkBuilder *builder);

void attach_details_updated_callback(void (*func)(int, DisplayInfo));
