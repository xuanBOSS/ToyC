# 由ToyC编译器生成
# RISC-V汇编代码
.text
	.global fibonacci
fibonacci:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
# t0 = n <= 1
	lw t1, -12(fp)
	li t2, 1
	slt t0, t2, t1
	xori t0, t0, 1
	sw t0, -16(fp)
# t1 = !t0
	lw t4, -16(fp)
	seqz t3, t4
	sw t3, -20(fp)
# if t1 goto L0
	lw t5, -20(fp)
	bnez t5, L0
# return n
	lw a0, -12(fp)
	j fibonacci_epilogue
# goto L1
	j L1
L0:
# t2 = n - 2
	lw t0, -12(fp)
	li t1, 2
	sub t6, t0, t1
	sw t6, -24(fp)
# param t2
# t3 = call fibonacci, 1
# 保存调用者保存的寄存器
	sw t0, -28(fp)
	sw t1, -32(fp)
	lw a0, -24(fp)
	call fibonacci
# 恢复调用者保存的寄存器
	lw t0, -28(fp)
	lw t1, -32(fp)
	addi t2, a0, 0
	sw t2, -36(fp)
# t4 = n - 1
	lw t4, -12(fp)
	li t5, 1
	sub t3, t4, t5
	sw t3, -40(fp)
# param t4
# t5 = call fibonacci, 1
# 保存调用者保存的寄存器
	sw t0, -28(fp)
	sw t1, -32(fp)
	sw t2, -44(fp)
	sw t3, -48(fp)
	sw t4, -52(fp)
	sw t5, -56(fp)
	lw a0, -40(fp)
	call fibonacci
# 恢复调用者保存的寄存器
	lw t0, -28(fp)
	lw t1, -32(fp)
	lw t2, -44(fp)
	lw t3, -48(fp)
	lw t4, -52(fp)
	lw t5, -56(fp)
	addi t6, a0, 0
	sw t6, -60(fp)
# t6 = t5 + t3
	lw t1, -60(fp)
	lw t2, -36(fp)
	add t0, t1, t2
	sw t0, -64(fp)
# return t6
	lw a0, -64(fp)
	j fibonacci_epilogue
L1:
fibonacci_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
	.global gcd
gcd:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
	sw a1, -16(fp)
# t7 = b == 0
	lw t4, -16(fp)
	li t5, 0
	xor t3, t4, t5
	seqz t3, t3
	sw t3, -20(fp)
# t8 = !t7
	lw t0, -20(fp)
	seqz t6, t0
	sw t6, -24(fp)
# if t8 goto L2
	lw t1, -24(fp)
	bnez t1, L2
# return a
	lw a0, -12(fp)
	j gcd_epilogue
L2:
# t9 = a % b
	lw t3, -12(fp)
	lw t4, -16(fp)
	rem t2, t3, t4
	sw t2, -28(fp)
# param b
# param t9
# t10 = call gcd, 2
# 保存调用者保存的寄存器
	sw t0, -32(fp)
	sw t1, -36(fp)
	sw t2, -40(fp)
	sw t3, -44(fp)
	sw t4, -48(fp)
	lw a0, -16(fp)
	lw a1, -28(fp)
	call gcd
# 恢复调用者保存的寄存器
	lw t0, -32(fp)
	lw t1, -36(fp)
	lw t2, -40(fp)
	lw t3, -44(fp)
	lw t4, -48(fp)
	addi t5, a0, 0
	sw t5, -52(fp)
# return t10
	lw a0, -52(fp)
	j gcd_epilogue
gcd_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
	.global isPrime
isPrime:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
# t11 = num <= 1
	lw t0, -12(fp)
	li t1, 1
	slt t6, t1, t0
	xori t6, t6, 1
	sw t6, -16(fp)
# t12 = !t11
	lw t3, -16(fp)
	seqz t2, t3
	sw t2, -20(fp)
# if t12 goto L3
	lw t4, -20(fp)
	bnez t4, L3
# return 0
	li a0, 0
	j isPrime_epilogue
L3:
# t13 = num <= 3
	lw t6, -12(fp)
	li t0, 3
	slt t5, t0, t6
	xori t5, t5, 1
	sw t5, -24(fp)
# t14 = !t13
	lw t2, -24(fp)
	seqz t1, t2
	sw t1, -28(fp)
# if t14 goto L4
	lw t3, -28(fp)
	bnez t3, L4
# return 1
	li a0, 1
	j isPrime_epilogue
L4:
# t15 = num % 2
	lw t5, -12(fp)
	li t6, 2
	rem t4, t5, t6
	sw t4, -32(fp)
# t16 = t15 == 0
	lw t1, -32(fp)
	li t2, 0
	xor t0, t1, t2
	seqz t0, t0
	sw t0, -36(fp)
# if t16 goto L6
	lw t3, -36(fp)
	bnez t3, L6
# t18 = num % 3
	lw t5, -12(fp)
	li t6, 3
	rem t4, t5, t6
	sw t4, -40(fp)
# t19 = t18 == 0
	lw t1, -40(fp)
	li t2, 0
	xor t0, t1, t2
	seqz t0, t0
	sw t0, -44(fp)
# t17 = t19
	lw t3, -44(fp)
	sw t3, -48(fp)
# goto L7
	j L7
L6:
# t17 = 1
	li t4, 1
	sw t4, -48(fp)
L7:
# t20 = !t17
	lw t6, -48(fp)
	seqz t5, t6
	sw t5, -52(fp)
# if t20 goto L5
	lw t0, -52(fp)
	bnez t0, L5
# return 0
	li a0, 0
	j isPrime_epilogue
L5:
# i_scope-206530389 = 5
	li t1, 5
	sw t1, -56(fp)
# goto L9
	j L9
L8:
# t21 = num % i_scope-206530389
	lw t3, -12(fp)
	lw t4, -56(fp)
	rem t2, t3, t4
	sw t2, -60(fp)
# t22 = t21 == 0
	lw t6, -60(fp)
	li t0, 0
	xor t5, t6, t0
	seqz t5, t5
	sw t5, -64(fp)
# if t22 goto L12
	lw t1, -64(fp)
	bnez t1, L12
# t24 = i_scope-206530389 + 2
	lw t3, -56(fp)
	li t4, 2
	add t2, t3, t4
	sw t2, -68(fp)
# t25 = num % t24
	lw t6, -12(fp)
	lw t0, -68(fp)
	rem t5, t6, t0
	sw t5, -72(fp)
# t26 = t25 == 0
	lw t2, -72(fp)
	li t3, 0
	xor t1, t2, t3
	seqz t1, t1
	sw t1, -76(fp)
# t23 = t26
	lw t4, -76(fp)
	sw t4, -80(fp)
# goto L13
	j L13
L12:
# t23 = 1
	li t5, 1
	sw t5, -80(fp)
L13:
# t27 = !t23
	lw t0, -80(fp)
	seqz t6, t0
	sw t6, -84(fp)
# if t27 goto L11
	lw t1, -84(fp)
	bnez t1, L11
# return 0
	li a0, 0
	j isPrime_epilogue
L11:
# t28 = i_scope-206530389 + 6
	lw t3, -56(fp)
	li t4, 6
	add t2, t3, t4
	sw t2, -88(fp)
# i_scope-206530389 = t28
	lw t5, -88(fp)
	sw t5, -56(fp)
L9:
# t29 = i_scope-206530389 * i_scope-206530389
	lw t0, -56(fp)
	lw t1, -56(fp)
	mul t6, t0, t1
	sw t6, -92(fp)
