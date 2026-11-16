;! runtime register scheme
;
;* rbx - scratch register
;? rax, rcx, r11 - syscall unusable (syscall #, ret, clobbers)
;? rsp, rbp - used conventionally (unusable as GPRs)
;
;  rdi, rdx - 1 (fd and size for read/write syscalls)
;* rsi - current tape pointer
;* r8 - min/starting tape address; included
;* r9 - max/ending tape address; excluded
;  r10 - tapeSize
;
;* r12 - read fptr
;* r13 - write fptr
;* r14 - shiftLeft fptr
;* r15 - shiftRight fptr
default rel


extern tapeSize
extern silentFail


section .rodata
	allocFailMessage db 0x1b,"[0;1;91m", "Error: tape memory allocation failed", 0x1b,"[0m", 0xa
	allocFailMessageLen equ $ - allocFailMessage

	deallocFailMessage db 0xa, 0x1b,"[0;1;93m", "Warn: tape memory deallocation failed. It will be garbage-collected anyway", 0x1b,"[0m", 0xa
	deallocFailMessageLen equ $ - deallocFailMessage


section .text
	global _start

	extern main

read:
	; todo
	ret

write:
	; todo
	ret

shiftLeft:
	; todo
	ret

shiftRight:
	; todo
	ret


allocFail:
	mov eax, silentFail
	test eax, eax
	jnz allocFailSilent

	mov edi, 1
	lea rsi, [allocFailMessage]
	mov edx, allocFailMessageLen
	mov eax, 1 ; write
	syscall

	allocFailSilent:
	mov edi, 1
	mov eax, 60 ; exit
	syscall

deallocFail:
	mov eax, silentFail
	test eax, eax
	jnz deallocFailSilent

	mov edi, 1
	lea rsi, [deallocFailMessage]
	mov edx, deallocFailMessageLen
	mov eax, 1 ; write
	syscall

	deallocFailSilent:
	xor edi, edi
	mov eax, 60 ; exit
	syscall


_start:
	xor edi, edi ; addr
	mov rsi, tapeSize ; length
	mov edx, 0x3 ; prot (PROT_READ | PROT_WRITE; 0b01 | 0b10)
	mov r10d, 0x22 ; flags (MAP_ANONYMOUS | MAP_PRIVATE; 0x20 | 0x02)
	mov r8, -1 ; fd
	xor r9d, r9d ; offset
	mov eax, 9 ; mmap
	syscall

	test rax, rax
	js allocFail

	; register setup
	mov edi, 1
	mov edx, 1
	mov r10, rsi ; tapeSize
	mov r8, rax ; tape begin addr
	lea r9, [rax + r10] ; tape end addr
	shr rsi, 1
	lea rsi, [rax + rsi] ; cur tape pointer (midway)
	lea r12, [read]
	lea r13, [write]
	lea r14, [shiftLeft]
	lea r15, [shiftRight]

	call main

	; cleanup
	mov rdi, r8
	mov rsi, r10
	mov eax, 11 ; munmap
	syscall

	test rax, rax
	jnz deallocFail

	xor edi, edi
	mov eax, 60 ; exit
	syscall