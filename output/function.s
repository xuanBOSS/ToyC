# 由ToyC编译器生成
# RISC-V汇编代码
.text
	.global factorial
factorial:
# 函数序言
	addi sp, sp, -64
	sw ra, 60(sp)
	sw fp, 56(sp)
	addi fp, sp, 64
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
# t0 = n <= 1
	addi t1, a0, 0
	li t2, 1
	slt t0, t2, t1
	xori t0, t0, 1
	sw t0, -16(fp)
# t1 = !t0
	lw t4, -16(fp)
	seqz t3, t4
	sw t3, -20(fp)
# if t1 goto L0
	lw t5, -20(fp)
	bnez t5, L0
# return 1
	li a0, 1
	j factorial_epilogue
L0:
# t2 = n - 1
	addi t0, a0, 0
	li t1, 1
	sub t6, t0, t1
	sw t6, -24(fp)
# param t2
# t3 = call factorial, 1
# 保存调用者保存的寄存器
	sw ra, -28(fp)
	sw t0, -32(fp)
	sw t1, -36(fp)
	sw t2, -40(fp)
	sw t3, -44(fp)
	sw t4, -48(fp)
	sw t5, -52(fp)
	lw a0, -24(fp)
	call factorial
# 恢复调用者保存的寄存器
	lw ra, -28(fp)
	lw t0, -32(fp)
	lw t1, -36(fp)
	lw t2, -40(fp)
	lw t3, -44(fp)
	lw t4, -48(fp)
	lw t5, -52(fp)
	addi t2, a0, 0
	sw t2, -56(fp)
# t4 = n * t3
	addi t4, a0, 0
	lw t5, -56(fp)
	mul t3, t4, t5
	sw t3, -60(fp)
# return t4
	lw a0, -60(fp)
	j factorial_epilogue
factorial_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	lw fp, 56(sp)
	lw ra, 60(sp)
	addi sp, sp, 64
	ret
	.global main
main:
# 函数序言
	addi sp, sp, -64
	sw ra, 60(sp)
	sw fp, 56(sp)
	addi fp, sp, 64
# 保存被调用者保存的寄存器
# param 5
# t5 = call factorial, 1
# 保存调用者保存的寄存器
	sw ra, -12(fp)
	sw t0, -16(fp)
	sw t1, -20(fp)
	sw t2, -24(fp)
	sw t3, -28(fp)
	sw t4, -32(fp)
	sw t5, -36(fp)
	li a0, 5
	call factorial
# 恢复调用者保存的寄存器
	lw ra, -12(fp)
	lw t0, -16(fp)
	lw t1, -20(fp)
	lw t2, -24(fp)
	lw t3, -28(fp)
	lw t4, -32(fp)
	lw t5, -36(fp)
	addi t6, a0, 0
	sw t6, -40(fp)
# result = t5
	lw t0, -40(fp)
	sw t0, -44(fp)
# return result
	lw a0, -44(fp)
	j main_epilogue
main_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	lw fp, 56(sp)
	lw ra, 60(sp)
	addi sp, sp, 64
	ret
