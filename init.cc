#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif 
#include <dlfcn.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <atomic>

extern "C" {
  extern void __ret_FunctionExit();
  extern int __ret_setCurrentRA(void* ra);
}

extern void InitThread();

typedef int (*pthread_create_type)(pthread_t *__restrict __newthread,
             const pthread_attr_t *__restrict __attr,
                      void *(*__start_routine) (void *),
                               void *__restrict __arg);

static pthread_create_type original_pthread_create;

void* trampoline_fn;

static __attribute__((constructor))
void __constructor() {
  original_pthread_create = (pthread_create_type)(intptr_t)dlsym(RTLD_NEXT, "pthread_create");
  if (!original_pthread_create) {
    fprintf(stderr, "failed to locate original pthread_create: %s\n", dlerror());
    abort();
  }

  trampoline_fn = (void*) &__ret_FunctionExit;

  // Initialzes the main thread
  // InitThread();
}

struct trampoline_data {
  void *(*original_fn)(void *);
  void *original_arg;
};

static void *trampoline(void *_arg) {
  struct trampoline_data data = *(struct trampoline_data *)_arg;
  free(_arg);
  // fprintf(stderr, "Activating profiler for newly created thread %p\n", (void *)pthread_self());
  InitThread();
  return data.original_fn(data.original_arg);
}

#ifdef __cplusplus  
extern "C" { 
#endif

int pthread_create(pthread_t *__restrict thread,
             const pthread_attr_t *__restrict attr,
                      void *(*fn) (void *),
                      void *__restrict arg) {
  printf("In create\n");
  struct trampoline_data *data;

  data = (struct trampoline_data*) malloc(sizeof(struct trampoline_data));
  if (!data) {
    return ENOMEM;
  }
  data->original_fn = fn;
  data->original_arg = arg;

  int rv = original_pthread_create(thread, attr, trampoline, data);
  if (rv)
    free(data);
  return rv;
}

#ifdef __cplusplus 
} 
#endif 

