#ifndef GETRA_H_
#define GETRA_H_

#include <ucontext.h>

struct CallUnrollInfo {
  // Offset from (e)ip register where this instruction sequence
  // should be matched. Interpreted as bytes. Offset 0 is the next
  // instruction to execute. Be extra careful with negative offsets in
  // architectures of variable instruction length (like x86) - it is
  // not that easy as taking an offset to step one instruction back!
  int pc_offset;
  // The actual instruction bytes.
  unsigned char* ins;
  // How many bytes to match from ins array?
  int ins_size;
  // The offset from the stack pointer (e)sp where to look for the
  // call return address. Interpreted as bytes.
  int return_sp_offset;
};

#if defined(__linux) && defined(__GNUC__)
#if defined(__i386) 
static const int PTR_SIZE = 4;
static unsigned char PROLOG[] = {0x55, 0x89, 0xe5};
static unsigned char EPILOG[] = {0xc3};
#endif
#if defined(__x86_64__) 
static const int PTR_SIZE = 8;
static unsigned char PROLOG[] = {0x55, 0x48, 0x89, 0xe5};
static unsigned char EPILOG[] = {0xc3};
#endif
#endif

static const CallUnrollInfo callunrollinfo[] = {
  // Entry to a function:  push %rbp;  mov  %rsp,%rbp
  // Top-of-stack contains the caller IP.
  { 0,
    PROLOG, 
    4,
    0
  },
  // Entry to a function, second instruction:  push %rbp;  mov  %rsp,%rbp
  // Top-of-stack contains the old frame, caller IP is +8.
  { -1,
    PROLOG, 
    4,
    PTR_SIZE
  },
  // Return from a function: RET.
  // Top-of-stack contains the caller IP.
  { 0,
    EPILOG, 
    1,
    0
  }
};

/* Attempts to get the on stack reference to the return address at current 
 * frame  using the signal handler context information if provided or using 
 * current register context.
 *
 * Currently only supports x86 or x64 on Linux. Call unrolling attempts to 
 * handle situations where callee was interrupted before callee stack frame was
 * properly setup or torn down. Inspired from getpc.h at [1].
 *
 * [1] https://github.com/gperftools/gperftools/blob/master/src/getpc.h
 */
inline void** getRA(const ucontext_t* uc) {
  void **ra = NULL; 
  if (!uc) {
    unsigned long bp = 0;
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __llvm__
      // __builtin_frame_address(0) can return the wrong address on gcc-4.1.0-k8.
      // It's always correct on llvm      
      ra = reinterpret_cast<void**>(__builtin_frame_address(0)) - PTR_SIZE;
#elif defined(__x86_64__)
      __asm__ volatile ("mov %%rbp, %0" : "=r" (bp));
      ra = (void**) (bp + PTR_SIZE);  
#elif defined(__i386__)
      __asm__ volatile ("mov %%ebp, %0" : "=r" (bp));
      ra = (void**) (bp + PTR_SIZE);  
#endif
  } else {
    unsigned long ip = 0; 
    unsigned long sp = 0;
    unsigned long bp = 0;
#if defined(__i386)
    ip = uc->uc_mcontext.gregs[REG_EIP];
    sp = uc->uc_mcontext.gregs[REG_ESP];
    bp = uc->uc_mcontext.gregs[REG_EBP];
#endif
#if defined(__x86_64__)
    ip = uc->uc_mcontext.gregs[REG_RIP];
    sp = uc->uc_mcontext.gregs[REG_RSP];
    bp = uc->uc_mcontext.gregs[REG_RBP];
#endif
    if ((ip & 0xffff0000) != 0 && (~ip & 0xffff0000) != 0 &&
        (sp & 0xffff0000) != 0) {
      char* ip_char = reinterpret_cast<char*>(ip);
      for (int i = 0; i < sizeof(callunrollinfo)/sizeof(*callunrollinfo); ++i) {
        if (!memcmp(ip_char + callunrollinfo[i].pc_offset,
             callunrollinfo[i].ins, callunrollinfo[i].ins_size)) {
          // We have a match.
          ra = (void**)(sp + callunrollinfo[i].return_sp_offset);
          return ra;
        }
      }
    }

    if ((bp & 0xffff0000) != 0) {
      ra = (void**) (bp + PTR_SIZE);  
    }
  }
  return ra;
}

#endif  // GETRA_H_
