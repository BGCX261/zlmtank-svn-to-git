
%include "def.inc"


[BITS 32]           

;ntldr的线性地址是0x90000+1024
ntldr_farmer:
	mov               ax , 0x10
	mov               ds , ax
	mov               es , ax
	mov               fs , ax
	mov               gs , ax
	mov               ss , ax
	mov               esp , NTLDR16_LINEADDR



        mov               esi , NTLDR16_LINEADDR + NTLDR16_SIZE+ NTLDR32_FARMER_SIZE
	mov               edi , NTLDR32_LINEADDR
	mov               ecx , NTLDR32_SIZE / 4
        rep               movsb

        jmp dword 0x08:NTLDR32_LINEADDR    ;jmp ntldr_entry
.hang
        jmp .hang

size	equ	$ - ntldr_farmer
%if size > 128
  %error "ntldr_farmer is too large"
%endif

times 128-($-$$) db 0

