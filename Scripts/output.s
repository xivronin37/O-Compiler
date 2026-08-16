.global main
.def main; .scl 2; .type 32; .endef
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $16, %rsp
	movq $10, %rax
	movq %rax, -8(%rbp)
	movq -8(%rbp), %rax
	pushq %rax
	movq $5, %rax
	movq %rax, %rbx
	popq %rax
	addq %rbx, %rax
	movq %rax, -16(%rbp)
	leave
	ret
