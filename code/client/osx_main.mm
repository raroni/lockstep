#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#include "lib/assert.h"
#include "common/net_messages.h"
#include "common/memory.h"
#include "net_commands.h"
#include "net_events.h"
#include "game.h"
#include "posix_net.h"
#include "opengl.h"

static bool TerminationRequested;

struct resolution {
  ui16 Width;
  ui16 Height;
};

struct osx_state {
  bool Running;
  void *Memory;
  NSWindow *Window;
  NSOpenGLContext *OGLContext;
  linear_allocator Allocator;
  buffer ClientMemory;
  chunk_list NetCommandList;
  chunk_list NetEventList;
  chunk_list RenderCommandList;
  resolution Resolution;
  pthread_t NetThread;
  posix_net_context NetContext;
};

@interface ClientAppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation ClientAppDelegate
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
  TerminationRequested = true;
  return NSTerminateCancel;
}
@end

@interface ClientWindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation ClientWindowDelegate
- (BOOL)windowShouldClose:(id)sender {
    TerminationRequested = true;
    return NO;
}
@end

static void HandleSigint(int signum) {
  TerminationRequested = true;
}

static void InitMemory(osx_state *State) {
  memsize MemorySize = 1024*1024;
  State->Memory = malloc(MemorySize);
  InitLinearAllocator(&State->Allocator, State->Memory, MemorySize);
}

static void TerminateMemory(osx_state *State) {
  TerminateLinearAllocator(&State->Allocator);
  free(State->Memory);
  State->Memory = NULL;
}

static void ExecuteNetCommands(posix_net_context *Context, chunk_list *Cmds) {
  for(;;) {
    buffer Command = ChunkListRead(Cmds);
    if(Command.Length == 0) {
      break;
    }
    net_command_type Type = UnserializeNetCommandType(Command);
    switch(Type) {
      case net_command_type_send: {
        send_net_command SendCommand = UnserializeSendNetCommand(Command);
        PosixNetSend(Context, SendCommand.Message);
        break;
      }
      case net_command_type_shutdown: {
        ShutdownPosixNet(Context);
        break;
      }
      default:
        InvalidCodePath;
    }
  }
  ResetChunkList(Cmds);
}

static void ExecuteRenderCommands(chunk_list *Commands) {
  DisplayOpenGL(Commands);
  ResetChunkList(Commands);
}

static void ReadNet(posix_net_context *Context, chunk_list *Events) {
  static ui8 ReadBufferBlock[NETWORK_EVENT_MAX_LENGTH];
  static buffer ReadBuffer = {
    .Addr = &ReadBufferBlock,
    .Length = sizeof(ReadBufferBlock)
  };
  memsize Length;
  while((Length = ReadPosixNetEvent(Context, ReadBuffer))) {
    buffer Event = {
      .Addr = ReadBuffer.Addr,
      .Length = Length
    };
    ChunkListWrite(Events, Event);
  }
}

static void SetupOSXMenu() {
  NSMenu *Menu = [[NSMenu alloc] init];
  NSMenuItem *QuitItem = [[NSMenuItem alloc] initWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
  [Menu addItem:QuitItem];

  NSMenuItem *BarItem = [[NSMenuItem alloc] init];
  [BarItem setSubmenu:Menu];

  NSMenu *Bar = [[NSMenu alloc] init];
  [Bar addItem:BarItem];

  [NSApp setMainMenu:Bar];

  [Menu release];
  [QuitItem release];
  [BarItem release];
  [Bar release];
}

static NSOpenGLContext* CreateOGLContext() {
  NSOpenGLPixelFormatAttribute Attributes[] = {
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAOpenGLProfile,
    NSOpenGLProfileVersionLegacy,
    0
  };
  NSOpenGLPixelFormat *PixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:Attributes];
  if(PixelFormat == nil) {
    return NULL;
  }

  NSOpenGLContext *Context = [[NSOpenGLContext alloc] initWithFormat:PixelFormat shareContext:nil];

  GLint Sync = 1;
  [Context setValues:&Sync forParameter:NSOpenGLCPSwapInterval];

  [PixelFormat release];

  return Context;
}

