#include "common/logger.hpp"
#include "gui/box.hpp"
#include "gui/copy.hpp"

#include <epoxy/gl.h>
#include <gtk/gtk.h>
#include <stb_image.h>
#include <sys/mman.h>
#include <vector>

std::vector<struct Box> boxes;
std::vector<std::string> names;
int gl_width;
int gl_height;

const std::string VERTEX_SOURCE =
#include "vs.glsl"
    ;
const std::string FRAGMENT_SOURCE =
#include "fs.glsl"
    ;

const int MAX_DISPLAYS = 3;

// OpenGL variables
static GLuint vao, vbo, ebo;
static GLuint program, vertex, fragment;
GLuint textures[MAX_DISPLAYS];

float vertices[MAX_DISPLAYS * 6 * 4];
// // Example layout of vertices variable:
// float vertices[] = {
//     // box    // positions (x,y) // texture coords
//     0.0, /**/ 1.0f,  1.0f,  /**/ 1.0f, 0.0f, // Box 0: top right
//     0.0, /**/ 1.0f,  -1.0f, /**/ 1.0f, 1.0f, // Box 0: bottom right
//     0.0, /**/ -1.0f, -1.0f, /**/ 0.0f, 1.0f, // Box 0: bottom left
//     0.0, /**/ -1.0f, 1.0f,  /**/ 0.0f, 0.0f, // Box 0: top left
//     1.0, /**/ 1.0f,  1.0f,  /**/ 1.0f, 0.0f, // Box 1: top right
//                                              // ...
// };
const int VERTICES_IDX_OFFSET = 0;
const int VERTICES_POS_OFFSET = 1;
const int VERTICES_TEX_OFFSET = 3;
const int VERTICES_IDX_COUNT = 1;
const int VERTICES_POS_COUNT = 2;
const int VERTICES_TEX_COUNT = 2;
const int VERTICES_STRIDE = 5;
const int VERTICES_PER_BOX = VERTICES_STRIDE * 4;

// unsigned int indices[] = {
//     0, 1,  3,  // triangle 1
//     1, 2,  3,  // triangle 2
//     4, 5,  7,  // triangle 3
//     5, 6,  7,  // triangle 4
//     8, 9,  11, // triangle 5
//     9, 10, 11  // triangle 6
// };

unsigned int indices[MAX_DISPLAYS * 6];
// Example layout of indices variable:
// unsigned int indices[] = {
//     0, 1, 3, // Box 0: first triangle
//     1, 2, 3, // Box 0: second triangle
//     4, 5, 7, // Box 0: second triangle
//     // ...
// };
const int INDICES_PER_BOX = 6;

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

bool init = false;
int N = 2;

static void realize(GtkWidget *widget) {
  init = true;
  printf("Realize\n");
  gtk_gl_area_make_current(GTK_GL_AREA(widget));
  GError *err = gtk_gl_area_get_error(GTK_GL_AREA(widget));
  if (err != NULL) {
    g_printerr("Error: %s\n", err->message);
    return;
  }

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  // 1. bind Vertex Array Object
  glBindVertexArray(vao);
  // 2. copy our vertices array in a vertex buffer for OpenGL to use
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * N * VERTICES_PER_BOX, vertices, GL_STATIC_DRAW);

  // // 3. copy our index array in a element buffer for OpenGL to use
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * N * INDICES_PER_BOX, indices,
               GL_STATIC_DRAW);
  // 4. then set the vertex attributes pointers
  glVertexAttribPointer(0, VERTICES_POS_COUNT, GL_FLOAT, GL_FALSE, VERTICES_STRIDE * sizeof(float),
                        (void *)(VERTICES_POS_OFFSET * sizeof(float)));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, VERTICES_TEX_COUNT, GL_FLOAT, GL_FALSE, VERTICES_STRIDE * sizeof(float),
                        (void *)(VERTICES_TEX_OFFSET * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, VERTICES_IDX_COUNT, GL_FLOAT, GL_FALSE, VERTICES_STRIDE * sizeof(float),
                        (void *)(VERTICES_IDX_OFFSET * sizeof(float)));
  glEnableVertexAttribArray(2);

  // Create vertex and fragment shaders (glCreateShader, glCompileShader), then do error checking
  vertex = create_shader(GL_VERTEX_SHADER);
  fragment = create_shader(GL_FRAGMENT_SHADER);

  // We are only using this one shader program
  program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);
  glDetachShader(program, vertex);
  glDetachShader(program, fragment);
  glUseProgram(program);

  glGenTextures(N, textures);
  int units[] = {1, 2};
  glUniform1iv(glGetUniformLocation(program, "textures"), 2, units);
}

