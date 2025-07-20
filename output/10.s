# 由ToyC编译器生成
# RISC-V汇编代码
.text
	.global print
print:
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
# x = 1
	li t0, 1
	sw t0, -136(fp)
# t0 = x + 1
	lw t2, -136(fp)
	li t3, 1
	add t1, t2, t3
	sw t1, -140(fp)
# x = t0
	lw t4, -140(fp)
	sw t4, -136(fp)
# return
	j print_epilogue
print_epilogue:
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
# t1 = call print, 0
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