# t30 = t29 <= num
	lw t3, -92(fp)
	lw t4, -12(fp)
	slt t2, t4, t3
	xori t2, t2, 1
	sw t2, -96(fp)
# if t30 goto L8
	lw t5, -96(fp)
	bnez t5, L8
L10:
# return 1
	li a0, 1
	j isPrime_epilogue
isPrime_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
	.global factorial
factorial:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
# result_scope-206530389 = 1
	li t6, 1
	sw t6, -16(fp)
# goto L15
	j L15
L14:
# t31 = result_scope-206530389 * n
	lw t1, -16(fp)
	lw t2, -12(fp)
	mul t0, t1, t2
	sw t0, -20(fp)
# result_scope-206530389 = t31
	lw t3, -20(fp)
	sw t3, -16(fp)
# t32 = n - 1
	lw t5, -12(fp)
	li t6, 1
	sub t4, t5, t6
	sw t4, -24(fp)
# n = t32
	lw t0, -24(fp)
	sw t0, -12(fp)
L15:
# t33 = n > 0
	lw t2, -12(fp)
	li t3, 0
	slt t1, t3, t2
	sw t1, -28(fp)
# if t33 goto L14
	lw t4, -28(fp)
	bnez t4, L14
L16:
# return result_scope-206530389
	lw a0, -16(fp)
	j factorial_epilogue
factorial_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
	.global combination
combination:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
	sw a1, -16(fp)
# t34 = k > n
	lw t6, -16(fp)
	lw t0, -12(fp)
	slt t5, t0, t6
	sw t5, -20(fp)
# t35 = !t34
	lw t2, -20(fp)
	seqz t1, t2
	sw t1, -24(fp)
# if t35 goto L17
	lw t3, -24(fp)
	bnez t3, L17
# return 0
	li a0, 0
	j combination_epilogue
L17:
# t36 = k == 0
	lw t5, -16(fp)
	li t6, 0
	xor t4, t5, t6
	seqz t4, t4
	sw t4, -28(fp)
# if t36 goto L19
	lw t0, -28(fp)
	bnez t0, L19
# t38 = k == n
	lw t2, -16(fp)
	lw t3, -12(fp)
	xor t1, t2, t3
	seqz t1, t1
	sw t1, -32(fp)
# t37 = t38
	lw t4, -32(fp)
	sw t4, -36(fp)
# goto L20
	j L20
L19:
# t37 = 1
	li t5, 1
	sw t5, -36(fp)
L20:
# t39 = !t37
	lw t0, -36(fp)
	seqz t6, t0
	sw t6, -40(fp)
# if t39 goto L18
	lw t1, -40(fp)
	bnez t1, L18
# return 1
	li a0, 1
	j combination_epilogue
L18:
# t40 = n - k
	lw t3, -12(fp)
	lw t4, -16(fp)
	sub t2, t3, t4
	sw t2, -44(fp)
# param t40
# t41 = call factorial, 1
# 保存调用者保存的寄存器
	sw t0, -48(fp)
	sw t1, -52(fp)
	sw t2, -56(fp)
	sw t3, -60(fp)
	sw t4, -64(fp)
	lw a0, -44(fp)
	call factorial
# 恢复调用者保存的寄存器
	lw t0, -48(fp)
	lw t1, -52(fp)
	lw t2, -56(fp)
	lw t3, -60(fp)
	lw t4, -64(fp)
	addi t5, a0, 0
	sw t5, -68(fp)
# param k
# t42 = call factorial, 1
# 保存调用者保存的寄存器
	sw t0, -48(fp)
	sw t1, -52(fp)
	sw t2, -56(fp)
	sw t3, -60(fp)
	sw t4, -64(fp)
	sw t5, -72(fp)
	lw a0, -16(fp)
	call factorial
# 恢复调用者保存的寄存器
	lw t0, -48(fp)
	lw t1, -52(fp)
	lw t2, -56(fp)
	lw t3, -60(fp)
	lw t4, -64(fp)
	lw t5, -72(fp)
	addi t6, a0, 0
	sw t6, -76(fp)
# t43 = t42 * t41
	lw t1, -76(fp)
	lw t2, -68(fp)
	mul t0, t1, t2
	sw t0, -80(fp)
# param n
# t44 = call factorial, 1
# 保存调用者保存的寄存器
	sw t0, -48(fp)
	sw t1, -52(fp)
	sw t2, -56(fp)
	lw a0, -12(fp)
	call factorial
# 恢复调用者保存的寄存器
	lw t0, -48(fp)
	lw t1, -52(fp)
	lw t2, -56(fp)
	addi t3, a0, 0
	sw t3, -84(fp)
# t45 = t44 / t43
	lw t5, -84(fp)
	lw t6, -80(fp)
	div t4, t5, t6
	sw t4, -88(fp)
# return t45
	lw a0, -88(fp)
	j combination_epilogue
combination_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
	.global power
power:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
	sw a1, -16(fp)
# result_scope-206530389 = 1
	li t0, 1
	sw t0, -20(fp)
# goto L22
	j L22
L21:
# t46 = exponent % 2
	lw t2, -16(fp)
	li t3, 2
	rem t1, t2, t3
	sw t1, -24(fp)
# t47 = t46 == 1
	lw t5, -24(fp)
	li t6, 1
	xor t4, t5, t6
	seqz t4, t4
	sw t4, -28(fp)
# t48 = !t47
	lw t1, -28(fp)
	seqz t0, t1
	sw t0, -32(fp)
# if t48 goto L24
	lw t2, -32(fp)
	bnez t2, L24
# t49 = result_scope-206530389 * base
	lw t4, -20(fp)
	lw t5, -12(fp)
	mul t3, t4, t5
	sw t3, -36(fp)
# result_scope-206530389 = t49
	lw t6, -36(fp)
	sw t6, -20(fp)
L24:
# t50 = base * base
	lw t1, -12(fp)
	lw t2, -12(fp)
	mul t0, t1, t2
	sw t0, -40(fp)
# base = t50
	lw t3, -40(fp)
	sw t3, -12(fp)
# t51 = exponent / 2
	lw t5, -16(fp)
	li t6, 2
	div t4, t5, t6
	sw t4, -44(fp)
# exponent = t51
	lw t0, -44(fp)
	sw t0, -16(fp)
L22:
# t52 = exponent > 0
	lw t2, -16(fp)
	li t3, 0
	slt t1, t3, t2
	sw t1, -48(fp)
# if t52 goto L21
	lw t4, -48(fp)
	bnez t4, L21
L23:
# return result_scope-206530389
	lw a0, -20(fp)
	j power_epilogue
power_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
	.global complexFunction
complexFunction:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
	sw a1, -16(fp)
	sw a2, -20(fp)
# result_scope-206530389 = 0
	li t5, 0
	sw t5, -24(fp)
# t53 = a > b
	lw t0, -12(fp)
	lw t1, -16(fp)
	slt t6, t1, t0
	sw t6, -28(fp)
# t55 = !t53
	lw t3, -28(fp)
	seqz t2, t3
	sw t2, -32(fp)
# if t55 goto L27
	lw t4, -32(fp)
	bnez t4, L27
# t56 = b > c
	lw t6, -16(fp)
	lw t0, -20(fp)
	slt t5, t0, t6
	sw t5, -36(fp)
# t54 = t56
	lw t1, -36(fp)
	sw t1, -40(fp)
# goto L28
	j L28
L27:
# t54 = 0
	li t2, 0
	sw t2, -40(fp)
L28:
# t57 = !t54
	lw t4, -40(fp)
	seqz t3, t4
	sw t3, -44(fp)
