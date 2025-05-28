#include "gui/box.hpp"
#include "gui/copy.hpp"

#include <epoxy/gl.h>
#include <gtk/gtk.h>
#include <iostream>
#include <ostream>
#include <stb_image.h>
#include <sys/mman.h>
#include <vector>

std::vector<struct Box> boxes;
int gl_width;
int gl_height;

const GLchar *VERTEX_SOURCE = "#version 330 core\n"
                              "layout (location = 0) in vec3 aPosition;\n"
                              "layout (location = 1) in vec2 aTexCoord;\n"
                              "layout (location = 2) in float idx;\n"

                              "out vec2 TexCoord;\n"
                              "flat out int i;\n"

                              "void main()\n"
                              "{\n"
                              "   gl_Position = vec4(aPosition, 1.0);\n"
                              "   TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
                              "   i = int(idx + 0.1);\n"
                              // "   i = 1;\n"
                              "}\n";

const GLchar *FRAGMENT_SOURCE =
    "#version 330 core\n"
    "out vec4 FragColor;\n"

    "in vec2 TexCoord;\n"
    "flat in int i;\n"

    // texture sampler
    "uniform sampler2D texture1;\n"
    "uniform sampler2D textures[3];\n"

    "void main()\n"
    "{\n"
    // "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    // "   FragColor =  mix(texture(texture2, TexCoord), vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.0f);\n"
    // "   FragColor =  mix(texture(textures[i], TexCoord), texture(texture1, TexCoord), 0.2f);\n"
    "vec4 color;\n"
    // Workaround to deal with cannot index textures with non-constant
    "if (i == 0) color = texture(textures[0], TexCoord);\n"
    "if (i == 1) color = texture(textures[1], TexCoord);\n"
    "if (i == 2) color = texture(textures[2], TexCoord);\n"
    "FragColor = color;\n"
    "}\n";

static GLuint vbo;
static GLuint vao;
static GLuint vertex, fragment;
static GLuint program;
unsigned int EBO;
unsigned int texture1, texture2;

const int MAX_DISPLAYS = 16;

GLuint textures[MAX_DISPLAYS];

// Swapped some coordinates to flip the image. Find a better way to deal with this, research origin
// of coord system between wayland output and OpenGL
float vertices[MAX_DISPLAYS * 5 * 4];
// = {
//     // float vertices[] = {
//     // positions         // texture coords                 // texture coords
//     1.0f,  1.0f,  0.0f, 1.0f, 0.0f, // top right          1.0f, 1.0f, // top right
//     1.0f,  -1.0f, 0.0f, 1.0f, 1.0f, // bottom right       1.0f, 0.0f, // bottom right
//     -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, // bottom left        0.0f, 0.0f, // bottom left
//     -1.0f, 1.0f,  0.0f, 0.0f, 0.0f  // top left           0.0f, 1.0f  // top left
// };
unsigned int indices[MAX_DISPLAYS * 6];
// unsigned int indices[] = {
//     // note that we start from 0!
//     0, 1, 3, // first triangle
//     1, 2, 3  // second triangle
// };

static GLuint create_shader(int type) {
  GLuint shader;
  shader = glCreateShader(type);
  if (type == GL_FRAGMENT_SHADER) {
    glShaderSource(shader, 1, &FRAGMENT_SOURCE, NULL);
  }
  if (type == GL_VERTEX_SHADER) {
    glShaderSource(shader, 1, &VERTEX_SOURCE, NULL);
  }
  glCompileShader(shader);
  // check for shader compile errors
  int success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    if (type == GL_FRAGMENT_SHADER)
      std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    else if (type == GL_VERTEX_SHADER)
      std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  return shader;
}

