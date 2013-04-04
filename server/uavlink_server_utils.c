#include <sys/time.h>
#include <stdlib.h>
#include <stdint.h>

uint64_t get_time_stamp() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}
