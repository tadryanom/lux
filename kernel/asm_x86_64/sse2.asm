
;; lux OS kernel
;; copyright (c) 2018 by Omar Mohammad

format elf64
use64

section '.text'

; void *memcpy(void *destination, const void *source, size_t count)
public memcpy
memcpy:
	push rdi

	mov rcx, rdx

	cmp rcx, 128
	jl .normal

	test rsi, 0x0F
	jnz .unaligned

	test rdi, 0x0F
	jnz .unaligned

.aligned:
	push rcx
	shr rcx, 7		; div 128

.aligned_loop:
	movdqa xmm0, [rsi]
	movdqa xmm1, [rsi+0x10]
	movdqa xmm2, [rsi+0x20]
	movdqa xmm3, [rsi+0x30]
	movdqa xmm4, [rsi+0x40]
	movdqa xmm5, [rsi+0x50]
	movdqa xmm6, [rsi+0x60]
	movdqa xmm7, [rsi+0x70]

	movdqa [rdi], xmm0
	movdqa [rdi+0x10], xmm1
	movdqa [rdi+0x20], xmm2
	movdqa [rdi+0x30], xmm3
	movdqa [rdi+0x40], xmm4
	movdqa [rdi+0x50], xmm5
	movdqa [rdi+0x60], xmm6
	movdqa [rdi+0x70], xmm7

	add rsi, 128
	add rdi, 128
	loop .aligned_loop

	pop rcx

.normal:
	push rcx
	and rcx, 0x7F
	shr rcx, 3
	rep movsq
	pop rcx
	and rcx, 7
	rep movsb

	pop rax
	ret

.unaligned:
	push rcx
	shr rcx, 7		; div 128

.unaligned_loop:
	movdqu xmm0, [rsi]
	movdqu xmm1, [rsi+0x10]
	movdqu xmm2, [rsi+0x20]
	movdqu xmm3, [rsi+0x30]
	movdqu xmm4, [rsi+0x40]
	movdqu xmm5, [rsi+0x50]
	movdqu xmm6, [rsi+0x60]
	movdqu xmm7, [rsi+0x70]

	movdqu [rdi], xmm0
	movdqu [rdi+0x10], xmm1
	movdqu [rdi+0x20], xmm2
	movdqu [rdi+0x30], xmm3
	movdqu [rdi+0x40], xmm4
	movdqu [rdi+0x50], xmm5
	movdqu [rdi+0x60], xmm6
	movdqu [rdi+0x70], xmm7

	add rsi, 128
	add rdi, 128
	loop .unaligned_loop

	pop rcx
	jmp .normal

; void sse2_copy(void *destination, void* source, size_t count)
public sse2_copy
sse2_copy:
	mov rcx, rdx

.loop:
	movdqa xmm0, [rsi]
	movdqa xmm1, [rsi+0x10]
	movdqa xmm2, [rsi+0x20]
	movdqa xmm3, [rsi+0x30]
	movdqa xmm4, [rsi+0x40]
	movdqa xmm5, [rsi+0x50]
	movdqa xmm6, [rsi+0x60]
	movdqa xmm7, [rsi+0x70]

	movdqa [rdi], xmm0
	movdqa [rdi+0x10], xmm1
	movdqa [rdi+0x20], xmm2
	movdqa [rdi+0x30], xmm3
	movdqa [rdi+0x40], xmm4
	movdqa [rdi+0x50], xmm5
	movdqa [rdi+0x60], xmm6
	movdqa [rdi+0x70], xmm7

	add rsi, 128
	add rdi, 128
	loop .loop

	ret