# t58 = !t57
	lw t6, -44(fp)
	seqz t5, t6
	sw t5, -48(fp)
# if t58 goto L25
	lw t0, -48(fp)
	bnez t0, L25
# t59 = -1
	li t2, 1
	neg t1, t2
	sw t1, -52(fp)
# t60 = c + t59
	lw t4, -20(fp)
	lw t5, -52(fp)
	add t3, t4, t5
	sw t3, -56(fp)
# t61 = -t60
	lw t0, -56(fp)
	neg t6, t0
	sw t6, -60(fp)
# t62 = a * b
	lw t2, -12(fp)
	lw t3, -16(fp)
	mul t1, t2, t3
	sw t1, -64(fp)
# t63 = t62 - t61
	lw t5, -64(fp)
	lw t6, -60(fp)
	sub t4, t5, t6
	sw t4, -68(fp)
# result_scope-206530389 = t63
	lw t0, -68(fp)
	sw t0, -24(fp)
# goto L26
	j L26
L25:
# t64 = a < c
	lw t2, -12(fp)
	lw t3, -20(fp)
	slt t1, t2, t3
	sw t1, -72(fp)
# t65 = !t64
	lw t5, -72(fp)
	seqz t4, t5
	sw t4, -76(fp)
# if t65 goto L31
	lw t6, -76(fp)
	bnez t6, L31
# t67 = c < b
	lw t1, -20(fp)
	lw t2, -16(fp)
	slt t0, t1, t2
	sw t0, -80(fp)
# t66 = t67
	lw t3, -80(fp)
	sw t3, -84(fp)
# goto L32
	j L32
L31:
# t66 = 1
	li t4, 1
	sw t4, -84(fp)
L32:
# t68 = !t66
	lw t6, -84(fp)
	seqz t5, t6
	sw t5, -88(fp)
# if t68 goto L29
	lw t0, -88(fp)
	bnez t0, L29
# t69 = -2
	li t2, 2
	neg t1, t2
	sw t1, -92(fp)
# t70 = b
	lw t3, -16(fp)
	sw t3, -96(fp)
# t71 = c - t70
	lw t5, -20(fp)
	lw t6, -96(fp)
	sub t4, t5, t6
	sw t4, -100(fp)
# t72 = t71 - t69
	lw t1, -100(fp)
	lw t2, -92(fp)
	sub t0, t1, t2
	sw t0, -104(fp)
# t73 = a * t72
	lw t4, -12(fp)
	lw t5, -104(fp)
	mul t3, t4, t5
	sw t3, -108(fp)
# result_scope-206530389 = t73
	lw t6, -108(fp)
	sw t6, -24(fp)
# goto L30
	j L30
L29:
# t74 = b >= a
	lw t1, -16(fp)
	lw t2, -12(fp)
	slt t0, t1, t2
	xori t0, t0, 1
	sw t0, -112(fp)
# t76 = !t74
	lw t4, -112(fp)
	seqz t3, t4
	sw t3, -116(fp)
# if t76 goto L35
	lw t5, -116(fp)
	bnez t5, L35
# t77 = a >= c
	lw t0, -12(fp)
	lw t1, -20(fp)
	slt t6, t0, t1
	xori t6, t6, 1
	sw t6, -120(fp)
# t75 = t77
	lw t2, -120(fp)
	sw t2, -124(fp)
# goto L36
	j L36
L35:
# t75 = 0
	li t3, 0
	sw t3, -124(fp)
L36:
# if t75 goto L37
	lw t4, -124(fp)
	bnez t4, L37
# t79 = b <= c
	lw t6, -16(fp)
	lw t0, -20(fp)
	slt t5, t0, t6
	xori t5, t5, 1
	sw t5, -128(fp)
# t78 = t79
	lw t1, -128(fp)
	sw t1, -132(fp)
# goto L38
	j L38
L37:
# t78 = 1
	li t2, 1
	sw t2, -132(fp)
L38:
# t80 = !t78
	lw t4, -132(fp)
	seqz t3, t4
	sw t3, -136(fp)
# if t80 goto L33
	lw t5, -136(fp)
	bnez t5, L33
# t81 = -3
	li t0, 3
	neg t6, t0
	sw t6, -140(fp)
# t82 = c + t81
	lw t2, -20(fp)
	lw t3, -140(fp)
	add t1, t2, t3
	sw t1, -144(fp)
# t83 = -t82
	lw t5, -144(fp)
	neg t4, t5
	sw t4, -148(fp)
# t84 = b * a
	lw t0, -16(fp)
	lw t1, -12(fp)
	mul t6, t0, t1
	sw t6, -152(fp)
# t85 = t84 - t83
	lw t3, -152(fp)
	lw t4, -148(fp)
	sub t2, t3, t4
	sw t2, -156(fp)
# result_scope-206530389 = t85
	lw t5, -156(fp)
	sw t5, -24(fp)
# goto L34
	j L34
L33:
# t86 = b > c
	lw t0, -16(fp)
	lw t1, -20(fp)
	slt t6, t1, t0
	sw t6, -160(fp)
# if t86 goto L41
	lw t2, -160(fp)
	bnez t2, L41
# t88 = c > a
	lw t4, -20(fp)
	lw t5, -12(fp)
	slt t3, t5, t4
	sw t3, -164(fp)
# t90 = !t88
	lw t0, -164(fp)
	seqz t6, t0
	sw t6, -168(fp)
# if t90 goto L43
	lw t1, -168(fp)
	bnez t1, L43
# t91 = a <= b
	lw t3, -12(fp)
	lw t4, -16(fp)
	slt t2, t4, t3
	xori t2, t2, 1
	sw t2, -172(fp)
# t89 = t91
	lw t5, -172(fp)
	sw t5, -176(fp)
# goto L44
	j L44
L43:
# t89 = 0
	li t6, 0
	sw t6, -176(fp)
L44:
# t87 = t89
	lw t0, -176(fp)
	sw t0, -180(fp)
# goto L42
	j L42
L41:
# t87 = 1
	li t1, 1
	sw t1, -180(fp)
L42:
# t92 = !t87
	lw t3, -180(fp)
	seqz t2, t3
	sw t2, -184(fp)
# if t92 goto L39
	lw t4, -184(fp)
	bnez t4, L39
# t93 = -4
	li t6, 4
	neg t5, t6
	sw t5, -188(fp)
# t94 = a
	lw t0, -12(fp)
	sw t0, -192(fp)
# t95 = c - t94
	lw t2, -20(fp)
	lw t3, -192(fp)
	sub t1, t2, t3
	sw t1, -196(fp)
# t96 = t95 - t93
	lw t5, -196(fp)
	lw t6, -188(fp)
	sub t4, t5, t6
	sw t4, -200(fp)
# t97 = b * t96
	lw t1, -16(fp)
	lw t2, -200(fp)
	mul t0, t1, t2
	sw t0, -204(fp)
# result_scope-206530389 = t97
	lw t3, -204(fp)
	sw t3, -24(fp)
# goto L40
	j L40
L39:
# t98 = c > a
	lw t5, -20(fp)
	lw t6, -12(fp)
	slt t4, t6, t5
	sw t4, -208(fp)
# if t98 goto L47
	lw t0, -208(fp)
	bnez t0, L47
# t100 = b != a
	lw t2, -16(fp)
	lw t3, -12(fp)
	xor t1, t2, t3
	snez t1, t1
	sw t1, -212(fp)
# t99 = t100
	lw t4, -212(fp)
	sw t4, -216(fp)
# goto L48
	j L48
L47:
# t99 = 1
	li t5, 1
	sw t5, -216(fp)
