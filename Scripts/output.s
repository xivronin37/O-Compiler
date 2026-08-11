.global main
.def main; .scl 2; .type 32; .endef
main:
	subq $8, %rsp
	movq %rsp, %rbp
	movq $5, %rax
	pushq %rax
	movq $3, %rax
	movq %rax, %rbx
	popq %rax
	addq %rbx, %rax
	movq %rax, -8(%rbp)
	ret
