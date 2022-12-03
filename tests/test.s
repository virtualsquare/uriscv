	.file	"test.c"
	.option nopic
	.attribute arch, "rv32i2p0_m2p0_a2p0_f2p0_d2p0_c2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.section	.sdata,"aw"
	.align	2
	.type	term0_reg, @object
	.size	term0_reg, 4
term0_reg:
	.word	268436052
	.section	.rodata
	.align	2
.LC0:
	.string	"hello, world\n"
	.text
	.align	1
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-32
	sw	ra,28(sp)
	sw	s0,24(sp)
	addi	s0,sp,32
	sw	a0,-20(s0)
	sw	a1,-24(s0)
	lui	a5,%hi(.LC0)
	addi	a0,a5,%lo(.LC0)
	call	term_puts
	call	WAIT
	li	a5,268435456
	addi	a5,a5,1300
	li	a4,255
	sw	a4,0(a5)
.L2:
	j	.L2
	.size	main, .-main
	.align	1
	.type	term_puts, @function
term_puts:
	addi	sp,sp,-32
	sw	ra,28(sp)
	sw	s0,24(sp)
	addi	s0,sp,32
	sw	a0,-20(s0)
	j	.L4
.L6:
	lw	a5,-20(s0)
	addi	a4,a5,1
	sw	a4,-20(s0)
	lbu	a5,0(a5)
	mv	a0,a5
	call	term_putchar
	mv	a5,a0
	bne	a5,zero,.L7
.L4:
	lw	a5,-20(s0)
	lbu	a5,0(a5)
	bne	a5,zero,.L6
	j	.L3
.L7:
	nop
.L3:
	lw	ra,28(sp)
	lw	s0,24(sp)
	addi	sp,sp,32
	jr	ra
	.size	term_puts, .-term_puts
	.align	1
	.type	term_putchar, @function
term_putchar:
	addi	sp,sp,-48
	sw	ra,44(sp)
	sw	s0,40(sp)
	addi	s0,sp,48
	mv	a5,a0
	sb	a5,-33(s0)
	lui	a5,%hi(term0_reg)
	lw	a5,%lo(term0_reg)(a5)
	mv	a0,a5
	call	tx_status
	sw	a0,-20(s0)
	lw	a4,-20(s0)
	li	a5,1
	beq	a4,a5,.L9
	lw	a4,-20(s0)
	li	a5,5
	beq	a4,a5,.L9
	li	a5,-1
	j	.L10
.L9:
	lbu	a5,-33(s0)
	slli	a5,a5,8
	ori	a4,a5,2
	lui	a5,%hi(term0_reg)
	lw	a5,%lo(term0_reg)(a5)
	sw	a4,12(a5)
	nop
.L11:
	lui	a5,%hi(term0_reg)
	lw	a5,%lo(term0_reg)(a5)
	mv	a0,a5
	call	tx_status
	sw	a0,-20(s0)
	lw	a4,-20(s0)
	li	a5,3
	beq	a4,a5,.L11
	lui	a5,%hi(term0_reg)
	lw	a5,%lo(term0_reg)(a5)
	li	a4,1
	sw	a4,12(a5)
	lw	a4,-20(s0)
	li	a5,5
	beq	a4,a5,.L12
	li	a5,-1
	j	.L10
.L12:
	li	a5,0
.L10:
	mv	a0,a5
	lw	ra,44(sp)
	lw	s0,40(sp)
	addi	sp,sp,48
	jr	ra
	.size	term_putchar, .-term_putchar
	.align	1
	.type	tx_status, @function
tx_status:
	addi	sp,sp,-32
	sw	s0,28(sp)
	addi	s0,sp,32
	sw	a0,-20(s0)
	lw	a5,-20(s0)
	lw	a5,8(a5)
	andi	a5,a5,255
	mv	a0,a5
	lw	s0,28(sp)
	addi	sp,sp,32
	jr	ra
	.size	tx_status, .-tx_status
	.ident	"GCC: (g2ee5e430018) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
