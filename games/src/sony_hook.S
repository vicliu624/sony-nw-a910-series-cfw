.syntax unified
.arch armv5te
.arm
.section .text.entry,"ax",%progbits
.global sony_home_entry
.type sony_home_entry,%function
sony_home_entry:
    /* Stock Home update entry: no original instruction has run yet. */
    push {r0-r12,lr}
    bl sony_home_hook
    cmp r0,#0
    bne 1f
    pop {r0-r12,lr}
    push {r4,r5,lr}              /* displaced instruction at 0x2a328 */
    ldr pc,=0x0002a32c
1:  pop {r0-r12,lr}
    mov r0,#0
    bx lr
.size sony_home_entry,.-sony_home_entry
.section .note.GNU-stack,"",%progbits
