# 由ToyC编译器生成
# RISC-V汇编代码
.text
	.global add
add:
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
	sw a0, -12(fp)
	sw a1, -16(fp)
# t0 = a + b
	lw t1, -12(fp)
	lw t2, -16(fp)
	add t0, t1, t2
	sw t0, -136(fp)
# return t0
	lw a0, -136(fp)
	j add_epilogue
add_epilogue:
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
# param 3
# param 4
# t1 = call add, 2
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
	li a0, 3
	li a1, 4
	call add
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
	mv t3, a0
	sw t3, -136(fp)
# x = t1
	lw t4, -136(fp)
	sw t4, -140(fp)
# return x
	lw a0, -140(fp)
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
