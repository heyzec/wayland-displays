#include "common/shapes.hpp"
#include <gtk/gtk.h>

// Only allow these globals to be modified from gui.cpp!
extern std::vector<DisplayInfo> displays;
extern int selected_display;

void update_gui_elements();

void setup_details(GtkBuilder *builder);

void replace_toggle_group(std::vector<DisplayInfo> displays);
