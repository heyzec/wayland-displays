#include "gui/box.hpp"

#include <gtk/gtk.h>
#include <vector>

/* Setup glarea module */
void glarea_setup(GtkWidget *gl_area);

/* Inform glarea module that boxes have changed  */
void glarea_update(std::vector<Box> new_boxes);
