
%include "def.inc"

[extern ntldr_main]

[global ntldr_entry]
;[global _idt_base]
;[global _os_entry]

[BITS 32]           

ntldr_entry:    
	;设置寄存器   
	;;初始化数据寄存器
	cli
	mov               ax , 0x10
	mov               ds , ax
	mov               es , ax
	mov               fs , ax
	mov               gs , ax
	mov               ss , ax
	mov               esp , NTLDR16_LINEADDR
	mov               ebp, esp
  
	;跳入c内核入口函数
	jmp ntldr_main

