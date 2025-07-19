# 由ToyC编译器生成
# RISC-V汇编代码
.text
	.global main
main:
# 函数序言
	addi sp, sp, -112
	sw ra, 108(sp)
	sw fp, 104(sp)
	addi fp, sp, 112
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
# x = 0
	li t0, 0
	sw t0, -4(fp)
# goto L1
	j L1
L0:
# t0 = x == 5
	lw t2, -4(fp)
	li t3, 5
	xor t1, t2, t3
	seqz t1, t1
	sw t1, -8(fp)
# t1 = !t0
	lw t5, -8(fp)
	seqz t4, t5
	sw t4, -12(fp)
# if t1 goto L3
	lw t6, -12(fp)
	bnez t6, L3
# goto L2
	j L2
L3:
# t2 = x + 1
	lw t1, -4(fp)
	li t2, 1
	add t0, t1, t2
	sw t0, -16(fp)
# x = t2
	lw t3, -16(fp)
	sw t3, -4(fp)
L1:
# t3 = x < 10
	lw t5, -4(fp)
	li t6, 10
	slt t4, t5, t6
	sw t4, -20(fp)
# if t3 goto L0
	lw t0, -20(fp)
	bnez t0, L0
L2:
# return x
	lw a0, -4(fp)
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
	lw fp, 104(sp)
	lw ra, 108(sp)
	addi sp, sp, 112
	ret
