
;; lux OS kernel
;; copyright (c) 2018 by Omar Mohammad

format elf
use32

section '.text'

; void write_cr0(uint32_t value)
public write_cr0
write_cr0:
	mov eax, [esp+4]
	mov cr0, eax
	ret

; void write_cr3(uint32_t value)
public write_cr3
write_cr3:
	mov eax, [esp+4]
	mov cr3, eax
	ret

; void write_cr4(uint32_t value)
public write_cr4
write_cr4:
	mov eax, [esp+4]
	mov cr4, eax
	ret

; uint32_t read_cr0()
public read_cr0
read_cr0:
	mov eax, cr0
	ret

; uint32_t read_cr2()
public read_cr2
read_cr2:
	mov eax, cr2
	ret

; uint32_t read_cr3()
public read_cr3
read_cr3:
	mov eax, cr3
	ret

; uint32_t read_cr4()
public read_cr4
read_cr4:
	mov eax, cr4
	ret

; void flush_tlb(size_t base, size_t count)
public flush_tlb
flush_tlb:
	mov eax, [esp+4]	; base
	mov ecx, [esp+8]	; count

	cmp ecx, 0
	je .done

.loop:
	invlpg [eax]
	add eax, 4096
	loop .loop

.done:
	ret


