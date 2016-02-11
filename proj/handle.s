	.file	"handle.c"
	.section	.rodata
.LC0:
	.string	"Nice try.\n"
	.text
	.globl	sigInt_handler
	.type	sigInt_handler, @function
sigInt_handler:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movl	$1, -12(%rbp)
	movl	-12(%rbp), %eax
	movl	$10, %edx
	movl	$.LC0, %esi
	movl	%eax, %edi
	call	write
	movq	%rax, -8(%rbp)
	cmpq	$10, -8(%rbp)
	je	.L2
	movl	$-999, %edi
	call	exit
.L2:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	sigInt_handler, .-sigInt_handler
	.section	.rodata
.LC1:
	.string	"exiting\n"
	.text
	.globl	sigUsr1_handler
	.type	sigUsr1_handler, @function
sigUsr1_handler:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movl	$1, -12(%rbp)
	movl	-12(%rbp), %eax
	movl	$10, %edx
	movl	$.LC1, %esi
	movl	%eax, %edi
	call	write
	movq	%rax, -8(%rbp)
	cmpq	$10, -8(%rbp)
	je	.L5
	movl	$-999, %edi
	call	exit
.L5:
	movl	$1, %edi
	call	exit
	.cfi_endproc
.LFE3:
	.size	sigUsr1_handler, .-sigUsr1_handler
	.section	.rodata
.LC2:
	.string	"%ld\n"
.LC3:
	.string	"signal error\n"
.LC4:
	.string	"Still here\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB4:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$64, %rsp
	movl	%edi, -52(%rbp)
	movq	%rsi, -64(%rbp)
	call	getpid
	movl	%eax, -48(%rbp)
	movl	$1, -44(%rbp)
	movq	$1, -32(%rbp)
	movl	-48(%rbp), %eax
	cltq
	movq	%rax, %rsi
	movl	$.LC2, %edi
	movl	$0, %eax
	call	printf
	movl	$sigInt_handler, %esi
	movl	$2, %edi
	call	Signal
	cmpq	$-1, %rax
	jne	.L7
	movl	$.LC3, %edi
	call	unix_error
.L7:
	movl	$sigUsr1_handler, %esi
	movl	$10, %edi
	call	Signal
	cmpq	$-1, %rax
	jne	.L8
	movl	$.LC3, %edi
	call	unix_error
.L8:
	leaq	-16(%rbp), %rdx
	leaq	-32(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	nanosleep
	movl	-44(%rbp), %eax
	movl	$12, %edx
	movl	$.LC4, %esi
	movl	%eax, %edi
	call	write
	movq	%rax, -40(%rbp)
	cmpq	$12, -40(%rbp)
	je	.L9
	movl	$-999, %edi
	call	exit
.L9:
	jmp	.L8
	.cfi_endproc
.LFE4:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 4.8.4-2ubuntu1~14.04) 4.8.4"
	.section	.note.GNU-stack,"",@progbits
