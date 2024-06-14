#pragma once

#include "canvas.hpp"
#include "display.hpp"

// TODO: Do not rely on these files directly
#include "outputs/outputs.hpp"
#include "outputs/shapes.hpp"

#include "resources.c"

#include <gtk/gtk.h>
#include <vector>

using namespace std;

#define GRESOURCE_PREFIX "/com/heyzec/wayland-displays/"

// ============================================================
// Global state
// ============================================================

/* Attributes of displays, source of truth */
vector<DisplayInfo> displays = vector<DisplayInfo>{DisplayInfo{}};

int selected_display = 0;
CanvasState *canvas_state = new struct CanvasState;

// GTK widgets that we need to set and get values from
vector<GtkToggleButton *> toggle_buttons = vector<GtkToggleButton *>{};
GtkSwitch *enabled_switch;
GtkLabel *name_label;
GtkLabel *description_label;
GtkSpinButton *pos_x_spin;
GtkSpinButton *pos_y_spin;
GtkSpinButton *size_x_spin;
GtkSpinButton *size_y_spin;
GtkSpinButton *dpi_spin;
GtkSpinButton *rate_spin;
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

  gtk_label_set_text(name_label, display.name);
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

  // Update transform button label
  update_transform_label(display.transform);
}

void on_active_toggled(GtkToggleButton *toggle_button, void *data) {
  // TODO: Functionality of this button group is incomplete!
  // Untoggle the current button
  GtkToggleButton *active_button = toggle_buttons.at(selected_display);
  gtk_toggle_button_set_active(active_button, false);
  g_signal_connect(active_button, "toggled", G_CALLBACK(on_active_toggled), NULL);

  // Update the global state
  for (int i = 0; i < displays.size(); i++) {
    GtkToggleButton *button = toggle_buttons.at(i);
    if (button == toggle_button) {
      // auto *display = &displays.at(i);
      selected_display = i;
      break;
    }
  }
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

void on_transform_menu_clicked(GtkWidget *transform_button) {
  if (selected_display == -1) {
    return;
  }

  std::string name = gtk_widget_get_name(transform_button);
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
  std::vector<DisplayConfig> temp;
  temp.reserve(displays.size());
  for (const auto &b : displays) {
    temp.emplace_back(b);
  }
  apply_configurations(temp);
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
  std::string resource_path = std::string(GRESOURCE_PREFIX) + "layout.ui";
  if (!gtk_builder_add_from_resource(builder, resource_path.c_str(), NULL)) {
    printf("Error loading resource: %s\n", resource_path.c_str());
    exit(1);
  }

  // Get the main window object
  GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));

  // Attach drawing canvas to empty box
  GtkWidget *top_box = GTK_WIDGET(gtk_builder_get_object(builder, "top_box"));
  GtkWidget *canvas = get_canvas(canvas_state);
  gtk_widget_set_size_request(canvas, 400, 400); // Set a size for visibility?
  gtk_container_add(GTK_CONTAINER(top_box), canvas);

  // Get name and description labels
  name_label = GTK_LABEL(gtk_builder_get_object(builder, "name_label"));
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

  // Get apply button and attach event handler
  GtkButton *apply_button = GTK_BUTTON(gtk_builder_get_object(builder, "apply_button"));
  g_signal_connect(apply_button, "clicked", G_CALLBACK(on_apply_clicked), NULL);

  // Get the 8 transform buttons within popover (one for each possible transform) and attach to
  // common event handler
  transform_menu = GTK_POPOVER(gtk_builder_get_object(builder, "transform_popover"));
  for (int i = 0; i < 8; i++) {
    std::string button_id = "transform_" + to_string(i);
    GtkButton *button = GTK_BUTTON(gtk_builder_get_object(builder, button_id.c_str()));
    g_signal_connect(button, "clicked", G_CALLBACK(on_transform_menu_clicked), NULL);
  }
  transform_button_label = GTK_LABEL(gtk_builder_get_object(builder, "transform_button_label"));

  // Attach drawing canvas to empty box
  GtkBox *button_box = GTK_BOX(gtk_builder_get_object(builder, "button_box"));
  for (int i = 0; i < displays.size(); i++) {
    DisplayInfo *display = &displays.at(i);
    GtkToggleButton *toggle_button =
        GTK_TOGGLE_BUTTON(gtk_toggle_button_new_with_label(display->name));
    gtk_widget_set_name(GTK_WIDGET(toggle_button), display->name);
    gtk_container_add(GTK_CONTAINER(button_box), GTK_WIDGET(toggle_button));
    if (i != selected_display) {
      g_signal_connect(toggle_button, "toggled", G_CALLBACK(on_active_toggled), display);
    }
    toggle_buttons.push_back(toggle_button);
  }

  // Add header bar
  GtkWidget *header_bar = GTK_WIDGET(gtk_builder_get_object(builder, "header"));
  gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);

  return window;
}

// ============================================================
// Entry Point
// ============================================================

void run_gui() {
  vector<DisplayInfo> displays_ = get_displays();
  displays = displays_; // Create a copy

  auto on_canvas_updated = [](const CanvasState canvas_state) {
    update_displays_from_boxes(&displays, canvas_state);
    update_gui_elements();
  };

  canvas_state->boxes = create_boxes_from_displays(displays);
  attach_canvas_updated_callback(on_canvas_updated);
  gtk_init(NULL, NULL); // NULL, NULL instead of argc, argv
  GtkWidget *window = get_window();

  update_gui_elements();

  gtk_widget_show_all(window);
  gtk_main();
  // g_object_unref(builder);
}
