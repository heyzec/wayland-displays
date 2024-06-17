#include "outputs/outputs.hpp"
#include "togglegroup/togglegroup.hpp"

#include <gtk/gtk.h>
#include <nlohmann/json.hpp>

// TODO: Turn gui.cpp into a header file, this variable is conflicting
ToggleGroup *toggle_group_;

void callback(ToggleGroup *toggle_group) {
  printf("Index %d\n", toggle_group->selected);
}

void rotate(ToggleGroup *_) {
  int index = toggle_group_get_active(toggle_group_);
  toggle_group_set_active(toggle_group_, (index + 1) % 3);
}

int sandbox() {
  gtk_init(NULL, NULL);
  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  GtkWidget *wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

  GtkWidget *btn = gtk_button_new_with_label("Rotate selected");
  gtk_container_add(GTK_CONTAINER(wrapper), GTK_WIDGET(btn));
  g_signal_connect(btn, "clicked", G_CALLBACK(rotate), NULL);

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

  const char *labels[] = {"hi", "bye", "ha"};
  toggle_group_ = toggle_group_new(3, 0, labels);
  gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(toggle_group_));
  g_signal_connect(toggle_group_, "changed", G_CALLBACK(callback), NULL);

  gtk_container_add(GTK_CONTAINER(wrapper), box);
  gtk_container_add(GTK_CONTAINER(window), wrapper);

  gtk_widget_show_all(window);
  gtk_main();

  return 0;
}
