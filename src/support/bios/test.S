
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
