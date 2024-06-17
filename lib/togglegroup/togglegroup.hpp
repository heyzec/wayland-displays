/* togglegroup.hpp
 * A ToggleGroup is a custom GTK widget containing an sequence of GtkToggleButton where only one can
 * be active at a time.
 */
// Creating a custom GTK3 widget is very painful because of the lack of documentation
//
// First, refer to this link on section - 20.3 Creating a Composite widget:
// https://sites.cc.gatech.edu/data_files/public/doc/gtk/tutorial/gtk_tut-20.html
// While outdated and only applicable for GTK3, it gives a good idea how custom widget work.
//
// Next refer to the official GTK2 to GTK3 migration guide:
// https://docs.gtk.org/gtk3/migrating-2to3.html
// Note that this doc does not mention that all gtk_signal_XXX functions have been deprecated
#ifndef __TOGGLEGROUP_H__
#define __TOGGLEGROUP_H__

#include <gtk/gtk.h>
#include <vector>

// Uncomment this if we rename this file to a .h
// #ifdef __cplusplus
// extern "C" {
// #endif /* __cplusplus */

#define TYPE_TOGGLE_GROUP (toggle_group_get_type())
#define TOGGLE_GROUP(object) G_TYPE_CHECK_INSTANCE_CAST((object), TYPE_TOGGLE_GROUP, ToggleGroup)
#define TOGGLE_GROUP_CLASS(klass)                                                                  \
  (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_TOGGLE_GROUP, ToggleGroupClass))
#define IS_TOGGLE_GROUP(object) G_TYPE_CHECK_INSTANCE_TYPE((object), TYPE_TOGGLE_GROUP))
#define IS_TOGGLE_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_TOGGLE_GROUP))
#define TOGGLE_GROUP_GET_CLASS(object)                                                             \
  (G_TYPE_INSTANCE_GET_CLASS((object), TYPE_TOGGLE_GROUP, ToggleGroupClass))

typedef struct _ToggleGroup ToggleGroup;
typedef struct _ToggleGroupClass ToggleGroupClass;

struct _ToggleGroup {
  GtkBox parent;

  int selected = 0;
  std::vector<GtkToggleButton *> btns;
};

struct _ToggleGroupClass {
  GtkBoxClass parent_class;
};

/* Tells GTK about the widget class, and gets an ID that uniquely identifies the widget class.
 * Upon subsequent calls, it just returns the ID
 */
static GType toggle_group_get_type(void) G_GNUC_CONST;

/* Create a new toggle group widget */
ToggleGroup *toggle_group_new(int n, int selected, const char *labels[]);

/* Our custom method to get index of the currently active toggle button */
int toggle_group_get_active(ToggleGroup *toggle_group);

/* Our custom method to set the active toggle button by index */
void toggle_group_set_active(ToggleGroup *self, int index);

struct _Data {
  int index;
  ToggleGroup *self;
};

// #ifdef __cplusplus
// }
// #endif /* __cplusplus */

#endif /* __TOGGLEGROUP_H__ */
