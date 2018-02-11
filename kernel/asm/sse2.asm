
;; lux OS kernel
;; copyright (c) 2018 by Omar Mohammad

format elf
use32

section '.text'

; void *memcpy(void *destination, const void *source, size_t count)
public memcpy
memcpy:
	push esi
	push edi

	mov esi, [esp+8+8]
	mov edi, [esp+8+4]
	mov ecx, [esp+8+12]

	mov [.return], edi

	cmp ecx, 128
	jl .normal

	test esi, 0x0F
	jnz .unaligned

	test edi, 0x0F
	jnz .unaligned

.aligned:
	push ecx
	shr ecx, 7		; div 128

.aligned_loop:
	movdqa xmm0, [esi]
	movdqa xmm1, [esi+0x10]
	movdqa xmm2, [esi+0x20]
	movdqa xmm3, [esi+0x30]
	movdqa xmm4, [esi+0x40]
	movdqa xmm5, [esi+0x50]
	movdqa xmm6, [esi+0x60]
	movdqa xmm7, [esi+0x70]

	movdqa [edi], xmm0
	movdqa [edi+0x10], xmm1
	movdqa [edi+0x20], xmm2
	movdqa [edi+0x30], xmm3
	movdqa [edi+0x40], xmm4
	movdqa [edi+0x50], xmm5
	movdqa [edi+0x60], xmm6
	movdqa [edi+0x70], xmm7

	add esi, 128
	add edi, 128
	loop .aligned_loop

	pop ecx

.normal:
	push ecx
	and ecx, 0x7F
	shr ecx, 2
	rep movsd
	pop ecx
	and ecx, 3
	rep movsb

	pop edi
	pop esi
	mov eax, [.return]
	ret

.unaligned:
	push ecx
	shr ecx, 7		; div 128

.unaligned_loop:
	movdqu xmm0, [esi]
	movdqu xmm1, [esi+0x10]
	movdqu xmm2, [esi+0x20]
	movdqu xmm3, [esi+0x30]
	movdqu xmm4, [esi+0x40]
	movdqu xmm5, [esi+0x50]
	movdqu xmm6, [esi+0x60]
	movdqu xmm7, [esi+0x70]

	movdqu [edi], xmm0
	movdqu [edi+0x10], xmm1
	movdqu [edi+0x20], xmm2
	movdqu [edi+0x30], xmm3
	movdqu [edi+0x40], xmm4
	movdqu [edi+0x50], xmm5
	movdqu [edi+0x60], xmm6
	movdqu [edi+0x70], xmm7

	add esi, 128
	add edi, 128
	loop .unaligned_loop

	pop ecx
	jmp .normal

align 4
.return			dd 0

; void sse2_copy(void *destination, void* source, size_t count)
public sse2_copy
sse2_copy:
	push esi
	push edi

	mov esi, [esp+8+8]
	mov edi, [esp+8+4]
	mov ecx, [esp+8+12]

.loop:
	movdqa xmm0, [esi]
	movdqa xmm1, [esi+0x10]
	movdqa xmm2, [esi+0x20]
	movdqa xmm3, [esi+0x30]
	movdqa xmm4, [esi+0x40]
	movdqa xmm5, [esi+0x50]
	movdqa xmm6, [esi+0x60]
	movdqa xmm7, [esi+0x70]

	movdqa [edi], xmm0
	movdqa [edi+0x10], xmm1
	movdqa [edi+0x20], xmm2
	movdqa [edi+0x30], xmm3
	movdqa [edi+0x40], xmm4
	movdqa [edi+0x50], xmm5
	movdqa [edi+0x60], xmm6
	movdqa [edi+0x70], xmm7

	add esi, 128
	add edi, 128
	loop .loop

	pop edi
	pop esi
	ret


