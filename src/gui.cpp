#include "canvas.cpp"
#include <gtk/gtk.h>
#include <vector>

using namespace std;

GtkWidget *get_top_pane() {
  // Mock data
  vector<Box> *boxes = new vector<Box>{{100, 100, 100, 100}, {200, 200, 100, 100}};

  GtkWidget *canvas = get_canvas(boxes);
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
  GtkSpinButton *position_x_entry = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 100, 1));
  GtkSpinButton *position_y_entry = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 100, 1));
  gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(position_x_entry), 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(position_y_entry), 2, 0, 1, 1);

  // Size
  GtkWidget *size_label = gtk_label_new("Size:");
  gtk_grid_attach(GTK_GRID(grid), size_label, 0, 1, 1, 1);
  GtkSpinButton *size_x_entry = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 100, 1));
  GtkSpinButton *size_y_entry = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 100, 1));
  gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(size_x_entry), 1, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(size_y_entry), 2, 1, 1, 1);

  // DPI Scale
  GtkWidget *dpi_label = gtk_label_new("DPI Scale:");
  gtk_grid_attach(GTK_GRID(grid), dpi_label, 0, 2, 1, 1);
  GtkSpinButton *dpi_button = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 100, 0.1));
  gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(dpi_button), 1, 2, 1, 1);

  // Refresh Rate
  GtkWidget *rate_label = gtk_label_new("Refresh rate:");
  gtk_grid_attach(GTK_GRID(grid), rate_label, 0, 3, 1, 1);
  GtkSpinButton *rate_button = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 1000, 1));
  gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(rate_button), 1, 3, 1, 1);

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

void start_gui() {
  gtk_init(NULL, NULL); // NULL, NULL instead of argc, argv
  GtkWidget *window = get_window();
  gtk_widget_show_all(window);
  gtk_main();
}
