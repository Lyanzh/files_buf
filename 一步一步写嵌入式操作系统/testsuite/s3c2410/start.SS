.text
	.align 4
	.global _start

_start:
	ldr sp, =1024*4
	bl  myhello

halt:
	b halt