L48:
# t101 = !t99
	lw t0, -216(fp)
	seqz t6, t0
	sw t6, -220(fp)
# t103 = !t101
	lw t2, -220(fp)
	seqz t1, t2
	sw t1, -224(fp)
# if t103 goto L49
	lw t3, -224(fp)
	bnez t3, L49
# t104 = a == b
	lw t5, -12(fp)
	lw t6, -16(fp)
	xor t4, t5, t6
	seqz t4, t4
	sw t4, -228(fp)
# t102 = t104
	lw t0, -228(fp)
	sw t0, -232(fp)
# goto L50
	j L50
L49:
# t102 = 0
	li t1, 0
	sw t1, -232(fp)
L50:
# t105 = !t102
	lw t3, -232(fp)
	seqz t2, t3
	sw t2, -236(fp)
# if t105 goto L45
	lw t4, -236(fp)
	bnez t4, L45
# t106 = -5
	li t6, 5
	neg t5, t6
	sw t5, -240(fp)
# t107 = b + t106
	lw t1, -16(fp)
	lw t2, -240(fp)
	add t0, t1, t2
	sw t0, -244(fp)
# t108 = t107
	lw t3, -244(fp)
	sw t3, -248(fp)
# t109 = c * a
	lw t5, -20(fp)
	lw t6, -12(fp)
	mul t4, t5, t6
	sw t4, -252(fp)
# t110 = t109 - t108
	lw t1, -252(fp)
	lw t2, -248(fp)
	sub t0, t1, t2
	sw t0, -256(fp)
# result_scope-206530389 = t110
	lw t3, -256(fp)
	sw t3, -24(fp)
# goto L46
	j L46
L45:
# t111 = -6
	li t5, 6
	neg t4, t5
	sw t4, -260(fp)
# t112 = -a
	lw t0, -12(fp)
	neg t6, t0
	sw t6, -264(fp)
# t113 = b - t112
	lw t2, -16(fp)
	lw t3, -264(fp)
	sub t1, t2, t3
	sw t1, -268(fp)
# t114 = t113 - t111
	lw t5, -268(fp)
	lw t6, -260(fp)
	sub t4, t5, t6
	sw t4, -272(fp)
# t115 = c * t114
	lw t1, -20(fp)
	lw t2, -272(fp)
	mul t0, t1, t2
	sw t0, -276(fp)
# result_scope-206530389 = t115
	lw t3, -276(fp)
	sw t3, -24(fp)
L46:
L40:
L34:
L30:
L26:
# i_scope-206530389 = 0
	li t4, 0
	sw t4, -280(fp)
# goto L52
	j L52
L51:
# t116 = i_scope-206530389 + 1
	lw t6, -280(fp)
	li t0, 1
	add t5, t6, t0
	sw t5, -284(fp)
# i_scope-206530389 = t116
	lw t1, -284(fp)
	sw t1, -280(fp)
# t117 = i_scope-206530389 % 3
	lw t3, -280(fp)
	li t4, 3
	rem t2, t3, t4
	sw t2, -288(fp)
# t118 = t117 == 0
	lw t6, -288(fp)
	li t0, 0
	xor t5, t6, t0
	seqz t5, t5
	sw t5, -292(fp)
# t119 = !t118
	lw t2, -292(fp)
	seqz t1, t2
	sw t1, -296(fp)
# if t119 goto L54
	lw t3, -296(fp)
	bnez t3, L54
# t120 = result_scope-206530389 + i_scope-206530389
	lw t5, -24(fp)
	lw t6, -280(fp)
	add t4, t5, t6
	sw t4, -300(fp)
# result_scope-206530389 = t120
	lw t0, -300(fp)
	sw t0, -24(fp)
# goto L55
	j L55
L54:
# t121 = i_scope-206530389 % 3
	lw t2, -280(fp)
	li t3, 3
	rem t1, t2, t3
	sw t1, -304(fp)
# t122 = t121 == 1
	lw t5, -304(fp)
	li t6, 1
	xor t4, t5, t6
	seqz t4, t4
	sw t4, -308(fp)
# t123 = !t122
	lw t1, -308(fp)
	seqz t0, t1
	sw t0, -312(fp)
# if t123 goto L56
	lw t2, -312(fp)
	bnez t2, L56
# t124 = result_scope-206530389 - i_scope-206530389
	lw t4, -24(fp)
	lw t5, -280(fp)
	sub t3, t4, t5
	sw t3, -316(fp)
# result_scope-206530389 = t124
	lw t6, -316(fp)
	sw t6, -24(fp)
# goto L57
	j L57
L56:
# t125 = result_scope-206530389 * 2
	lw t1, -24(fp)
	li t2, 2
	mul t0, t1, t2
	sw t0, -320(fp)
# result_scope-206530389 = t125
	lw t3, -320(fp)
	sw t3, -24(fp)
# t126 = result_scope-206530389 < 50
	lw t5, -24(fp)
	li t6, 50
	slt t4, t5, t6
	sw t4, -324(fp)
# t127 = !t126
	lw t1, -324(fp)
	seqz t0, t1
	sw t0, -328(fp)
# if t127 goto L58
	lw t2, -328(fp)
	bnez t2, L58
# goto L52
	j L52
L58:
# t128 = result_scope-206530389 + 1
	lw t4, -24(fp)
	li t5, 1
	add t3, t4, t5
	sw t3, -332(fp)
# result_scope-206530389 = t128
	lw t6, -332(fp)
	sw t6, -24(fp)
# t129 = result_scope-206530389 > 100
	lw t1, -24(fp)
	li t2, 100
	slt t0, t2, t1
	sw t0, -336(fp)
# t130 = !t129
	lw t4, -336(fp)
	seqz t3, t4
	sw t3, -340(fp)
# if t130 goto L59
	lw t5, -340(fp)
	bnez t5, L59
# goto L53
	j L53
L59:
L57:
L55:
L52:
# t131 = i_scope-206530389 < 10
	lw t0, -280(fp)
	li t1, 10
	slt t6, t0, t1
	sw t6, -344(fp)
# if t131 goto L51
	lw t2, -344(fp)
	bnez t2, L51
L53:
# return result_scope-206530389
	lw a0, -24(fp)
	j complexFunction_epilogue
complexFunction_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
	.global shortCircuit
shortCircuit:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
	sw a1, -16(fp)
# result_scope-206530389 = 0
	li t3, 0
	sw t3, -20(fp)
# t132 = a > 0
	lw t5, -12(fp)
	li t6, 0
	slt t4, t6, t5
	sw t4, -24(fp)
# t134 = !t132
	lw t1, -24(fp)
	seqz t0, t1
	sw t0, -28(fp)
# if t134 goto L61
	lw t2, -28(fp)
	bnez t2, L61
# t135 = b / a
	lw t4, -16(fp)
	lw t5, -12(fp)
	div t3, t4, t5
	sw t3, -32(fp)
# t136 = t135 > 2
	lw t0, -32(fp)
	li t1, 2
	slt t6, t1, t0
	sw t6, -36(fp)
# t133 = t136
	lw t2, -36(fp)
	sw t2, -40(fp)
# goto L62
	j L62
L61:
# t133 = 0
	li t3, 0
	sw t3, -40(fp)
L62:
# t137 = !t133
	lw t5, -40(fp)
	seqz t4, t5
	sw t4, -44(fp)
# if t137 goto L60
	lw t6, -44(fp)
	bnez t6, L60
# t138 = result_scope-206530389 + 12
	lw t1, -20(fp)
	li t2, 12
	add t0, t1, t2
	sw t0, -48(fp)
