# 由ToyC编译器生成
# RISC-V汇编代码
.text
	.global sum8
sum8:
# 函数序言
	li t0, -2464
	add sp, sp, t0
	li t0, 2460
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2456
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2464
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
	sw a1, -16(fp)
	sw a2, -20(fp)
	sw a3, -24(fp)
	sw a4, -28(fp)
	sw a5, -32(fp)
	sw a6, -36(fp)
	sw a7, -40(fp)
# t0 = a1 + a2
	lw t1, -12(fp)
	lw t2, -16(fp)
	add t0, t1, t2
	sw t0, -44(fp)
# t1 = t0 + a3
	lw t4, -44(fp)
	lw t5, -20(fp)
	add t3, t4, t5
	sw t3, -48(fp)
# t2 = t1 + a4
	lw t0, -48(fp)
	lw t1, -24(fp)
	add t6, t0, t1
	sw t6, -52(fp)
# t3 = t2 + a5
	lw t3, -52(fp)
	lw t4, -28(fp)
	add t2, t3, t4
	sw t2, -56(fp)
# t4 = t3 + a6
	lw t6, -56(fp)
	lw t0, -32(fp)
	add t5, t6, t0
	sw t5, -60(fp)
# t5 = t4 + a7
	lw t2, -60(fp)
	lw t3, -36(fp)
	add t1, t2, t3
	sw t1, -64(fp)
# t6 = t5 + a8
	lw t5, -64(fp)
	lw t6, -40(fp)
	add t4, t5, t6
	sw t4, -68(fp)
# return t6
	lw a0, -68(fp)
	j sum8_epilogue
sum8_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2456
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2460
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2464
	add sp, sp, t0
	ret
	.global sum16
sum16:
# 函数序言
	li t0, -2464
	add sp, sp, t0
	li t0, 2460
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2456
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2464
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
	sw a1, -16(fp)
	sw a2, -20(fp)
	sw a3, -24(fp)
	sw a4, -28(fp)
	sw a5, -32(fp)
	sw a6, -36(fp)
	sw a7, -40(fp)
	lw t0, 0(fp)
	sw t0, -12(fp)
	lw t1, 4(fp)
	sw t1, -16(fp)
	lw t2, 8(fp)
	sw t2, -20(fp)
	lw t3, 12(fp)
	sw t3, -24(fp)
	lw t4, 16(fp)
	sw t4, -28(fp)
	lw t5, 20(fp)
	sw t5, -32(fp)
	lw t6, 24(fp)
	sw t6, -36(fp)
	lw t0, 28(fp)
	sw t0, -40(fp)
# t7 = a1 + a2
	lw t2, -12(fp)
	lw t3, -16(fp)
	add t1, t2, t3
	sw t1, -44(fp)
# t8 = t7 + a3
	lw t5, -44(fp)
	lw t6, -20(fp)
	add t4, t5, t6
	sw t4, -48(fp)
# t9 = t8 + a4
	lw t1, -48(fp)
	lw t2, -24(fp)
	add t0, t1, t2
	sw t0, -52(fp)
# t10 = t9 + a5
	lw t4, -52(fp)
	lw t5, -28(fp)
	add t3, t4, t5
	sw t3, -56(fp)
# t11 = t10 + a6
	lw t0, -56(fp)
	lw t1, -32(fp)
	add t6, t0, t1
	sw t6, -60(fp)
# t12 = t11 + a7
	lw t3, -60(fp)
	lw t4, -36(fp)
	add t2, t3, t4
	sw t2, -64(fp)
# t13 = t12 + a8
	lw t6, -64(fp)
	lw t0, -40(fp)
	add t5, t6, t0
	sw t5, -68(fp)
# t14 = t13 + a9
	lw t2, -68(fp)
	lw t3, -12(fp)
	add t1, t2, t3
	sw t1, -72(fp)
# t15 = t14 + a10
	lw t5, -72(fp)
	lw t6, -16(fp)
	add t4, t5, t6
	sw t4, -76(fp)
# t16 = t15 + a11
	lw t1, -76(fp)
	lw t2, -20(fp)
	add t0, t1, t2
	sw t0, -80(fp)
# t17 = t16 + a12
	lw t4, -80(fp)
	lw t5, -24(fp)
	add t3, t4, t5
	sw t3, -84(fp)
# t18 = t17 + a13
	lw t0, -84(fp)
	lw t1, -28(fp)
	add t6, t0, t1
	sw t6, -88(fp)
# t19 = t18 + a14
	lw t3, -88(fp)
	lw t4, -32(fp)
	add t2, t3, t4
	sw t2, -92(fp)
# t20 = t19 + a15
	lw t6, -92(fp)
	lw t0, -36(fp)
	add t5, t6, t0
	sw t5, -96(fp)
# t21 = t20 + a16
	lw t2, -96(fp)
	lw t3, -40(fp)
	add t1, t2, t3
	sw t1, -100(fp)
# return t21
	lw a0, -100(fp)
	j sum16_epilogue
sum16_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2456
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2460
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2464
	add sp, sp, t0
	ret
	.global sum32
sum32:
# 函数序言
	li t0, -2464
	add sp, sp, t0
	li t0, 2460
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2456
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2464
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
	sw a1, -16(fp)
	sw a2, -20(fp)
	sw a3, -24(fp)
	sw a4, -28(fp)
	sw a5, -32(fp)
	sw a6, -36(fp)
	sw a7, -40(fp)
	lw t4, 0(fp)
	sw t4, -12(fp)
	lw t5, 4(fp)
	sw t5, -16(fp)
	lw t6, 8(fp)
	sw t6, -20(fp)
	lw t0, 12(fp)
	sw t0, -24(fp)
	lw t1, 16(fp)
	sw t1, -28(fp)
	lw t2, 20(fp)
	sw t2, -32(fp)
	lw t3, 24(fp)
	sw t3, -36(fp)
	lw t4, 28(fp)
	sw t4, -40(fp)
	lw t5, 32(fp)
	sw t5, -44(fp)
	lw t6, 36(fp)
	sw t6, -48(fp)
	lw t0, 40(fp)
	sw t0, -52(fp)
	lw t1, 44(fp)
	sw t1, -56(fp)
	lw t2, 48(fp)
	sw t2, -60(fp)
	lw t3, 52(fp)
	sw t3, -64(fp)
	lw t4, 56(fp)
	sw t4, -68(fp)
	lw t5, 60(fp)
	sw t5, -72(fp)
	lw t6, 64(fp)
	sw t6, -76(fp)
	lw t0, 68(fp)
	sw t0, -80(fp)
	lw t1, 72(fp)
	sw t1, -84(fp)
	lw t2, 76(fp)
	sw t2, -88(fp)
	lw t3, 80(fp)
	sw t3, -92(fp)
	lw t4, 84(fp)
	sw t4, -96(fp)
	lw t5, 88(fp)
	sw t5, -100(fp)
	lw t6, 92(fp)
	sw t6, -104(fp)
