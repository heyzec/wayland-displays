#include "gui/details.hpp"
#include "gui/gui.hpp"

#include "common/ipc.hpp"
#include "common/ipc_request.hpp"
#include "common/shapes.hpp"
#include "togglegroup/togglegroup.hpp"

#include <gtk/gtk.h>
#include <string>
#include <vector>

using string = std::string;
template <class T> using vector = std::vector<T>;

// ============================================================
// Internal variables
// ============================================================

// GTK widgets that we need to set and get values from
static GtkWidget *button_box;
static ToggleGroup *toggle_group;
static GtkSwitch *enabled_switch;
static GtkLabel *description_label;
static GtkSpinButton *pos_x_spin;
static GtkSpinButton *pos_y_spin;
static GtkSpinButton *size_x_spin;
static GtkSpinButton *size_y_spin;
static GtkSpinButton *dpi_spin;
static GtkSpinButton *rate_spin;
static GtkComboBoxText *modes_combobox;
static GtkLabel *transform_button_label;
static GtkPopover *transform_menu;

// ============================================================
// External variables
// ============================================================

// Only allow these globals to be modified from gui.cpp!
// TODO: Move these back to gui.cpp
vector<DisplayInfo> displays;
int selected_display = 0;

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

void setup_details(GtkBuilder *builder) {
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
}
