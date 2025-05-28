#include "gui/box.hpp"
#include "gui/copy.hpp"

#include <epoxy/gl.h>
#include <gtk/gtk.h>
#include <iostream>
#include <ostream>
#include <stb_image.h>
#include <vector>

std::vector<struct Box> boxes;
int gl_width;
int gl_height;

const GLchar *VERTEX_SOURCE = "#version 330 core\n"
                              "layout (location = 0) in vec3 aPosition;\n"
                              "layout (location = 1) in vec2 aTexCoord;\n"

                              "out vec2 TexCoord;\n"

                              "void main()\n"
                              "{\n"
                              "   gl_Position = vec4(aPosition, 1.0);\n"
                              "   TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
                              "}\n";

const GLchar *FRAGMENT_SOURCE =
    "#version 330 core\n"
    "out vec4 FragColor;\n"

    "in vec2 TexCoord;\n"

    // texture sampler
    "uniform sampler2D texture1;\n"
    "uniform sampler2D texture2;\n"

    "void main()\n"
    "{\n"
    // "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    // "   FragColor =  mix(texture(texture2, TexCoord), vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.0f);\n"
    "   FragColor =  mix(texture(texture2, TexCoord), texture(texture1, TexCoord), 0.2f);\n"
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
// float vertices[MAX_DISPLAYS * 5 * 4]
float vertices[] = {
    // positions         // texture coords                 // texture coords
    1.0f,  1.0f,  0.0f, 1.0f, 0.0f, // top right          1.0f, 1.0f, // top right
    1.0f,  -1.0f, 0.0f, 1.0f, 1.0f, // bottom right       1.0f, 0.0f, // bottom right
    -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, // bottom left        0.0f, 0.0f, // bottom left
    -1.0f, 1.0f,  0.0f, 0.0f, 0.0f  // top left           0.0f, 1.0f  // top left
};
// float indices[MAX_DISPLAYS * 6]
unsigned int indices[] = {
    // note that we start from 0!
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

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

static void realize(GtkWidget *widget) {
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
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // // 3. copy our index array in a element buffer for OpenGL to use
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  // 4. then set the vertex attributes pointers
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

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
}

int prev = 0;
int count = 0;

static gboolean render(GtkGLArea *area, GdkGLContext *context) {
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
  std::vector<CopyOutput *> copy_outputs = *get_pixels();
  for (int i = 0; i < copy_outputs.size(); i++) {
    CopyOutput *out = copy_outputs.at(i);
    glActiveTexture(GL_TEXTURE1 + i);
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, out->width, out->height, 0, GL_BGRA, GL_UNSIGNED_BYTE,
                 out->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    // glUniform1i(glGetUniformLocation(program, "texture2"), i + 1);
  }
  int idx = 0;
  for (int i = 0; i < copy_outputs.size(); i++) {
    CopyOutput *coutput = copy_outputs.at(i);
    if (strcmp(coutput->name, "DP-6") == 0) {
      printf("Found DP-5 at index %d\n", i);
      idx = i;
      break;
    }
  }
  glActiveTexture(GL_TEXTURE1 + idx);
  glBindTexture(GL_TEXTURE_2D, textures[idx]);
  glUniform1i(glGetUniformLocation(program, "texture2"), idx + 1);

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
  for (int i = 0; i < 2; i++) {
    Box box = boxes.at(i);
    if (i != 1) {
      continue;
    }
    float x1 = box.x * FAC / gl_width * 2 - 1;
    float x2 = x1 + box.width * FAC / gl_width * 2;
    float y1 = -(box.y * FAC / gl_height * 2 - 1);
    float y2 = y1 - box.height * FAC / gl_height * 2;
    vertices[0] = x2;
    vertices[1] = y1;

    vertices[5] = x2;
    vertices[6] = y2;

    vertices[10] = x1;
    vertices[11] = y2;

    vertices[15] = x1;
    vertices[16] = y1;
    printf("(x1, y1) = (%f, %f)\n", x1, y1);
    printf("(x2, y1) = (%f, %f)\n", x2, y1);
    printf("(x1, y2) = (%f, %f)\n", x1, y2);
    printf("(x2, y2) = (%f, %f)\n", x2, y2);
  }
}

void setup_glarea(GtkWidget *gl_area) {
  g_signal_connect(gl_area, "realize", G_CALLBACK(realize), NULL);
  g_signal_connect(gl_area, "render", G_CALLBACK(render), NULL);
  g_signal_connect(gl_area, "size-allocate", G_CALLBACK(on_size_allocate), NULL);
}