# t22 = a1 + a2
	lw t1, -12(fp)
	lw t2, -16(fp)
	add t0, t1, t2
	sw t0, -44(fp)
# t23 = t22 + a3
	lw t4, -44(fp)
	lw t5, -20(fp)
	add t3, t4, t5
	sw t3, -48(fp)
# t24 = t23 + a4
	lw t0, -48(fp)
	lw t1, -24(fp)
	add t6, t0, t1
	sw t6, -52(fp)
# t25 = t24 + a5
	lw t3, -52(fp)
	lw t4, -28(fp)
	add t2, t3, t4
	sw t2, -56(fp)
# t26 = t25 + a6
	lw t6, -56(fp)
	lw t0, -32(fp)
	add t5, t6, t0
	sw t5, -60(fp)
# t27 = t26 + a7
	lw t2, -60(fp)
	lw t3, -36(fp)
	add t1, t2, t3
	sw t1, -64(fp)
# t28 = t27 + a8
	lw t5, -64(fp)
	lw t6, -40(fp)
	add t4, t5, t6
	sw t4, -68(fp)
# sum1_scope-2115024725 = t28
	lw t0, -68(fp)
	sw t0, -72(fp)
# t29 = a9 + a10
	lw t2, -12(fp)
	lw t3, -16(fp)
	add t1, t2, t3
	sw t1, -76(fp)
# t30 = t29 + a11
	lw t5, -76(fp)
	lw t6, -20(fp)
	add t4, t5, t6
	sw t4, -80(fp)
# t31 = t30 + a12
	lw t1, -80(fp)
	lw t2, -24(fp)
	add t0, t1, t2
	sw t0, -84(fp)
# t32 = t31 + a13
	lw t4, -84(fp)
	lw t5, -28(fp)
	add t3, t4, t5
	sw t3, -88(fp)
# t33 = t32 + a14
	lw t0, -88(fp)
	lw t1, -32(fp)
	add t6, t0, t1
	sw t6, -92(fp)
# t34 = t33 + a15
	lw t3, -92(fp)
	lw t4, -36(fp)
	add t2, t3, t4
	sw t2, -96(fp)
# t35 = t34 + a16
	lw t6, -96(fp)
	lw t0, -40(fp)
	add t5, t6, t0
	sw t5, -100(fp)
# sum2_scope-2115024725 = t35
	lw t1, -100(fp)
	sw t1, -104(fp)
# t36 = a17 + a18
	lw t3, -44(fp)
	lw t4, -48(fp)
	add t2, t3, t4
	sw t2, -108(fp)
# t37 = t36 + a19
	lw t6, -108(fp)
	lw t0, -52(fp)
	add t5, t6, t0
	sw t5, -112(fp)
# t38 = t37 + a20
	lw t2, -112(fp)
	lw t3, -56(fp)
	add t1, t2, t3
	sw t1, -116(fp)
# t39 = t38 + a21
	lw t5, -116(fp)
	lw t6, -60(fp)
	add t4, t5, t6
	sw t4, -120(fp)
# t40 = t39 + a22
	lw t1, -120(fp)
	lw t2, -64(fp)
	add t0, t1, t2
	sw t0, -124(fp)
# t41 = t40 + a23
	lw t4, -124(fp)
	lw t5, -68(fp)
	add t3, t4, t5
	sw t3, -128(fp)
# t42 = t41 + a24
	lw t0, -128(fp)
	lw t1, -72(fp)
	add t6, t0, t1
	sw t6, -132(fp)
# sum3_scope-2115024725 = t42
	lw t2, -132(fp)
	sw t2, -136(fp)
# t43 = a25 + a26
	lw t4, -76(fp)
	lw t5, -80(fp)
	add t3, t4, t5
	sw t3, -140(fp)
# t44 = t43 + a27
	lw t0, -140(fp)
	lw t1, -84(fp)
	add t6, t0, t1
	sw t6, -144(fp)
# t45 = t44 + a28
	lw t3, -144(fp)
	lw t4, -88(fp)
	add t2, t3, t4
	sw t2, -148(fp)
# t46 = t45 + a29
	lw t6, -148(fp)
	lw t0, -92(fp)
	add t5, t6, t0
	sw t5, -152(fp)
# t47 = t46 + a30
	lw t2, -152(fp)
	lw t3, -96(fp)
	add t1, t2, t3
	sw t1, -156(fp)
# t48 = t47 + a31
	lw t5, -156(fp)
	lw t6, -100(fp)
	add t4, t5, t6
	sw t4, -160(fp)
# t49 = t48 + a32
	lw t1, -160(fp)
	lw t2, -104(fp)
	add t0, t1, t2
	sw t0, -164(fp)
# sum4_scope-2115024725 = t49
	lw t3, -164(fp)
	sw t3, -168(fp)
# t50 = sum1_scope-2115024725 + sum2_scope-2115024725
	lw t5, -72(fp)
	lw t6, -104(fp)
	add t4, t5, t6
	sw t4, -172(fp)
# t51 = t50 + sum3_scope-2115024725
	lw t1, -172(fp)
	lw t2, -136(fp)
	add t0, t1, t2
	sw t0, -176(fp)
# t52 = t51 + sum4_scope-2115024725
	lw t4, -176(fp)
	lw t5, -168(fp)
	add t3, t4, t5
	sw t3, -180(fp)
# return t52
	lw a0, -180(fp)
	j sum32_epilogue
sum32_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2456
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2460
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2464
	add sp, sp, t0
	ret
	.global sum64
