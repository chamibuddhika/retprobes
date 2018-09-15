
#include <stdio.h>

#include <cstdint>

#if defined(__x86_64__)
thread_local uint64_t* currentRA;
#elif defined(__i386__)
thread_local uint32_t* currentRA;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void* __ret_getCurrentRA();
void __ret_printCurrentRA(void**);
int __ret_setCurrentRA(void*);

#ifdef __cplusplus
}
#endif

void* __ret_getCurrentRA() {
  return &currentRA;
}

void __ret_printCurrentRA(void** ra) {
  printf("Current RA is : %p\n", currentRA);
}

int __ret_setCurrentRA(void* ra) {
  currentRA = reinterpret_cast<uint64_t*>(ra);
  return 0;
}