# result_scope-206530389 = t138
	lw t3, -48(fp)
	sw t3, -20(fp)
L60:
# t139 = a < 0
	lw t5, -12(fp)
	li t6, 0
	slt t4, t5, t6
	sw t4, -52(fp)
# if t139 goto L64
	lw t0, -52(fp)
	bnez t0, L64
# t141 = a - a
	lw t2, -12(fp)
	lw t3, -12(fp)
	sub t1, t2, t3
	sw t1, -56(fp)
# t142 = t141 + 1
	lw t5, -56(fp)
	li t6, 1
	add t4, t5, t6
	sw t4, -60(fp)
# t143 = b / t142
	lw t1, -16(fp)
	lw t2, -60(fp)
	div t0, t1, t2
	sw t0, -64(fp)
# t144 = t143 < 0
	lw t4, -64(fp)
	li t5, 0
	slt t3, t4, t5
	sw t3, -68(fp)
# t140 = t144
	lw t6, -68(fp)
	sw t6, -72(fp)
# goto L65
	j L65
L64:
# t140 = 1
	li t0, 1
	sw t0, -72(fp)
L65:
# t145 = !t140
	lw t2, -72(fp)
	seqz t1, t2
	sw t1, -76(fp)
# if t145 goto L63
	lw t3, -76(fp)
	bnez t3, L63
# t146 = result_scope-206530389 + 30
	lw t5, -20(fp)
	li t6, 30
	add t4, t5, t6
	sw t4, -80(fp)
# result_scope-206530389 = t146
	lw t0, -80(fp)
	sw t0, -20(fp)
L63:
# return result_scope-206530389
	lw a0, -20(fp)
	j shortCircuit_epilogue
shortCircuit_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
	.global nestedLoopsAndConditions
nestedLoopsAndConditions:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
# sum_scope-206530389 = 0
	li t1, 0
	sw t1, -16(fp)
# i_scope-206530389 = 0
	li t2, 0
	sw t2, -20(fp)
# goto L67
	j L67
L66:
# j_scope-206530388 = 0
	li t3, 0
	sw t3, -24(fp)
# goto L70
	j L70
L69:
# t147 = i_scope-206530389 + j_scope-206530388
	lw t5, -20(fp)
	lw t6, -24(fp)
	add t4, t5, t6
	sw t4, -28(fp)
# t148 = t147 % 2
	lw t1, -28(fp)
	li t2, 2
	rem t0, t1, t2
	sw t0, -32(fp)
# t149 = t148 == 0
	lw t4, -32(fp)
	li t5, 0
	xor t3, t4, t5
	seqz t3, t3
	sw t3, -36(fp)
# t150 = !t149
	lw t0, -36(fp)
	seqz t6, t0
	sw t6, -40(fp)
# if t150 goto L72
	lw t1, -40(fp)
	bnez t1, L72
# t151 = i_scope-206530389 * j_scope-206530388
	lw t3, -20(fp)
	lw t4, -24(fp)
	mul t2, t3, t4
	sw t2, -44(fp)
# t152 = sum_scope-206530389 - t151
	lw t6, -16(fp)
	lw t0, -44(fp)
	sub t5, t6, t0
	sw t5, -48(fp)
# sum_scope-206530389 = t152
	lw t1, -48(fp)
	sw t1, -16(fp)
# goto L73
	j L73
L72:
# t153 = i_scope-206530389 * j_scope-206530388
	lw t3, -20(fp)
	lw t4, -24(fp)
	mul t2, t3, t4
	sw t2, -52(fp)
# t154 = sum_scope-206530389 + t153
	lw t6, -16(fp)
	lw t0, -52(fp)
	add t5, t6, t0
	sw t5, -56(fp)
# sum_scope-206530389 = t154
	lw t1, -56(fp)
	sw t1, -16(fp)
# t155 = sum_scope-206530389 < 0
	lw t3, -16(fp)
	li t4, 0
	slt t2, t3, t4
	sw t2, -60(fp)
# t156 = !t155
	lw t6, -60(fp)
	seqz t5, t6
	sw t5, -64(fp)
# if t156 goto L74
	lw t0, -64(fp)
	bnez t0, L74
# sum_scope-206530389 = 0
	li t1, 0
	sw t1, -16(fp)
# goto L70
	j L70
L74:
L73:
# t157 = sum_scope-206530389 > 1053
	lw t3, -16(fp)
	li t4, 1053
	slt t2, t4, t3
	sw t2, -68(fp)
# t158 = !t157
	lw t6, -68(fp)
	seqz t5, t6
	sw t5, -72(fp)
# if t158 goto L75
	lw t0, -72(fp)
	bnez t0, L75
# goto L71
	j L71
L75:
# t159 = j_scope-206530388 + 1
	lw t2, -24(fp)
	li t3, 1
	add t1, t2, t3
	sw t1, -76(fp)
# j_scope-206530388 = t159
	lw t4, -76(fp)
	sw t4, -24(fp)
L70:
# t160 = j_scope-206530388 < i_scope-206530389
	lw t6, -24(fp)
	lw t0, -20(fp)
	slt t5, t6, t0
	sw t5, -80(fp)
# if t160 goto L69
	lw t1, -80(fp)
	bnez t1, L69
L71:
# t161 = sum_scope-206530389 > 913
	lw t3, -16(fp)
	li t4, 913
	slt t2, t4, t3
	sw t2, -84(fp)
# t162 = !t161
	lw t6, -84(fp)
	seqz t5, t6
	sw t5, -88(fp)
# if t162 goto L76
	lw t0, -88(fp)
	bnez t0, L76
# goto L68
	j L68
L76:
# t163 = i_scope-206530389 + 1
	lw t2, -20(fp)
	li t3, 1
	add t1, t2, t3
	sw t1, -92(fp)
# i_scope-206530389 = t163
	lw t4, -92(fp)
	sw t4, -20(fp)
L67:
# t164 = i_scope-206530389 < n
	lw t6, -20(fp)
	lw t0, -12(fp)
	slt t5, t6, t0
	sw t5, -96(fp)
# if t164 goto L66
	lw t1, -96(fp)
	bnez t1, L66
L68:
# return sum_scope-206530389
	lw a0, -16(fp)
	j nestedLoopsAndConditions_epilogue
nestedLoopsAndConditions_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
	.global func1
func1:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
	sw a1, -16(fp)
	sw a2, -20(fp)
# t165 = z == 0
	lw t3, -20(fp)
	li t4, 0
	xor t2, t3, t4
	seqz t2, t2
	sw t2, -24(fp)
# t166 = !t165
	lw t6, -24(fp)
	seqz t5, t6
	sw t5, -28(fp)
# if t166 goto L77
	lw t0, -28(fp)
	bnez t0, L77
# t167 = x
	lw t1, -12(fp)
	sw t1, -32(fp)
# t168 = t167 * y
	lw t3, -32(fp)
	lw t4, -16(fp)
	mul t2, t3, t4
	sw t2, -36(fp)
# return t168
	lw a0, -36(fp)
	j func1_epilogue
# goto L78
	j L78
L77:
# t169 = y - z
	lw t6, -16(fp)
	lw t0, -20(fp)
	sub t5, t6, t0
	sw t5, -40(fp)
# param x
# param t169
# param 0
# t170 = call func1, 3
# 保存调用者保存的寄存器
	sw t0, -44(fp)
	lw a0, -12(fp)
	lw a1, -40(fp)
	li a2, 0
	call func1
# 恢复调用者保存的寄存器
	lw t0, -44(fp)
	addi t1, a0, 0
	sw t1, -48(fp)