sum64:
# 函数序言
	li t0, -2464
	add sp, sp, t0
	li t0, 2460
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2456
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2464
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
	sw a1, -16(fp)
	sw a2, -20(fp)
	sw a3, -24(fp)
	sw a4, -28(fp)
	sw a5, -32(fp)
	sw a6, -36(fp)
	sw a7, -40(fp)
	lw t6, 0(fp)
	sw t6, -12(fp)
	lw t0, 4(fp)
	sw t0, -16(fp)
	lw t1, 8(fp)
	sw t1, -20(fp)
	lw t2, 12(fp)
	sw t2, -24(fp)
	lw t3, 16(fp)
	sw t3, -28(fp)
	lw t4, 20(fp)
	sw t4, -32(fp)
	lw t5, 24(fp)
	sw t5, -36(fp)
	lw t6, 28(fp)
	sw t6, -40(fp)
	lw t0, 32(fp)
	sw t0, -44(fp)
	lw t1, 36(fp)
	sw t1, -48(fp)
	lw t2, 40(fp)
	sw t2, -52(fp)
	lw t3, 44(fp)
	sw t3, -56(fp)
	lw t4, 48(fp)
	sw t4, -60(fp)
	lw t5, 52(fp)
	sw t5, -64(fp)
	lw t6, 56(fp)
	sw t6, -68(fp)
	lw t0, 60(fp)
	sw t0, -72(fp)
	lw t1, 64(fp)
	sw t1, -76(fp)
	lw t2, 68(fp)
	sw t2, -80(fp)
	lw t3, 72(fp)
	sw t3, -84(fp)
	lw t4, 76(fp)
	sw t4, -88(fp)
	lw t5, 80(fp)
	sw t5, -92(fp)
	lw t6, 84(fp)
	sw t6, -96(fp)
	lw t0, 88(fp)
	sw t0, -100(fp)
	lw t1, 92(fp)
	sw t1, -104(fp)
	lw t2, 96(fp)
	sw t2, -108(fp)
	lw t3, 100(fp)
	sw t3, -112(fp)
	lw t4, 104(fp)
	sw t4, -116(fp)
	lw t5, 108(fp)
	sw t5, -120(fp)
	lw t6, 112(fp)
	sw t6, -124(fp)
	lw t0, 116(fp)
	sw t0, -128(fp)
	lw t1, 120(fp)
	sw t1, -132(fp)
	lw t2, 124(fp)
	sw t2, -136(fp)
	lw t3, 128(fp)
	sw t3, -140(fp)
	lw t4, 132(fp)
	sw t4, -144(fp)
	lw t5, 136(fp)
	sw t5, -148(fp)
	lw t6, 140(fp)
	sw t6, -152(fp)
	lw t0, 144(fp)
	sw t0, -156(fp)
	lw t1, 148(fp)
	sw t1, -160(fp)
	lw t2, 152(fp)
	sw t2, -164(fp)
	lw t3, 156(fp)
	sw t3, -168(fp)
	lw t4, 160(fp)
	sw t4, -172(fp)
	lw t5, 164(fp)
	sw t5, -176(fp)
	lw t6, 168(fp)
	sw t6, -180(fp)
	lw t0, 172(fp)
	sw t0, -184(fp)
	lw t1, 176(fp)
	sw t1, -188(fp)
	lw t2, 180(fp)
	sw t2, -192(fp)
	lw t3, 184(fp)
	sw t3, -196(fp)
	lw t4, 188(fp)
	sw t4, -200(fp)
	lw t5, 192(fp)
	sw t5, -204(fp)
	lw t6, 196(fp)
	sw t6, -208(fp)
	lw t0, 200(fp)
	sw t0, -212(fp)
	lw t1, 204(fp)
	sw t1, -216(fp)
	lw t2, 208(fp)
	sw t2, -220(fp)
	lw t3, 212(fp)
	sw t3, -224(fp)
	lw t4, 216(fp)
	sw t4, -228(fp)
	lw t5, 220(fp)
	sw t5, -232(fp)
# t53 = a1 + a2
	lw t0, -12(fp)
	lw t1, -16(fp)
	add t6, t0, t1
	sw t6, -44(fp)
# t54 = t53 + a3
	lw t3, -44(fp)
	lw t4, -20(fp)
	add t2, t3, t4
	sw t2, -48(fp)
# t55 = t54 + a4
	lw t6, -48(fp)
	lw t0, -24(fp)
	add t5, t6, t0
	sw t5, -52(fp)
# t56 = t55 + a5
	lw t2, -52(fp)
	lw t3, -28(fp)
	add t1, t2, t3
	sw t1, -56(fp)
# t57 = t56 + a6
	lw t5, -56(fp)
	lw t6, -32(fp)
	add t4, t5, t6
	sw t4, -60(fp)
# t58 = t57 + a7
	lw t1, -60(fp)
	lw t2, -36(fp)
	add t0, t1, t2
	sw t0, -64(fp)
# t59 = t58 + a8
	lw t4, -64(fp)
	lw t5, -40(fp)
	add t3, t4, t5
	sw t3, -68(fp)
# sum1_scope-2115024725 = t59
	lw t6, -68(fp)
	sw t6, -72(fp)
# t60 = a9 + a10
	lw t1, -12(fp)
	lw t2, -16(fp)
	add t0, t1, t2
	sw t0, -76(fp)
# t61 = t60 + a11
	lw t4, -76(fp)
	lw t5, -20(fp)
	add t3, t4, t5
	sw t3, -80(fp)
# t62 = t61 + a12
	lw t0, -80(fp)
	lw t1, -24(fp)
	add t6, t0, t1
	sw t6, -84(fp)
# t63 = t62 + a13
	lw t3, -84(fp)
	lw t4, -28(fp)
	add t2, t3, t4
	sw t2, -88(fp)
# t64 = t63 + a14
	lw t6, -88(fp)
	lw t0, -32(fp)
	add t5, t6, t0
	sw t5, -92(fp)
# t65 = t64 + a15
	lw t2, -92(fp)
	lw t3, -36(fp)
	add t1, t2, t3
	sw t1, -96(fp)
# t66 = t65 + a16
	lw t5, -96(fp)
	lw t6, -40(fp)
	add t4, t5, t6
	sw t4, -100(fp)
# sum2_scope-2115024725 = t66
	lw t0, -100(fp)
	sw t0, -104(fp)
# t67 = a17 + a18
	lw t2, -44(fp)
	lw t3, -48(fp)
	add t1, t2, t3
	sw t1, -108(fp)
# t68 = t67 + a19
	lw t5, -108(fp)
	lw t6, -52(fp)
	add t4, t5, t6
	sw t4, -112(fp)
# t69 = t68 + a20
	lw t1, -112(fp)
	lw t2, -56(fp)
	add t0, t1, t2
	sw t0, -116(fp)
