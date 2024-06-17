#include "togglegroup.hpp"

// Magic macro that defines our custom GTK widget class and template methods
G_DEFINE_TYPE(ToggleGroup, toggle_group, GTK_TYPE_BOX)

// Create an array of custom signals
enum { CHANGED_SIGNAL, LAST_SIGNAL };
static gint signals[LAST_SIGNAL] = {0};

/* Initializes fields of the widget's class structure, and sets up any signals for the class */
static void toggle_group_class_init(ToggleGroupClass *klass) {
  signals[CHANGED_SIGNAL] = g_signal_new("changed",          // signal_name
                                         TYPE_TOGGLE_GROUP,  // itype
                                         G_SIGNAL_RUN_FIRST, // signal_flags
                                         0,                  // class_offset
                                         NULL,               // accumulator
                                         NULL,               // accu_data
                                         NULL,               // c_marshaller
                                         G_TYPE_NONE,        // return_type
                                         0                   // n_params
  );
}

/* Initialize the object structure */
static void toggle_group_init(ToggleGroup *self) {}

static void toggle_button_callback(GtkToggleButton *pressed_btn, _Data *data) {
  ToggleGroup *self = data->self;
  bool activated = gtk_toggle_button_get_active(pressed_btn);

  if (!activated && data->index == self->selected) {
    // Undo the effect of user unpressing a pressed button directly
    gtk_toggle_button_set_active(pressed_btn, true);
    return;
  }

  if (activated && data->index != self->selected) {
    int original = self->selected;
    self->selected = data->index;
    GtkToggleButton *btn = self->btns.at(original);
    gtk_toggle_button_set_active(self->btns.at(original), false);
    g_signal_emit(self, signals[CHANGED_SIGNAL], 0);
  }
}

ToggleGroup *toggle_group_new(int n, int selected, const char **labels) {
  ToggleGroup *self = TOGGLE_GROUP(g_object_new(TYPE_TOGGLE_GROUP, NULL));

  // Add this magic CSS class "linked" that visually joins the buttons
  GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(self));
  gtk_style_context_add_class(context, "linked");

  self->selected = selected;

  for (int i = 0; i < n; i++) {
    GtkToggleButton *btn = GTK_TOGGLE_BUTTON(gtk_toggle_button_new_with_label(labels[i]));
    self->btns.push_back(btn);
    gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(btn));
    if (i == self->selected) {
      gtk_toggle_button_set_active(btn, true);
    }

    _Data *data = new _Data{.index = i, .self = self};
    g_signal_connect(btn, "toggled", G_CALLBACK(toggle_button_callback), data);
  }

  return self;
}

int toggle_group_get_active(ToggleGroup *self) {
  return self->selected;
}

void toggle_group_set_active(ToggleGroup *self, int index) {
  if (index == self->selected) {
    return;
  }
  // Activate the index-th button, the handlers will deactivate the currently
  // selected button and set the internal state of self->selected too
  gtk_toggle_button_set_active(self->btns.at(index), true);
}
