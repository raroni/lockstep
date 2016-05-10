#include <OpenGL/gl.h>
#include "lib/assert.h"
#include "render_commands.h"
#include "opengl.h"

void InitOpenGL() {
  glDisable(GL_DEPTH_TEST);
}

struct opengl_color {
  r32 R;
  r32 G;
  r32 B;
};

static const r32 Inv255 = 1.0f / 255.0f;

static opengl_color UnpackColor(ui32 Color) {
  ui8 R = Color >> 16;
  ui8 G = (Color & 0x0000FF00) >> 8;
  ui8 B = Color & 0x000000FF;

  opengl_color C;
  C.R = (r32)R * Inv255;
  C.G = (r32)G * Inv255;
  C.B = (r32)B * Inv255;

  return C;
}

static void SetClearColor(ui32 Color) {
  opengl_color C = UnpackColor(Color);
  glClearColor(C.R, C.G, C.B, 1.0f);
}

static void DrawSquare(r32 X, r32 Y, r32 HalfSize, ui32 Color) {
  opengl_color C = UnpackColor(Color);
  glColor3f(C.R, C.G, C.B);
  glRectf(
    X - HalfSize, Y - HalfSize,
    X + HalfSize, Y + HalfSize
  );
}

static void DrawRect(rrect Rect, ui32 Color) {
  opengl_color C = UnpackColor(Color);
  glColor3f(C.R, C.G, C.B);
  glRectf(
    Rect.Min.X, Rect.Min.Y,
    Rect.Max.X, Rect.Max.Y
  );
}

static void UpdateProjection(r32 AspectRatio, r32 Zoom) {
  glMatrixMode(GL_PROJECTION);
  r32 a = 1.0f*Zoom;
  r32 b = 1.0f*AspectRatio*Zoom;
  r32 Proj[] = {
    a,  0,  0,  0,
    0,  b,  0,  0,
    0,  0,  1,  0,
    0,  0,  0,  1,
  };
  glLoadMatrixf(Proj);
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
        DrawSquare(DrawCommand->X, DrawCommand->Y, DrawCommand->HalfSize, DrawCommand->Color);
        break;
      }
      case render_command_type_draw_rect: {
        draw_rect_render_command *DrawCommand = (draw_rect_render_command*)Body;
        DrawRect(DrawCommand->Rect, DrawCommand->Color);
        break;
      }
      case render_command_type_projection: {
        projection_render_command *ProjectionCommand = (projection_render_command*)Body;
        UpdateProjection(ProjectionCommand->AspectRatio, ProjectionCommand->Zoom);
        break;
      }
      case render_command_type_clear_color: {
        clear_color_render_command *ClearColorCommand = (clear_color_render_command*)Body;
        SetClearColor(ClearColorCommand->Color);
        break;
      }
      default:
        InvalidCodePath;
    }
  }
}
