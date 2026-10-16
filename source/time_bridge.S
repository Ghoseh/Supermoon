// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
.text
.globl TimeSetterBridge
.seh_proc TimeSetterBridge
TimeSetterBridge:
 subq $264,%rsp
 .seh_stackalloc 264
 movdqu %xmm6,240(%rsp)
 .seh_savexmm xmm6,240
 .seh_endprologue
 movq %rcx,32(%rsp)
 movq %rdx,40(%rsp)
 movq %r8,48(%rsp)
 movq %r9,56(%rsp)
 movq %r10,64(%rsp)
 movq %r11,72(%rsp)
 movq %rax,80(%rsp)
 movdqu %xmm0,96(%rsp)
 movdqu %xmm1,112(%rsp)
 movdqu %xmm2,128(%rsp)
 movdqu %xmm3,144(%rsp)
 movdqu %xmm4,160(%rsp)
 movdqu %xmm5,176(%rsp)
 stmxcsr 192(%rsp)
 movq 264(%rsp),%rax
 movq %rax,200(%rsp)
 movq %r15,208(%rsp)
 movq %r12,216(%rsp)
 movq %rbp,224(%rsp)
 movq %rdx,232(%rsp)
 leaq 200(%rsp),%rdx
 call TimeSelect
 movq %rax,40(%rsp)
 movq %rax,88(%rsp)
 // Mirror caller home words, including the original setter's home-area writes.
 movq 272(%rsp),%rax
 movq %rax,0(%rsp)
 movq 280(%rsp),%rax
 movq %rax,8(%rsp)
 movq 288(%rsp),%rax
 movq %rax,16(%rsp)
 movq 296(%rsp),%rax
 movq %rax,24(%rsp)
 movdqu 96(%rsp),%xmm0
 movdqu 112(%rsp),%xmm1
 movdqu 128(%rsp),%xmm2
 movdqu 144(%rsp),%xmm3
 movdqu 160(%rsp),%xmm4
 movdqu 176(%rsp),%xmm5
 ldmxcsr 192(%rsp)
 movq 32(%rsp),%rcx
 movq 40(%rsp),%rdx
 movq 48(%rsp),%r8
 movq 56(%rsp),%r9
 movq 64(%rsp),%r10
 movq 72(%rsp),%r11
 movq 80(%rsp),%rax
 call *TimeOriginals+8(%rip)
 // Save native result registers before the guarded presentation helper.
 movq %rcx,32(%rsp)
 movq %rdx,40(%rsp)
 movq %r8,48(%rsp)
 movq %r9,56(%rsp)
 movq %r10,64(%rsp)
 movq %r11,72(%rsp)
 movq %rax,80(%rsp)
 movdqu %xmm0,96(%rsp)
 movdqu %xmm1,112(%rsp)
 movdqu %xmm2,128(%rsp)
 movdqu %xmm3,144(%rsp)
 movdqu %xmm4,160(%rsp)
 movdqu %xmm5,176(%rsp)
 stmxcsr 192(%rsp)
 movq 0(%rsp),%r10
 movq %r10,272(%rsp)
 movq 8(%rsp),%r10
 movq %r10,280(%rsp)
 movq 16(%rsp),%r10
 movq %r10,288(%rsp)
 movq 24(%rsp),%r10
 movq %r10,296(%rsp)
 leaq 200(%rsp),%rcx
 movq 88(%rsp),%rdx
 call TimeProgressToken
 movq %rax,88(%rsp)
 // Native layout/controller memory is untouched. Preserve upper96 bits.
 movdqu 240(%rsp),%xmm6
 shrq $32,%rax
 testl %eax,%eax
 jz 1f
 movss 88(%rsp),%xmm0
 movss %xmm0,%xmm6
1:

 movq 32(%rsp),%rcx
 movq 40(%rsp),%rdx
 movq 48(%rsp),%r8
 movq 56(%rsp),%r9
 movq 64(%rsp),%r10
 movq 72(%rsp),%r11
 movq 80(%rsp),%rax
 movdqu 96(%rsp),%xmm0
 movdqu 112(%rsp),%xmm1
 movdqu 128(%rsp),%xmm2
 movdqu 144(%rsp),%xmm3
 movdqu 160(%rsp),%xmm4
 movdqu 176(%rsp),%xmm5
 ldmxcsr 192(%rsp)
 addq $264,%rsp
 ret
.seh_endproc
