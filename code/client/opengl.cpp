#include <OpenGL/gl.h>

void InitOpenGL() {
  glClearColor(1.0, 0.0, 0.0, 1.0);
}

void DisplayOpenGL() {
  glClear(GL_COLOR_BUFFER_BIT);

  glColor3f(0.0, 1.0, 0.0);
  glRectf(0.0, 0.0, 0.5, 0.5);
}
