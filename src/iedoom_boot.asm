; ELF reset trampoline for linked Intuition Engine x86 flat binaries.

bits 32

%include "ie86.inc"

section .reset progbits alloc exec nowrite align=1
global _start
extern iedoom_entry
extern __bss_start
extern __bss_end

_start:
    mov     esp, STACK_TOP
    cld
    xor     eax, eax
    mov     edi, __bss_start
    mov     ecx, __bss_end
    sub     ecx, edi
    rep     stosb
    jmp     iedoom_entry

section .note.GNU-stack noalloc noexec nowrite progbits
