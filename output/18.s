# 由ToyC编译器生成
# RISC-V汇编代码
.text
	.global abs
abs:
# 函数序言
	li t0, -2304
	add sp, sp, t0
	li t0, 2300
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2296
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2304
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
# t0 = x < 0
	lw t1, -12(fp)
	li t2, 0
	slt t0, t1, t2
	sw t0, -16(fp)
# t1 = !t0
	lw t4, -16(fp)
	seqz t3, t4
	sw t3, -20(fp)
# if t1 goto L0
	lw t5, -20(fp)
	bnez t5, L0
# t2 = -x
	lw t0, -12(fp)
	neg t6, t0
	sw t6, -24(fp)
# return t2
	lw a0, -24(fp)
	j abs_epilogue
# goto L1
	j L1
L0:
# return x
	lw a0, -12(fp)
	j abs_epilogue
L1:
abs_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2296
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2300
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2304
	add sp, sp, t0
	ret
	.global compute
compute:
# 函数序言
	li t0, -2304
	add sp, sp, t0
	li t0, 2300
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2296
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2304
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
# param g
# t3 = call abs, 1
# 保存调用者保存的寄存器
	sw t0, -44(fp)
	lw a0, -36(fp)
	call abs
# 恢复调用者保存的寄存器
	lw t0, -44(fp)
	addi t1, a0, 0
	sw t1, -48(fp)
# t4 = t3 + 1
	lw t3, -48(fp)
	li t4, 1
	add t2, t3, t4
	sw t2, -52(fp)
# t5 = f % t4
	lw t6, -32(fp)
	lw t0, -52(fp)
	rem t5, t6, t0
	sw t5, -56(fp)
# t6 = t5 * h
	lw t2, -56(fp)
	lw t3, -40(fp)
	mul t1, t2, t3
	sw t1, -60(fp)
