#include <OpenGL/gl.h>
#include "lib/assert.h"
#include "render_commands.h"
#include "opengl.h"

void InitOpenGL() {
  glClearColor(1.0, 0.0, 0.0, 1.0);
  glDisable(GL_DEPTH_TEST);
}

void DrawSquare(ui16 X, ui16 Y) {
  glColor3f(0.0, 1.0, 0.0);
  glRectf(0.0, 0.0, 0.5, 0.5);
}

void DisplayOpenGL(chunk_list *Commands) {
  glClear(GL_COLOR_BUFFER_BIT);

  for(;;) {
    buffer Command = ChunkListRead(Commands);
    if(Command.Length == 0) {
      break;
    }
    render_command_type Type = *(render_command_type*)(Command.Addr);
    void *Body = (ui8*)Command.Addr + sizeof(render_command_type);
    switch(Type) {
      case render_command_type_draw_square: {
        draw_square_render_command *DrawCommand = (draw_square_render_command*)Body;
        DrawSquare(DrawCommand->X, DrawCommand->Y);
        break;
      }
      default:
        InvalidCodePath;
    }
  }
}