bool init = false;
int N = 3;

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
  glGenBuffers(1, &EBO);

  // 1. bind Vertex Array Object
  glBindVertexArray(vao);
  // 2. copy our vertices array in a vertex buffer for OpenGL to use
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * N * 5 * 4, vertices, GL_STATIC_DRAW);

  // // 3. copy our index array in a element buffer for OpenGL to use
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * N * 6, indices, GL_STATIC_DRAW);
  // 4. then set the vertex attributes pointers
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(5 * sizeof(float)));
  glEnableVertexAttribArray(2);

  // TEXTURES
  int width, height, nrChannels;

  glGenTextures(1, &texture1);
  // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
  glBindTexture(GL_TEXTURE_2D, texture1);
  unsigned char *data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(data);

  vertex = create_shader(GL_VERTEX_SHADER);
  fragment = create_shader(GL_FRAGMENT_SHADER);

  program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);
  glDetachShader(program, vertex);
  glDetachShader(program, fragment);

  glUseProgram(program);
  glUniform1i(glGetUniformLocation(program, "texture1"), 0);

  glGenTextures(boxes.size(), textures);
  int units[] = {1, 2, 3};
  glUniform1iv(glGetUniformLocation(program, "textures"), 3, units);
  GLenum err2;
  while ((err2 = glGetError()) != GL_NO_ERROR) {
    printf("OpenGL error after glTexSubImage2D: 0x%x\n", err2);
  }
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
    printf("FPS: %d\n", count);
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
    printf("============Getting pixels============\n");
    std::vector<CopyOutput *> copy_outputs = *get_pixels();
    for (int i = 0; i < copy_outputs.size(); i++) {
      CopyOutput *out = copy_outputs.at(i);

      // if (out->copied) {
      //   printf("Copying output %s\n", out->name);
      // } else {
      //   printf("Output %s not copied yet\n", out->name);
      //   continue;
      // }

      glActiveTexture(GL_TEXTURE1 + i);
      glBindTexture(GL_TEXTURE_2D, textures[i]);
      if (first) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, out->width, out->height, 0, GL_BGRA,
                     GL_UNSIGNED_BYTE, out->pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
        // glUniform1i(glGetUniformLocation(program, "texture2"), i + 1);
      } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, out->width, out->height, GL_BGRA, GL_UNSIGNED_BYTE,
                        out->pixels);
        // Check for errors
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
          printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
          printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
          printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
          printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
          printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
          printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
        printf("Bytes: ");
        for (int i = 0; i < 100; i++) {
          printf("%02X ", ((unsigned char *)out->pixels)[i]);
        }
        printf("\n");
      }
      // printf("Munmapping with size %d\n", out->size);
      munmap(out->pixels, out->size);
      wl_buffer_destroy(out->buffer);
      close(out->fd);
    }
    // Check for errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
      printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
      printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
      printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
      printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
      printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
      printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
      printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
      printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
      printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
      printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
      printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
      printf("OpenGL error after glTexSubImage2D: 0x%x\n", err);
    }
    if (first) {
      first = false;
    }
    if (copy_outputs.size() == 0) {
      printf("No outputs found, using default texture\n");
    }
    int idx = -1;
    for (int i = 0; i < copy_outputs.size(); i++) {
      CopyOutput *coutput = copy_outputs.at(i);
      if (strcmp(coutput->name, "DP-6") == 0) {
        // printf("Found DP-5 at index %d\n", i);
        idx = i;
        break;
      }
    }
    if (idx == -1) {
      printf("No DP-6 found, using first texture\n");
      idx = 0;
    }
    glActiveTexture(GL_TEXTURE1 + idx);
    glBindTexture(GL_TEXTURE_2D, textures[idx]);
    // glUniform1i(glGetUniformLocation(program, "texture2"), idx + 1);
  }

  // ==============================================================
  // OTHERS
  // ==============================================================

  // 2. copy our vertices array in a vertex buffer for OpenGL to use
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // as we only have a single shader, we could also just activate
  // our shader once beforehand if we want to
  glUseProgram(program);

  // render
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // bind Texture
  // glActiveTexture(GL_TEXTURE0);
  // glBindTexture(GL_TEXTURE_2D, texture1);
  // glActiveTexture(GL_TEXTURE1);
  // glBindTexture(GL_TEXTURE_2D, texture2);
  // render the triangle
  glBindVertexArray(vao);
  // glDrawArrays(GL_TRIANGLES, 0, 3);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  // glBindVertexArray(0);

  // float greenValue = (sin(time_in_seconds) / 2.0f) + 0.5f;
  // int vertexColorLocation = glGetUniformLocation(program, "ourColor");
  // glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  gtk_gl_area_queue_render(area);
  return TRUE;
}

static void on_size_allocate(GtkWidget *widget, GdkRectangle *allocation, gpointer user_data) {
  // printf("New size: %d x %d\n", allocation->width, allocation->height);
  gl_width = allocation->width;
  gl_height = allocation->height;
}

void update_glarea(std::vector<Box> new_boxes, std::vector<std::string> names) {
  printf("update gl alled\n");
  boxes = new_boxes;

  float FAC = 0.15;
  for (int i = 0; i < 3; i++) {
    Box box = boxes.at(i);
    printf("N BOXES: %d\n", boxes.size());
    // if (i != 1) {
    //   continue;
    // }
    float x1 = box.x * FAC / gl_width * 2 - 1;
    float x2 = x1 + box.width * FAC / gl_width * 2;
    float y1 = -(box.y * FAC / gl_height * 2 - 1);
    float y2 = y1 - box.height * FAC / gl_height * 2;
    x1 += i * 0.3;
    x2 += i * 0.3;
    y1 += i * 0.3;
    y2 += i * 0.3;
    int V = 6 * 4;
    vertices[i * V + 0] = x2;
    vertices[i * V + 1] = y1;
    vertices[i * V + 3] = 1.0f;
    vertices[i * V + 4] = 0.0f;
    vertices[i * V + 6] = x2;
    vertices[i * V + 7] = y2;
    vertices[i * V + 9] = 1.0f;
    vertices[i * V + 10] = 1.0f;
    vertices[i * V + 12] = x1;
    vertices[i * V + 13] = y2;
    vertices[i * V + 15] = 0.0f;
    vertices[i * V + 16] = 1.0f;
    vertices[i * V + 18] = x1;
    vertices[i * V + 19] = y1;
    vertices[i * V + 21] = 0.0f;
    vertices[i * V + 22] = 0.0f;
    vertices[i * V + 5] = i;
    vertices[i * V + 11] = i;
    vertices[i * V + 17] = i;
    vertices[i * V + 23] = i;
    int I = 6;
    indices[i * I + 0] = 0;
    indices[i * I + 1] = 1;
    indices[i * I + 2] = 3;
    indices[i * I + 3] = 1;
    indices[i * I + 4] = 2;
    indices[i * I + 5] = 3;
    // printf("(x1, y1) = (%f, %f)\n", x1, y1);
    // printf("(x2, y1) = (%f, %f)\n", x2, y1);
    // printf("(x1, y2) = (%f, %f)\n", x1, y2);
    // printf("(x2, y2) = (%f, %f)\n", x2, y2);
  }
  if (!init) {
    return;
  }
  printf("Updating GL area with %d boxes\n", boxes.size());
  int N = 3;
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * N * 5 * 4, vertices, GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * N * 6, indices, GL_STATIC_DRAW);
}

void setup_glarea(GtkWidget *gl_area) {
  g_signal_connect(gl_area, "realize", G_CALLBACK(realize), NULL);
  g_signal_connect(gl_area, "render", G_CALLBACK(render), NULL);
  g_signal_connect(gl_area, "size-allocate", G_CALLBACK(on_size_allocate), NULL);
}
