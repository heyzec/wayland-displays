#pragma once

#include "common/ipc/get.hpp"
#include "gui/canvas.hpp"
#include "gui/details.hpp"
#include "gui/glarea.hpp"
#include "gui/screencopy.hpp"

#include "common/ipc.hpp"
#include "common/logger.hpp"
#include "common/shapes.hpp"

#include "resources.c"

#include <gtk/gtk.h>
#include <signal.h>
#include <string>
#include <vector>

#include <epoxy/gl.h>
#include <gtk/gtk.h>

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
    if (!display.enabled) {
      continue;
    }

    Box box = Box{};
    box.name = display.name;

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

    box.transform = display.transform;

    boxes.push_back(box);
  }
  return boxes;
}

void update_displays_from_boxes(vector<DisplayInfo> *displays, const vector<Box> boxes) {
  // Account for when size of boxes less than displays (because some displays are disabled)
  auto it_display = displays->begin();
  auto it_box = boxes.begin();

  while (it_box != boxes.end()) {
    if (it_display == displays->end()) {
      log_warn("Assertion failed: Trying to update display {} but not found!",
               it_box->name.c_str());
      return;
    }
    DisplayInfo *display = &(*it_display);
    Box box = *it_box;
    if (display->name != box.name) {
      ++it_display;
      continue;
    }

    // Note that only position attributes are updated back
    display->pos_x = box.x;
    display->pos_y = box.y;

    // Move to next elements
    ++it_display;
    ++it_box;
  }
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

  // Get and setup OpenGL area
  GtkWidget *gl_area = GTK_WIDGET(gtk_builder_get_object(builder, "gl_area"));
  glarea_setup(gl_area);

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
  IpcGetResponse response = std::get<IpcGetResponse>(send_ipc_request(request));
  displays = response.heads;
}

void refresh_gui() {
  vector<Box> boxes = create_boxes_from_displays(displays);
  refresh_canvas(boxes);
  glarea_update(boxes);
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

  attach_canvas_updated_callback([](string selected_box, const vector<Box> boxes) {
    update_displays_from_boxes(&displays, boxes);
    for (int i = 0; i < displays.size(); i++) {
      if (displays.at(i).name == selected_box) {
        selected_display = i;
        break;
      }
    }
    refresh_gui();
  });

  attach_details_updated_callback([](int new_selected_display, DisplayInfo display) {
    displays.at(selected_display) = display;
    refresh_gui();
  });

  gtk_widget_show_all(window); // Mark all widgets to be displayed
}

void run_gui() {
  screencopy_init();
  setup_gui();
  gtk_main();
  // g_object_unref(builder);
}
