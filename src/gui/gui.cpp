#pragma once

#include "common/ipc/get.hpp"
#include "gui/canvas.hpp"
#include "gui/copy.hpp"
#include "gui/details.hpp"

#include "common/ipc.hpp"
#include "common/shapes.hpp"

#include "resources.c"

#include <gtk/gtk.h>
#include <signal.h>
#include <string>
#include <vector>

#include <epoxy/gl.h>
#include <gtk/gtk.h>

unsigned int WIDTH = 800;
unsigned int HEIGHT = 600;

const GLchar *VERTEX_SOURCE = "#version 300 es\n"
                              "layout (location = 0) in vec3 aPosition;\n"
                              "void main()\n"
                              "{\n"
                              "   gl_Position = vec4(aPosition, 1.0);\n"
                              "}\n";

const GLchar *FRAGMENT_SOURCE = "#version 300 es\n"
                                "precision mediump float;\n"
                                "out mediump vec4 FragColor;\n"
                                "void main()\n"
                                "{\n"
                                "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                                "}\n";

static GtkWidget *gl_area = NULL;
static GLuint vbo;
static GLuint vao;
static GLuint vertex, fragment;
static GLuint program;

static const GLfloat vertex_data[] = {0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f};

static GLuint create_shader(int type) {
  GLuint shader, program;
  shader = glCreateShader(type);
  if (type == GL_FRAGMENT_SHADER) {
    glShaderSource(shader, 1, &FRAGMENT_SOURCE, NULL);
  }
  if (type == GL_VERTEX_SHADER) {
    glShaderSource(shader, 1, &VERTEX_SOURCE, NULL);
  }
  glCompileShader(shader);

  return shader;
}

static void realize(GtkWidget *widget) {
  printf("Realize\n");
  GdkGLContext *context;
  gtk_gl_area_make_current(GTK_GL_AREA(widget));
  GError *err = gtk_gl_area_get_error(GTK_GL_AREA(widget));
  if (err != NULL) {
    g_printerr("Error: %s\n", err->message);
    return;
  }
  context = gtk_gl_area_get_context(GTK_GL_AREA(widget));

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  vertex = create_shader(GL_VERTEX_SHADER);
  fragment = create_shader(GL_FRAGMENT_SHADER);

  program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);
  glDetachShader(program, vertex);
  glDetachShader(program, fragment);
}

static gboolean render(GtkGLArea *area, GdkGLContext *context) {
  printf("Render\n");
  if (gtk_gl_area_get_error(area) != NULL)
    return FALSE;

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(program);

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);

  gtk_gl_area_queue_render(area);
  return TRUE;
}

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

  GList *children = gtk_container_get_children(GTK_CONTAINER(window));
  for (GList *iter = children; iter != NULL; iter = iter->next) {
    gtk_container_remove(GTK_CONTAINER(window), GTK_WIDGET(iter->data));
  }
  g_list_free(children);

  // gtk_container_add(GTK_CONTAINER(window), header_bar);

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
  refresh_canvas();
  refresh_details();
}

/* On SIGUSR1, refresh GUI */
void usr1_signal_handler(int signal) {
  update_displays_from_server();
  refresh_gui();
}

GtkWidget *window;

void setup_gui() {
  signal(SIGUSR1, usr1_signal_handler);

  update_displays_from_server();

  gtk_init(NULL, NULL); // NULL, NULL instead of argc, argv

  // Setup contents in window
  window = get_window();

  // Update content values in window
  refresh_gui();

  attach_canvas_updated_callback([](int selected_box, const vector<Box> boxes) {
    update_displays_from_boxes(&displays, boxes);
    selected_display = selected_box;
    refresh_gui();
  });

  attach_details_updated_callback([](int new_selected_display, DisplayInfo display) {
    displays.at(selected_display) = display;
    refresh_gui();
  });

  // gtk_widget_show_all(window); // Mark all widgets to be displayed
}

void run_gui() {
  setup_gui();
  wlr_screencopy_init();

  printf("Bytes: %.*s\n", 100, (char *)pixels);

  // GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data((guchar *)pixels, GDK_COLORSPACE_RGB,
  //                                              TRUE, // has_alpha (RGBA)
  //                                              8,    // bits per sample
  //                                              global_width, global_height, global_stride, NULL,
  //                                              NULL // no destroy notification
  // );
  // if (!pixbuf) {
  //   fprintf(stderr, "Failed to create pixbuf\n");
  //   // Handle error, maybe exit or fallback
  // }

  // GtkWidget *area = gtk_gl_area_new();
  // GtkWidget *area = do_glarea(window);
  // g_signal_connect(area, "render", G_CALLBACK(render), NULL);
  //
  gl_area = gtk_gl_area_new();
  gtk_gl_area_set_use_es(GTK_GL_AREA(gl_area), true);

  // g_signal_connect(gl_area, "realize", G_CALLBACK(realize), NULL);
  g_signal_connect(gl_area, "render", G_CALLBACK(render), NULL);
  // g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(gtk_main_quit), NULL);
  //
  // gtk_widget_realize(gl_area);
  // GdkGLContext *context;
  // gtk_gl_area_set_use_es(GTK_GL_AREA(gl_area), TRUE);
  // context = gtk_gl_area_get_context(GTK_GL_AREA(gl_area));
  // gdk_gl_context_set_use_es(context, TRUE);
  // gdk_gl_context_set_debug_enabled(context, TRUE);

  gtk_container_add(GTK_CONTAINER(window), gl_area);
  gtk_widget_show_all(window); // Mark all widgets to be displayed

  gtk_main();
  // g_object_unref(builder);
}