# t70 = t69 + a21
	lw t4, -116(fp)
	lw t5, -60(fp)
	add t3, t4, t5
	sw t3, -120(fp)
# t71 = t70 + a22
	lw t0, -120(fp)
	lw t1, -64(fp)
	add t6, t0, t1
	sw t6, -124(fp)
# t72 = t71 + a23
	lw t3, -124(fp)
	lw t4, -68(fp)
	add t2, t3, t4
	sw t2, -128(fp)
# t73 = t72 + a24
	lw t6, -128(fp)
	lw t0, -72(fp)
	add t5, t6, t0
	sw t5, -132(fp)
# sum3_scope-2115024725 = t73
	lw t1, -132(fp)
	sw t1, -136(fp)
# t74 = a25 + a26
	lw t3, -76(fp)
	lw t4, -80(fp)
	add t2, t3, t4
	sw t2, -140(fp)
# t75 = t74 + a27
	lw t6, -140(fp)
	lw t0, -84(fp)
	add t5, t6, t0
	sw t5, -144(fp)
# t76 = t75 + a28
	lw t2, -144(fp)
	lw t3, -88(fp)
	add t1, t2, t3
	sw t1, -148(fp)
# t77 = t76 + a29
	lw t5, -148(fp)
	lw t6, -92(fp)
	add t4, t5, t6
	sw t4, -152(fp)
# t78 = t77 + a30
	lw t1, -152(fp)
	lw t2, -96(fp)
	add t0, t1, t2
	sw t0, -156(fp)
# t79 = t78 + a31
	lw t4, -156(fp)
	lw t5, -100(fp)
	add t3, t4, t5
	sw t3, -160(fp)
# t80 = t79 + a32
	lw t0, -160(fp)
	lw t1, -104(fp)
	add t6, t0, t1
	sw t6, -164(fp)
# sum4_scope-2115024725 = t80
	lw t2, -164(fp)
	sw t2, -168(fp)
# t81 = a33 + a34
	lw t4, -108(fp)
	lw t5, -112(fp)
	add t3, t4, t5
	sw t3, -172(fp)
# t82 = t81 + a35
	lw t0, -172(fp)
	lw t1, -116(fp)
	add t6, t0, t1
	sw t6, -176(fp)
# t83 = t82 + a36
	lw t3, -176(fp)
	lw t4, -120(fp)
	add t2, t3, t4
	sw t2, -180(fp)
# t84 = t83 + a37
	lw t6, -180(fp)
	lw t0, -124(fp)
	add t5, t6, t0
	sw t5, -184(fp)
# t85 = t84 + a38
	lw t2, -184(fp)
	lw t3, -128(fp)
	add t1, t2, t3
	sw t1, -188(fp)
# t86 = t85 + a39
	lw t5, -188(fp)
	lw t6, -132(fp)
	add t4, t5, t6
	sw t4, -192(fp)
# t87 = t86 + a40
	lw t1, -192(fp)
	lw t2, -136(fp)
	add t0, t1, t2
	sw t0, -196(fp)
# sum5_scope-2115024725 = t87
	lw t3, -196(fp)
	sw t3, -200(fp)
# t88 = a41 + a42
	lw t5, -140(fp)
	lw t6, -144(fp)
	add t4, t5, t6
	sw t4, -204(fp)
# t89 = t88 + a43
	lw t1, -204(fp)
	lw t2, -148(fp)
	add t0, t1, t2
	sw t0, -208(fp)
# t90 = t89 + a44
	lw t4, -208(fp)
	lw t5, -152(fp)
	add t3, t4, t5
	sw t3, -212(fp)
# t91 = t90 + a45
	lw t0, -212(fp)
	lw t1, -156(fp)
	add t6, t0, t1
	sw t6, -216(fp)
# t92 = t91 + a46
	lw t3, -216(fp)
	lw t4, -160(fp)
	add t2, t3, t4
	sw t2, -220(fp)
# t93 = t92 + a47
	lw t6, -220(fp)
	lw t0, -164(fp)
	add t5, t6, t0
	sw t5, -224(fp)
# t94 = t93 + a48
	lw t2, -224(fp)
	lw t3, -168(fp)
	add t1, t2, t3
	sw t1, -228(fp)
# sum6_scope-2115024725 = t94
	lw t4, -228(fp)
	sw t4, -232(fp)
# t95 = a49 + a50
	lw t6, -172(fp)
	lw t0, -176(fp)
	add t5, t6, t0
	sw t5, -236(fp)
# t96 = t95 + a51
	lw t2, -236(fp)
	lw t3, -180(fp)
	add t1, t2, t3
	sw t1, -240(fp)
# t97 = t96 + a52
	lw t5, -240(fp)
	lw t6, -184(fp)
	add t4, t5, t6
	sw t4, -244(fp)
# t98 = t97 + a53
	lw t1, -244(fp)
	lw t2, -188(fp)
	add t0, t1, t2
	sw t0, -248(fp)
# t99 = t98 + a54
	lw t4, -248(fp)
	lw t5, -192(fp)
	add t3, t4, t5
	sw t3, -252(fp)
# t100 = t99 + a55
	lw t0, -252(fp)
	lw t1, -196(fp)
	add t6, t0, t1
	sw t6, -256(fp)
# t101 = t100 + a56
	lw t3, -256(fp)
	lw t4, -200(fp)
	add t2, t3, t4
	sw t2, -260(fp)
# sum7_scope-2115024725 = t101
	lw t5, -260(fp)
	sw t5, -264(fp)
# t102 = a57 + a58
	lw t0, -204(fp)
	lw t1, -208(fp)
	add t6, t0, t1
	sw t6, -268(fp)
# t103 = t102 + a59
	lw t3, -268(fp)
	lw t4, -212(fp)
	add t2, t3, t4
	sw t2, -272(fp)
# t104 = t103 + a60
	lw t6, -272(fp)
	lw t0, -216(fp)
	add t5, t6, t0
	sw t5, -276(fp)
# t105 = t104 + a61
	lw t2, -276(fp)
	lw t3, -220(fp)
	add t1, t2, t3
	sw t1, -280(fp)
# t106 = t105 + a62
	lw t5, -280(fp)
	lw t6, -224(fp)
	add t4, t5, t6
	sw t4, -284(fp)
# t107 = t106 + a63
	lw t1, -284(fp)
	lw t2, -228(fp)
	add t0, t1, t2
	sw t0, -288(fp)
# t108 = t107 + a64
	lw t4, -288(fp)
	lw t5, -232(fp)
	add t3, t4, t5
	sw t3, -292(fp)
