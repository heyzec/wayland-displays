#pragma once

#include "wlr-output.cpp"
#include "canvas.hpp"
#include "display.cpp"

#include <gtk/gtk.h>
#include <vector>

using namespace std;

/* Attributes of displays, source of truth */
vector<Display> displays = vector<Display>{Display{}};

CanvasState *canvas_state = new struct CanvasState;

GtkSpinButton *position_x_entry;
GtkSpinButton *position_y_entry;
GtkSpinButton *size_x_entry;
GtkSpinButton *size_y_entry;
GtkSpinButton *dpi_button;
GtkSpinButton *rate_button;
// GtkWidget *transform_button;

int selected_display = 0;

// ============================================================
// Functions to pass data to and from the canvas
// ============================================================

vector<Box> create_boxes_from_displays(vector<Display> displays) {
  auto boxes = vector<Box>();
  for (auto display : displays) {
    Box box = Box{};
    box.x = display.pos_x;
    box.y = display.pos_y;
    box.width = display.width;
    box.height = display.height;
    boxes.push_back(box);
  }
  return boxes;
}

void update_displays_from_boxes(vector<Display> *displays, const CanvasState canvas_state) {
  vector<Box> boxes = canvas_state.boxes;

  selected_display = canvas_state.selected_box;

  if (displays->size() != boxes.size()) {
    printf("Vector sizes do not match!\n");
    return;
  }

  for (int i = 0; i < boxes.size(); i++) {
    Box box = boxes.at(i);
    Display *display = &displays->at(i);
    display->pos_x = box.x;
    display->pos_y = box.y;
  }
}

void update_canvas() {
  canvas_state->boxes = create_boxes_from_displays(displays);
  redraw_canvas();
}

// ============================================================
// Set and get values (with callbacks attached to GUI elements)
// ============================================================

void update_selected_display() {
  if (selected_display == -1) {
    return;
  }
  gtk_spin_button_set_value(position_x_entry, displays.at(selected_display).pos_x);
  gtk_spin_button_set_value(position_y_entry, displays.at(selected_display).pos_y);
  gtk_spin_button_set_value(size_x_entry, displays.at(selected_display).width);
  gtk_spin_button_set_value(size_y_entry, displays.at(selected_display).height);
  // gtk_spin_button_set_value(dpi_button, displays.at(0).dpi);
}

void on_position_x_changed(GtkSpinButton *position_x_entry) {
  if (selected_display != -1) {
    displays.at(selected_display).pos_x = gtk_spin_button_get_value(position_x_entry);
    update_canvas();
  }
}
void on_position_y_changed(GtkSpinButton *position_y_entry) {
  if (selected_display != -1) {
    displays.at(selected_display).pos_y = gtk_spin_button_get_value(position_y_entry);
    update_canvas();
  }
}
void on_size_x_changed(GtkSpinButton *size_x_entry) {
  if (selected_display != -1) {
    displays.at(selected_display).width = gtk_spin_button_get_value(size_x_entry);
    update_canvas();
  }
}
void on_size_y_changed(GtkSpinButton *size_y_entry) {
  if (selected_display != -1) {
    displays.at(selected_display).height = gtk_spin_button_get_value(size_y_entry);
    update_canvas();
  }
}
void on_dpi_changed(GtkSpinButton *dpi_button) {
  // if (selected_display != -1) {
  //   displays.at(selected_display).pos_x  = gtk_spin_button_get_value(dpi_button);
  //   update_canvas();
  // }
}
void on_rate_changed(GtkSpinButton *rate_button) {
  // if (selected_display != -1) {
  //   displays.at(selected_display).pos_x = gtk_spin_button_get_value(rate_button);
  //   update_canvas();
  // }
}

// ============================================================
// App layout
// ============================================================

GtkWidget *get_top_pane() {
  GtkWidget *canvas = get_canvas(canvas_state);
  gtk_widget_set_size_request(canvas, 400, 400); // Set a size for visibility?
  return canvas;
}

