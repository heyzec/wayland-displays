#pragma once

#include "gui/canvas.hpp"

#include "common/ipc.cpp"
#include "common/ipc_request.hpp"
#include "common/shapes.hpp"
#include "togglegroup/togglegroup.hpp"

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
vector<DisplayInfo> displays = vector<DisplayInfo>{DisplayInfo{}};

int selected_display = 0;
CanvasState *canvas_state = new struct CanvasState;

// GTK widgets that we need to set and get values from
GtkWidget *button_box;
ToggleGroup *toggle_group;
GtkSwitch *enabled_switch;
GtkLabel *description_label;
GtkSpinButton *pos_x_spin;
GtkSpinButton *pos_y_spin;
GtkSpinButton *size_x_spin;
GtkSpinButton *size_y_spin;
GtkSpinButton *dpi_spin;
GtkSpinButton *rate_spin;
GtkComboBoxText *modes_combobox;
GtkLabel *transform_button_label;
GtkPopover *transform_menu;

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

void update_displays_from_boxes(vector<DisplayInfo> *displays, const CanvasState canvas_state) {
  vector<Box> boxes = canvas_state.boxes;
  if (displays->size() != boxes.size()) {
    printf("Vector sizes do not match!\n");
    return;
  }

  selected_display = canvas_state.selected_box;

  // Note that only position attributes are updated back
  for (int i = 0; i < boxes.size(); i++) {
    Box box = boxes.at(i);
    DisplayInfo *display = &displays->at(i);
    display->pos_x = box.x;
    display->pos_y = box.y;
  }
}

void update_canvas() {
  // TODO: Improve code org: it is not clear that canvas_state and redraw_canvas is related
  canvas_state->boxes = create_boxes_from_displays(displays);
  redraw_canvas();
}

// ============================================================
// Set and get values (with callbacks attached to GUI elements)
// ============================================================

void update_available_modes(DisplayInfo display) {
  // TODO: Do not call this function on drag! The app is lagging
  gtk_combo_box_text_remove_all(modes_combobox);
  for (int i = 0; i < display.modes.size(); i++) {
    ModeInfo mode = display.modes.at(i);
    string id = std::to_string(i);

    std::stringstream buf;
    buf << mode.size_x << "x" << mode.size_y << "@";
    buf << (float)mode.rate / 1000 << "Hz";
    string text = buf.str();

    gtk_combo_box_text_append(modes_combobox, id.c_str(), text.c_str());
  }

  // Cap max height to 10
  int n_columns = display.modes.size() / 10 + 1;
  gtk_combo_box_set_wrap_width(GTK_COMBO_BOX(modes_combobox), n_columns);
}

void update_transform_label(int enum_value) {
  const char transform_readable[][20] = {
      "Normal",  "Rotate 90",          "Rotate 180",          "Rotate 270",
      "Flipped", "Rotate 90, Flipped", "Rotate 180, Flipped", "Rotate 270, Flipped",
  };
  gtk_label_set_text(transform_button_label, transform_readable[enum_value]);
};

void update_gui_elements() {
  if (selected_display == -1) {
    return;
  }
  auto display = displays.at(selected_display);

  // Update toggle group
  toggle_group_set_active(toggle_group, selected_display);

  // Update label
  gtk_label_set_text(description_label, display.description);

  // Update enabled switch
  gtk_switch_set_active(enabled_switch, display.enabled);

  // Update spin buttons
  gtk_spin_button_set_value(pos_x_spin, display.pos_x);
  gtk_spin_button_set_value(pos_y_spin, display.pos_y);
  gtk_spin_button_set_value(size_x_spin, display.size_x);
  gtk_spin_button_set_value(size_y_spin, display.size_y);
  gtk_spin_button_set_value(dpi_spin, display.scale);
  gtk_spin_button_set_value(rate_spin, (float)display.rate / 1000);

  update_available_modes(display);

  // Update transform button label
  update_transform_label(display.transform);
}

void on_active_toggled(ToggleGroup *toggle_group, void *data) {
  selected_display = toggle_group_get_active(toggle_group);
  update_gui_elements();
}
gboolean on_enabled_changed(GtkSwitch *enabled_switch) {
  // Return true to prevent the default handler from running
  if (selected_display != -1) {
    displays.at(selected_display).enabled = gtk_switch_get_active(enabled_switch);
  }
  return false;
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
  if (selected_display != -1) {
    displays.at(selected_display).scale = gtk_spin_button_get_value(dpi_button);
    update_canvas();
  }
}
void on_rate_changed(GtkSpinButton *rate_button) {
  if (selected_display != -1) {
    displays.at(selected_display).rate = gtk_spin_button_get_value(rate_button) * 1000;
    update_canvas();
  }
}

void on_mode_changed(GtkComboBoxText *combo_box, void *data) {
  if (selected_display == -1) {
    return;
  }
  const gchar *id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo_box));
  if (id == nullptr) {
    return;
  }

  int index = std::stoi(id);
  DisplayInfo *display = &displays.at(selected_display);

  display->size_x = display->modes.at(index).size_x;
  display->size_y = display->modes.at(index).size_y;
  display->rate = display->modes.at(index).rate;
  gtk_spin_button_set_value(size_x_spin, display->size_x);
  gtk_spin_button_set_value(size_y_spin, display->size_y);
  gtk_spin_button_set_value(dpi_spin, display->scale);
  gtk_spin_button_set_value(rate_spin, (float)display->rate / 1000);

  update_canvas();
}

