#include "common/logger.hpp"
#include "gui/box.hpp"
#include "gui/screencopy.hpp"

#include <epoxy/gl.h>
#include <gtk/gtk.h>
#include <sys/mman.h>
#include <vector>

// ==============================================================
// State and OpenGL variables
// ==============================================================

// 16 is the minimum number of textures units available in OpenGL
const int MAX_DISPLAYS = 16;
std::vector<struct Box> boxes;

bool gl_realised = false;
int gl_width;
int gl_height;

const std::string VERTEX_SOURCE =
#include "glarea.vs.glsl"
    ;
const std::string FRAGMENT_SOURCE =
#include "glarea.fs.glsl"
    ;

// OpenGL variables
static GLuint vao, vbo, ebo;
static GLuint program, vertex, fragment;
static GLuint textures[MAX_DISPLAYS];

// Define the vertex structure, example layout:
// float vertices[] = {
//     // box idx // positions (x,y) // texture coords
//     0.0, /* */ 1.0f,  1.0f,  /**/ 1.0f, 0.0f, // Box 0: top right
//     0.0, /* */ 1.0f,  -1.0f, /**/ 1.0f, 1.0f, // Box 0: bottom right
//     0.0, /* */ -1.0f, -1.0f, /**/ 0.0f, 1.0f, // Box 0: bottom left
//     0.0, /* */ -1.0f, 1.0f,  /**/ 0.0f, 0.0f, // Box 0: top left
//     1.0, /* */ 1.0f,  1.0f,  /**/ 1.0f, 0.0f, // Box 1: top right
//                                              // ...
// };
const int VERT_STRIDE = 5;
const int VERT_PER_BOX = 4;
const int VERT_COMP_PER_BOX = VERT_STRIDE * VERT_PER_BOX;
const int VERT_IDX_OFFSET = 0;
const int VERT_POS_OFFSET = 1;
const int VERT_TEX_OFFSET = 3;
const int VERT_IDX_COUNT = 1;
const int VERT_POS_COUNT = 2;
const int VERT_TEX_COUNT = 2;
float vertices[MAX_DISPLAYS * VERT_COMP_PER_BOX];

// Define the indices structure, example layout:
// unsigned int indices[] = {
//     0, 1, 3, // Box 0: first triangle
//     1, 2, 3, // Box 0: second triangle
//     4, 5, 7, // Box 0: second triangle
//     // ...
// };
const int IND_COMP_PER_BOX = 6;
unsigned int indices[MAX_DISPLAYS * IND_COMP_PER_BOX];

