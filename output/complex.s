	# 由ToyC编译器生成
	# RISC-V汇编代码
	.text
	.global max
max:
	# 函数序言
	addi sp, sp, -8
	sw ra, 4(sp)
	sw fp, 0(sp)
	addi fp, sp, 8
	# t0 = a > b
	lw t1, -4(fp)
	lw t2, -8(fp)
	slt t0, t2, t1
	sw t0, -12(fp)
	# t1 = !t0
	lw t4, -12(fp)
	seqz t3, t4
	sw t3, -16(fp)
	# if t1 goto L0
	lw t5, -16(fp)
	bnez t5, L0
	# return a
	lw a0, -4(fp)
	j max_epilogue
L0:
	# return b
	lw a0, -8(fp)
	j max_epilogue
max_epilogue:
	# 函数后记
	lw fp, 0(sp)
	lw ra, 4(sp)
	addi sp, sp, 8
	ret
	.global abs
abs:
	# 函数序言
	addi sp, sp, -8
	sw ra, 4(sp)
	sw fp, 0(sp)
	addi fp, sp, 8
	# t2 = x < 0
	lw t0, -4(fp)
	li t1, 0
	slt t6, t0, t1
	sw t6, -8(fp)
	# t3 = !t2
	lw t3, -8(fp)
	seqz t2, t3
	sw t2, -12(fp)
	# if t3 goto L1
	lw t4, -12(fp)
	bnez t4, L1
	# t4 = -x
	lw t6, -4(fp)
	neg t5, t6
	sw t5, -16(fp)
	# return t4
	lw a0, -16(fp)
	j abs_epilogue
L1:
	# return x
	lw a0, -4(fp)
	j abs_epilogue
abs_epilogue:
	# 函数后记
	lw fp, 0(sp)
	lw ra, 4(sp)
	addi sp, sp, 8
	ret
	.global main
main:
	# 函数序言
	addi sp, sp, -8
	sw ra, 4(sp)
	sw fp, 0(sp)
	addi fp, sp, 8
	# a = 10
	li t0, 10
	sw t0, -4(fp)
	# t5 = -20
	li t2, 20
	neg t1, t2
	sw t1, -8(fp)
	# b = t5
	lw t3, -8(fp)
	sw t3, -12(fp)
	# c = 15
	li t4, 15
	sw t4, -16(fp)
	# t6 = a - b
	lw t6, -4(fp)
	lw t0, -12(fp)
	sub t5, t6, t0
	sw t5, -20(fp)
	# param t6
	# t7 = call abs, 1
	# 保存调用者保存的寄存器
	sw t0, -12(fp)
	sw t1, -16(fp)
	sw t2, -20(fp)
	sw t3, -24(fp)
	sw t4, -28(fp)
	sw t5, -32(fp)
	sw t6, -36(fp)
	sw a0, -36(fp)
	sw a1, -40(fp)
	sw a2, -44(fp)
	sw a3, -48(fp)
	sw a4, -52(fp)
	sw a5, -56(fp)
	sw a6, -60(fp)
	sw a7, -64(fp)
	lw a0, -20(fp)
	call abs
	# 恢复调用者保存的寄存器
	lw a0, -36(fp)
	lw a1, -40(fp)
	lw a2, -44(fp)
	lw a3, -48(fp)
	lw a4, -52(fp)
	lw a5, -56(fp)
	lw a6, -60(fp)
	lw a7, -64(fp)
	lw t0, -8(fp)
	lw t1, -12(fp)
	lw t2, -16(fp)
	lw t3, -20(fp)
	lw t4, -24(fp)
	lw t5, -28(fp)
	lw t6, -32(fp)
	mv t1, a0
	sw t1, -24(fp)
	# param c
	# param t7
	# t8 = call max, 2
	# 保存调用者保存的寄存器
	sw t0, -12(fp)
	sw t1, -16(fp)
	sw t2, -20(fp)
	sw t3, -24(fp)
	sw t4, -28(fp)
	sw t5, -32(fp)
	sw t6, -36(fp)
	sw a0, -36(fp)
	sw a1, -40(fp)
	sw a2, -44(fp)
	sw a3, -48(fp)
	sw a4, -52(fp)
	sw a5, -56(fp)
	sw a6, -60(fp)
	sw a7, -64(fp)
	lw a0, -16(fp)
	lw a1, -24(fp)
	call max
	# 恢复调用者保存的寄存器
	lw a0, -36(fp)
	lw a1, -40(fp)
	lw a2, -44(fp)
	lw a3, -48(fp)
	lw a4, -52(fp)
	lw a5, -56(fp)
	lw a6, -60(fp)
	lw a7, -64(fp)
	lw t0, -8(fp)
	lw t1, -12(fp)
	lw t2, -16(fp)
	lw t3, -20(fp)
	lw t4, -24(fp)
	lw t5, -28(fp)
	lw t6, -32(fp)
	mv t2, a0
	sw t2, -28(fp)
	# result = t8
	lw t3, -28(fp)
	sw t3, -32(fp)
	# i = 0
	li t4, 0
	sw t4, -36(fp)
	# goto L3
	j L3
L2:
	# t9 = result + i
	lw t6, -32(fp)
	lw t0, -36(fp)
	add t5, t6, t0
	sw t5, -40(fp)
	# result = t9
	lw t1, -40(fp)
	sw t1, -32(fp)
	# t10 = result > 50
	lw t3, -32(fp)
	li t4, 50
	slt t2, t4, t3
	sw t2, -44(fp)
	# t11 = !t10
	lw t6, -44(fp)
	seqz t5, t6
	sw t5, -48(fp)
	# if t11 goto L5
	lw t0, -48(fp)
	bnez t0, L5
	# goto L4
	j L4
L5:
	# t12 = i + 1
	lw t2, -36(fp)
	li t3, 1
	add t1, t2, t3
	sw t1, -52(fp)
	# i = t12
	lw t4, -52(fp)
	sw t4, -36(fp)
L3:
	# t13 = i < 5
	lw t6, -36(fp)
	li t0, 5
	slt t5, t6, t0
	sw t5, -56(fp)
	# if t13 goto L2
	lw t1, -56(fp)
	bnez t1, L2
L4:
	# return result
	lw a0, -32(fp)
	j main_epilogue
main_epilogue:
	# 函数后记
	lw fp, 0(sp)
	lw ra, 4(sp)
	addi sp, sp, 8
	ret