# sum8_scope-2115024725 = t108
	lw t6, -292(fp)
	sw t6, -296(fp)
# t109 = sum1_scope-2115024725 + sum2_scope-2115024725
	lw t1, -72(fp)
	lw t2, -104(fp)
	add t0, t1, t2
	sw t0, -300(fp)
# t110 = t109 + sum3_scope-2115024725
	lw t4, -300(fp)
	lw t5, -136(fp)
	add t3, t4, t5
	sw t3, -304(fp)
# t111 = t110 + sum4_scope-2115024725
	lw t0, -304(fp)
	lw t1, -168(fp)
	add t6, t0, t1
	sw t6, -308(fp)
# t112 = t111 + sum5_scope-2115024725
	lw t3, -308(fp)
	lw t4, -200(fp)
	add t2, t3, t4
	sw t2, -312(fp)
# t113 = t112 + sum6_scope-2115024725
	lw t6, -312(fp)
	lw t0, -232(fp)
	add t5, t6, t0
	sw t5, -316(fp)
# t114 = t113 + sum7_scope-2115024725
	lw t2, -316(fp)
	lw t3, -264(fp)
	add t1, t2, t3
	sw t1, -320(fp)
# t115 = t114 + sum8_scope-2115024725
	lw t5, -320(fp)
	lw t6, -296(fp)
	add t4, t5, t6
	sw t4, -324(fp)
# return t115
	lw a0, -324(fp)
	j sum64_epilogue
sum64_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2456
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2460
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2464
	add sp, sp, t0
	ret
	.global main
main:
# 函数序言
	li t0, -2464
	add sp, sp, t0
	li t0, 2460
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2456
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2464
	add fp, sp, t0
# 保存被调用者保存的寄存器
# v1_scope-2115024725 = 1
	li t0, 1
	sw t0, -12(fp)
# v2_scope-2115024725 = 2
	li t1, 2
	sw t1, -16(fp)
# v3_scope-2115024725 = 3
	li t2, 3
	sw t2, -20(fp)
# v4_scope-2115024725 = 4
	li t3, 4
	sw t3, -24(fp)
# v5_scope-2115024725 = 5
	li t4, 5
	sw t4, -28(fp)
# v6_scope-2115024725 = 6
	li t5, 6
	sw t5, -32(fp)
# v7_scope-2115024725 = 7
	li t6, 7
	sw t6, -36(fp)
# v8_scope-2115024725 = 8
	li t0, 8
	sw t0, -40(fp)
# v9_scope-2115024725 = 9
	li t1, 9
	sw t1, -44(fp)
# v10_scope-2115024725 = 10
	li t2, 10
	sw t2, -48(fp)
# v11_scope-2115024725 = 11
	li t3, 11
	sw t3, -52(fp)
# v12_scope-2115024725 = 12
	li t4, 12
	sw t4, -56(fp)
# v13_scope-2115024725 = 13
	li t5, 13
	sw t5, -60(fp)
# v14_scope-2115024725 = 14
	li t6, 14
	sw t6, -64(fp)
# v15_scope-2115024725 = 15
	li t0, 15
	sw t0, -68(fp)
# v16_scope-2115024725 = 16
	li t1, 16
	sw t1, -72(fp)
# param v1_scope-2115024725
# param 1
# param v3_scope-2115024725
# param 2
# param v5_scope-2115024725
# param 3
# param v7_scope-2115024725
# param 4
# t116 = call sum8, 8
# 保存调用者保存的寄存器
	sw t0, -76(fp)
	sw t1, -80(fp)
	lw a0, -12(fp)
	li a1, 1
	lw a2, -20(fp)
	li a3, 2
	lw a4, -28(fp)
	li a5, 3
	lw a6, -36(fp)
	li a7, 4
	call sum8
# 恢复调用者保存的寄存器
	lw t0, -76(fp)
	lw t1, -80(fp)
	addi t2, a0, 0
	sw t2, -84(fp)
# result1_scope-2115024725 = t116
	lw t3, -84(fp)
	sw t3, -88(fp)
# t117 = result1_scope-2115024725 + v13_scope-2115024725
	lw t5, -88(fp)
	lw t6, -60(fp)
	add t4, t5, t6
	sw t4, -92(fp)
# t118 = result1_scope-2115024725 + v14_scope-2115024725
	lw t1, -88(fp)
	lw t2, -64(fp)
	add t0, t1, t2
	sw t0, -96(fp)
# t119 = result1_scope-2115024725 + v15_scope-2115024725
	lw t4, -88(fp)
	lw t5, -68(fp)
	add t3, t4, t5
	sw t3, -100(fp)
# t120 = result1_scope-2115024725 + v16_scope-2115024725
	lw t0, -88(fp)
	lw t1, -72(fp)
	add t6, t0, t1
	sw t6, -104(fp)
# param v1_scope-2115024725
# param v2_scope-2115024725
# param v3_scope-2115024725
# param v4_scope-2115024725
# param v5_scope-2115024725
# param v6_scope-2115024725
# param v7_scope-2115024725
# param v8_scope-2115024725
# param 1
# param 2
# param 3
# param 4
# param t117
# param t118
# param t119
# param t120
# t121 = call sum16, 16
# 保存调用者保存的寄存器
	sw t0, -76(fp)
	sw t1, -80(fp)
	lw a0, -12(fp)
	lw a1, -16(fp)
	lw a2, -20(fp)
	lw a3, -24(fp)
	lw a4, -28(fp)
	lw a5, -32(fp)
	lw a6, -36(fp)
	lw a7, -40(fp)
	li t2, 1
	sw t2, 1768(sp)
	li t3, 2
	sw t3, 1772(sp)
	li t4, 3
	sw t4, 1776(sp)
	li t5, 4
	sw t5, 1780(sp)
	lw t6, -92(fp)
	sw t6, 1784(sp)
	lw t0, -96(fp)
	sw t0, 1788(sp)
	lw t1, -100(fp)
	sw t1, 1792(sp)
	lw t2, -104(fp)
	sw t2, 1796(sp)
	call sum16
# 恢复调用者保存的寄存器
	lw t0, -76(fp)
	lw t1, -80(fp)
	lw t2, -108(fp)
	lw t3, -112(fp)
	lw t4, -116(fp)
	lw t5, -120(fp)
	lw t6, -124(fp)
	addi t3, a0, 0
	sw t3, -128(fp)