static void update_vertices_and_indices() {
  if (!gl_realised) {
    return;
  }

  // Shorten constant names for readability
  int A = VERT_COMP_PER_BOX;
  int B = VERT_STRIDE;
  int C = IND_COMP_PER_BOX;
  int D = VERT_PER_BOX;

  for (int i = 0; i < (int)boxes.size(); i++) {
    Box box = boxes.at(i);

    // Set index attributes for vertices of box
    vertices[A * i + B * 0 + VERT_IDX_OFFSET] = i;
    vertices[A * i + B * 1 + VERT_IDX_OFFSET] = i;
    vertices[A * i + B * 2 + VERT_IDX_OFFSET] = i;
    vertices[A * i + B * 3 + VERT_IDX_OFFSET] = i;

    // Set position attributes for vertices of box
    // (x1, y2) --> (x2, y2)
    //    ^            ^
    //    |            |
    // (x1, y1) --> (x2, y1)
    float x1 = box.x * CANVAS_FAC / gl_width * 2 - 1;
    float x2 = x1 + box.width * CANVAS_FAC / gl_width * 2;
    float y2 = -(box.y * CANVAS_FAC / gl_height * 2 - 1);
    float y1 = y2 - box.height * CANVAS_FAC / gl_height * 2;
    vertices[A * i + B * 0 + VERT_POS_OFFSET + 0] = x2;
    vertices[A * i + B * 0 + VERT_POS_OFFSET + 1] = y2;
    vertices[A * i + B * 1 + VERT_POS_OFFSET + 0] = x2;
    vertices[A * i + B * 1 + VERT_POS_OFFSET + 1] = y1;
    vertices[A * i + B * 2 + VERT_POS_OFFSET + 0] = x1;
    vertices[A * i + B * 2 + VERT_POS_OFFSET + 1] = y1;
    vertices[A * i + B * 3 + VERT_POS_OFFSET + 0] = x1;
    vertices[A * i + B * 3 + VERT_POS_OFFSET + 1] = y2;

    // Set texture coord attributes for vertices of box, accounting for screen transformation
    // (t7, t8) --> (t1, t2)
    //    ^            ^
    //    |            |
    // (t5, t6) --> (t3, t4)
    // These bits are determined by trial and error
    float t1 = (0b11001001 >> box.transform) & 1;
    float t2 = (0b10010011 >> box.transform) & 1;
    float t3 = (0b01100011 >> box.transform) & 1;
    float t4 = (0b11000110 >> box.transform) & 1;
    float t5 = 1 - t1;
    float t6 = 1 - t2;
    float t7 = 1 - t3;
    float t8 = 1 - t4;
    vertices[A * i + B * 0 + VERT_TEX_OFFSET + 0] = t1;
    vertices[A * i + B * 0 + VERT_TEX_OFFSET + 1] = t2;
    vertices[A * i + B * 1 + VERT_TEX_OFFSET + 0] = t3;
    vertices[A * i + B * 1 + VERT_TEX_OFFSET + 1] = t4;
    vertices[A * i + B * 2 + VERT_TEX_OFFSET + 0] = t5;
    vertices[A * i + B * 2 + VERT_TEX_OFFSET + 1] = t6;
    vertices[A * i + B * 3 + VERT_TEX_OFFSET + 0] = t7;
    vertices[A * i + B * 3 + VERT_TEX_OFFSET + 1] = t8;

    // Set indices of box
    indices[C * i + 0] = D * i + 0;
    indices[C * i + 1] = D * i + 1;
    indices[C * i + 2] = D * i + 3;
    indices[C * i + 3] = D * i + 1;
    indices[C * i + 4] = D * i + 2;
    indices[C * i + 5] = D * i + 3;
  }
}

static GLuint create_shader(int type) {
  GLuint shader = glCreateShader(type);
  const char *src;
  if (type == GL_VERTEX_SHADER) {
    src = VERTEX_SOURCE.c_str();
  } else if (type == GL_FRAGMENT_SHADER) {
    src = FRAGMENT_SOURCE.c_str();
  }
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);

  // Check for compile errors
  int success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    if (type == GL_VERTEX_SHADER) {
      log_critical("Failed to compile vertex shader: {}", infoLog);
    } else if (type == GL_FRAGMENT_SHADER) {
      log_critical("Failed to compile fragment shader: {}", infoLog);
    }
    exit(1);
  }

  return shader;
}

