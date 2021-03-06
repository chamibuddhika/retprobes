#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif 
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>
#include <ucontext.h>
#include <time.h>
#include <string.h>

#include "getra.h"

#include <atomic>

extern "C" {
  extern void __ret_FunctionExit();
  extern int __ret_setCurrentRA(void* ra);
}

void createTimer() {

  int rv;
  struct sigevent sevp;
  timer_t timerid;
  struct itimerspec its;

  memset(&sevp, 0, sizeof(sevp));

  sevp.sigev_notify = SIGEV_THREAD_ID;
  sevp._sigev_un._tid = syscall(SYS_gettid);
  sevp.sigev_signo = SIGPROF;
  clockid_t clock = CLOCK_THREAD_CPUTIME_ID;

  rv = timer_create(clock, &sevp, &timerid);
  if (rv) {
    fprintf(stderr, "aborting due to timer_create error: %s");
  }

  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = 1000000000 / 1000;
  its.it_value = its.it_interval;
  rv = timer_settime(timerid, 0, &its, 0);
  if (rv) {
    fprintf(stderr, "aborting due to timer_settime error: %s");
  }

}

void profHandler(int sig, siginfo_t *siginfo, void *context) {
  // printf ("Sending PID: %ld, UID: %ld\n",
  //    (long)siginfo->si_pid, (long)siginfo->si_uid);
  // printf("Inside profiler handler\n"); 
  ucontext_t* sigctx = reinterpret_cast<ucontext_t*>(context);
  // printf("RIP : %p\n", (char*) sigctx->uc_mcontext.gregs[REG_RIP]);
  // printf("RSP : %p\n", (char*) sigctx->uc_mcontext.gregs[REG_RSP]);
  // printf("RBP : %p\n", (char*) sigctx->uc_mcontext.gregs[REG_RBP]);
  void** ra = getRA(reinterpret_cast<const ucontext_t*>(context));
  // printf("[Handler] Current RA is : %p\n", *ra); 
  if (!ra) {
    fprintf(stderr, "Error obtaining the return address..\n");
  }


  /*
  sigset_t block;
  sigemptyset (&block);
  sigaddset (&block, SIGPROF);
  sigprocmask (SIG_BLOCK, &block, NULL);
  */

  if (*ra != &__ret_FunctionExit) {
    int res = __ret_setCurrentRA(*ra);
    if (res) {
      fprintf(stderr, "Invalid return address on stack..\n");
    }
    *reinterpret_cast<void (**)(void)>(ra) = &__ret_FunctionExit;
  } else {
    // fprintf(stderr, "Stack frame already instrumented..\n");
  }

  /* Disable the signal handler. */
  // signal(sig, SIG_IGN);
}


void setupProfHandler() {
  struct sigaction sa;

  memset (&sa, 0, sizeof (sa));
  sa.sa_sigaction = &profHandler;

  sa.sa_flags = SA_SIGINFO;
  if (sigaction(SIGPROF, &sa, NULL) < 0) {
    perror ("sigaction");
  }
}

void initThread() {
  setupProfHandler();
  createTimer();
}

/*
int main() {

  SetupProfHandler();
  CreateTimer();
  while(1) ;

}
*/