# result2_scope-2115024725 = t121
	lw t4, -128(fp)
	sw t4, -132(fp)
# v17_scope-2115024725 = 17
	li t5, 17
	sw t5, -136(fp)
# v18_scope-2115024725 = 18
	li t6, 18
	sw t6, -140(fp)
# v19_scope-2115024725 = 19
	li t0, 19
	sw t0, -144(fp)
# v20_scope-2115024725 = 20
	li t1, 20
	sw t1, -148(fp)
# v21_scope-2115024725 = 21
	li t2, 21
	sw t2, -152(fp)
# v22_scope-2115024725 = 22
	li t3, 22
	sw t3, -156(fp)
# v23_scope-2115024725 = 23
	li t4, 23
	sw t4, -160(fp)
# v24_scope-2115024725 = 24
	li t5, 24
	sw t5, -164(fp)
# v25_scope-2115024725 = 25
	li t6, 25
	sw t6, -168(fp)
# v26_scope-2115024725 = 26
	li t0, 26
	sw t0, -172(fp)
# v27_scope-2115024725 = 27
	li t1, 27
	sw t1, -176(fp)
# v28_scope-2115024725 = 28
	li t2, 28
	sw t2, -180(fp)
# v29_scope-2115024725 = 29
	li t3, 29
	sw t3, -184(fp)
# v30_scope-2115024725 = 30
	li t4, 30
	sw t4, -188(fp)
# v31_scope-2115024725 = 31
	li t5, 31
	sw t5, -192(fp)
# v32_scope-2115024725 = 32
	li t6, 32
	sw t6, -196(fp)
# param v1_scope-2115024725
# param v2_scope-2115024725
# param v3_scope-2115024725
# param v4_scope-2115024725
# param v5_scope-2115024725
# param v6_scope-2115024725
# param v7_scope-2115024725
# param v8_scope-2115024725
# param v9_scope-2115024725
# param v10_scope-2115024725
# param v11_scope-2115024725
# param v12_scope-2115024725
# param v13_scope-2115024725
# param v14_scope-2115024725
# param v15_scope-2115024725
# param v16_scope-2115024725
# param v17_scope-2115024725
# param v18_scope-2115024725
# param v19_scope-2115024725
# param v20_scope-2115024725
# param v21_scope-2115024725
# param v22_scope-2115024725
# param v23_scope-2115024725
# param v24_scope-2115024725
# param v25_scope-2115024725
# param v26_scope-2115024725
# param v27_scope-2115024725
# param v28_scope-2115024725
# param v29_scope-2115024725
# param v30_scope-2115024725
# param v31_scope-2115024725
# param v32_scope-2115024725
# t122 = call sum32, 32
# 保存调用者保存的寄存器
	sw t0, -76(fp)
	sw t1, -80(fp)
	sw t2, -108(fp)
	sw t3, -112(fp)
	sw t4, -116(fp)
	sw t5, -120(fp)
	sw t6, -124(fp)
	lw a0, -12(fp)
	lw a1, -16(fp)
	lw a2, -20(fp)
	lw a3, -24(fp)
	lw a4, -28(fp)
	lw a5, -32(fp)
	lw a6, -36(fp)
	lw a7, -40(fp)
	lw t0, -44(fp)
	sw t0, 1768(sp)
	lw t1, -48(fp)
	sw t1, 1772(sp)
	lw t2, -52(fp)
	sw t2, 1776(sp)
	lw t3, -56(fp)
	sw t3, 1780(sp)
	lw t4, -60(fp)
	sw t4, 1784(sp)
	lw t5, -64(fp)
	sw t5, 1788(sp)
	lw t6, -68(fp)
	sw t6, 1792(sp)
	lw t0, -72(fp)
	sw t0, 1796(sp)
	lw t1, -136(fp)
	sw t1, 1800(sp)
	lw t2, -140(fp)
	sw t2, 1804(sp)
	lw t3, -144(fp)
	sw t3, 1808(sp)
	lw t4, -148(fp)
	sw t4, 1812(sp)
	lw t5, -152(fp)
	sw t5, 1816(sp)
	lw t6, -156(fp)
	sw t6, 1820(sp)
	lw t0, -160(fp)
	sw t0, 1824(sp)
	lw t1, -164(fp)
	sw t1, 1828(sp)
	lw t2, -168(fp)
	sw t2, 1832(sp)
	lw t3, -172(fp)
	sw t3, 1836(sp)
	lw t4, -176(fp)
	sw t4, 1840(sp)
	lw t5, -180(fp)
	sw t5, 1844(sp)
	lw t6, -184(fp)
	sw t6, 1848(sp)
	lw t0, -188(fp)
	sw t0, 1852(sp)
	lw t1, -192(fp)
	sw t1, 1856(sp)
	lw t2, -196(fp)
	sw t2, 1860(sp)
	call sum32
# 恢复调用者保存的寄存器
	lw t0, -76(fp)
	lw t1, -80(fp)
	lw t2, -108(fp)
	lw t3, -112(fp)
	lw t4, -116(fp)
	lw t5, -120(fp)
	lw t6, -124(fp)
	addi t3, a0, 0
	sw t3, -200(fp)
# result3_scope-2115024725 = t122
	lw t4, -200(fp)
	sw t4, -204(fp)
# t123 = v1_scope-2115024725 + 1
	lw t6, -12(fp)
	li t0, 1
	add t5, t6, t0
	sw t5, -208(fp)
# t124 = v2_scope-2115024725 + 2
	lw t2, -16(fp)
	li t3, 2
	add t1, t2, t3
	sw t1, -212(fp)
# t125 = v3_scope-2115024725 + 3
	lw t5, -20(fp)
	li t6, 3
	add t4, t5, t6
	sw t4, -216(fp)
# t126 = v4_scope-2115024725 + 4
	lw t1, -24(fp)
	li t2, 4
	add t0, t1, t2
	sw t0, -220(fp)
# t127 = v5_scope-2115024725 + 5
	lw t4, -28(fp)
	li t5, 5
	add t3, t4, t5
	sw t3, -224(fp)
# t128 = v6_scope-2115024725 + 6
	lw t0, -32(fp)
	li t1, 6
	add t6, t0, t1
	sw t6, -228(fp)
# t129 = v7_scope-2115024725 + 7
	lw t3, -36(fp)
	li t4, 7
	add t2, t3, t4
	sw t2, -232(fp)
