#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "../shared.h"

static volatile bool Running;

static void HandleSigint(int signum) {
  Running = false;
}

#define TEST_BUFFER_SIZE 4096
ui8 TestBuffer[TEST_BUFFER_SIZE];

int main() {
  Running = true;

  signal(SIGINT, HandleSigint);

  int FD = socket(PF_INET, SOCK_STREAM, 0);
  if(FD < 0) {
    printf("Could not get socket.\n");
    return 1;
  }
  fcntl(FD, F_SETFL, O_NONBLOCK);

  struct sockaddr_in Address;
  memset(&Address, 0, sizeof(Address));
  Address.sin_len = sizeof(Address);
  Address.sin_family = AF_INET;
  Address.sin_port = htons(4321);
  Address.sin_addr.s_addr = 0;

  bool Connected = false;

  int ConnectResult = connect(FD, (struct sockaddr *)&Address, sizeof(Address));
  if(ConnectResult == -1 && errno != errno_code_in_progress) {
    printf("connect() failed (%s, %d).\n", strerror(errno), errno);
    return 1;
  }
  else if(ConnectResult == 0) {
    printf("Connected\n");
    Connected = true;
  }

  int SelectResult;
  struct timeval Timeout;
  fd_set FDSet;
  while(Running) {
    Timeout.tv_sec = 0;
    Timeout.tv_usec = 500;
    FD_ZERO(&FDSet);
    FD_SET(FD, &FDSet);

    if(Connected) {
      SelectResult = select(FD+1, &FDSet, NULL, NULL, &Timeout);
      if(SelectResult == -1) {
        if(errno == errno_code_interrupted_system_call) {
          break;
        }
        else {
          printf("Select failed (%s).\n", strerror(errno));
          return 1;
        }
      }
      if(SelectResult != 0 && FD_ISSET(FD, &FDSet)) {
        ssize_t Result = recv(FD, TestBuffer, TEST_BUFFER_SIZE, 0); // TODO: Loop until you have all
        if(Result == 0) {
          Connected = false;
          printf("Server disconnected.\n");
          break;
        }
        else {
          printf("Got something %d, %s\n", (int)Result, (char*)TestBuffer);
        }
      }
    }
    else {
      SelectResult = select(FD+1, NULL, &FDSet, NULL, &Timeout);
      if(SelectResult == -1) {
        if(errno == errno_code_interrupted_system_call) {
          break;
        }
        else {
          printf("Select failed (%s).\n", strerror(errno));
          return 1;
        }
      }
      if(SelectResult != 0) {
        int OptionValue;
        socklen_t OptionLength = sizeof(OptionValue);
        getsockopt(FD, SOL_SOCKET, SO_ERROR, &OptionValue, &OptionLength);
        if(OptionValue == 0) {
          Connected = true;
          printf("Connected.\n");
        }
        else {
          printf("Connection failed.\n");
          break;
        }
      }
    }
  }

  close(FD);
  printf("\nGracefully terminated.\n");
  return 0;
}