# return t170
	lw a0, -48(fp)
	j func1_epilogue
L78:
func1_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
	.global func2
func2:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
	sw a1, -16(fp)
# t171 = !y
	lw t3, -16(fp)
	seqz t2, t3
	sw t2, -20(fp)
# if t171 goto L79
	lw t4, -20(fp)
	bnez t4, L79
# t172 = y
	lw t5, -16(fp)
	sw t5, -24(fp)
# t173 = x % t172
	lw t0, -12(fp)
	lw t1, -24(fp)
	rem t6, t0, t1
	sw t6, -28(fp)
# param t173
# param 0
# t174 = call func2, 2
# 保存调用者保存的寄存器
	sw t0, -32(fp)
	sw t1, -36(fp)
	lw a0, -28(fp)
	li a1, 0
	call func2
# 恢复调用者保存的寄存器
	lw t0, -32(fp)
	lw t1, -36(fp)
	addi t2, a0, 0
	sw t2, -40(fp)
# return t174
	lw a0, -40(fp)
	j func2_epilogue
# goto L80
	j L80
L79:
# return x
	lw a0, -12(fp)
	j func2_epilogue
L80:
func2_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
	.global func3
func3:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
	sw a1, -16(fp)
# t175 = y == 0
	lw t4, -16(fp)
	li t5, 0
	xor t3, t4, t5
	seqz t3, t3
	sw t3, -20(fp)
# t176 = !t175
	lw t0, -20(fp)
	seqz t6, t0
	sw t6, -24(fp)
# if t176 goto L81
	lw t1, -24(fp)
	bnez t1, L81
# t177 = x + 1
	lw t3, -12(fp)
	li t4, 1
	add t2, t3, t4
	sw t2, -28(fp)
# return t177
	lw a0, -28(fp)
	j func3_epilogue
# goto L82
	j L82
L81:
# t178 = x + y
	lw t6, -12(fp)
	lw t0, -16(fp)
	add t5, t6, t0
	sw t5, -32(fp)
# param t178
# param 0
# t179 = call func3, 2
# 保存调用者保存的寄存器
	sw t0, -36(fp)
	lw a0, -32(fp)
	li a1, 0
	call func3
# 恢复调用者保存的寄存器
	lw t0, -36(fp)
	addi t1, a0, 0
	sw t1, -40(fp)
# return t179
	lw a0, -40(fp)
	j func3_epilogue
L82:
func3_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
	.global func4
func4:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
	sw a1, -16(fp)
	sw a2, -20(fp)
# t180 = !x
	lw t3, -12(fp)
	seqz t2, t3
	sw t2, -24(fp)
# if t180 goto L83
	lw t4, -24(fp)
	bnez t4, L83
# return y
	lw a0, -16(fp)
	j func4_epilogue
# goto L84
	j L84
L83:
# return z
	lw a0, -20(fp)
	j func4_epilogue
L84:
func4_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
	.global func5
func5:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
# t181 = -x
	lw t6, -12(fp)
	neg t5, t6
	sw t5, -16(fp)
# return t181
	lw a0, -16(fp)
	j func5_epilogue
func5_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
	.global func6
func6:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
	sw a1, -16(fp)
# t183 = !x
	lw t1, -12(fp)
	seqz t0, t1
	sw t0, -20(fp)
# if t183 goto L87
	lw t2, -20(fp)
	bnez t2, L87
# t182 = y
	lw t3, -16(fp)
	sw t3, -24(fp)
# goto L88
	j L88
L87:
# t182 = 0
	li t4, 0
	sw t4, -24(fp)
L88:
# t184 = !t182
	lw t6, -24(fp)
	seqz t5, t6
	sw t5, -28(fp)
# if t184 goto L85
	lw t0, -28(fp)
	bnez t0, L85
# return 1
	li a0, 1
	j func6_epilogue
# goto L86
	j L86
L85:
# return 0
	li a0, 0
	j func6_epilogue
L86:
func6_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
	.global func7
func7:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
	add fp, sp, t0
# 保存被调用者保存的寄存器
# 函数形参压栈
	sw a0, -12(fp)
# t185 = !x
	lw t2, -12(fp)
	seqz t1, t2
	sw t1, -16(fp)
# t186 = !t185
	lw t4, -16(fp)
	seqz t3, t4
	sw t3, -20(fp)
# if t186 goto L89
	lw t5, -20(fp)
	bnez t5, L89
# return 1
	li a0, 1
	j func7_epilogue
# goto L90
	j L90
L89:
# return 0
	li a0, 0
	j func7_epilogue
L90:
func7_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
	.global nestedCalls
nestedCalls:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
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
# i1_scope-206530389 = 2
	li t1, 2
	sw t1, -44(fp)
# i2_scope-206530389 = 8
	li t2, 8
	sw t2, -48(fp)
# i3_scope-206530389 = 8
	li t3, 8
	sw t3, -52(fp)
# i4_scope-206530389 = 9
	li t4, 9
	sw t4, -56(fp)
# param i1_scope-206530389
# t187 = call func7, 1
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	sw t4, -76(fp)
	lw a0, -44(fp)
	call func7
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	lw t4, -76(fp)
	addi t5, a0, 0
	sw t5, -80(fp)
# param i2_scope-206530389
# t188 = call func5, 1
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	sw t4, -76(fp)
	sw t5, -84(fp)
	lw a0, -48(fp)
	call func5
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	lw t4, -76(fp)
	lw t5, -84(fp)
	addi t6, a0, 0
	sw t6, -88(fp)
# param t187
# param t188
# t189 = call func6, 2
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	sw t4, -76(fp)
	sw t5, -84(fp)
	sw t6, -92(fp)
	lw a0, -80(fp)
	lw a1, -88(fp)
	call func6
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	lw t4, -76(fp)
	lw t5, -84(fp)
	lw t6, -92(fp)
	addi t0, a0, 0
	sw t0, -96(fp)
# param t189
# param i3_scope-206530389
# t190 = call func2, 2
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	lw a0, -96(fp)
	lw a1, -52(fp)
	call func2
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	addi t1, a0, 0
	sw t1, -100(fp)
# param t190
# param i4_scope-206530389
# t191 = call func3, 2
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	lw a0, -100(fp)
	lw a1, -56(fp)
	call func3
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	addi t2, a0, 0
	sw t2, -104(fp)
# param t191
# t192 = call func5, 1
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	lw a0, -104(fp)
	call func5
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	addi t3, a0, 0
	sw t3, -108(fp)
# param a1
# t193 = call func5, 1
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	lw a0, -16(fp)
	call func5
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	addi t4, a0, 0
	sw t4, -112(fp)
# param a3
# t194 = call func7, 1
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	sw t4, -76(fp)
	lw a0, -24(fp)
	call func7
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	lw t4, -76(fp)
	addi t5, a0, 0
	sw t5, -116(fp)
# param a2
# param t194
# t195 = call func6, 2
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	sw t4, -76(fp)
	sw t5, -84(fp)
	lw a0, -20(fp)
	lw a1, -116(fp)
	call func6
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	lw t4, -76(fp)
	lw t5, -84(fp)
	addi t6, a0, 0
	sw t6, -120(fp)
# param a5
# t196 = call func7, 1
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	sw t4, -76(fp)
	sw t5, -84(fp)
	sw t6, -92(fp)
	lw a0, -32(fp)
	call func7
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	lw t4, -76(fp)
	lw t5, -84(fp)
	lw t6, -92(fp)
	addi t0, a0, 0
	sw t0, -124(fp)
