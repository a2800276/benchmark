/*
 * this is a fairly straightforward port of go-langs bench utility
 * to C ...
 * usage:
 *   see sample `main` at bottom
 * original code see:
 *   http://golang.org/src/pkg/testing/benchmark.go
 *
 * Thanks to https://github.com/jbenet for "portable" c nanosec timer:
 *   https://gist.github.com/1087739
 *
 * Javascript version of bench:
 *   https://gist.github.com/1892127
 *
 * Java version
 *   https://gist.github.com/2000141
 */


#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>



typedef struct B {
  unsigned int n;
  uint64_t ns;
  size_t bytes;
  uint64_t start;
  /* public */
  void (*benchmark)(struct B *);
  char * name;
} B;

#include <sys/time.h>
#include <time.h>


#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#define BILLION ( (uint64_t)1e9)

static uint64_t nanoseconds () {
  /* ugh */
  /* forced to use cludgy ns time code based on : 
   *   https://gist.github.com/1087739 
   * b/c osx is borked.
   */
  uint64_t result = 0;

  #ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    result = mts.tv_sec * BILLION;
    result +=  mts.tv_nsec;
  #else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    result = ts.tv_sec * BILLION;
    result += ts.tv_nsec;
  #endif
  return result;
}

static void start_timer( B * b) {
  if (b->start == 0) {
    b->start = nanoseconds(); 
  }
}
static void stop_timer( B * b) {
  if (b->start > 0) {
    b->ns += nanoseconds() - b->start;
  }
  b->start = 0;
}
static void reset_timer( B * b) {
  if (b->start > 0) {
    b->start = nanoseconds();
  }
  b->ns = 0;
}

static void set_bytes(B * b,  size_t n){
  b->bytes = n; 
}

static uint64_t ns_per_op(B * b) {
  if (b->n == 0) {
    return 0;
  }
  return b->ns / (uint64_t)b->n;
}

// Ask the benchmark to run itself N times
// and measure how long it takes.
static void run_n(B * b, unsigned int n) {
  b->n = n;
  reset_timer(b);
  start_timer(b);
  b->benchmark(b);
  stop_timer(b);
  // printf("ran n: %i times took: %lu ns\n", b->n, b->ns);
}

#define min(x,y)  (((x) > (y))?(y):(x))
#define max(x,y)  (((x) < (y))?(y):(x))

static unsigned int round_down_ten(unsigned int n){
  int tens = 0, i = 0;
  for (;n>10;) {
    n/=10;
    tens++;
  }
  unsigned int result = 1;
  for(i=0; i< tens; i++) {
    result *= 10;
  }
  return result;
}
static unsigned int round_up(unsigned int n) {
  unsigned int base = round_down_ten(n);
  if (n < (2*base)) {
    return 2*base;
  }
  if (n < (5 * base)) {
    return 5 * base;
  }
  return 10*base;
}


static void run (B * b){
  unsigned int n = 1, last;
  
  run_n(b, n);
  
  for (;b->ns < BILLION && n < BILLION;) {
    last = n;
    if (ns_per_op(b) == 0) {
      n = BILLION;
    } else {
      n = (unsigned int)(BILLION/ns_per_op(b));
    }
    n = max(min(n+n/2, 100*last), last+1);
    n = round_up(n);
    run_n(b,n);
  }
}
//void launch (B * bench){}

static double mb_per_sec(B * b) {
  if (b->bytes ==0 || b->ns == 0 || b->n ==0) {
    return 0.0;
  }
  return (b->n * ((double)b->bytes / (double) b->ns)) * 1000; 
  // N times byte/ns 
  //   *= 10e9 -> bytes/s 
  //   /= 10e6 -> mb/s
}


#define EPS 0.000000000001

static void print_results(B*b) {
  
  printf("%8u\t", b->n);

  uint64_t nsop = ns_per_op(b);
  if (b->n > 0 &&  nsop < 100) {
    if (nsop < 10) {
      printf("%13.2f ns/op", (double)b->ns/(double)b->n);
    } else {
      printf("%12.1f ns/op", (double)b->ns/(double)b->n);
    }
  } else {
    printf("%10lu ns/op", nsop);
  }
  double mbs = mb_per_sec(b);
  if (fabs(0.0 - mbs) > EPS) {
    printf("\t%9.2f MB/s", mbs);
  }

  printf("\t(%s)\n", b->name);
}

#if MAIN

#include <string.h>

void bench (B * b) {
  int i;
  void * t, * t2;
  set_bytes(b, 5000);
  for (i=0; i!=b->n; ++i) {
    t = malloc(1000);
    t2 = malloc(4000);
    memcpy(t2, t, 1000);
    free(t);
    free(t2);
  }
}

void bench2 (B * b) {
  int i;
  void * t;
  set_bytes(b, 5000);
  for (i=0; i!=b->n; ++i) {
    t = malloc(1000);
    t = realloc(t, 4000);
    free(t);
  }
}
int main () {
  B b;
  bzero(&b, sizeof(b));
  b.name = "malloc";
  b.benchmark = &bench;
  run(&b);
  print_results(&b);

  bzero(&b, sizeof(b));
  b.name = "realloc";
  b.benchmark = &bench2;
  run(&b);
  print_results(&b);
  return 0; 
}
#endif
