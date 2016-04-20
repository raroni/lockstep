#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#include "lib/assert.h"
#include "common/network_messages.h"
#include "common/memory.h"
#include "network_commands.h"
#include "network_events.h"
#include "client.h"
#include "posix_network.h"

static bool TerminationRequested;

struct osx_state {
  bool Running;
  void *Memory;
  NSWindow *Window;
  linear_allocator Allocator;
  buffer ClientMemory;
  chunk_list NetworkCommandList;
  chunk_list NetworkEventList;
  pthread_t NetworkThread;
  posix_network_context NetworkContext;
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

void InitMemory(osx_state *State) {
  memsize MemorySize = 1024*1024;
  State->Memory = malloc(MemorySize);
  InitLinearAllocator(&State->Allocator, State->Memory, MemorySize);
}

void TerminateMemory(osx_state *State) {
  TerminateLinearAllocator(&State->Allocator);
  free(State->Memory);
  State->Memory = NULL;
}

void FlushNetworkCommands(posix_network_context *Context, chunk_list *Cmds) {
  for(;;) {
    buffer Command = ChunkListRead(Cmds);
    if(Command.Length == 0) {
      break;
    }
    network_command_type Type = UnserializeNetworkCommandType(Command);
    switch(Type) {
      case network_command_type_send: {
        send_network_command SendCommand = UnserializeSendNetworkCommand(Command);
        NetworkSend(Context, SendCommand.Message);
        break;
      }
      case network_command_type_shutdown: {
        ShutdownNetwork(Context);
        break;
      }
      default:
        InvalidCodePath;
    }
  }
  ResetChunkList(Cmds);
}

void ReadNetwork(posix_network_context *Context, chunk_list *Events) {
  static ui8 ReadBufferBlock[NETWORK_EVENT_MAX_LENGTH];
  static buffer ReadBuffer = {
    .Addr = &ReadBufferBlock,
    .Length = sizeof(ReadBufferBlock)
  };
  memsize Length;
  while((Length = ReadNetworkEvent(Context, ReadBuffer))) {
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

  Window.delegate = [[ClientWindowDelegate alloc] init];
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

int main() {
  osx_state State;

  InitMemory(&State);

  {
    buffer Buffer;
    Buffer.Length = NETWORK_COMMAND_MAX_LENGTH*100;
    Buffer.Addr = LinearAllocate(&State.Allocator, Buffer.Length);
    InitChunkList(&State.NetworkCommandList, Buffer);
  }

  {
    buffer Buffer;
    Buffer.Length = NETWORK_EVENT_MAX_LENGTH*100;
    Buffer.Addr = LinearAllocate(&State.Allocator, Buffer.Length);
    InitChunkList(&State.NetworkEventList, Buffer);
  }

  InitNetwork(&State.NetworkContext);
  {
    int Result = pthread_create(&State.NetworkThread, 0, RunNetwork, &State.NetworkContext);
    Assert(Result == 0);
  }

  {
    buffer *B = &State.ClientMemory;
    B->Length = 1024*512;
    B->Addr = LinearAllocate(&State.Allocator, B->Length);
  }
  InitClient(State.ClientMemory);

  NSApplication *App = [NSApplication sharedApplication];
  App.delegate = [[ClientAppDelegate alloc] init];
  App.activationPolicy = NSApplicationActivationPolicyRegular;
  SetupOSXMenu();
  [App finishLaunching];

  State.Window = CreateOSXWindow(800, 600);
  Assert(State.Window != NULL);

#ifdef DEBUG
  [NSApp activateIgnoringOtherApps:YES];
#endif

  signal(SIGINT, HandleSigint);
  State.Running = true;
  while(State.Running) {
    ProcessOSXMessages();
    ReadNetwork(&State.NetworkContext, &State.NetworkEventList);

    UpdateClient(
      TerminationRequested,
      &State.NetworkEventList,
      &State.NetworkCommandList,
      &State.Running,
      State.ClientMemory
    );
    FlushNetworkCommands(&State.NetworkContext, &State.NetworkCommandList);
    // Render();
  }

  {
    printf("Waiting for thread join...\n");
    int Result = pthread_join(State.NetworkThread, 0);
    Assert(Result == 0);
  }

  TerminateChunkList(&State.NetworkEventList);
  TerminateChunkList(&State.NetworkCommandList);
  TerminateNetwork(&State.NetworkContext);
  TerminateMemory(&State);
  printf("Gracefully terminated.\n");
  return 0;
}
