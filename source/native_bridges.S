// SPDX-FileCopyrightText: 2026 Supermoon contributors
// SPDX-License-Identifier: MIT
// x64 Windows mid-function call bridges. Original live flags, volatile GPRs,
// XMM0..5 and MXCSR are preserved; the C++ ABI preserves other nonvolatiles.
// Frame-pointer epilog is canonical for the Windows unwinder. The initial flags
// slot becomes the saved RBP slot after flags have been copied into the frame.
.intel_syntax noprefix
.text
// A helper is inserted in the middle of native code, where ordinary volatile
// registers and condition flags can still be live. SAVE protects that state
// beyond what a normal C++ call promises. Each .seh directive describes the
// corresponding stack operation so Windows can unwind through this bridge.
.macro SAVE name
.p2align 4
.globl \name
.seh_proc \name
\name:
    pushfq
    .seh_stackalloc 8
    push rax
    .seh_stackalloc 8
    sub rsp,232
    // Bottom 32 bytes are Windows call shadow space. Saved registers/SIMD and
    // control state occupy the rest; total frame keeps helper calls aligned.
    .seh_stackalloc 232
    mov rax,[rsp+240]
    mov [rsp+200],rax
    mov [rsp+240],rbp
    .seh_savereg rbp,240
    lea rbp,[rsp+128]
    .seh_setframe rbp,128
    .seh_endprologue
    mov [rsp+32],rcx
    mov [rsp+40],rdx
    mov [rsp+48],r8
    mov [rsp+56],r9
    mov [rsp+64],r10
    mov [rsp+72],r11
    movdqu [rsp+96],xmm0
    movdqu [rsp+112],xmm1
    movdqu [rsp+128],xmm2
    movdqu [rsp+144],xmm3
    movdqu [rsp+160],xmm4
    movdqu [rsp+176],xmm5
    stmxcsr [rsp+192]
.endm
.macro RESTORE
    // Restore the native floating-point mode as well as register values. The
    // final LEA/POP/RET epilog does not overwrite the restored condition flags.
    ldmxcsr [rsp+192]
    movdqu xmm0,[rsp+96]
    movdqu xmm1,[rsp+112]
    movdqu xmm2,[rsp+128]
    movdqu xmm3,[rsp+144]
    movdqu xmm4,[rsp+160]
    movdqu xmm5,[rsp+176]
    mov rcx,[rsp+32]
    mov rdx,[rsp+40]
    mov r8,[rsp+48]
    mov r9,[rsp+56]
    mov r10,[rsp+64]
    mov r11,[rsp+72]
    push qword ptr [rsp+200]
    popfq
    mov rax,[rsp+232]
    lea rsp,[rbp+112]
    pop rbp
    ret
    .seh_endproc
.endm
SAVE AdapterFrameBridge
    // Stub uses a tail jump after the clone's CALL, so there is just one return
    // address. Entry RSP is native RSP-8; SAVE consumes another 248 bytes.
    mov rcx,r15
    lea rdx,[rsp+256] // original native body RSP, before CALL
    call AdapterFrame
RESTORE
SAVE AdapterBeforeBridge
    // Translate recovered native registers into Windows RCX/RDX/R8/R9 arguments.
    mov rcx,[rsp+240] // original native RBP = component
    mov rdx,rdi
    mov r8,r14
    mov r9,[rsp+40] // original RDX = selected key
    call AdapterBefore
RESTORE
SAVE AdapterAfterBridge
    mov rcx,[rsp+240]
    mov rdx,[rsp+232] // original RAX = resolver result
    call AdapterAfter
RESTORE
SAVE AdapterCommitBridge
    mov rcx,[rsp+240]
    mov rdx,rdi
    call AdapterCommit
RESTORE
SAVE AdapterRetireBridge
    mov rcx,rdi
    call AdapterRetire
RESTORE

// Only low float of progress XMM1 changes. Native clock/synthesis/gain remain.
SAVE AdapterCarrierBridge
    mov rcx,r15
    call AdapterCarrier
    test al,al
    jz 1f
    mov dword ptr [rsp+112],0
1:
RESTORE

#ifdef ADAPTER_RECOVERY_BUILD
SAVE RecoveryPostedBridge
    mov rcx,[rsp+232] // already-returned native PlayingID
    call RecoveryPosted
RESTORE

#endif
