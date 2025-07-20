# 由ToyC编译器生成
# RISC-V汇编代码
.text
	.global main
main:
# 函数序言
	addi sp, sp, -128
	sw ra, 124(sp)
	sw fp, 120(sp)
	addi fp, sp, 128
# 保存被调用者保存的寄存器
	sw s1, -12(fp)
	sw s2, -16(fp)
	sw s3, -20(fp)
	sw s4, -24(fp)
	sw s5, -28(fp)
	sw s6, -32(fp)
	sw s7, -36(fp)
	sw s8, -40(fp)
	sw s9, -44(fp)
	sw s10, -48(fp)
	sw s11, -52(fp)
# x = 5
	li t0, 5
	sw t0, -136(fp)
# t0 = x > 0
	lw t2, -136(fp)
	li t3, 0
	slt t1, t3, t2
	sw t1, -140(fp)
# t1 = !t0
	lw t5, -140(fp)
	seqz t4, t5
	sw t4, -144(fp)
# if t1 goto L0
	lw t6, -144(fp)
	bnez t6, L0
# return 1
	li a0, 1
	j main_epilogue
L0:
# return 0
	li a0, 0
	j main_epilogue
main_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	lw s1, -12(fp)
	lw s2, -16(fp)
	lw s3, -20(fp)
	lw s4, -24(fp)
	lw s5, -28(fp)
	lw s6, -32(fp)
	lw s7, -36(fp)
	lw s8, -40(fp)
	lw s9, -44(fp)
	lw s10, -48(fp)
	lw s11, -52(fp)
	lw fp, 120(sp)
	lw ra, 124(sp)
	addi sp, sp, 128
	ret
