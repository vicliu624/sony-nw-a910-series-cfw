.syntax unified
.arch armv5te
.arm
.section .text.entry,"ax",%progbits
.global sony_home_entry
.global sony_present_entry
sony_home_entry:
    push {r0-r12,lr}
    bl sony_inline_update
    cmp r0,#0
    bne 1f
    pop {r0-r12,lr}
    push {r4,r5,lr}
    ldr pc,=0x0002a32c
1:  pop {r0-r12,lr}
    mov r0,#0
    bx lr
sony_present_entry:
    push {r0-r12,lr}
    bl sony_inline_present
    pop {r0-r12,lr}
    push {r4,lr}
    ldr pc,=0x001bca0c
.section .note.GNU-stack,"",%progbits
