# 由ToyC编译器生成
# RISC-V汇编代码
.text
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
# return 0
	li a0, 0
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