int prev = 0;
int count = 0;
int first = true;
int n = 0;

static gboolean render(GtkGLArea *area, GdkGLContext *context) {
  n++;
  // ==============================================================
  // FPS
  // ==============================================================
  gint64 usec = g_get_monotonic_time();
  int time_in_seconds = usec / 1000000.0;
  count += 1;
  if (prev != time_in_seconds) {
    // printf("FPS: %d\n", count);
    count = 0;
  }
  prev = time_in_seconds;

  if (gtk_gl_area_get_error(area) != NULL)
    return FALSE;
  // ==============================================================
  // COPY PIXELS
  // ==============================================================
  // TODO: Solve memory leak

  // glDeleteTextures(boxes.size(), textures);
  // glGenTextures(boxes.size(), textures);
  if (n % 10 == 0) {
    // printf("============Getting pixels============\n");
    ScreencopyObject copy_outputs = screencopy_get();

    std::vector<Box> sorted_boxes;
    std::vector<std::string> sorted_names;

    for (int i = 0; i < copy_outputs.frames.size(); i++) {
      ScreencopyFrame out = copy_outputs.frames.at(i);

      // if (out->copied) {
      //   printf("Copying output %s\n", out->name);
      // } else {
      //   printf("Output %s not copied yet\n", out->name);
      //   continue;
      // }
      printf("Copying output %s with size %d x %d\n", out.name, out.width, out.height);

      glActiveTexture(GL_TEXTURE1 + i);
      glBindTexture(GL_TEXTURE_2D, textures[i]);
      if (first) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, out.width, out.height, 0, GL_BGRA, GL_UNSIGNED_BYTE,
                     out.pixels);
      } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, out.width, out.height, GL_BGRA, GL_UNSIGNED_BYTE,
                        out.pixels);
        printf("Bytes: ");
        for (int i = 0; i < 100; i++) {
          printf("%02X ", ((unsigned char *)out.pixels)[i]);
        }
        printf("\n");
      }
      glGenerateMipmap(GL_TEXTURE_2D);
      // munmap(out.pixels, out.size);
      // wl_buffer_destroy(out.buffer);
      // close(out.fd);
    }
    if (first) {
      first = false;
    }
    // if (copy_outputs.size() == 0) {
    //   printf("No outputs found, using default texture\n");
    // }
  }

  // ==============================================================
  // OTHERS
  // ==============================================================

  // 2. copy our vertices array in a vertex buffer for OpenGL to use
  // printf("Vertices: \n");
  // for (int i = 0; i < MAX_DISPLAYS * 6 * 4; i++) {
  //   printf("%f ", vertices[i]);
  // }
  // printf("\nIndices: \n");
  // for (int i = 0; i < MAX_DISPLAYS * 6; i++) {
  //   printf("%u ", indices[i]);
  // }
  // printf("\n");

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * N * VERTICES_PER_BOX, vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * N * INDICES_PER_BOX, indices,
               GL_STATIC_DRAW);

  // Draw
  glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, N * 6, GL_UNSIGNED_INT, 0);

  gtk_gl_area_queue_render(area);
  return TRUE;
}

static void on_size_allocate(GtkWidget *widget, GdkRectangle *allocation, gpointer user_data) {
  // printf("New size: %d x %d\n", allocation->width, allocation->height);
  gl_width = allocation->width;
  gl_height = allocation->height;
}

std::vector<std::string> output_names;