GtkWidget *get_bottom_pane() {
  // Create a box to hold the image and description
  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

  // Create a grid
  GtkWidget *grid = gtk_grid_new();
  gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
  gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

  // Position X and Y
  GtkWidget *position_label = gtk_label_new("Position:");
  gtk_grid_attach(GTK_GRID(grid), position_label, 0, 0, 1, 1);
  position_x_entry = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 10000, 1));
  position_y_entry = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 10000, 1));
  gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(position_x_entry), 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(position_y_entry), 2, 0, 1, 1);
  g_signal_connect(G_OBJECT(position_x_entry), "value-changed", G_CALLBACK(on_position_x_changed), NULL);
  g_signal_connect(G_OBJECT(position_y_entry), "value-changed", G_CALLBACK(on_position_y_changed), NULL);

  // Size
  GtkWidget *size_label = gtk_label_new("Size:");
  gtk_grid_attach(GTK_GRID(grid), size_label, 0, 1, 1, 1);
  size_x_entry = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 10000, 1));
  size_y_entry = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 10000, 1));
  gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(size_x_entry), 1, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(size_y_entry), 2, 1, 1, 1);
  g_signal_connect(G_OBJECT(size_x_entry), "value-changed", G_CALLBACK(on_size_x_changed), NULL);
  g_signal_connect(G_OBJECT(size_y_entry), "value-changed", G_CALLBACK(on_size_y_changed), NULL);

  // DPI Scale
  GtkWidget *dpi_label = gtk_label_new("DPI Scale:");
  gtk_grid_attach(GTK_GRID(grid), dpi_label, 0, 2, 1, 1);
  dpi_button = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 100, 0.1));
  gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(dpi_button), 1, 2, 1, 1);
  g_signal_connect(G_OBJECT(dpi_button), "value-changed", G_CALLBACK(on_dpi_changed), NULL);

  // Refresh Rate
  GtkWidget *rate_label = gtk_label_new("Refresh rate:");
  gtk_grid_attach(GTK_GRID(grid), rate_label, 0, 3, 1, 1);
  rate_button = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 1000, 1));
  gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(rate_button), 1, 3, 1, 1);
  g_signal_connect(G_OBJECT(rate_button), "value-changed", G_CALLBACK(on_rate_changed), NULL);

  // Transformations
  GtkWidget *transform_label = gtk_label_new("Transform:");
  gtk_grid_attach(GTK_GRID(grid), transform_label, 0, 4, 1, 1);
  GtkWidget *transform_button = gtk_menu_button_new();
  gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(transform_button), 1, 4, 1, 1);
  // These APIs are deprecated!
  GMenu *menu = g_menu_new();
  g_menu_append(menu, "Normal", "app.new");
  g_menu_append(menu, "Rotate", "win.about");
  gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(transform_button), G_MENU_MODEL(menu));

  // Attach the grid to the box
  gtk_box_pack_start(GTK_BOX(vbox), grid, FALSE, FALSE, 0);

  return vbox;
}

GtkWidget *get_header_bar() {
  GtkWidget *header_bar = gtk_header_bar_new();
  gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
  gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "wayland-displays");
  gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header_bar), "wayland-displays");

  GtkWidget *menu = gtk_button_new_from_icon_name("open-menu-symbolic", GTK_ICON_SIZE_BUTTON);
  gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar), menu);

  return header_bar;
}

GtkWidget *get_window() {
  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "wayland-displays");
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  gtk_widget_set_size_request(window, 300, 200);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);

  // Create top pane
  GtkWidget *box1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  GtkWidget *pane1_widget = get_top_pane();
  gtk_container_add(GTK_CONTAINER(box1), pane1_widget);
  gtk_paned_add1(GTK_PANED(paned), box1);

  // Create bottom pane
  GtkWidget *box2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  GtkWidget *pane2_widget = get_bottom_pane();
  gtk_container_add(GTK_CONTAINER(box2), pane2_widget);
  gtk_paned_add2(GTK_PANED(paned), box2);

  // Attach panes
  gtk_container_add(GTK_CONTAINER(window), paned);

  // Add header bar
  GtkWidget *header_bar = get_header_bar();
  gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);

  // Set initial size for the first pane
  gtk_paned_set_position(GTK_PANED(paned), 500);
  return window;
}

// ============================================================
// Entry Point
// ============================================================

void run_gui() {
  vector<Display> displays_ = get_displays();
  displays = displays_; // Create a copy

  auto on_canvas_updated = [](const CanvasState canvas_state) {
    update_displays_from_boxes(&displays, canvas_state);
    update_selected_display();
  };

  canvas_state->boxes = create_boxes_from_displays(displays);
  attach_canvas_updated_callback(on_canvas_updated);
  gtk_init(NULL, NULL); // NULL, NULL instead of argc, argv
  GtkWidget *window = get_window();

  update_selected_display();

  gtk_widget_show_all(window);
  gtk_main();
}
