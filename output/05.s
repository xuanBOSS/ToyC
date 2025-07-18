# 由ToyC编译器生成
# RISC-V汇编代码
.text
	.global add
add:
# 函数序言
	addi sp, sp, -8
	sw ra, 4(sp)
	sw fp, 0(sp)
	addi fp, sp, 8
# 保存被调用者保存的寄存器
	sw s1, -72(fp)
	sw s2, -76(fp)
	sw s3, -80(fp)
	sw s4, -84(fp)
	sw s5, -88(fp)
	sw s6, -92(fp)
	sw s7, -96(fp)
	sw s8, -100(fp)
	sw s9, -104(fp)
	sw s10, -108(fp)
	sw s11, -112(fp)
	sw a0, -4(fp)
	sw a1, -8(fp)
# t0 = a + b
	lw t1, -4(fp)
	lw t2, -8(fp)
	add t0, t1, t2
	sw t0, -12(fp)
# return t0
	lw a0, -12(fp)
	j add_epilogue
add_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	lw s1, -72(fp)
	lw s2, -76(fp)
	lw s3, -80(fp)
	lw s4, -84(fp)
	lw s5, -88(fp)
	lw s6, -92(fp)
	lw s7, -96(fp)
	lw s8, -100(fp)
	lw s9, -104(fp)
	lw s10, -108(fp)
	lw s11, -112(fp)
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
# 保存被调用者保存的寄存器
	sw s1, -72(fp)
	sw s2, -76(fp)
	sw s3, -80(fp)
	sw s4, -84(fp)
	sw s5, -88(fp)
	sw s6, -92(fp)
	sw s7, -96(fp)
	sw s8, -100(fp)
	sw s9, -104(fp)
	sw s10, -108(fp)
	sw s11, -112(fp)
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
	sw t3, -4(fp)
# x = t1
	lw t4, -4(fp)
	sw t4, -8(fp)
# return x
	lw a0, -8(fp)
	j main_epilogue
main_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	lw s1, -72(fp)
	lw s2, -76(fp)
	lw s3, -80(fp)
	lw s4, -84(fp)
	lw s5, -88(fp)
	lw s6, -92(fp)
	lw s7, -96(fp)
	lw s8, -100(fp)
	lw s9, -104(fp)
	lw s10, -108(fp)
	lw s11, -112(fp)
	lw fp, 0(sp)
	lw ra, 4(sp)
	addi sp, sp, 8
	ret
