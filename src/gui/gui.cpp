#pragma once

#include "gui/canvas.hpp"
#include "gui/details.hpp"

#include "common/ipc.hpp"
#include "common/ipc_request.hpp"
#include "common/shapes.hpp"

#include "resources.c"

#include <gtk/gtk.h>
#include <signal.h>
#include <string>
#include <vector>

#define GRESOURCE_PREFIX "/com/heyzec/wayland-displays/"

using string = std::string;
template <class T> using vector = std::vector<T>;

// ============================================================
// Global state
// ============================================================

/* Attributes of displays, source of truth */
// vector<DisplayInfo> displays = vector<DisplayInfo>{DisplayInfo{}};
// int selected_display = 0;

// ============================================================
// Functions to pass data to and from the canvas
// ============================================================

vector<Box> create_boxes_from_displays(vector<DisplayInfo> displays) {
  auto boxes = vector<Box>();
  for (auto display : displays) {
    Box box = Box{};
    box.x = display.pos_x;
    box.y = display.pos_y;

    int width, height;
    // Resize accounting for whether transform rotates
    if (display.transform % 2 == 0) {
      width = display.size_x;
      height = display.size_y;
    } else {
      width = display.size_y;
      height = display.size_x;
    }
    // Resize accounting for DPI scale
    width /= display.scale;
    height /= display.scale;

    box.width = width;
    box.height = height;

    boxes.push_back(box);
  }
  return boxes;
}

void update_displays_from_boxes(vector<DisplayInfo> *displays, const vector<Box> boxes) {
  if (displays->size() != boxes.size()) {
    printf("Vector sizes do not match!\n");
    return;
  }

  // Note that only position attributes are updated back
  for (int i = 0; i < boxes.size(); i++) {
    Box box = boxes.at(i);
    DisplayInfo *display = &displays->at(i);
    display->pos_x = box.x;
    display->pos_y = box.y;
  }
}

void refresh_canvas() {
  // TODO: Improve code org: it is not clear that canvas_state and redraw_canvas is related
  vector<Box> boxes = create_boxes_from_displays(displays);
  refresh_canvas(boxes);
}

// ============================================================
// App layout
// ============================================================

GtkWidget *get_window() {
  // Create a GtkBuilder instance
  GtkBuilder *builder = gtk_builder_new();

  // Initialize the resource
  g_resources_register(resources_get_resource());

  // Load the UI description from the resource
  string resource_path = string(GRESOURCE_PREFIX) + "layout.ui";
  if (!gtk_builder_add_from_resource(builder, resource_path.c_str(), NULL)) {
    printf("Error loading resource: %s\n", resource_path.c_str());
    exit(1);
  }

  // Get the main window object
  GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));

  // Get and setup drawing canvas (don't draw boxes yet)
  GtkDrawingArea *drawing_area = GTK_DRAWING_AREA(gtk_builder_get_object(builder, "drawing_area"));
  setup_canvas(drawing_area, std::vector<Box>());

  setup_details(builder);

  // Add header bar
  GtkWidget *header_bar = GTK_WIDGET(gtk_builder_get_object(builder, "header"));
  gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);
  // Stop gtk_main when GUI closed
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  return window;
}

// ============================================================
// Entry Point
// ============================================================

void update_displays_from_server() {
  // Get displays state via IPC
  IpcGetRequest request = {};
  YAML::Node response = send_ipc_request(request);
  displays = response["STATE"]["HEADS"].as<vector<DisplayInfo>>();
}

void refresh_gui() {
  refresh_canvas();
  refresh_details();
}

/* On SIGUSR1, refresh GUI */
void usr1_signal_handler(int signal) {
  update_displays_from_server();
  refresh_gui();
}

void setup_gui() {
  signal(SIGUSR1, usr1_signal_handler);

  update_displays_from_server();

  gtk_init(NULL, NULL); // NULL, NULL instead of argc, argv

  // Setup contents in window
  GtkWidget *window = get_window();

  // Update content values in window
  refresh_gui();

  attach_canvas_updated_callback([](int selected_box, const vector<Box> boxes) {
    update_displays_from_boxes(&displays, boxes);
    selected_display = selected_box;
    refresh_details();
  });

  attach_details_updated_callback([](int new_selected_display, DisplayInfo display) {
    if (new_selected_display != -1) {
      selected_display = new_selected_display;
      return;
    }
    displays.at(selected_display) = display;
    refresh_gui();
  });

  gtk_widget_show_all(window); // Mark all widgets to be displayed
}

void run_gui() {
  setup_gui();
  gtk_main();
  // g_object_unref(builder);
}