/* Callback for seting up GtkGLArea widget */
static void realize(GtkWidget *widget) {
  gl_realised = true;
  int N = 3;
  gtk_gl_area_make_current(GTK_GL_AREA(widget));
  GError *err = gtk_gl_area_get_error(GTK_GL_AREA(widget));
  if (err != NULL) {
    log_warn("Unable to setup GtkGlArea: {}", err->message);
    return;
  }

  // Create buffers
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
  glBindVertexArray(vao);

  // Copy mock vertices and indices to the GPU so that we can assign vertex attributes
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MAX_DISPLAYS * VERT_COMP_PER_BOX, nullptr,
               GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * MAX_DISPLAYS * IND_COMP_PER_BOX, nullptr,
               GL_DYNAMIC_DRAW);

  // Set vertex attributes (this configuration is fixed)
  glVertexAttribPointer(0, VERT_POS_COUNT, GL_FLOAT, GL_FALSE, VERT_STRIDE * sizeof(float),
                        (void *)(VERT_POS_OFFSET * sizeof(float)));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, VERT_TEX_COUNT, GL_FLOAT, GL_FALSE, VERT_STRIDE * sizeof(float),
                        (void *)(VERT_TEX_OFFSET * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, VERT_IDX_COUNT, GL_FLOAT, GL_FALSE, VERT_STRIDE * sizeof(float),
                        (void *)(VERT_IDX_OFFSET * sizeof(float)));
  glEnableVertexAttribArray(2);

  // Create vertex and fragment shaders (glCreateShader, glCompileShader), then do error checking
  vertex = create_shader(GL_VERTEX_SHADER);
  fragment = create_shader(GL_FRAGMENT_SHADER);

  // Setup shader program
  program = glCreateProgram(); // We are only using this one shader program
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);
  glDetachShader(program, vertex);
  glDetachShader(program, fragment);
  glUseProgram(program);

  // Allocate textures and assign "texture[i]" uniform to i-th texture unit
  glGenTextures(N, textures);
  int units[MAX_DISPLAYS];
  for (int i = 0; i < MAX_DISPLAYS; i++) {
    units[i] = i;
  }
  glUniform1iv(glGetUniformLocation(program, "textures"), MAX_DISPLAYS, units);
}

/* Callback for when GtkGLArea widget is to be rendered */
static gboolean render(GtkGLArea *area, GdkGLContext *context) {
  int N = boxes.size();
  if (gtk_gl_area_get_error(area) != NULL) {
    log_warn("Error in GL area: {}", gtk_gl_area_get_error(area)->message);
    return FALSE;
  }

  ScreencopyFrames frames = screencopy_get();
  for (ScreencopyFrame frame : frames) {
    // Figure out which texture unit this frame goes
    int idx = 0;
    for (idx = 0; idx < (int)boxes.size(); idx++) {
      if (boxes.at(idx).name == frame.name) {
        break;
      }
    }
    glActiveTexture(GL_TEXTURE0 + idx);
    glBindTexture(GL_TEXTURE_2D, textures[idx]);

    // Upload texture data to GPU
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame.width, frame.height, 0, GL_BGRA, GL_UNSIGNED_BYTE,
                 frame.pixels);
    // TODO: glTexSubImage2D has better performance but we need to ensure width and height are the
    // same glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, out.width, out.height, GL_BGRA,
    // GL_UNSIGNED_BYTE,
    //                 out.pixels);

    glGenerateMipmap(GL_TEXTURE_2D);
  }
  screencopy_destroy(frames);

  // Copy actual vertices and indices to GPU
  // TODO: Figure out why we can't move this chunk to the update_vertices_and_indices() function
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * N * VERT_COMP_PER_BOX, vertices);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(float) * N * IND_COMP_PER_BOX, indices);

  // Draw in the GL area
  glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, N * IND_COMP_PER_BOX, GL_UNSIGNED_INT, 0);

  gtk_gl_area_queue_render(area);
  return TRUE;
}

/* Callback for when GtkGLArea widget resized */
static void on_size_allocate(GtkWidget *widget, GdkRectangle *allocation, gpointer user_data) {
  gl_width = allocation->width;
  gl_height = allocation->height;
  update_vertices_and_indices();
}

// ============================================================
// Entry Point
// ============================================================

void glarea_setup(GtkWidget *gl_area) {
  g_signal_connect(gl_area, "realize", G_CALLBACK(realize), NULL);
  g_signal_connect(gl_area, "render", G_CALLBACK(render), NULL);
  g_signal_connect(gl_area, "size-allocate", G_CALLBACK(on_size_allocate), NULL);
}

void glarea_update(std::vector<Box> new_boxes) {
  boxes = new_boxes;
  update_vertices_and_indices();
}