void on_transform_menu_clicked(GtkWidget *transform_button) {
  if (selected_display == -1) {
    return;
  }

  string name = gtk_widget_get_name(transform_button);
  int enum_value = name[name.length() - 1] - '0';
  if (!(0 <= enum_value && enum_value < 8)) {
    printf("transform enum_value is unexpected\n");
    return;
  }

  displays.at(selected_display).transform = enum_value;
  gtk_popover_popdown(transform_menu);
  update_transform_label(enum_value);
  update_canvas();
}

void on_apply_clicked(GtkButton *apply_button) {
  // Need to slice subclass to parent class
  vector<DisplayConfig> temp;
  temp.reserve(displays.size());
  for (const auto &b : displays) {
    temp.emplace_back(b);
  }

  IpcSetRequest request = IpcSetRequest(temp);

  // TODO: This will actually block the GTK loop
  send_ipc_request(request);

  // TODO: Also get result back
}

// ============================================================
// App layout
// ============================================================

/* Create or replace toggle group */
void replace_toggle_group(vector<DisplayInfo> displays) {
  const char *labels[displays.size()];
  for (int i = 0; i < displays.size(); i++) {
    DisplayInfo *display = &displays.at(i);
    labels[i] = display->name;
  }
  ToggleGroup *new_toggle_group = toggle_group_new(displays.size(), selected_display, labels);
  g_signal_connect(new_toggle_group, "changed", G_CALLBACK(on_active_toggled), NULL);

  if (toggle_group != nullptr) {
    gtk_container_remove(GTK_CONTAINER(button_box), GTK_WIDGET(toggle_group));
    gtk_container_add(GTK_CONTAINER(button_box), GTK_WIDGET(new_toggle_group));
    gtk_widget_show_all(GTK_WIDGET(button_box));
  } else {
    gtk_container_add(GTK_CONTAINER(button_box), GTK_WIDGET(new_toggle_group));
  }
  toggle_group = new_toggle_group;
}

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
  setup_canvas(drawing_area, canvas_state);

  // Get description labels
  description_label = GTK_LABEL(gtk_builder_get_object(builder, "description_label"));

  // Get enabled switch
  enabled_switch = GTK_SWITCH(gtk_builder_get_object(builder, "enabled_switch"));
  g_signal_connect(enabled_switch, "state-set", G_CALLBACK(on_enabled_changed), NULL);

  // Get spin buttons and attach event handlers
  pos_x_spin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "pos_x_spin"));
  g_signal_connect(pos_x_spin, "value-changed", G_CALLBACK(on_position_x_changed), NULL);
  pos_y_spin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "pos_y_spin"));
  g_signal_connect(pos_y_spin, "value-changed", G_CALLBACK(on_position_y_changed), NULL);
  size_x_spin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "size_x_spin"));
  g_signal_connect(size_x_spin, "value-changed", G_CALLBACK(on_size_x_changed), NULL);
  size_y_spin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "size_y_spin"));
  g_signal_connect(size_y_spin, "value-changed", G_CALLBACK(on_size_y_changed), NULL);
  dpi_spin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "dpi_spin"));
  g_signal_connect(dpi_spin, "value-changed", G_CALLBACK(on_dpi_changed), NULL);
  rate_spin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "rate_spin"));
  g_signal_connect(rate_spin, "value-changed", G_CALLBACK(on_rate_changed), NULL);

  modes_combobox = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "modes_combobox"));
  g_signal_connect(modes_combobox, "changed", G_CALLBACK(on_mode_changed), NULL);

  // Get apply button and attach event handler
  GtkButton *apply_button = GTK_BUTTON(gtk_builder_get_object(builder, "apply_button"));
  g_signal_connect(apply_button, "clicked", G_CALLBACK(on_apply_clicked), NULL);

  // Get the 8 transform buttons within popover (one for each possible transform) and attach to
  // common event handler
  transform_menu = GTK_POPOVER(gtk_builder_get_object(builder, "transform_popover"));
  for (int i = 0; i < 8; i++) {
    string button_id = "transform_" + std::to_string(i);
    GtkButton *button = GTK_BUTTON(gtk_builder_get_object(builder, button_id.c_str()));
    g_signal_connect(button, "clicked", G_CALLBACK(on_transform_menu_clicked), NULL);
  }
  transform_button_label = GTK_LABEL(gtk_builder_get_object(builder, "transform_button_label"));

  // Get container for toggle group (don't create it yet)
  button_box = GTK_WIDGET(gtk_builder_get_object(builder, "button_box"));

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
//

void update_displays_from_server() {
  // Get displays state via IPC
  IpcGetRequest request = {};
  YAML::Node response = send_ipc_request(request);
  displays = response["STATE"]["HEADS"].as<vector<DisplayInfo>>();
}

/* On SIGUSR1, refresh GUI */
void usr1_signal_handler(int signal) {
  update_displays_from_server();
  replace_toggle_group(displays);

  update_canvas();
  update_gui_elements();
}

void run_gui() {
  signal(SIGUSR1, usr1_signal_handler);

  update_displays_from_server();

  gtk_init(NULL, NULL); // NULL, NULL instead of argc, argv

  // Setup contents in window
  GtkWidget *window = get_window();
  replace_toggle_group(displays);

  // Update content values in window
  update_canvas();
  update_gui_elements();

  attach_canvas_updated_callback([](const CanvasState canvas_state) {
    update_displays_from_boxes(&displays, canvas_state);
    update_gui_elements();
  });

  gtk_widget_show_all(window); // Mark all widgets to be displayed
  gtk_main();

  // g_object_unref(builder);
}
