# 由ToyC编译器生成
# RISC-V汇编代码
.text
	.global main
main:
# 函数序言
	addi sp, sp, -16
	sw ra, 12(sp)
	sw fp, 8(sp)
	addi fp, sp, 16
# 保存被调用者保存的寄存器
# x_scope-18548565 = 1
	li t0, 1
	sw t0, -12(fp)
# x_scope-18548564 = 2
	li t1, 2
	sw t1, -16(fp)
# t0 = x_scope-18548564 + 1
	lw t3, -16(fp)
	li t4, 1
	add t2, t3, t4
	sw t2, -20(fp)
# x_scope-18548564 = t0
	lw t5, -20(fp)
	sw t5, -16(fp)
# return x_scope-18548565
	lw a0, -12(fp)
	j main_epilogue
main_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	lw fp, 8(sp)
	lw ra, 12(sp)
	addi sp, sp, 16
	ret
