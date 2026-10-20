/* Original A918 2.6.10 kernel: ARM SWI immediate carries the syscall number.
 * The C interface remains AAPCS; this is NOT a wholesale OABI C rebuild.
 * r0=nr,r1=a,r2=b,r3=c, stacked d/e/f. Preserve AAPCS callee-save registers.
 */
.syntax unified
.arm
.text
.align 2
.global a918_oabi_call
.type a918_oabi_call,%function
a918_oabi_call:
 cmp r0,#288
 mvnhs r0,#37
 bxhs lr
 push {r4,r5,r6,lr}
 mov ip,r0
 mov r0,r1
 mov r1,r2
 mov r2,r3
 ldr r3,[sp,#16]
 ldr r4,[sp,#20]
 ldr r5,[sp,#24]
 adr r6,.Lsyscalls
 add r6,r6,ip,lsl #3
 mov lr,pc
 bx r6
 pop {r4,r5,r6,pc}
.align 2
.Lsyscalls:
.set call_number,0
.rept 288
 .word 0xef900000 + call_number
 bx lr
 .set call_number,call_number+1
.endr
.size a918_oabi_call,.-a918_oabi_call
.section .note.GNU-stack,"",%progbits
