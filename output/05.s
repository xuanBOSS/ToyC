# 由ToyC编译器生成
# RISC-V汇编代码
.text
	.global add
add:
# 函数序言
	addi sp, sp, -48
	sw ra, 44(sp)
	sw fp, 40(sp)
	addi fp, sp, 48
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
	sw a1, -16(fp)
# t0 = a + b
	lw t1, -12(fp)
	lw t2, -16(fp)
	add t0, t1, t2
	sw t0, -20(fp)
# return t0
	lw a0, -20(fp)
	j add_epilogue
add_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	lw fp, 40(sp)
	lw ra, 44(sp)
	addi sp, sp, 48
	ret
	.global main
main:
# 函数序言
	addi sp, sp, -48
	sw ra, 44(sp)
	sw fp, 40(sp)
	addi fp, sp, 48
# 保存被调用者保存的寄存器
# param 3
# param 4
# t1 = call add, 2
# 保存调用者保存的寄存器
	sw t0, -12(fp)
	sw t1, -16(fp)
	sw t2, -20(fp)
	li a0, 3
	li a1, 4
	call add
# 恢复调用者保存的寄存器
	lw t0, -12(fp)
	lw t1, -16(fp)
	lw t2, -20(fp)
	addi t3, a0, 0
	sw t3, -24(fp)
# x_scope95283371 = t1
	lw t4, -24(fp)
	sw t4, -28(fp)
# return x_scope95283371
	lw a0, -28(fp)
	j main_epilogue
main_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	lw fp, 40(sp)
	lw ra, 44(sp)
	addi sp, sp, 48
	ret
