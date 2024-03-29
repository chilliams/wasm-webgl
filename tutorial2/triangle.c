#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* Use glew.h instead of gl.h to get all the GL prototypes declared */
#include <GL/glew.h>
/* Using SDL2 for the base window and OpenGL context init */
#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

#endif

#include "../common-sdl2/shader_utils.h"

SDL_Window* window;
GLuint vbo_triangle;
GLuint program;
GLint attribute_coord2d;

bool
init_resources()
{
  GLfloat triangle_vertices[] = {
    0.0, 0.8, -0.8, -0.8, 0.8, -0.8,
  };
  glGenBuffers(1, &vbo_triangle);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(triangle_vertices),
               triangle_vertices,
               GL_STATIC_DRAW);

  GLint link_ok = GL_FALSE;

  GLuint vs, fs;
  if ((vs = create_shader("attribute vec2 coord2d;\n"
                          "void main(void) {\n"
                          "  gl_Position = vec4(coord2d, 0.0, 1.0);\n"
                          "}\n",
                          GL_VERTEX_SHADER)) == 0)
    return false;
  if ((fs =
         create_shader("void main(void) {\n"
                       "  gl_FragColor[0] = 0.0;\n"
                       "  gl_FragColor[1] = 0.0;\n"
                       "  gl_FragColor[2] = 1.0;\n"
                       "  gl_FragColor[3] = floor(mod(gl_FragCoord.y, 2.0));\n"
                       "}\n",
                       GL_FRAGMENT_SHADER)) == 0)
    return false;

  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
  if (!link_ok) {
    printf("glLinkProgram:\n");
    print_log(program);
    return false;
  }

  const char* attribute_name = "coord2d";
  attribute_coord2d = glGetAttribLocation(program, attribute_name);
  if (attribute_coord2d == -1) {
    printf("Could not bind attribute %s\n", attribute_name);
    return false;
  }

  return true;
}

void
render(SDL_Window* window)
{
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(program);
  glEnableVertexAttribArray(attribute_coord2d);
  /* Describe our vertices array to OpenGL (it can't guess its format
   * automatically) */
  glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
  glVertexAttribPointer(attribute_coord2d, // attribute
                        2,        // number of elements per vertex, here (x,y)
                        GL_FLOAT, // the type of each element
                        GL_FALSE, // take our values as-is
                        0,        // no extra data between each position
                        0         // offset of first element
  );

  /* Push each element in buffer_vertices to the vertex shader */
  glDrawArrays(GL_TRIANGLES, 0, 3);

  glDisableVertexAttribArray(attribute_coord2d);
  SDL_GL_SwapWindow(window);
}

void
free_resources()
{
  glDeleteProgram(program);
  glDeleteBuffers(1, &vbo_triangle);
}

bool
mainLoop()
{
  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    if (ev.type == SDL_QUIT)
      return false;
  }
  render(window);
  return true;
}

// type must match that expected by emscripten_main_loop
void
emMainLoop()
{
  mainLoop();
}

int
main(int argc, char* argv[])
{
  SDL_Init(SDL_INIT_VIDEO);
  window = SDL_CreateWindow("My Second Triangle",
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            640,
                            480,
                            SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
  if (window == NULL) {
    printf("Error: can't create window: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
  // SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);
  if (SDL_GL_CreateContext(window) == NULL) {
    printf("Error: SDL_GL_CreateContext: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  GLenum glew_status = glewInit();
  if (glew_status != GLEW_OK) {
    printf("Error: glewInit: %s\n", glewGetErrorString(glew_status));
    return EXIT_FAILURE;
  }
  if (!GLEW_VERSION_2_0) {
    printf("Error: your graphic card does not support OpenGL 2.0\n");
    return EXIT_FAILURE;
  }

  if (!init_resources())
    return EXIT_FAILURE;

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(emMainLoop, 0, true);
#else
  while (mainLoop())
    ;
#endif

  free_resources();
  return EXIT_SUCCESS;
}
