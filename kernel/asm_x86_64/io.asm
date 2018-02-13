
;; lux OS kernel
;; copyright (c) 2018 by Omar Mohammad

format elf64
use64

section '.text'

; void iowait()
; Waits for I/O to complete
public iowait
iowait:
	out 0x80, al
	out 0x80, al
	ret

; void outb(uint16_t port, uint8_t data)
; Writes to a port
public outb
outb:
	mov rdx, rdi
	mov rax, rsi
	out dx, al

	ret

; void outw(uint16_t port, uint16_t data)
; Writes to a port
public outw
outw:
	mov rdx, rdi
	mov rax, rsi
	out dx, ax

	ret

; void outd(uint16_t port, uint32_t data)
; Writes to a port
public outd
outd:
	mov rdx, rdi
	mov rax, rsi
	out dx, eax

	ret

; uint8_t inb(uint16_t port)
; Reads from a port
public inb
inb:
	mov rdx, rdi
	in al, dx
	ret

; uint16_t inw(uint16_t port)
; Reads from a port
public inw
inw:
	mov rdx, rdi
	in ax, dx
	ret

; uint32_t ind(uint16_t port)
; Reads from a port
public ind
ind:
	mov rdx, rdi
	in eax, dx
	ret




