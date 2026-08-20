.global main
.def main; .scl 2; .type 32; .endef
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $40, %rsp
	movq $1, %rax
	movq %rax, -8(%rbp)
	movq $2, %rax
	movq %rax, -16(%rbp)
	movq $3, %rax
	movq %rax, -24(%rbp)
	movq $0, %rax
	movq -8(%rbp, %rax, 8), %rax
	movq %rax, -32(%rbp)
	leave
	ret