static NSWindow* CreateOSXWindow(ui16 Width, ui16 Height) {
  int StyleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;

  CGRect Rect = NSMakeRect(0, 0, Width, Height);
  NSWindow *Window = [[NSWindow alloc] initWithContentRect:Rect
                                          styleMask:StyleMask
                                            backing:NSBackingStoreBuffered
                                              defer:NO
                                              screen:[NSScreen mainScreen]];
  if(Window == nil) {
    return NULL;
  }

  ClientWindowDelegate *Delegate = [[ClientWindowDelegate alloc] init];
  Window.delegate = Delegate;
  Window.title = [NSString stringWithUTF8String:"Lockstep Client"];

  [Window center];
  [Window makeKeyAndOrderFront:nil];

  return Window;
}

static void ProcessOSXMessages() {
  while(true) {
    NSEvent *Event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                        untilDate:[NSDate distantPast]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
    if(Event == nil) {
      return;
    }
    else {
      [NSApp sendEvent:Event];
    }
  }
}

r32 GetAspectRatio(resolution Resolution) {
  return r32(Resolution.Width) / r32(Resolution.Height);
}

int main() {
  osx_state State;
  State.Resolution.Width = 800;
  State.Resolution.Height = 600;

  InitMemory(&State);

  {
    buffer Buffer;
    Buffer.Length = NETWORK_COMMAND_MAX_LENGTH*100;
    Buffer.Addr = LinearAllocate(&State.Allocator, Buffer.Length);
    InitChunkList(&State.NetCommandList, Buffer);
  }

  {
    buffer Buffer;
    Buffer.Length = NETWORK_EVENT_MAX_LENGTH*100;
    Buffer.Addr = LinearAllocate(&State.Allocator, Buffer.Length);
    InitChunkList(&State.NetEventList, Buffer);
  }

  {
    buffer Buffer;
    Buffer.Length = 1024*200;
    Buffer.Addr = LinearAllocate(&State.Allocator, Buffer.Length);
    InitChunkList(&State.RenderCommandList, Buffer);
  }

  InitPosixNet(&State.NetContext);
  {
    int Result = pthread_create(&State.NetThread, 0, RunPosixNet, &State.NetContext);
    Assert(Result == 0);
  }

  {
    buffer *B = &State.ClientMemory;
    B->Length = 1024*512;
    B->Addr = LinearAllocate(&State.Allocator, B->Length);
  }
  InitGame(State.ClientMemory);

  NSApplication *App = [NSApplication sharedApplication];
  App.delegate = [[ClientAppDelegate alloc] init];
  App.activationPolicy = NSApplicationActivationPolicyRegular;
  SetupOSXMenu();
  [App finishLaunching];

  State.Window = CreateOSXWindow(State.Resolution.Width, State.Resolution.Height);
  Assert(State.Window != NULL);

  State.OGLContext = CreateOGLContext();
  Assert(State.OGLContext != NULL);
  [State.OGLContext makeCurrentContext];
  [State.OGLContext setView:State.Window.contentView];

#ifdef DEBUG
  [NSApp activateIgnoringOtherApps:YES];
#endif

  signal(SIGINT, HandleSigint);
  InitOpenGL(GetAspectRatio(State.Resolution));
  State.Running = true;
  while(State.Running) {
    ProcessOSXMessages();
    ReadNet(&State.NetContext, &State.NetEventList);

    UpdateGame(
      TerminationRequested,
      &State.NetEventList,
      &State.NetCommandList,
      &State.RenderCommandList,
      &State.Running,
      State.ClientMemory
    );
    ResetChunkList(&State.NetEventList);
    ExecuteNetCommands(&State.NetContext, &State.NetCommandList);
    ExecuteRenderCommands(&State.RenderCommandList);
    [State.OGLContext flushBuffer];
  }

  {
    printf("Waiting for thread join...\n");
    int Result = pthread_join(State.NetThread, 0);
    Assert(Result == 0);
  }

  {
    ClientAppDelegate *D = App.delegate;
    App.delegate = nil;
    [D release];
  }
  {
    ClientWindowDelegate *D = State.Window.delegate;
    State.Window.delegate = nil;
    [D release];
  }
  [State.Window release];
  [State.OGLContext release];

  TerminateChunkList(&State.RenderCommandList);
  TerminateChunkList(&State.NetEventList);
  TerminateChunkList(&State.NetCommandList);
  TerminatePosixNet(&State.NetContext);
  TerminateMemory(&State);
  printf("Gracefully terminated.\n");
  return 0;
}