# param a4
# param t196
# t197 = call func2, 2
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	lw a0, -28(fp)
	lw a1, -124(fp)
	call func2
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	addi t1, a0, 0
	sw t1, -128(fp)
# param t193
# param t195
# param t197
# t198 = call func4, 3
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	lw a0, -112(fp)
	lw a1, -120(fp)
	lw a2, -128(fp)
	call func4
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	addi t2, a0, 0
	sw t2, -132(fp)
# param t198
# param a6
# t199 = call func3, 2
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	lw a0, -132(fp)
	lw a1, -36(fp)
	call func3
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	addi t3, a0, 0
	sw t3, -136(fp)
# param t199
# param a7
# t200 = call func2, 2
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	lw a0, -136(fp)
	lw a1, -40(fp)
	call func2
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	addi t4, a0, 0
	sw t4, -140(fp)
# param a9
# t201 = call func7, 1
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	sw t4, -76(fp)
	lw a0, -16(fp)
	call func7
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	lw t4, -76(fp)
	addi t5, a0, 0
	sw t5, -144(fp)
# param a8
# param t201
# t202 = call func3, 2
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	sw t4, -76(fp)
	sw t5, -84(fp)
	lw a0, -12(fp)
	lw a1, -144(fp)
	call func3
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	lw t4, -76(fp)
	lw t5, -84(fp)
	addi t6, a0, 0
	sw t6, -148(fp)
# param t200
# param t202
# param i1_scope-206530389
# t203 = call func1, 3
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	sw t4, -76(fp)
	sw t5, -84(fp)
	sw t6, -92(fp)
	lw a0, -140(fp)
	lw a1, -148(fp)
	lw a2, -44(fp)
	call func1
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	lw t4, -76(fp)
	lw t5, -84(fp)
	lw t6, -92(fp)
	addi t0, a0, 0
	sw t0, -152(fp)
# param t192
# param a0
# param t203
# t204 = call func4, 3
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	lw a0, -108(fp)
	lw a1, -12(fp)
	lw a2, -152(fp)
	call func4
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	addi t1, a0, 0
	sw t1, -156(fp)
# param i3_scope-206530389
# t205 = call func7, 1
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	lw a0, -52(fp)
	call func7
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	addi t2, a0, 0
	sw t2, -160(fp)
# param t205
# param i4_scope-206530389
# t206 = call func3, 2
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	lw a0, -160(fp)
	lw a1, -56(fp)
	call func3
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	addi t3, a0, 0
	sw t3, -164(fp)
# param i2_scope-206530389
# param t206
# t207 = call func2, 2
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	lw a0, -48(fp)
	lw a1, -164(fp)
	call func2
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	addi t4, a0, 0
	sw t4, -168(fp)
# param t204
# param t207
# t208 = call func3, 2
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	sw t4, -76(fp)
	lw a0, -156(fp)
	lw a1, -168(fp)
	call func3
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	lw t4, -76(fp)
	addi t5, a0, 0
	sw t5, -172(fp)
# param t208
# param a0
# param a1
# t209 = call func1, 3
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	sw t4, -76(fp)
	sw t5, -84(fp)
	lw a0, -172(fp)
	lw a1, -12(fp)
	lw a2, -16(fp)
	call func1
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	lw t4, -76(fp)
	lw t5, -84(fp)
	addi t6, a0, 0
	sw t6, -176(fp)
# param t209
# param a2
# t210 = call func2, 2
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	sw t4, -76(fp)
	sw t5, -84(fp)
	sw t6, -92(fp)
	lw a0, -176(fp)
	lw a1, -20(fp)
	call func2
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	lw t4, -76(fp)
	lw t5, -84(fp)
	lw t6, -92(fp)
	addi t0, a0, 0
	sw t0, -180(fp)
# param a5
# t211 = call func5, 1
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	lw a0, -32(fp)
	call func5
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	addi t1, a0, 0
	sw t1, -184(fp)
# param a4
# param t211
# t212 = call func3, 2
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	lw a0, -28(fp)
	lw a1, -184(fp)
	call func3
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	addi t2, a0, 0
	sw t2, -188(fp)
# param a6
# t213 = call func5, 1
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	lw a0, -36(fp)
	call func5
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	addi t3, a0, 0
	sw t3, -192(fp)
# param t212
# param t213
# t214 = call func2, 2
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	lw a0, -188(fp)
	lw a1, -192(fp)
	call func2
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	addi t4, a0, 0
	sw t4, -196(fp)
# param a8
# t215 = call func7, 1
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	sw t4, -76(fp)
	lw a0, -12(fp)
	call func7
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	lw t4, -76(fp)
	addi t5, a0, 0
	sw t5, -200(fp)
# param t214
# param a7
# param t215
# t216 = call func1, 3
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	sw t4, -76(fp)
	sw t5, -84(fp)
	lw a0, -196(fp)
	lw a1, -40(fp)
	lw a2, -200(fp)
	call func1
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	lw t4, -76(fp)
	lw t5, -84(fp)
	addi t6, a0, 0
	sw t6, -204(fp)
# param a9
# t217 = call func5, 1
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	sw t3, -72(fp)
	sw t4, -76(fp)
	sw t5, -84(fp)
	sw t6, -92(fp)
	lw a0, -16(fp)
	call func5
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	lw t3, -72(fp)
	lw t4, -76(fp)
	lw t5, -84(fp)
	lw t6, -92(fp)
	addi t0, a0, 0
	sw t0, -208(fp)
# param t216
# param t217
# t218 = call func2, 2
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	lw a0, -204(fp)
	lw a1, -208(fp)
	call func2
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	addi t1, a0, 0
	sw t1, -212(fp)
# param t218
# param i1_scope-206530389
# t219 = call func3, 2
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	lw a0, -212(fp)
	lw a1, -44(fp)
	call func3
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	addi t2, a0, 0
	sw t2, -216(fp)
# param t210
# param a3
# param t219
# t220 = call func1, 3
# 保存调用者保存的寄存器
	sw t0, -60(fp)
	sw t1, -64(fp)
	sw t2, -68(fp)
	lw a0, -180(fp)
	lw a1, -24(fp)
	lw a2, -216(fp)
	call func1
# 恢复调用者保存的寄存器
	lw t0, -60(fp)
	lw t1, -64(fp)
	lw t2, -68(fp)
	addi t3, a0, 0
	sw t3, -220(fp)
# a_scope-206530389 = t220
	lw t4, -220(fp)
	sw t4, -224(fp)
# return a_scope-206530389
	lw a0, -224(fp)
	j nestedCalls_epilogue
nestedCalls_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
	.global main
main:
# 函数序言
	li t0, -2816
	add sp, sp, t0
	li t0, 2812
	add t0, sp, t0
	sw ra, 0(t0)
	li t0, 2808
	add t0, sp, t0
	sw fp, 0(t0)
	li t0, 2816
	add fp, sp, t0
# 保存被调用者保存的寄存器
# result_scope-206530389 = 0
	li t5, 0
	sw t5, -12(fp)
# param 12
# t221 = call fibonacci, 1
# 保存调用者保存的寄存器
	sw t0, -16(fp)
	sw t1, -20(fp)
	sw t2, -24(fp)
	sw t3, -28(fp)
	sw t4, -32(fp)
	sw t5, -36(fp)
	li a0, 12
	call fibonacci
# 恢复调用者保存的寄存器
	lw t0, -16(fp)
	lw t1, -20(fp)
	lw t2, -24(fp)
	lw t3, -28(fp)
	lw t4, -32(fp)
	lw t5, -36(fp)
	addi t6, a0, 0
	sw t6, -40(fp)