# t130 = v8_scope-2115024725 + 8
	lw t6, -40(fp)
	li t0, 8
	add t5, t6, t0
	sw t5, -236(fp)
# t131 = v9_scope-2115024725 * 9
	lw t2, -44(fp)
	li t3, 9
	mul t1, t2, t3
	sw t1, -240(fp)
# t132 = v10_scope-2115024725 * 10
	lw t5, -48(fp)
	li t6, 10
	mul t4, t5, t6
	sw t4, -244(fp)
# t133 = v11_scope-2115024725 * 11
	lw t1, -52(fp)
	li t2, 11
	mul t0, t1, t2
	sw t0, -248(fp)
# t134 = v12_scope-2115024725 * 12
	lw t4, -56(fp)
	li t5, 12
	mul t3, t4, t5
	sw t3, -252(fp)
# t135 = v13_scope-2115024725 * 13
	lw t0, -60(fp)
	li t1, 13
	mul t6, t0, t1
	sw t6, -256(fp)
# t136 = v14_scope-2115024725 * 14
	lw t3, -64(fp)
	li t4, 14
	mul t2, t3, t4
	sw t2, -260(fp)
# t137 = v15_scope-2115024725 * 15
	lw t6, -68(fp)
	li t0, 15
	mul t5, t6, t0
	sw t5, -264(fp)
# t138 = v16_scope-2115024725 * 16
	lw t2, -72(fp)
	li t3, 16
	mul t1, t2, t3
	sw t1, -268(fp)
# t139 = v1_scope-2115024725 + v17_scope-2115024725
	lw t5, -12(fp)
	lw t6, -136(fp)
	add t4, t5, t6
	sw t4, -272(fp)
# t140 = v2_scope-2115024725 + v18_scope-2115024725
	lw t1, -16(fp)
	lw t2, -140(fp)
	add t0, t1, t2
	sw t0, -276(fp)
# t141 = v3_scope-2115024725 + v19_scope-2115024725
	lw t4, -20(fp)
	lw t5, -144(fp)
	add t3, t4, t5
	sw t3, -280(fp)
# t142 = v4_scope-2115024725 + v20_scope-2115024725
	lw t0, -24(fp)
	lw t1, -148(fp)
	add t6, t0, t1
	sw t6, -284(fp)
# t143 = v5_scope-2115024725 + v21_scope-2115024725
	lw t3, -28(fp)
	lw t4, -152(fp)
	add t2, t3, t4
	sw t2, -288(fp)
# t144 = v6_scope-2115024725 + v22_scope-2115024725
	lw t6, -32(fp)
	lw t0, -156(fp)
	add t5, t6, t0
	sw t5, -292(fp)
# t145 = v7_scope-2115024725 + v23_scope-2115024725
	lw t2, -36(fp)
	lw t3, -160(fp)
	add t1, t2, t3
	sw t1, -296(fp)
# t146 = v8_scope-2115024725 + v24_scope-2115024725
	lw t5, -40(fp)
	lw t6, -164(fp)
	add t4, t5, t6
	sw t4, -300(fp)
# t147 = v1_scope-2115024725 * v9_scope-2115024725
	lw t1, -12(fp)
	lw t2, -44(fp)
	mul t0, t1, t2
	sw t0, -304(fp)
# t148 = t147 + result3_scope-2115024725
	lw t4, -304(fp)
	lw t5, -204(fp)
	add t3, t4, t5
	sw t3, -308(fp)
# t149 = v2_scope-2115024725 * v10_scope-2115024725
	lw t0, -16(fp)
	lw t1, -48(fp)
	mul t6, t0, t1
	sw t6, -312(fp)
# t150 = t149 + result3_scope-2115024725
	lw t3, -312(fp)
	lw t4, -204(fp)
	add t2, t3, t4
	sw t2, -316(fp)
# t151 = v3_scope-2115024725 * v11_scope-2115024725
	lw t6, -20(fp)
	lw t0, -52(fp)
	mul t5, t6, t0
	sw t5, -320(fp)
# t152 = t151 + result3_scope-2115024725
	lw t2, -320(fp)
	lw t3, -204(fp)
	add t1, t2, t3
	sw t1, -324(fp)
# t153 = v4_scope-2115024725 * v12_scope-2115024725
	lw t5, -24(fp)
	lw t6, -56(fp)
	mul t4, t5, t6
	sw t4, -328(fp)
# t154 = t153 + result3_scope-2115024725
	lw t1, -328(fp)
	lw t2, -204(fp)
	add t0, t1, t2
	sw t0, -332(fp)
# t155 = v5_scope-2115024725 * v13_scope-2115024725
	lw t4, -28(fp)
	lw t5, -60(fp)
	mul t3, t4, t5
	sw t3, -336(fp)
# t156 = t155 + result3_scope-2115024725
	lw t0, -336(fp)
	lw t1, -204(fp)
	add t6, t0, t1
	sw t6, -340(fp)
# t157 = v6_scope-2115024725 * v14_scope-2115024725
	lw t3, -32(fp)
	lw t4, -64(fp)
	mul t2, t3, t4
	sw t2, -344(fp)
# t158 = t157 + result3_scope-2115024725
	lw t6, -344(fp)
	lw t0, -204(fp)
	add t5, t6, t0
	sw t5, -348(fp)
# t159 = v7_scope-2115024725 * v15_scope-2115024725
	lw t2, -36(fp)
	lw t3, -68(fp)
	mul t1, t2, t3
	sw t1, -352(fp)
# t160 = t159 + result3_scope-2115024725
	lw t5, -352(fp)
	lw t6, -204(fp)
	add t4, t5, t6
	sw t4, -356(fp)
# t161 = result1_scope-2115024725 + result2_scope-2115024725
	lw t1, -88(fp)
	lw t2, -132(fp)
	add t0, t1, t2
	sw t0, -360(fp)
# t162 = t161 + result3_scope-2115024725
	lw t4, -360(fp)
	lw t5, -204(fp)
	add t3, t4, t5
	sw t3, -364(fp)
# t163 = v8_scope-2115024725 * v16_scope-2115024725
	lw t0, -40(fp)
	lw t1, -72(fp)
	mul t6, t0, t1
	sw t6, -368(fp)
# t164 = t163 + t162
	lw t3, -368(fp)
	lw t4, -364(fp)
	add t2, t3, t4
	sw t2, -372(fp)
