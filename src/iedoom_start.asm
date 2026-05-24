; Reset trampoline for Intuition Engine x86 flat binaries.

bits 32
org 0

%include "ie86.inc"

iedoom_reset:
    mov     esp, STACK_TOP
    jmp     PROGRAM_START

times PROGRAM_START - ($ - $$) db 0

iedoom_entry_placeholder:
    hlt
