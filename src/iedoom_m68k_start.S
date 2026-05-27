#ifdef __MINT__
    .text
#else
    .section .text.start,"ax"
#endif
    .globl _start
    .globl iedoom_m68k_halt
#ifdef __MINT__
    .extern _iedoom_entry
#else
    .extern iedoom_entry
#endif
    .extern __bss_start
    .extern __bss_end
    .extern __stack_top

_start:
    move.l  #__stack_top,%sp
    lea     __bss_start,%a0
    lea     __bss_end,%a1
1:
    cmp.l   %a1,%a0
    bcc.s   2f
    clr.b   (%a0)+
    bra.s   1b
2:
#ifdef __MINT__
    jsr     _iedoom_entry
#else
    jsr     iedoom_entry
#endif

iedoom_m68k_halt:
3:
    stop    #0x2700
    bra.s   3b
