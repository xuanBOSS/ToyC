# 由ToyC编译器生成
# RISC-V汇编代码
.text
	.global add
add:
# 函数序言
	addi sp, sp, -32
	sw ra, 28(sp)
	sw fp, 24(sp)
	addi fp, sp, 32
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
	sw a1, -16(fp)
# t0 = a + b
	addi t1, a0, 0
	addi t2, a1, 0
	add t0, t1, t2
	sw t0, -20(fp)
# return t0
	lw a0, -20(fp)
	j add_epilogue
add_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	lw fp, 24(sp)
	lw ra, 28(sp)
	addi sp, sp, 32
	ret
	.global main
main:
# 函数序言
	addi sp, sp, -32
	sw ra, 28(sp)
	sw fp, 24(sp)
	addi fp, sp, 32
# 保存被调用者保存的寄存器
# param 3
# param 4
# t1 = call add, 2
# 保存调用者保存的寄存器
	sw ra, -12(fp)
	sw t0, -16(fp)
	sw t1, -20(fp)
	li a0, 3
	li a1, 4
	call add
# 恢复调用者保存的寄存器
	lw ra, -12(fp)
	lw t0, -16(fp)
	lw t1, -20(fp)
	addi t3, a0, 0
	sw t3, -24(fp)
# x = t1
	lw t4, -24(fp)
	sw t4, -28(fp)
# return x
	lw a0, -28(fp)
	j main_epilogue
main_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	lw fp, 24(sp)
	lw ra, 28(sp)
	addi sp, sp, 32
	ret
