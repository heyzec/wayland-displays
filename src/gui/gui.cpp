#pragma once

#include "wlr_output.cpp"
#include "canvas.hpp"
#include "display.hpp"
#include "wlr_output/shapes.hpp"

#include "resources.c"

#include <gtk/gtk.h>
#include <vector>

using namespace std;

#define GRESOURCE_PREFIX "/com/heyzec/wayland-displays/"

typedef HeadDyanamicInfo Display;

/* Attributes of displays, source of truth */
vector<Display> displays = vector<Display>{Display{}};

CanvasState *canvas_state = new struct CanvasState;

GtkSpinButton *pos_x_spin;
GtkSpinButton *pos_y_spin;
GtkSpinButton *size_x_spin;
GtkSpinButton *size_y_spin;
GtkSpinButton *dpi_spin;
GtkSpinButton *rate_spin;
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
    box.width = display.size_x;
    box.height = display.size_y;
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
  gtk_spin_button_set_value(pos_x_spin, displays.at(selected_display).pos_x);
  gtk_spin_button_set_value(pos_y_spin, displays.at(selected_display).pos_y);
  gtk_spin_button_set_value(size_x_spin, displays.at(selected_display).size_x);
  gtk_spin_button_set_value(size_y_spin, displays.at(selected_display).size_y);
  gtk_spin_button_set_value(dpi_spin, displays.at(selected_display).rate);
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
    displays.at(selected_display).size_x = gtk_spin_button_get_value(size_x_entry);
    update_canvas();
  }
}
void on_size_y_changed(GtkSpinButton *size_y_entry) {
  if (selected_display != -1) {
    displays.at(selected_display).size_y = gtk_spin_button_get_value(size_y_entry);
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

void on_apply_clicked(GtkButton *apply_button) {
  apply_configurations(displays);
}

// ============================================================
// App layout
// ============================================================

GtkWidget *get_top_pane() {
  GtkWidget *canvas = get_canvas(canvas_state);
  gtk_widget_set_size_request(canvas, 400, 400); // Set a size for visibility?
  return canvas;
}


GtkWidget *get_window() {
  // Create a GtkBuilder instance
  GtkBuilder *builder = gtk_builder_new();

  // Initialize the resource
  g_resources_register(resources_get_resource());

  // Load the UI description from the resource
  std::string resource_path = std::string(GRESOURCE_PREFIX) + "layout.ui";
  if (!gtk_builder_add_from_resource(builder, resource_path.c_str(), NULL)) {
    printf("Error loading resource: %s\n", resource_path.c_str());
    exit(1);
  }

  // Get the main window object
  GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));

  // Attach drawing canvas
  GtkWidget *top_box = GTK_WIDGET(gtk_builder_get_object(builder, "top_box"));
  GtkWidget *pane1_widget = get_top_pane();
  gtk_container_add(GTK_CONTAINER(top_box), pane1_widget);

  // Get buttons
  pos_x_spin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "pos_x_spin"));
  pos_y_spin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "pos_y_spin"));
  size_x_spin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "size_x_spin"));
  size_y_spin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "size_y_spin"));
  rate_spin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "rate_spin"));
  dpi_spin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "dpi_spin"));
  GtkButton *apply_button = GTK_BUTTON(gtk_builder_get_object(builder, "apply_button"));

  // Attach event handlers to buttons
  g_signal_connect(G_OBJECT(pos_x_spin), "value-changed", G_CALLBACK(on_position_x_changed), NULL);
  g_signal_connect(G_OBJECT(pos_y_spin), "value-changed", G_CALLBACK(on_position_y_changed), NULL);
  g_signal_connect(G_OBJECT(size_x_spin), "value-changed", G_CALLBACK(on_size_x_changed), NULL);
  g_signal_connect(G_OBJECT(size_y_spin), "value-changed", G_CALLBACK(on_size_y_changed), NULL);
  g_signal_connect(G_OBJECT(dpi_spin), "value-changed", G_CALLBACK(on_dpi_changed), NULL);
  g_signal_connect(G_OBJECT(rate_spin), "value-changed", G_CALLBACK(on_rate_changed), NULL);
  g_signal_connect(G_OBJECT(apply_button), "clicked", G_CALLBACK(on_apply_clicked), NULL);

  // Add header bar
  GtkWidget *header_bar = GTK_WIDGET(gtk_builder_get_object(builder, "header"));
  gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);


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
  // g_object_unref(builder);
}
