
#if !defined(__APPLE__)
# define ASM_HIDDEN(symbol) .hidden symbol
# define ASM_TYPE_FUNCTION(symbol) .type symbol, @function
# define ASM_SIZE(symbol) .size symbol, .-symbol
# define ASM_SYMBOL(symbol) symbol
# define ASM_SYMBOL_INTERCEPTOR(symbol) symbol
#else
# define ASM_HIDDEN(symbol)
# define ASM_TYPE_FUNCTION(symbol)
# define ASM_SIZE(symbol)
# define ASM_SYMBOL(symbol) _##symbol
# define ASM_SYMBOL_INTERCEPTOR(symbol) _wrap_##symbol
#endif

.macro SAVE_REGISTERS
	subq $240, %rsp
//	CFI_DEF_CFA_OFFSET(248)
	movq %rbp, 232(%rsp)
	movupd	%xmm0, 216(%rsp)
	movupd	%xmm1, 200(%rsp)
	movupd	%xmm2, 184(%rsp)
	movupd	%xmm3, 168(%rsp)
	movupd	%xmm4, 152(%rsp)
	movupd	%xmm5, 136(%rsp)
	movupd	%xmm6, 120(%rsp)
	movupd	%xmm7, 104(%rsp)
	movq	%rdi, 96(%rsp)
	movq	%rax, 88(%rsp)
	movq	%rdx, 80(%rsp)
	movq	%rsi, 72(%rsp)
	movq	%rcx, 64(%rsp)
	movq	%r8, 56(%rsp)
	movq	%r9, 48(%rsp)
	movq  %r10, 40(%rsp)
	movq  %r11, 32(%rsp)
	movq  %r12, 24(%rsp)
	movq  %r13, 16(%rsp)
	movq  %r14, 8(%rsp)
	movq  %r15, 0(%rsp)
.endm

.macro RESTORE_REGISTERS
	movq  232(%rsp), %rbp
	movupd	216(%rsp), %xmm0
	movupd	200(%rsp), %xmm1
	movupd	184(%rsp), %xmm2
	movupd	168(%rsp), %xmm3
	movupd	152(%rsp), %xmm4
	movupd	136(%rsp), %xmm5
	movupd	120(%rsp) , %xmm6
	movupd	104(%rsp) , %xmm7
	movq	96(%rsp), %rdi
	movq	88(%rsp), %rax
	movq	80(%rsp), %rdx
	movq	72(%rsp), %rsi
	movq	64(%rsp), %rcx
	movq	56(%rsp), %r8
	movq	48(%rsp), %r9
	movq  40(%rsp), %r10
	movq  32(%rsp), %r11
	movq  24(%rsp), %r12
	movq  16(%rsp), %r13
	movq  8(%rsp), %r14
	movq  0(%rsp), %r15
	addq	$240, %rsp
//	CFI_DEF_CFA_OFFSET(8)
.endm

	.text
#if !defined(__APPLE__)
	.section .text
#else
	.section __TEXT,__text
#endif
	.file "trampoline_x86.S"


//===----------------------------------------------------------------------===//

  // .globl ASM_SYMBOL(__ret_getCurrentRA)
  // .globl ASM_SYMBOL(__ret_printCurrentRA)

	.globl ASM_SYMBOL(__ret_FunctionExit)
	.align 16, 0x90
	ASM_TYPE_FUNCTION(__ret_FunctionExit)
# LLVM-MCA-BEGIN __xray_FunctionExit
ASM_SYMBOL(__ret_FunctionExit):
	// CFI_STARTPROC
	// Save the important registers first. Since we're assuming that this
	// function is only jumped into, we only preserve the registers for
	// returning.
	subq	$64, %rsp
	// CFI_DEF_CFA_OFFSET(64)
	movq  %rbp, 48(%rsp)
	movupd	%xmm0, 32(%rsp)
	movupd	%xmm1, 16(%rsp)
	movq	%rax, 8(%rsp)
	movq	%rdx, 0(%rsp)
  callq  ASM_SYMBOL(__ret_getCurrentRA@PLT)
  movq  (%rax), %rax
  movq %rax, 56(%rsp)
  // xor   %rsi,%rsi
  callq __ret_printCurrentRA@PLT
  
  // Fix up the next frame
  // Save next return address in the TL
	movq  48(%rsp), %rbp
  movq 8(%rbp), %rdi
  callq ASM_SYMBOL(__ret_setCurrentRA@PLT)
  testq %rax, %rax 
  jne .Ltmp
  // Overwrite trampoline as the return address of the frame above
  movq ASM_SYMBOL(trampoline_fn@GOTPCREL)(%rip), %rax
  movq (%rax), %rax
  mov %rax, 8(%rbp)

.Ltmp:
	// Restore the important registers.
	movq  48(%rsp), %rbp
	movupd	32(%rsp), %xmm0
	movupd	16(%rsp), %xmm1
	movq	8(%rsp), %rax
	movq	0(%rsp), %rdx
	addq	$56, %rsp
	// CFI_DEF_CFA_OFFSET(8)
	retq
# LLVM-MCA-END
	ASM_SIZE(__ret_FunctionExit)
	// CFI_ENDPROC