# param v1_scope-2115024725
# param v2_scope-2115024725
# param v3_scope-2115024725
# param v4_scope-2115024725
# param v5_scope-2115024725
# param v6_scope-2115024725
# param v7_scope-2115024725
# param v8_scope-2115024725
# param 9
# param 10
# param 11
# param 12
# param 13
# param 14
# param 15
# param 16
# param v17_scope-2115024725
# param v18_scope-2115024725
# param v19_scope-2115024725
# param v20_scope-2115024725
# param v21_scope-2115024725
# param v22_scope-2115024725
# param v23_scope-2115024725
# param v24_scope-2115024725
# param 25
# param 26
# param 27
# param 28
# param 29
# param 30
# param 31
# param 32
# param t123
# param t124
# param t125
# param t126
# param t127
# param t128
# param t129
# param t130
# param t131
# param t132
# param t133
# param t134
# param t135
# param t136
# param t137
# param t138
# param t139
# param t140
# param t141
# param t142
# param t143
# param t144
# param t145
# param t146
# param t148
# param t150
# param t152
# param t154
# param t156
# param t158
# param t160
# param t164
# t165 = call sum64, 64
# 保存调用者保存的寄存器
	sw t0, -76(fp)
	sw t1, -80(fp)
	sw t2, -108(fp)
	sw t3, -112(fp)
	sw t4, -116(fp)
	lw a0, -12(fp)
	lw a1, -16(fp)
	lw a2, -20(fp)
	lw a3, -24(fp)
	lw a4, -28(fp)
	lw a5, -32(fp)
	lw a6, -36(fp)
	lw a7, -40(fp)
	li t5, 9
	sw t5, 1768(sp)
	li t6, 10
	sw t6, 1772(sp)
	li t0, 11
	sw t0, 1776(sp)
	li t1, 12
	sw t1, 1780(sp)
	li t2, 13
	sw t2, 1784(sp)
	li t3, 14
	sw t3, 1788(sp)
	li t4, 15
	sw t4, 1792(sp)
	li t5, 16
	sw t5, 1796(sp)
	lw t6, -136(fp)
	sw t6, 1800(sp)
	lw t0, -140(fp)
	sw t0, 1804(sp)
	lw t1, -144(fp)
	sw t1, 1808(sp)
	lw t2, -148(fp)
	sw t2, 1812(sp)
	lw t3, -152(fp)
	sw t3, 1816(sp)
	lw t4, -156(fp)
	sw t4, 1820(sp)
	lw t5, -160(fp)
	sw t5, 1824(sp)
	lw t6, -164(fp)
	sw t6, 1828(sp)
	li t0, 25
	sw t0, 1832(sp)
	li t1, 26
	sw t1, 1836(sp)
	li t2, 27
	sw t2, 1840(sp)
	li t3, 28
	sw t3, 1844(sp)
	li t4, 29
	sw t4, 1848(sp)
	li t5, 30
	sw t5, 1852(sp)
	li t6, 31
	sw t6, 1856(sp)
	li t0, 32
	sw t0, 1860(sp)
	lw t1, -208(fp)
	sw t1, 1864(sp)
	lw t2, -212(fp)
	sw t2, 1868(sp)
	lw t3, -216(fp)
	sw t3, 1872(sp)
	lw t4, -220(fp)
	sw t4, 1876(sp)
	lw t5, -224(fp)
	sw t5, 1880(sp)
	lw t6, -228(fp)
	sw t6, 1884(sp)
	lw t0, -232(fp)
	sw t0, 1888(sp)
	lw t1, -236(fp)
	sw t1, 1892(sp)
	lw t2, -240(fp)
	sw t2, 1896(sp)
	lw t3, -244(fp)
	sw t3, 1900(sp)
	lw t4, -248(fp)
	sw t4, 1904(sp)
	lw t5, -252(fp)
	sw t5, 1908(sp)
	lw t6, -256(fp)
	sw t6, 1912(sp)
	lw t0, -260(fp)
	sw t0, 1916(sp)
	lw t1, -264(fp)
	sw t1, 1920(sp)
	lw t2, -268(fp)
	sw t2, 1924(sp)
	lw t3, -272(fp)
	sw t3, 1928(sp)
	lw t4, -276(fp)
	sw t4, 1932(sp)
	lw t5, -280(fp)
	sw t5, 1936(sp)
	lw t6, -284(fp)
	sw t6, 1940(sp)
	lw t0, -288(fp)
	sw t0, 1944(sp)
	lw t1, -292(fp)
	sw t1, 1948(sp)
	lw t2, -296(fp)
	sw t2, 1952(sp)
	lw t3, -300(fp)
	sw t3, 1956(sp)
	lw t4, -308(fp)
	sw t4, 1960(sp)
	lw t5, -316(fp)
	sw t5, 1964(sp)
	lw t6, -324(fp)
	sw t6, 1968(sp)
	lw t0, -332(fp)
	sw t0, 1972(sp)
	lw t1, -340(fp)
	sw t1, 1976(sp)
	lw t2, -348(fp)
	sw t2, 1980(sp)
	lw t3, -356(fp)
	sw t3, 1984(sp)
	lw t4, -372(fp)
	sw t4, 1988(sp)
	call sum64
# 恢复调用者保存的寄存器
	lw t0, -76(fp)
	lw t1, -80(fp)
	lw t2, -108(fp)
	lw t3, -112(fp)
	lw t4, -116(fp)
	lw t5, -120(fp)
	lw t6, -124(fp)
	addi t5, a0, 0
	sw t5, -376(fp)
# result4_scope-2115024725 = t165
	lw t6, -376(fp)
	sw t6, -380(fp)
# t166 = result1_scope-2115024725 + result2_scope-2115024725
	lw t1, -88(fp)
	lw t2, -132(fp)
	add t0, t1, t2
	sw t0, -384(fp)
# t167 = t166 + result3_scope-2115024725
	lw t4, -384(fp)
	lw t5, -204(fp)
	add t3, t4, t5
	sw t3, -388(fp)
# t168 = t167 + result4_scope-2115024725
	lw t0, -388(fp)
	lw t1, -380(fp)
	add t6, t0, t1
	sw t6, -392(fp)
# final_result_scope-2115024725 = t168
	lw t2, -392(fp)
	sw t2, -396(fp)
# t169 = final_result_scope-2115024725 % 256
	lw t4, -396(fp)
	li t5, 256
	rem t3, t4, t5
	sw t3, -400(fp)
# return t169
	lw a0, -400(fp)
	j main_epilogue
main_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2456
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2460
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2464
	add sp, sp, t0
	ret