# param e
# t7 = call abs, 1
# 保存调用者保存的寄存器
	sw t0, -44(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	lw a0, -28(fp)
	call abs
# 恢复调用者保存的寄存器
	lw t0, -44(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	addi t4, a0, 0
	sw t4, -76(fp)
# t8 = t7 + 1
	lw t6, -76(fp)
	li t0, 1
	add t5, t6, t0
	sw t5, -80(fp)
# t9 = d / t8
	lw t2, -24(fp)
	lw t3, -80(fp)
	div t1, t2, t3
	sw t1, -84(fp)
# t10 = b * c
	lw t5, -16(fp)
	lw t6, -20(fp)
	mul t4, t5, t6
	sw t4, -88(fp)
# t11 = a + t10
	lw t1, -12(fp)
	lw t2, -88(fp)
	add t0, t1, t2
	sw t0, -92(fp)
# t12 = t11 - t9
	lw t4, -92(fp)
	lw t5, -84(fp)
	sub t3, t4, t5
	sw t3, -96(fp)
# t13 = t12 + t6
	lw t0, -96(fp)
	lw t1, -60(fp)
	add t6, t0, t1
	sw t6, -100(fp)
# return t13
	lw a0, -100(fp)
	j compute_epilogue
compute_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2296
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2300
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2304
	add sp, sp, t0
	ret
	.global main
main:
# 函数序言
	li t0, -2304
	add sp, sp, t0
	li t0, 2300
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2296
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2304
	add fp, sp, t0
# 保存被调用者保存的寄存器
# v1_scope2104072363 = 1
	li t2, 1
	sw t2, -12(fp)
# v2_scope2104072363 = 2
	li t3, 2
	sw t3, -16(fp)
# v3_scope2104072363 = 3
	li t4, 3
	sw t4, -20(fp)
# v4_scope2104072363 = 4
	li t5, 4
	sw t5, -24(fp)
# v5_scope2104072363 = 5
	li t6, 5
	sw t6, -28(fp)
# v6_scope2104072363 = 6
	li t0, 6
	sw t0, -32(fp)
# v7_scope2104072363 = 7
	li t1, 7
	sw t1, -36(fp)
# v8_scope2104072363 = 8
	li t2, 8
	sw t2, -40(fp)
# v9_scope2104072363 = 9
	li t3, 9
	sw t3, -44(fp)
# v10_scope2104072363 = 10
	li t4, 10
	sw t4, -48(fp)
# v11_scope2104072363 = 11
	li t5, 11
	sw t5, -52(fp)
# v12_scope2104072363 = 12
	li t6, 12
	sw t6, -56(fp)
# v13_scope2104072363 = 13
	li t0, 13
	sw t0, -60(fp)
# v14_scope2104072363 = 14
	li t1, 14
	sw t1, -64(fp)
# v15_scope2104072363 = 15
	li t2, 15
	sw t2, -68(fp)
# v16_scope2104072363 = 16
	li t3, 16
	sw t3, -72(fp)
# v17_scope2104072363 = 17
	li t4, 17
	sw t4, -76(fp)
# v18_scope2104072363 = 18
	li t5, 18
	sw t5, -80(fp)
# v19_scope2104072363 = 19
	li t6, 19
	sw t6, -84(fp)
# v20_scope2104072363 = 20
	li t0, 20
	sw t0, -88(fp)
# v21_scope2104072363 = 21
	li t1, 21
	sw t1, -92(fp)
# v22_scope2104072363 = 22
	li t2, 22
	sw t2, -96(fp)
# v23_scope2104072363 = 23
	li t3, 23
	sw t3, -100(fp)
# v24_scope2104072363 = 24
	li t4, 24
	sw t4, -104(fp)
# v25_scope2104072363 = 25
	li t5, 25
	sw t5, -108(fp)
# v26_scope2104072363 = 26
	li t6, 26
	sw t6, -112(fp)
# v27_scope2104072363 = 27
	li t0, 27
	sw t0, -116(fp)
# v28_scope2104072363 = 28
	li t1, 28
	sw t1, -120(fp)
# v29_scope2104072363 = 29
	li t2, 29
	sw t2, -124(fp)
# v30_scope2104072363 = 30
	li t3, 30
	sw t3, -128(fp)
# v31_scope2104072363 = 31
	li t4, 31
	sw t4, -132(fp)
# v32_scope2104072363 = 32
	li t5, 32
	sw t5, -136(fp)
# t14 = v1_scope2104072363 + v2_scope2104072363
	lw t0, -12(fp)
	lw t1, -16(fp)
	add t6, t0, t1
	sw t6, -140(fp)
# t15 = t14 + v3_scope2104072363
	lw t3, -140(fp)
	lw t4, -20(fp)
	add t2, t3, t4
	sw t2, -144(fp)
# t16 = t15 + v4_scope2104072363
	lw t6, -144(fp)
	lw t0, -24(fp)
	add t5, t6, t0
	sw t5, -148(fp)
# t17 = t16 + v5_scope2104072363
	lw t2, -148(fp)
	lw t3, -28(fp)
	add t1, t2, t3
	sw t1, -152(fp)
# t18 = t17 + v6_scope2104072363
	lw t5, -152(fp)
	lw t6, -32(fp)
	add t4, t5, t6
	sw t4, -156(fp)
# t19 = t18 + v7_scope2104072363
	lw t1, -156(fp)
	lw t2, -36(fp)
	add t0, t1, t2
	sw t0, -160(fp)
# t20 = t19 + v8_scope2104072363
	lw t4, -160(fp)
	lw t5, -40(fp)
	add t3, t4, t5
	sw t3, -164(fp)
# sum1_scope2104072363 = t20
	lw t6, -164(fp)
	sw t6, -168(fp)
# t21 = v9_scope2104072363 + v10_scope2104072363
	lw t1, -44(fp)
	lw t2, -48(fp)
	add t0, t1, t2
	sw t0, -172(fp)
# t22 = t21 + v11_scope2104072363
	lw t4, -172(fp)
	lw t5, -52(fp)
	add t3, t4, t5
	sw t3, -176(fp)
# t23 = t22 + v12_scope2104072363
	lw t0, -176(fp)
	lw t1, -56(fp)
	add t6, t0, t1
	sw t6, -180(fp)
# t24 = t23 + v13_scope2104072363
	lw t3, -180(fp)
	lw t4, -60(fp)
	add t2, t3, t4
	sw t2, -184(fp)
# t25 = t24 + v14_scope2104072363
	lw t6, -184(fp)
	lw t0, -64(fp)
	add t5, t6, t0
	sw t5, -188(fp)
# t26 = t25 + v15_scope2104072363
	lw t2, -188(fp)
	lw t3, -68(fp)
	add t1, t2, t3
	sw t1, -192(fp)
# t27 = t26 + v16_scope2104072363
	lw t5, -192(fp)
	lw t6, -72(fp)
	add t4, t5, t6
	sw t4, -196(fp)
# sum2_scope2104072363 = t27
	lw t0, -196(fp)
	sw t0, -200(fp)
# t28 = v17_scope2104072363 + v18_scope2104072363
	lw t2, -76(fp)
	lw t3, -80(fp)
	add t1, t2, t3
	sw t1, -204(fp)
# t29 = t28 + v19_scope2104072363
	lw t5, -204(fp)
	lw t6, -84(fp)
	add t4, t5, t6
	sw t4, -208(fp)
# t30 = t29 + v20_scope2104072363
	lw t1, -208(fp)
	lw t2, -88(fp)
	add t0, t1, t2
	sw t0, -212(fp)
# t31 = t30 + v21_scope2104072363
	lw t4, -212(fp)
	lw t5, -92(fp)
	add t3, t4, t5
	sw t3, -216(fp)
# t32 = t31 + v22_scope2104072363
	lw t0, -216(fp)
	lw t1, -96(fp)
	add t6, t0, t1
	sw t6, -220(fp)
# t33 = t32 + v23_scope2104072363
	lw t3, -220(fp)
	lw t4, -100(fp)
	add t2, t3, t4
	sw t2, -224(fp)
# t34 = t33 + v24_scope2104072363
	lw t6, -224(fp)
	lw t0, -104(fp)
	add t5, t6, t0
	sw t5, -228(fp)
# sum3_scope2104072363 = t34
	lw t1, -228(fp)
	sw t1, -232(fp)
# t35 = v25_scope2104072363 + v26_scope2104072363
	lw t3, -108(fp)
	lw t4, -112(fp)
	add t2, t3, t4
	sw t2, -236(fp)
# t36 = t35 + v27_scope2104072363
	lw t6, -236(fp)
	lw t0, -116(fp)
	add t5, t6, t0
	sw t5, -240(fp)
# t37 = t36 + v28_scope2104072363
	lw t2, -240(fp)
	lw t3, -120(fp)
	add t1, t2, t3
	sw t1, -244(fp)
# t38 = t37 + v29_scope2104072363
	lw t5, -244(fp)
	lw t6, -124(fp)
	add t4, t5, t6
	sw t4, -248(fp)
# t39 = t38 + v30_scope2104072363
	lw t1, -248(fp)
	lw t2, -128(fp)
	add t0, t1, t2
	sw t0, -252(fp)
# t40 = t39 + v31_scope2104072363
	lw t4, -252(fp)
	lw t5, -132(fp)
	add t3, t4, t5
	sw t3, -256(fp)
# t41 = t40 + v32_scope2104072363
	lw t0, -256(fp)
	lw t1, -136(fp)
	add t6, t0, t1
	sw t6, -260(fp)
# sum4_scope2104072363 = t41
	lw t2, -260(fp)
	sw t2, -264(fp)
# t42 = sum1_scope2104072363 + sum2_scope2104072363
	lw t4, -168(fp)
	lw t5, -200(fp)
	add t3, t4, t5
	sw t3, -268(fp)
# t43 = t42 + sum3_scope2104072363
	lw t0, -268(fp)
	lw t1, -232(fp)
	add t6, t0, t1
	sw t6, -272(fp)
# t44 = t43 + sum4_scope2104072363
	lw t3, -272(fp)
	lw t4, -264(fp)
	add t2, t3, t4
	sw t2, -276(fp)
# result1_scope2104072363 = t44
	lw t5, -276(fp)
	sw t5, -280(fp)
# t45 = v1_scope2104072363 * 2
	lw t0, -12(fp)
	li t1, 2
	mul t6, t0, t1
	sw t6, -284(fp)
# u1_scope2104072364 = t45
	lw t2, -284(fp)
	sw t2, -288(fp)
# t46 = v2_scope2104072363 * 2
	lw t4, -16(fp)
	li t5, 2
	mul t3, t4, t5
	sw t3, -292(fp)
# u2_scope2104072364 = t46
	lw t6, -292(fp)
	sw t6, -296(fp)
# t47 = v3_scope2104072363 * 2
	lw t1, -20(fp)
	li t2, 2
	mul t0, t1, t2
	sw t0, -300(fp)
# u3_scope2104072364 = t47
	lw t3, -300(fp)
	sw t3, -304(fp)
# t48 = v4_scope2104072363 * 2
	lw t5, -24(fp)
	li t6, 2
	mul t4, t5, t6
	sw t4, -308(fp)
# u4_scope2104072364 = t48
	lw t0, -308(fp)
	sw t0, -312(fp)
# t49 = v5_scope2104072363 * 2
	lw t2, -28(fp)
	li t3, 2
	mul t1, t2, t3
	sw t1, -316(fp)
# u5_scope2104072364 = t49
	lw t4, -316(fp)
	sw t4, -320(fp)
# t50 = v6_scope2104072363 * 2
	lw t6, -32(fp)
	li t0, 2
	mul t5, t6, t0
	sw t5, -324(fp)
# u6_scope2104072364 = t50
	lw t1, -324(fp)
	sw t1, -328(fp)
# t51 = v7_scope2104072363 * 2
	lw t3, -36(fp)
	li t4, 2
	mul t2, t3, t4
	sw t2, -332(fp)
# u7_scope2104072364 = t51
	lw t5, -332(fp)
	sw t5, -336(fp)
# t52 = v8_scope2104072363 * 2
	lw t0, -40(fp)
	li t1, 2
	mul t6, t0, t1
	sw t6, -340(fp)
# u8_scope2104072364 = t52
	lw t2, -340(fp)
	sw t2, -344(fp)
# t53 = v9_scope2104072363 * 2
	lw t4, -44(fp)
	li t5, 2
	mul t3, t4, t5
	sw t3, -348(fp)
# u9_scope2104072364 = t53
	lw t6, -348(fp)
	sw t6, -352(fp)
# t54 = v10_scope2104072363 * 2
	lw t1, -48(fp)
	li t2, 2
	mul t0, t1, t2
	sw t0, -356(fp)
# u10_scope2104072364 = t54
	lw t3, -356(fp)
	sw t3, -360(fp)
# t55 = v11_scope2104072363 * 2
	lw t5, -52(fp)
	li t6, 2
	mul t4, t5, t6
	sw t4, -364(fp)
# u11_scope2104072364 = t55
	lw t0, -364(fp)
	sw t0, -368(fp)
# t56 = v12_scope2104072363 * 2
	lw t2, -56(fp)
	li t3, 2
	mul t1, t2, t3
	sw t1, -372(fp)
# u12_scope2104072364 = t56
	lw t4, -372(fp)
	sw t4, -376(fp)
# t57 = v13_scope2104072363 * 2
	lw t6, -60(fp)
	li t0, 2
	mul t5, t6, t0
	sw t5, -380(fp)
# u13_scope2104072364 = t57
	lw t1, -380(fp)
	sw t1, -384(fp)
# t58 = v14_scope2104072363 * 2
	lw t3, -64(fp)
	li t4, 2
	mul t2, t3, t4
	sw t2, -388(fp)
# u14_scope2104072364 = t58
	lw t5, -388(fp)
	sw t5, -392(fp)
# t59 = v15_scope2104072363 * 2
	lw t0, -68(fp)
	li t1, 2
	mul t6, t0, t1
	sw t6, -396(fp)
# u15_scope2104072364 = t59
	lw t2, -396(fp)
	sw t2, -400(fp)
# t60 = v16_scope2104072363 * 2
	lw t4, -72(fp)
	li t5, 2
	mul t3, t4, t5
	sw t3, -404(fp)
# u16_scope2104072364 = t60
	lw t6, -404(fp)
	sw t6, -408(fp)
# t61 = v17_scope2104072363 * 2
	lw t1, -76(fp)
	li t2, 2
	mul t0, t1, t2
	sw t0, -412(fp)
# u17_scope2104072364 = t61
	lw t3, -412(fp)
	sw t3, -416(fp)
# t62 = v18_scope2104072363 * 2
	lw t5, -80(fp)
	li t6, 2
	mul t4, t5, t6
	sw t4, -420(fp)
# u18_scope2104072364 = t62
	lw t0, -420(fp)
	sw t0, -424(fp)
# t63 = v19_scope2104072363 * 2
	lw t2, -84(fp)
	li t3, 2
	mul t1, t2, t3
	sw t1, -428(fp)
# u19_scope2104072364 = t63
	lw t4, -428(fp)
	sw t4, -432(fp)
# t64 = v20_scope2104072363 * 2
	lw t6, -88(fp)
	li t0, 2
	mul t5, t6, t0
	sw t5, -436(fp)
# u20_scope2104072364 = t64
	lw t1, -436(fp)
	sw t1, -440(fp)
# t65 = v21_scope2104072363 * 2
	lw t3, -92(fp)
	li t4, 2
	mul t2, t3, t4
	sw t2, -444(fp)
# u21_scope2104072364 = t65
	lw t5, -444(fp)
	sw t5, -448(fp)
# t66 = v22_scope2104072363 * 2
	lw t0, -96(fp)
	li t1, 2
	mul t6, t0, t1
	sw t6, -452(fp)
# u22_scope2104072364 = t66
	lw t2, -452(fp)
	sw t2, -456(fp)
# t67 = v23_scope2104072363 * 2
	lw t4, -100(fp)
	li t5, 2
	mul t3, t4, t5
	sw t3, -460(fp)
# u23_scope2104072364 = t67
	lw t6, -460(fp)
	sw t6, -464(fp)
# t68 = v24_scope2104072363 * 2
	lw t1, -104(fp)
	li t2, 2
	mul t0, t1, t2
	sw t0, -468(fp)
# u24_scope2104072364 = t68
	lw t3, -468(fp)
	sw t3, -472(fp)
# t69 = v25_scope2104072363 * 2
	lw t5, -108(fp)
	li t6, 2
	mul t4, t5, t6
	sw t4, -476(fp)
# u25_scope2104072364 = t69
	lw t0, -476(fp)
	sw t0, -480(fp)
# t70 = v26_scope2104072363 * 2
	lw t2, -112(fp)
	li t3, 2
	mul t1, t2, t3
	sw t1, -484(fp)
# u26_scope2104072364 = t70
	lw t4, -484(fp)
	sw t4, -488(fp)
# t71 = v27_scope2104072363 * 2
	lw t6, -116(fp)
	li t0, 2
	mul t5, t6, t0
	sw t5, -492(fp)
# u27_scope2104072364 = t71
	lw t1, -492(fp)
	sw t1, -496(fp)
# t72 = v28_scope2104072363 * 2
	lw t3, -120(fp)
	li t4, 2
	mul t2, t3, t4
	sw t2, -500(fp)
# u28_scope2104072364 = t72
	lw t5, -500(fp)
	sw t5, -504(fp)
# t73 = v29_scope2104072363 * 2
	lw t0, -124(fp)
	li t1, 2
	mul t6, t0, t1
	sw t6, -508(fp)
# u29_scope2104072364 = t73
	lw t2, -508(fp)
	sw t2, -512(fp)
# t74 = v30_scope2104072363 * 2
	lw t4, -128(fp)
	li t5, 2
	mul t3, t4, t5
	sw t3, -516(fp)
# u30_scope2104072364 = t74
	lw t6, -516(fp)
	sw t6, -520(fp)
# t75 = v31_scope2104072363 * 2
	lw t1, -132(fp)
	li t2, 2
	mul t0, t1, t2
	sw t0, -524(fp)
# u31_scope2104072364 = t75
	lw t3, -524(fp)
	sw t3, -528(fp)
# t76 = v32_scope2104072363 * 2
	lw t5, -136(fp)
	li t6, 2
	mul t4, t5, t6
	sw t4, -532(fp)
# u32_scope2104072364 = t76
	lw t0, -532(fp)
	sw t0, -536(fp)
# t77 = u1_scope2104072364 + u2_scope2104072364
	lw t2, -288(fp)
	lw t3, -296(fp)
	add t1, t2, t3
	sw t1, -540(fp)
# t78 = t77 + u3_scope2104072364
	lw t5, -540(fp)
	lw t6, -304(fp)
	add t4, t5, t6
	sw t4, -544(fp)
# t79 = t78 + u4_scope2104072364
	lw t1, -544(fp)
	lw t2, -312(fp)
	add t0, t1, t2
	sw t0, -548(fp)
# t80 = t79 + u5_scope2104072364
	lw t4, -548(fp)
	lw t5, -320(fp)
	add t3, t4, t5
	sw t3, -552(fp)
# t81 = t80 + u6_scope2104072364
	lw t0, -552(fp)
	lw t1, -328(fp)
	add t6, t0, t1
	sw t6, -556(fp)
# t82 = t81 + u7_scope2104072364
	lw t3, -556(fp)
	lw t4, -336(fp)
	add t2, t3, t4
	sw t2, -560(fp)
# t83 = t82 + u8_scope2104072364
	lw t6, -560(fp)
	lw t0, -344(fp)
	add t5, t6, t0
	sw t5, -564(fp)
# sum5_scope2104072364 = t83
	lw t1, -564(fp)
	sw t1, -568(fp)
# t84 = u9_scope2104072364 + u10_scope2104072364
	lw t3, -352(fp)
	lw t4, -360(fp)
	add t2, t3, t4
	sw t2, -572(fp)
# t85 = t84 + u11_scope2104072364
	lw t6, -572(fp)
	lw t0, -368(fp)
	add t5, t6, t0
	sw t5, -576(fp)
# t86 = t85 + u12_scope2104072364
	lw t2, -576(fp)
	lw t3, -376(fp)
	add t1, t2, t3
	sw t1, -580(fp)
# t87 = t86 + u13_scope2104072364
	lw t5, -580(fp)
	lw t6, -384(fp)
	add t4, t5, t6
	sw t4, -584(fp)
# t88 = t87 + u14_scope2104072364
	lw t1, -584(fp)
	lw t2, -392(fp)
	add t0, t1, t2
	sw t0, -588(fp)
# t89 = t88 + u15_scope2104072364
	lw t4, -588(fp)
	lw t5, -400(fp)
	add t3, t4, t5
	sw t3, -592(fp)
# t90 = t89 + u16_scope2104072364
	lw t0, -592(fp)
	lw t1, -408(fp)
	add t6, t0, t1
	sw t6, -596(fp)
# sum6_scope2104072364 = t90
	lw t2, -596(fp)
	sw t2, -600(fp)
# t91 = u17_scope2104072364 + u18_scope2104072364
	lw t4, -416(fp)
	lw t5, -424(fp)
	add t3, t4, t5
	sw t3, -604(fp)
# t92 = t91 + u19_scope2104072364
	lw t0, -604(fp)
	lw t1, -432(fp)
	add t6, t0, t1
	sw t6, -608(fp)
# t93 = t92 + u20_scope2104072364
	lw t3, -608(fp)
	lw t4, -440(fp)
	add t2, t3, t4
	sw t2, -612(fp)
# t94 = t93 + u21_scope2104072364
	lw t6, -612(fp)
	lw t0, -448(fp)
	add t5, t6, t0
	sw t5, -616(fp)
# t95 = t94 + u22_scope2104072364
	lw t2, -616(fp)
	lw t3, -456(fp)
	add t1, t2, t3
	sw t1, -620(fp)
# t96 = t95 + u23_scope2104072364
	lw t5, -620(fp)
	lw t6, -464(fp)
	add t4, t5, t6
	sw t4, -624(fp)
# t97 = t96 + u24_scope2104072364
	lw t1, -624(fp)
	lw t2, -472(fp)
	add t0, t1, t2
	sw t0, -628(fp)
# sum7_scope2104072364 = t97
	lw t3, -628(fp)
	sw t3, -632(fp)
# t98 = u25_scope2104072364 + u26_scope2104072364
	lw t5, -480(fp)
	lw t6, -488(fp)
	add t4, t5, t6
	sw t4, -636(fp)
# t99 = t98 + u27_scope2104072364
	lw t1, -636(fp)
	lw t2, -496(fp)
	add t0, t1, t2
	sw t0, -640(fp)
# t100 = t99 + u28_scope2104072364
	lw t4, -640(fp)
	lw t5, -504(fp)
	add t3, t4, t5
	sw t3, -644(fp)
# t101 = t100 + u29_scope2104072364
	lw t0, -644(fp)
	lw t1, -512(fp)
	add t6, t0, t1
	sw t6, -648(fp)
# t102 = t101 + u30_scope2104072364
	lw t3, -648(fp)
	lw t4, -520(fp)
	add t2, t3, t4
	sw t2, -652(fp)
# t103 = t102 + u31_scope2104072364
	lw t6, -652(fp)
	lw t0, -528(fp)
	add t5, t6, t0
	sw t5, -656(fp)
# t104 = t103 + u32_scope2104072364
	lw t2, -656(fp)
	lw t3, -536(fp)
	add t1, t2, t3
	sw t1, -660(fp)
# sum8_scope2104072364 = t104
	lw t4, -660(fp)
	sw t4, -664(fp)
# t105 = sum5_scope2104072364 + sum6_scope2104072364
	lw t6, -568(fp)
	lw t0, -600(fp)
	add t5, t6, t0
	sw t5, -668(fp)
# t106 = t105 + sum7_scope2104072364
	lw t2, -668(fp)
	lw t3, -632(fp)
	add t1, t2, t3
	sw t1, -672(fp)
# t107 = t106 + sum8_scope2104072364
	lw t5, -672(fp)
	lw t6, -664(fp)
	add t4, t5, t6
	sw t4, -676(fp)
# result2_scope2104072364 = t107
	lw t0, -676(fp)
	sw t0, -680(fp)
# t108 = u1_scope2104072364 + v1_scope2104072363
	lw t2, -288(fp)
	lw t3, -12(fp)
	add t1, t2, t3
	sw t1, -684(fp)
# w1_scope2104072365 = t108
	lw t4, -684(fp)
	sw t4, -688(fp)
# t109 = u2_scope2104072364 + v2_scope2104072363
	lw t6, -296(fp)
	lw t0, -16(fp)
	add t5, t6, t0
	sw t5, -692(fp)
# w2_scope2104072365 = t109
	lw t1, -692(fp)
	sw t1, -696(fp)
# t110 = u3_scope2104072364 + v3_scope2104072363
	lw t3, -304(fp)
	lw t4, -20(fp)
	add t2, t3, t4
	sw t2, -700(fp)
# w3_scope2104072365 = t110
	lw t5, -700(fp)
	sw t5, -704(fp)
# t111 = u4_scope2104072364 + v4_scope2104072363
	lw t0, -312(fp)
	lw t1, -24(fp)
	add t6, t0, t1
	sw t6, -708(fp)
# w4_scope2104072365 = t111
	lw t2, -708(fp)
	sw t2, -712(fp)
# t112 = u5_scope2104072364 + v5_scope2104072363
	lw t4, -320(fp)
	lw t5, -28(fp)
	add t3, t4, t5
	sw t3, -716(fp)
# w5_scope2104072365 = t112
	lw t6, -716(fp)
	sw t6, -720(fp)
# t113 = u6_scope2104072364 + v6_scope2104072363
	lw t1, -328(fp)
	lw t2, -32(fp)
	add t0, t1, t2
	sw t0, -724(fp)
# w6_scope2104072365 = t113
	lw t3, -724(fp)
	sw t3, -728(fp)
# t114 = u7_scope2104072364 + v7_scope2104072363
	lw t5, -336(fp)
	lw t6, -36(fp)
	add t4, t5, t6
	sw t4, -732(fp)
# w7_scope2104072365 = t114
	lw t0, -732(fp)
	sw t0, -736(fp)
# t115 = u8_scope2104072364 + v8_scope2104072363
	lw t2, -344(fp)
	lw t3, -40(fp)
	add t1, t2, t3
	sw t1, -740(fp)
# w8_scope2104072365 = t115
	lw t4, -740(fp)
	sw t4, -744(fp)
# t116 = u9_scope2104072364 + v9_scope2104072363
	lw t6, -352(fp)
	lw t0, -44(fp)
	add t5, t6, t0
	sw t5, -748(fp)
# w9_scope2104072365 = t116
	lw t1, -748(fp)
	sw t1, -752(fp)
# t117 = u10_scope2104072364 + v10_scope2104072363
	lw t3, -360(fp)
	lw t4, -48(fp)
	add t2, t3, t4
	sw t2, -756(fp)
# w10_scope2104072365 = t117
	lw t5, -756(fp)
	sw t5, -760(fp)
# t118 = u11_scope2104072364 + v11_scope2104072363
	lw t0, -368(fp)
	lw t1, -52(fp)
	add t6, t0, t1
	sw t6, -764(fp)
# w11_scope2104072365 = t118
	lw t2, -764(fp)
	sw t2, -768(fp)
# t119 = u12_scope2104072364 + v12_scope2104072363
	lw t4, -376(fp)
	lw t5, -56(fp)
	add t3, t4, t5
	sw t3, -772(fp)
# w12_scope2104072365 = t119
	lw t6, -772(fp)
	sw t6, -776(fp)
# t120 = u13_scope2104072364 + v13_scope2104072363
	lw t1, -384(fp)
	lw t2, -60(fp)
	add t0, t1, t2
	sw t0, -780(fp)
# w13_scope2104072365 = t120
	lw t3, -780(fp)
	sw t3, -784(fp)
# t121 = u14_scope2104072364 + v14_scope2104072363
	lw t5, -392(fp)
	lw t6, -64(fp)
	add t4, t5, t6
	sw t4, -788(fp)
# w14_scope2104072365 = t121
	lw t0, -788(fp)
	sw t0, -792(fp)
# t122 = u15_scope2104072364 + v15_scope2104072363
	lw t2, -400(fp)
	lw t3, -68(fp)
	add t1, t2, t3
	sw t1, -796(fp)
# w15_scope2104072365 = t122
	lw t4, -796(fp)
	sw t4, -800(fp)
# t123 = u16_scope2104072364 + v16_scope2104072363
	lw t6, -408(fp)
	lw t0, -72(fp)
	add t5, t6, t0
	sw t5, -804(fp)
# w16_scope2104072365 = t123
	lw t1, -804(fp)
	sw t1, -808(fp)
# t124 = u17_scope2104072364 + v17_scope2104072363
	lw t3, -416(fp)
	lw t4, -76(fp)
	add t2, t3, t4
	sw t2, -812(fp)
# w17_scope2104072365 = t124
	lw t5, -812(fp)
	sw t5, -816(fp)
# t125 = u18_scope2104072364 + v18_scope2104072363
	lw t0, -424(fp)
	lw t1, -80(fp)
	add t6, t0, t1
	sw t6, -820(fp)
# w18_scope2104072365 = t125
	lw t2, -820(fp)
	sw t2, -824(fp)
# t126 = u19_scope2104072364 + v19_scope2104072363
	lw t4, -432(fp)
	lw t5, -84(fp)
	add t3, t4, t5
	sw t3, -828(fp)
# w19_scope2104072365 = t126
	lw t6, -828(fp)
	sw t6, -832(fp)
# t127 = u20_scope2104072364 + v20_scope2104072363
	lw t1, -440(fp)
	lw t2, -88(fp)
	add t0, t1, t2
	sw t0, -836(fp)
# w20_scope2104072365 = t127
	lw t3, -836(fp)
	sw t3, -840(fp)
# t128 = u21_scope2104072364 + v21_scope2104072363
	lw t5, -448(fp)
	lw t6, -92(fp)
	add t4, t5, t6
	sw t4, -844(fp)
# w21_scope2104072365 = t128
	lw t0, -844(fp)
	sw t0, -848(fp)
# t129 = u22_scope2104072364 + v22_scope2104072363
	lw t2, -456(fp)
	lw t3, -96(fp)
	add t1, t2, t3
	sw t1, -852(fp)
# w22_scope2104072365 = t129
	lw t4, -852(fp)
	sw t4, -856(fp)
# t130 = u23_scope2104072364 + v23_scope2104072363
	lw t6, -464(fp)
	lw t0, -100(fp)
	add t5, t6, t0
	sw t5, -860(fp)
# w23_scope2104072365 = t130
	lw t1, -860(fp)
	sw t1, -864(fp)
# t131 = u24_scope2104072364 + v24_scope2104072363
	lw t3, -472(fp)
	lw t4, -104(fp)
	add t2, t3, t4
	sw t2, -868(fp)
# w24_scope2104072365 = t131
	lw t5, -868(fp)
	sw t5, -872(fp)
# t132 = u25_scope2104072364 + v25_scope2104072363
	lw t0, -480(fp)
	lw t1, -108(fp)
	add t6, t0, t1
	sw t6, -876(fp)
# w25_scope2104072365 = t132
	lw t2, -876(fp)
	sw t2, -880(fp)
# t133 = u26_scope2104072364 + v26_scope2104072363
	lw t4, -488(fp)
	lw t5, -112(fp)
	add t3, t4, t5
	sw t3, -884(fp)
# w26_scope2104072365 = t133
	lw t6, -884(fp)
	sw t6, -888(fp)
# t134 = u27_scope2104072364 + v27_scope2104072363
	lw t1, -496(fp)
	lw t2, -116(fp)
	add t0, t1, t2
	sw t0, -892(fp)
# w27_scope2104072365 = t134
	lw t3, -892(fp)
	sw t3, -896(fp)
# t135 = u28_scope2104072364 + v28_scope2104072363
	lw t5, -504(fp)
	lw t6, -120(fp)
	add t4, t5, t6
	sw t4, -900(fp)
# w28_scope2104072365 = t135
	lw t0, -900(fp)
	sw t0, -904(fp)
# t136 = u29_scope2104072364 + v29_scope2104072363
	lw t2, -512(fp)
	lw t3, -124(fp)
	add t1, t2, t3
	sw t1, -908(fp)
# w29_scope2104072365 = t136
	lw t4, -908(fp)
	sw t4, -912(fp)
# t137 = u30_scope2104072364 + v30_scope2104072363
	lw t6, -520(fp)
	lw t0, -128(fp)
	add t5, t6, t0
	sw t5, -916(fp)
# w30_scope2104072365 = t137
	lw t1, -916(fp)
	sw t1, -920(fp)
# t138 = u31_scope2104072364 + v31_scope2104072363
	lw t3, -528(fp)
	lw t4, -132(fp)
	add t2, t3, t4
	sw t2, -924(fp)
# w31_scope2104072365 = t138
	lw t5, -924(fp)
	sw t5, -928(fp)
# t139 = u32_scope2104072364 + v32_scope2104072363
	lw t0, -536(fp)
	lw t1, -136(fp)
	add t6, t0, t1
	sw t6, -932(fp)
# w32_scope2104072365 = t139
	lw t2, -932(fp)
	sw t2, -936(fp)
# t140 = w1_scope2104072365 + w2_scope2104072365
	lw t4, -688(fp)
	lw t5, -696(fp)
	add t3, t4, t5
	sw t3, -940(fp)
# t141 = t140 + w3_scope2104072365
	lw t0, -940(fp)
	lw t1, -704(fp)
	add t6, t0, t1
	sw t6, -944(fp)
# t142 = t141 + w4_scope2104072365
	lw t3, -944(fp)
	lw t4, -712(fp)
	add t2, t3, t4
	sw t2, -948(fp)
# t143 = t142 + w5_scope2104072365
	lw t6, -948(fp)
	lw t0, -720(fp)
	add t5, t6, t0
	sw t5, -952(fp)
# t144 = t143 + w6_scope2104072365
	lw t2, -952(fp)
	lw t3, -728(fp)
	add t1, t2, t3
	sw t1, -956(fp)
# t145 = t144 + w7_scope2104072365
	lw t5, -956(fp)
	lw t6, -736(fp)
	add t4, t5, t6
	sw t4, -960(fp)
# t146 = t145 + w8_scope2104072365
	lw t1, -960(fp)
	lw t2, -744(fp)
	add t0, t1, t2
	sw t0, -964(fp)
# sum9_scope2104072365 = t146
	lw t3, -964(fp)
	sw t3, -968(fp)
# t147 = w9_scope2104072365 + w10_scope2104072365
	lw t5, -752(fp)
	lw t6, -760(fp)
	add t4, t5, t6
	sw t4, -972(fp)
# t148 = t147 + w11_scope2104072365
	lw t1, -972(fp)
	lw t2, -768(fp)
	add t0, t1, t2
	sw t0, -976(fp)
# t149 = t148 + w12_scope2104072365
	lw t4, -976(fp)
	lw t5, -776(fp)
	add t3, t4, t5
	sw t3, -980(fp)
# t150 = t149 + w13_scope2104072365
	lw t0, -980(fp)
	lw t1, -784(fp)
	add t6, t0, t1
	sw t6, -984(fp)
# t151 = t150 + w14_scope2104072365
	lw t3, -984(fp)
	lw t4, -792(fp)
	add t2, t3, t4
	sw t2, -988(fp)
# t152 = t151 + w15_scope2104072365
	lw t6, -988(fp)
	lw t0, -800(fp)
	add t5, t6, t0
	sw t5, -992(fp)
# t153 = t152 + w16_scope2104072365
	lw t2, -992(fp)
	lw t3, -808(fp)
	add t1, t2, t3
	sw t1, -996(fp)
# sum10_scope2104072365 = t153
	lw t4, -996(fp)
	sw t4, -1000(fp)
# t154 = w17_scope2104072365 + w18_scope2104072365
	lw t6, -816(fp)
	lw t0, -824(fp)
	add t5, t6, t0
	sw t5, -1004(fp)
# t155 = t154 + w19_scope2104072365
	lw t2, -1004(fp)
	lw t3, -832(fp)
	add t1, t2, t3
	sw t1, -1008(fp)
# t156 = t155 + w20_scope2104072365
	lw t5, -1008(fp)
	lw t6, -840(fp)
	add t4, t5, t6
	sw t4, -1012(fp)
# t157 = t156 + w21_scope2104072365
	lw t1, -1012(fp)
	lw t2, -848(fp)
	add t0, t1, t2
	sw t0, -1016(fp)
# t158 = t157 + w22_scope2104072365
	lw t4, -1016(fp)
	lw t5, -856(fp)
	add t3, t4, t5
	sw t3, -1020(fp)
# t159 = t158 + w23_scope2104072365
	lw t0, -1020(fp)
	lw t1, -864(fp)
	add t6, t0, t1
	sw t6, -1024(fp)
# t160 = t159 + w24_scope2104072365
	lw t3, -1024(fp)
	lw t4, -872(fp)
	add t2, t3, t4
	sw t2, -1028(fp)
# sum11_scope2104072365 = t160
	lw t5, -1028(fp)
	sw t5, -1032(fp)
# t161 = w25_scope2104072365 + w26_scope2104072365
	lw t0, -880(fp)
	lw t1, -888(fp)
	add t6, t0, t1
	sw t6, -1036(fp)
# t162 = t161 + w27_scope2104072365
	lw t3, -1036(fp)
	lw t4, -896(fp)
	add t2, t3, t4
	sw t2, -1040(fp)
# t163 = t162 + w28_scope2104072365
	lw t6, -1040(fp)
	lw t0, -904(fp)
	add t5, t6, t0
	sw t5, -1044(fp)
# t164 = t163 + w29_scope2104072365
	lw t2, -1044(fp)
	lw t3, -912(fp)
	add t1, t2, t3
	sw t1, -1048(fp)
# t165 = t164 + w30_scope2104072365
	lw t5, -1048(fp)
	lw t6, -920(fp)
	add t4, t5, t6
	sw t4, -1052(fp)
# t166 = t165 + w31_scope2104072365
	lw t1, -1052(fp)
	lw t2, -928(fp)
	add t0, t1, t2
	sw t0, -1056(fp)
# t167 = t166 + w32_scope2104072365
	lw t4, -1056(fp)
	lw t5, -936(fp)
	add t3, t4, t5
	sw t3, -1060(fp)
# sum12_scope2104072365 = t167
	lw t6, -1060(fp)
	sw t6, -1064(fp)
# t168 = sum9_scope2104072365 + sum10_scope2104072365
	lw t1, -968(fp)
	lw t2, -1000(fp)
	add t0, t1, t2
	sw t0, -1068(fp)
# t169 = t168 + sum11_scope2104072365
	lw t4, -1068(fp)
	lw t5, -1032(fp)
	add t3, t4, t5
	sw t3, -1072(fp)
# t170 = t169 + sum12_scope2104072365
	lw t0, -1072(fp)
	lw t1, -1064(fp)
	add t6, t0, t1
	sw t6, -1076(fp)
# result3_scope2104072365 = t170
	lw t2, -1076(fp)
	sw t2, -1080(fp)
# t171 = w1_scope2104072365 + 1
	lw t4, -688(fp)
	li t5, 1
	add t3, t4, t5
	sw t3, -1084(fp)
# x1_scope2104072366 = t171
	lw t6, -1084(fp)
	sw t6, -1088(fp)
# t172 = w2_scope2104072365 + 1
	lw t1, -696(fp)
	li t2, 1
	add t0, t1, t2
	sw t0, -1092(fp)
# x2_scope2104072366 = t172
	lw t3, -1092(fp)
	sw t3, -1096(fp)
# t173 = w3_scope2104072365 + 1
	lw t5, -704(fp)
	li t6, 1
	add t4, t5, t6
	sw t4, -1100(fp)
# x3_scope2104072366 = t173
	lw t0, -1100(fp)
	sw t0, -1104(fp)
# t174 = w4_scope2104072365 + 1
	lw t2, -712(fp)
	li t3, 1
	add t1, t2, t3
	sw t1, -1108(fp)
# x4_scope2104072366 = t174
	lw t4, -1108(fp)
	sw t4, -1112(fp)
# t175 = w5_scope2104072365 + 1
	lw t6, -720(fp)
	li t0, 1
	add t5, t6, t0
	sw t5, -1116(fp)
# x5_scope2104072366 = t175
	lw t1, -1116(fp)
	sw t1, -1120(fp)
# t176 = w6_scope2104072365 + 1
	lw t3, -728(fp)
	li t4, 1
	add t2, t3, t4
	sw t2, -1124(fp)
# x6_scope2104072366 = t176
	lw t5, -1124(fp)
	sw t5, -1128(fp)
# t177 = w7_scope2104072365 + 1
	lw t0, -736(fp)
	li t1, 1
	add t6, t0, t1
	sw t6, -1132(fp)
# x7_scope2104072366 = t177
	lw t2, -1132(fp)
	sw t2, -1136(fp)
# t178 = w8_scope2104072365 + 1
	lw t4, -744(fp)
	li t5, 1
	add t3, t4, t5
	sw t3, -1140(fp)
# x8_scope2104072366 = t178
	lw t6, -1140(fp)
	sw t6, -1144(fp)
# t179 = w9_scope2104072365 + 1
	lw t1, -752(fp)
	li t2, 1
	add t0, t1, t2
	sw t0, -1148(fp)
# x9_scope2104072366 = t179
	lw t3, -1148(fp)
	sw t3, -1152(fp)
# t180 = w10_scope2104072365 + 1
	lw t5, -760(fp)
	li t6, 1
	add t4, t5, t6
	sw t4, -1156(fp)
# x10_scope2104072366 = t180
	lw t0, -1156(fp)
	sw t0, -1160(fp)
# t181 = w11_scope2104072365 + 1
	lw t2, -768(fp)
	li t3, 1
	add t1, t2, t3
	sw t1, -1164(fp)
# x11_scope2104072366 = t181
	lw t4, -1164(fp)
	sw t4, -1168(fp)
# t182 = w12_scope2104072365 + 1
	lw t6, -776(fp)
	li t0, 1
	add t5, t6, t0
	sw t5, -1172(fp)
# x12_scope2104072366 = t182
	lw t1, -1172(fp)
	sw t1, -1176(fp)
# t183 = w13_scope2104072365 + 1
	lw t3, -784(fp)
	li t4, 1
	add t2, t3, t4
	sw t2, -1180(fp)
# x13_scope2104072366 = t183
	lw t5, -1180(fp)
	sw t5, -1184(fp)
# t184 = w14_scope2104072365 + 1
	lw t0, -792(fp)
	li t1, 1
	add t6, t0, t1
	sw t6, -1188(fp)
# x14_scope2104072366 = t184
	lw t2, -1188(fp)
	sw t2, -1192(fp)
# t185 = w15_scope2104072365 + 1
	lw t4, -800(fp)
	li t5, 1
	add t3, t4, t5
	sw t3, -1196(fp)
# x15_scope2104072366 = t185
	lw t6, -1196(fp)
	sw t6, -1200(fp)
# t186 = w16_scope2104072365 + 1
	lw t1, -808(fp)
	li t2, 1
	add t0, t1, t2
	sw t0, -1204(fp)
# x16_scope2104072366 = t186
	lw t3, -1204(fp)
	sw t3, -1208(fp)
# t187 = w17_scope2104072365 + 1
	lw t5, -816(fp)
	li t6, 1
	add t4, t5, t6
	sw t4, -1212(fp)
# x17_scope2104072366 = t187
	lw t0, -1212(fp)
	sw t0, -1216(fp)
# t188 = w18_scope2104072365 + 1
	lw t2, -824(fp)
	li t3, 1
	add t1, t2, t3
	sw t1, -1220(fp)
# x18_scope2104072366 = t188
	lw t4, -1220(fp)
	sw t4, -1224(fp)
# t189 = w19_scope2104072365 + 1
	lw t6, -832(fp)
	li t0, 1
	add t5, t6, t0
	sw t5, -1228(fp)
# x19_scope2104072366 = t189
	lw t1, -1228(fp)
	sw t1, -1232(fp)
# t190 = w20_scope2104072365 + 1
	lw t3, -840(fp)
	li t4, 1
	add t2, t3, t4
	sw t2, -1236(fp)
# x20_scope2104072366 = t190
	lw t5, -1236(fp)
	sw t5, -1240(fp)
# t191 = w21_scope2104072365 + 1
	lw t0, -848(fp)
	li t1, 1
	add t6, t0, t1
	sw t6, -1244(fp)
# x21_scope2104072366 = t191
	lw t2, -1244(fp)
	sw t2, -1248(fp)
# t192 = w22_scope2104072365 + 1
	lw t4, -856(fp)
	li t5, 1
	add t3, t4, t5
	sw t3, -1252(fp)
# x22_scope2104072366 = t192
	lw t6, -1252(fp)
	sw t6, -1256(fp)
# t193 = w23_scope2104072365 + 1
	lw t1, -864(fp)
	li t2, 1
	add t0, t1, t2
	sw t0, -1260(fp)
# x23_scope2104072366 = t193
	lw t3, -1260(fp)
	sw t3, -1264(fp)
# t194 = w24_scope2104072365 + 1
	lw t5, -872(fp)
	li t6, 1
	add t4, t5, t6
	sw t4, -1268(fp)
# x24_scope2104072366 = t194
	lw t0, -1268(fp)
	sw t0, -1272(fp)
# t195 = w25_scope2104072365 + 1
	lw t2, -880(fp)
	li t3, 1
	add t1, t2, t3
	sw t1, -1276(fp)
# x25_scope2104072366 = t195
	lw t4, -1276(fp)
	sw t4, -1280(fp)
# t196 = w26_scope2104072365 + 1
	lw t6, -888(fp)
	li t0, 1
	add t5, t6, t0
	sw t5, -1284(fp)
# x26_scope2104072366 = t196
	lw t1, -1284(fp)
	sw t1, -1288(fp)
# t197 = w27_scope2104072365 + 1
	lw t3, -896(fp)
	li t4, 1
	add t2, t3, t4
	sw t2, -1292(fp)
# x27_scope2104072366 = t197
	lw t5, -1292(fp)
	sw t5, -1296(fp)
# t198 = w28_scope2104072365 + 1
	lw t0, -904(fp)
	li t1, 1
	add t6, t0, t1
	sw t6, -1300(fp)
# x28_scope2104072366 = t198
	lw t2, -1300(fp)
	sw t2, -1304(fp)
# t199 = w29_scope2104072365 + 1
	lw t4, -912(fp)
	li t5, 1
	add t3, t4, t5
	sw t3, -1308(fp)
# x29_scope2104072366 = t199
	lw t6, -1308(fp)
	sw t6, -1312(fp)
# t200 = w30_scope2104072365 + 1
	lw t1, -920(fp)
	li t2, 1
	add t0, t1, t2
	sw t0, -1316(fp)
# x30_scope2104072366 = t200
	lw t3, -1316(fp)
	sw t3, -1320(fp)
# t201 = w31_scope2104072365 + 1
	lw t5, -928(fp)
	li t6, 1
	add t4, t5, t6
	sw t4, -1324(fp)
# x31_scope2104072366 = t201
	lw t0, -1324(fp)
	sw t0, -1328(fp)
# t202 = w32_scope2104072365 + 1
	lw t2, -936(fp)
	li t3, 1
	add t1, t2, t3
	sw t1, -1332(fp)
# x32_scope2104072366 = t202
	lw t4, -1332(fp)
	sw t4, -1336(fp)
# t203 = x1_scope2104072366 + x2_scope2104072366
	lw t6, -1088(fp)
	lw t0, -1096(fp)
	add t5, t6, t0
	sw t5, -1340(fp)
# t204 = t203 + x3_scope2104072366
	lw t2, -1340(fp)
	lw t3, -1104(fp)
	add t1, t2, t3
	sw t1, -1344(fp)
# t205 = t204 + x4_scope2104072366
	lw t5, -1344(fp)
	lw t6, -1112(fp)
	add t4, t5, t6
	sw t4, -1348(fp)
# t206 = t205 + x5_scope2104072366
	lw t1, -1348(fp)
	lw t2, -1120(fp)
	add t0, t1, t2
	sw t0, -1352(fp)
# t207 = t206 + x6_scope2104072366
	lw t4, -1352(fp)
	lw t5, -1128(fp)
	add t3, t4, t5
	sw t3, -1356(fp)
# t208 = t207 + x7_scope2104072366
	lw t0, -1356(fp)
	lw t1, -1136(fp)
	add t6, t0, t1
	sw t6, -1360(fp)
# t209 = t208 + x8_scope2104072366
	lw t3, -1360(fp)
	lw t4, -1144(fp)
	add t2, t3, t4
	sw t2, -1364(fp)
# sum13_scope2104072366 = t209
	lw t5, -1364(fp)
	sw t5, -1368(fp)
# t210 = x9_scope2104072366 + x10_scope2104072366
	lw t0, -1152(fp)
	lw t1, -1160(fp)
	add t6, t0, t1
	sw t6, -1372(fp)
# t211 = t210 + x11_scope2104072366
	lw t3, -1372(fp)
	lw t4, -1168(fp)
	add t2, t3, t4
	sw t2, -1376(fp)
# t212 = t211 + x12_scope2104072366
	lw t6, -1376(fp)
	lw t0, -1176(fp)
	add t5, t6, t0
	sw t5, -1380(fp)
# t213 = t212 + x13_scope2104072366
	lw t2, -1380(fp)
	lw t3, -1184(fp)
	add t1, t2, t3
	sw t1, -1384(fp)
# t214 = t213 + x14_scope2104072366
	lw t5, -1384(fp)
	lw t6, -1192(fp)
	add t4, t5, t6
	sw t4, -1388(fp)
# t215 = t214 + x15_scope2104072366
	lw t1, -1388(fp)
	lw t2, -1200(fp)
	add t0, t1, t2
	sw t0, -1392(fp)
# t216 = t215 + x16_scope2104072366
	lw t4, -1392(fp)
	lw t5, -1208(fp)
	add t3, t4, t5
	sw t3, -1396(fp)
# sum14_scope2104072366 = t216
	lw t6, -1396(fp)
	sw t6, -1400(fp)
# t217 = x17_scope2104072366 + x18_scope2104072366
	lw t1, -1216(fp)
	lw t2, -1224(fp)
	add t0, t1, t2
	sw t0, -1404(fp)
# t218 = t217 + x19_scope2104072366
	lw t4, -1404(fp)
	lw t5, -1232(fp)
	add t3, t4, t5
	sw t3, -1408(fp)
# t219 = t218 + x20_scope2104072366
	lw t0, -1408(fp)
	lw t1, -1240(fp)
	add t6, t0, t1
	sw t6, -1412(fp)
# t220 = t219 + x21_scope2104072366
	lw t3, -1412(fp)
	lw t4, -1248(fp)
	add t2, t3, t4
	sw t2, -1416(fp)
# t221 = t220 + x22_scope2104072366
	lw t6, -1416(fp)
	lw t0, -1256(fp)
	add t5, t6, t0
	sw t5, -1420(fp)
# t222 = t221 + x23_scope2104072366
	lw t2, -1420(fp)
	lw t3, -1264(fp)
	add t1, t2, t3
	sw t1, -1424(fp)
# t223 = t222 + x24_scope2104072366
	lw t5, -1424(fp)
	lw t6, -1272(fp)
	add t4, t5, t6
	sw t4, -1428(fp)
# sum15_scope2104072366 = t223
	lw t0, -1428(fp)
	sw t0, -1432(fp)
# t224 = x25_scope2104072366 + x26_scope2104072366
	lw t2, -1280(fp)
	lw t3, -1288(fp)
	add t1, t2, t3
	sw t1, -1436(fp)
# t225 = t224 + x27_scope2104072366
	lw t5, -1436(fp)
	lw t6, -1296(fp)
	add t4, t5, t6
	sw t4, -1440(fp)
# t226 = t225 + x28_scope2104072366
	lw t1, -1440(fp)
	lw t2, -1304(fp)
	add t0, t1, t2
	sw t0, -1444(fp)
# t227 = t226 + x29_scope2104072366
	lw t4, -1444(fp)
	lw t5, -1312(fp)
	add t3, t4, t5
	sw t3, -1448(fp)
# t228 = t227 + x30_scope2104072366
	lw t0, -1448(fp)
	lw t1, -1320(fp)
	add t6, t0, t1
	sw t6, -1452(fp)
# t229 = t228 + x31_scope2104072366
	lw t3, -1452(fp)
	lw t4, -1328(fp)
	add t2, t3, t4
	sw t2, -1456(fp)
# t230 = t229 + x32_scope2104072366
	lw t6, -1456(fp)
	lw t0, -1336(fp)
	add t5, t6, t0
	sw t5, -1460(fp)
# sum16_scope2104072366 = t230
	lw t1, -1460(fp)
	sw t1, -1464(fp)
# t231 = sum13_scope2104072366 + sum14_scope2104072366
	lw t3, -1368(fp)
	lw t4, -1400(fp)
	add t2, t3, t4
	sw t2, -1468(fp)
# t232 = t231 + sum15_scope2104072366
	lw t6, -1468(fp)
	lw t0, -1432(fp)
	add t5, t6, t0
	sw t5, -1472(fp)
# t233 = t232 + sum16_scope2104072366
	lw t2, -1472(fp)
	lw t3, -1464(fp)
	add t1, t2, t3
	sw t1, -1476(fp)
# result4_scope2104072366 = t233
	lw t4, -1476(fp)
	sw t4, -1480(fp)
# param result1_scope2104072363
# param result2_scope2104072364
# param result3_scope2104072365
# param result4_scope2104072366
# param sum1_scope2104072363
# param sum5_scope2104072364
# param sum9_scope2104072365
# param sum13_scope2104072366
# t234 = call compute, 8
# 保存调用者保存的寄存器
	sw t0, -1484(fp)
	sw t1, -1488(fp)
	sw t2, -1492(fp)
	sw t3, -1496(fp)
	sw t4, -1500(fp)
	lw a0, -280(fp)
	lw a1, -680(fp)
	lw a2, -1080(fp)
	lw a3, -1480(fp)
	lw a4, -168(fp)
	lw a5, -568(fp)
	lw a6, -968(fp)
	lw a7, -1368(fp)
	call compute
# 恢复调用者保存的寄存器
	lw t0, -1484(fp)
	lw t1, -1488(fp)
	lw t2, -1492(fp)
	lw t3, -1496(fp)
	lw t4, -1500(fp)
	addi t5, a0, 0
	sw t5, -1504(fp)
# final_result_scope2104072366 = t234
	lw t6, -1504(fp)
	sw t6, -1508(fp)
# return final_result_scope2104072366
	lw a0, -1508(fp)
	j main_epilogue
main_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2296
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2300
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2304
	add sp, sp, t0
	ret