void update_glarea(std::vector<Box> new_boxes, std::vector<std::string> new_names) {
  printf("update gl alled\n");
  if (output_names.size() == 0) {
    ScreencopyObject copy_outputs = screencopy_get();

    for (int i = 0; i < copy_outputs.frames.size(); i++) {
      ScreencopyFrame out = copy_outputs.frames.at(i);
      output_names.push_back(out.name);
    }
  }

  std::vector<Box> sorted_boxes;
  std::vector<std::string> sorted_names;
  boxes = new_boxes;
  names = new_names;
  for (int i = 0; i < output_names.size(); i++) {
    for (int j = 0; j < boxes.size(); j++) {
      std::string name = names.at(j);
      printf("Comparing %s with %s\n", name.c_str(), output_names.at(i).c_str());

      if (strcmp(name.c_str(), output_names.at(i).c_str()) == 0) {
        sorted_boxes.push_back(boxes.at(j));
        sorted_names.push_back(name);
        break;
      }
    }
  }
  boxes = sorted_boxes;
  names = sorted_names;
  for (int i = 0; i < boxes.size(); i++) {
    printf("Box %d: (%f, %f, %f, %f) - %s\n", i, boxes.at(i).x, boxes.at(i).y, boxes.at(i).width,
           boxes.at(i).height, names.at(i).c_str());
  }
  printf("Size of boxes: %d\n", boxes.size());

  float FAC = 0.15;
  for (int i = 0; i < boxes.size(); i++) {
    Box box = boxes.at(i);
    // (x1, y2) --> (x2, y2)
    //    ^            ^
    //    |            |
    // (x1, y1) --> (x2, y1)
    float x1 = box.x * FAC / gl_width * 2 - 1;
    float x2 = x1 + box.width * FAC / gl_width * 2;
    float y2 = -(box.y * FAC / gl_height * 2 - 1);
    float y1 = y2 - box.height * FAC / gl_height * 2;
    int A = VERTICES_PER_BOX;
    int B = VERTICES_STRIDE;
    vertices[A * i + B * 0 + VERTICES_POS_OFFSET + 0] = x2;
    vertices[A * i + B * 0 + VERTICES_POS_OFFSET + 1] = y2;
    vertices[A * i + B * 1 + VERTICES_POS_OFFSET + 0] = x2;
    vertices[A * i + B * 1 + VERTICES_POS_OFFSET + 1] = y1;
    vertices[A * i + B * 2 + VERTICES_POS_OFFSET + 0] = x1;
    vertices[A * i + B * 2 + VERTICES_POS_OFFSET + 1] = y1;
    vertices[A * i + B * 3 + VERTICES_POS_OFFSET + 0] = x1;
    vertices[A * i + B * 3 + VERTICES_POS_OFFSET + 1] = y2;
    vertices[A * i + B * 0 + VERTICES_TEX_OFFSET + 0] = 1.0f;
    vertices[A * i + B * 0 + VERTICES_TEX_OFFSET + 1] = 0.0f;
    vertices[A * i + B * 1 + VERTICES_TEX_OFFSET + 0] = 1.0f;
    vertices[A * i + B * 1 + VERTICES_TEX_OFFSET + 1] = 1.0f;
    vertices[A * i + B * 2 + VERTICES_TEX_OFFSET + 0] = 0.0f;
    vertices[A * i + B * 2 + VERTICES_TEX_OFFSET + 1] = 1.0f;
    vertices[A * i + B * 3 + VERTICES_TEX_OFFSET + 0] = 0.0f;
    vertices[A * i + B * 3 + VERTICES_TEX_OFFSET + 1] = 0.0f;
    vertices[A * i + B * 0 + VERTICES_IDX_OFFSET] = i;
    vertices[A * i + B * 1 + VERTICES_IDX_OFFSET] = i;
    vertices[A * i + B * 2 + VERTICES_IDX_OFFSET] = i;
    vertices[A * i + B * 3 + VERTICES_IDX_OFFSET] = i;
    int I = 6;
    int J = 4;
    // TODO: Swapped some coordinates to flip the image. Find a better way to deal with this,
    // research origin of coord system between wayland output and OpenGL
    indices[i * I + 0] = i * J + 0;
    indices[i * I + 1] = i * J + 1;
    indices[i * I + 2] = i * J + 3;
    indices[i * I + 3] = i * J + 1;
    indices[i * I + 4] = i * J + 2;
    indices[i * I + 5] = i * J + 3;
    // printf("(x1, y1) = (%f, %f)\n", x1, y1);
    // printf("(x2, y1) = (%f, %f)\n", x2, y1);
    // printf("(x1, y2) = (%f, %f)\n", x1, y2);
    // printf("(x2, y2) = (%f, %f)\n", x2, y2);
  }
  if (!init) {
    return;
  }
  // printf("Updating GL area with %d boxes\n", boxes.size());
  // int N = 3;
  // glBufferData(GL_ARRAY_BUFFER, sizeof(float) * N * 6 * 4, vertices, GL_STATIC_DRAW);
  // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * N * 6, indices, GL_STATIC_DRAW);
}

void setup_glarea(GtkWidget *gl_area) {
  g_signal_connect(gl_area, "realize", G_CALLBACK(realize), NULL);
  g_signal_connect(gl_area, "render", G_CALLBACK(render), NULL);
  g_signal_connect(gl_area, "size-allocate", G_CALLBACK(on_size_allocate), NULL);
}
