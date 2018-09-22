
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int fac(int n);

void* run(void* tid) {
  int count = 10;
  while(1) {
    fac(10);
    int x=0;
    int y = x+2;
    // sleep(1);
  }
}

void create_n_join_threads(int n_threads) {

  pthread_attr_t attr;
  /* Initialize and set thread detached attribute */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  /* Create threads */
  pthread_t threads[n_threads];
  for (int i=0; i < n_threads; i++) {
    int rc = pthread_create(&threads[i], &attr, run, (void *)NULL);
  }

  /* Free attribute and wait for the other threads */
  pthread_attr_destroy(&attr);

  void *status;
  for (int i=0; i < n_threads; i++) {
    int rc = pthread_join(threads[i], &status);
  }
}


int fac(int n)
{
  if ( n == 0 ) {
    return 1;
  } else {
    return n*fac(n-1);
  }
}

int main() {
  printf("In main..\n");
  create_n_join_threads(10);
  // fac(10);
}
