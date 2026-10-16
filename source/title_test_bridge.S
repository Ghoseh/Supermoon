// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
.text
.globl SupermoonTitleTestBridge
.seh_proc SupermoonTitleTestBridge
SupermoonTitleTestBridge:
 subq $72,%rsp
 .seh_stackalloc 72
 .seh_endprologue
 movq 72(%rsp),%rax
 movq %rax,32(%rsp)
 movq %r15,40(%rsp)
 movq %r12,48(%rsp)
 movq %rbp,56(%rsp)
 leaq 32(%rsp),%r9
 call SupermoonTitleTestHandler
 addq $72,%rsp
 ret
.seh_endproc
