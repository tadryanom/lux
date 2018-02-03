
;; lux OS kernel
;; copyright (c) 2018 by Omar Mohammad

format elf
use32

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
	mov edx, [esp+4]
	mov eax, [esp+8]
	out dx, al

	ret

; void outw(uint16_t port, uint16_t data)
; Writes to a port
public outw
outw:
	mov edx, [esp+4]
	mov eax, [esp+8]
	out dx, ax

	ret

; void outd(uint16_t port, uint32_t data)
; Writes to a port
public outd
outd:
	mov edx, [esp+4]
	mov eax, [esp+8]
	out dx, eax

	ret

; uint8_t inb(uint16_t port)
; Reads from a port
public inb
inb:
	mov edx, [esp+4]
	in al, dx
	ret

; uint16_t inw(uint16_t port)
; Reads from a port
public inw
inw:
	mov edx, [esp+4]
	in ax, dx
	ret

; uint32_t ind(uint16_t port)
; Reads from a port
public ind
ind:
	mov edx, [esp+4]
	in eax, dx
	ret