# fib_scope-206530389 = t221
	lw t0, -40(fp)
	sw t0, -44(fp)
# param 22
# param 15
# t222 = call gcd, 2
# 保存调用者保存的寄存器
	sw t0, -16(fp)
	li a0, 22
	li a1, 15
	call gcd
# 恢复调用者保存的寄存器
	lw t0, -16(fp)
	addi t1, a0, 0
	sw t1, -48(fp)
# gcd_result_scope-206530389 = t222
	lw t2, -48(fp)
	sw t2, -52(fp)
# param 17
# t223 = call isPrime, 1
# 保存调用者保存的寄存器
	sw t0, -16(fp)
	sw t1, -20(fp)
	sw t2, -24(fp)
	li a0, 17
	call isPrime
# 恢复调用者保存的寄存器
	lw t0, -16(fp)
	lw t1, -20(fp)
	lw t2, -24(fp)
	addi t3, a0, 0
	sw t3, -56(fp)
# prime_check_scope-206530389 = t223
	lw t4, -56(fp)
	sw t4, -60(fp)
# param 8
# t224 = call factorial, 1
# 保存调用者保存的寄存器
	sw t0, -16(fp)
	sw t1, -20(fp)
	sw t2, -24(fp)
	sw t3, -28(fp)
	sw t4, -32(fp)
	li a0, 8
	call factorial
# 恢复调用者保存的寄存器
	lw t0, -16(fp)
	lw t1, -20(fp)
	lw t2, -24(fp)
	lw t3, -28(fp)
	lw t4, -32(fp)
	addi t5, a0, 0
	sw t5, -64(fp)
# fact_scope-206530389 = t224
	lw t6, -64(fp)
	sw t6, -68(fp)
# param 7
# param 3
# t225 = call combination, 2
# 保存调用者保存的寄存器
	sw t0, -16(fp)
	sw t1, -20(fp)
	sw t2, -24(fp)
	sw t3, -28(fp)
	sw t4, -32(fp)
	sw t5, -36(fp)
	sw t6, -72(fp)
	li a0, 7
	li a1, 3
	call combination
# 恢复调用者保存的寄存器
	lw t0, -16(fp)
	lw t1, -20(fp)
	lw t2, -24(fp)
	lw t3, -28(fp)
	lw t4, -32(fp)
	lw t5, -36(fp)
	lw t6, -72(fp)
	addi t0, a0, 0
	sw t0, -76(fp)
# comb_scope-206530389 = t225
	lw t1, -76(fp)
	sw t1, -80(fp)
# param 3
# param 11
# t226 = call power, 2
# 保存调用者保存的寄存器
	sw t0, -16(fp)
	sw t1, -20(fp)
	li a0, 3
	li a1, 11
	call power
# 恢复调用者保存的寄存器
	lw t0, -16(fp)
	lw t1, -20(fp)
	addi t2, a0, 0
	sw t2, -84(fp)
# pow_result_scope-206530389 = t226
	lw t3, -84(fp)
	sw t3, -88(fp)
# param 3
# param 5
# param 1
# t227 = call complexFunction, 3
# 保存调用者保存的寄存器
	sw t0, -16(fp)
	sw t1, -20(fp)
	sw t2, -24(fp)
	sw t3, -28(fp)
	li a0, 3
	li a1, 5
	li a2, 1
	call complexFunction
# 恢复调用者保存的寄存器
	lw t0, -16(fp)
	lw t1, -20(fp)
	lw t2, -24(fp)
	lw t3, -28(fp)
	addi t4, a0, 0
	sw t4, -92(fp)
# complex_result_scope-206530389 = t227
	lw t5, -92(fp)
	sw t5, -96(fp)
# t228 = -5
	li t0, 5
	neg t6, t0
	sw t6, -100(fp)
# param t228
# param 10
# t229 = call shortCircuit, 2
# 保存调用者保存的寄存器
	sw t0, -16(fp)
	lw a0, -100(fp)
	li a1, 10
	call shortCircuit
# 恢复调用者保存的寄存器
	lw t0, -16(fp)
	addi t1, a0, 0
	sw t1, -104(fp)
# short_circuit_scope-206530389 = t229
	lw t2, -104(fp)
	sw t2, -108(fp)
# param 10
# t230 = call nestedLoopsAndConditions, 1
# 保存调用者保存的寄存器
	sw t0, -16(fp)
	sw t1, -20(fp)
	sw t2, -24(fp)
	li a0, 10
	call nestedLoopsAndConditions
# 恢复调用者保存的寄存器
	lw t0, -16(fp)
	lw t1, -20(fp)
	lw t2, -24(fp)
	addi t3, a0, 0
	sw t3, -112(fp)
# nested_loops_conds_result_scope-206530389 = t230
	lw t4, -112(fp)
	sw t4, -116(fp)
# param 1
# param 2
# param 3
# param 4
# param 5
# param 6
# param 7
# param 8
# param 9
# param 10
# t231 = call nestedCalls, 10
# 保存调用者保存的寄存器
	sw t0, -16(fp)
	sw t1, -20(fp)
	sw t2, -24(fp)
	sw t3, -28(fp)
	sw t4, -32(fp)
	li a0, 1
	li a1, 2
	li a2, 3
	li a3, 4
	li a4, 5
	li a5, 6
	li a6, 7
	li a7, 8
	li t5, 9
	sw t5, 1852(sp)
	li t6, 10
	sw t6, 1856(sp)
	call nestedCalls
# 恢复调用者保存的寄存器
	lw t0, -16(fp)
	lw t1, -20(fp)
	lw t2, -24(fp)
	lw t3, -28(fp)
	lw t4, -32(fp)
	lw t5, -36(fp)
	lw t6, -72(fp)
	addi t0, a0, 0
	sw t0, -120(fp)
# nested_calls_result_scope-206530389 = t231
	lw t1, -120(fp)
	sw t1, -124(fp)
# t232 = fib_scope-206530389 + gcd_result_scope-206530389
	lw t3, -44(fp)
	lw t4, -52(fp)
	add t2, t3, t4
	sw t2, -128(fp)
# t233 = t232 + prime_check_scope-206530389
	lw t6, -128(fp)
	lw t0, -60(fp)
	add t5, t6, t0
	sw t5, -132(fp)
# t234 = t233 + fact_scope-206530389
	lw t2, -132(fp)
	lw t3, -68(fp)
	add t1, t2, t3
	sw t1, -136(fp)
# t235 = t234 - comb_scope-206530389
	lw t5, -136(fp)
	lw t6, -80(fp)
	sub t4, t5, t6
	sw t4, -140(fp)
# t236 = t235 + pow_result_scope-206530389
	lw t1, -140(fp)
	lw t2, -88(fp)
	add t0, t1, t2
	sw t0, -144(fp)
# t237 = t236 - nested_loops_conds_result_scope-206530389
	lw t4, -144(fp)
	lw t5, -116(fp)
	sub t3, t4, t5
	sw t3, -148(fp)
# t238 = t237 % 256
	lw t0, -148(fp)
	li t1, 256
	rem t6, t0, t1
	sw t6, -152(fp)
# result_scope-206530389 = t238
	lw t2, -152(fp)
	sw t2, -12(fp)
# return result_scope-206530389
	lw a0, -12(fp)
	j main_epilogue
main_epilogue:
# 函数后记
# 恢复被调用者保存的寄存器
	li t0, 2808
	add t0, sp, t0
	lw fp, 0(t0)
	li t0, 2812
	add t0, sp, t0
	lw ra, 0(t0)
	li t0, 2816
	add sp, sp, t0
	ret
