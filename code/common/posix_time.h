#include <sys/time.h>
#include <unistd.h>

uusec64 GetTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec*1000000+tv.tv_usec);
}
