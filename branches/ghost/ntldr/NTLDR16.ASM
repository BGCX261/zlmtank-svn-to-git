;----------------------------------------------------------------------
; 
; 
;
; 
;   
;----------------------------------------------------------------------
%include "def.inc"

;------------------------------------------------------------------------
 jmp begin                  ;���
;-----------------------------------------------------------------------
gdt_addr:
                    dw                  gdtend - gdt - 1        ;GDT���Ĵ�С
                    dw                  gdt           ;GDT����λ��
                    dw                  0x0009
gdt:
  gdt_null:
                    dw                  0x0000
                    dw                  0x0000
                    dw                  0x0000
                    dw                  0x0000
  gdt_system_code:
                    dw                  0xffff        ;����4G
                    dw                  0x0000
                    dw                  0x9a00
                    dw                  0x00cf
  gdt_system_data:
                    dw                  0xffff			;����4G
                    dw                  0x0000
                    dw                  0x9200
                    dw                  0x00cf
gdtend:

gdt_code_sel equ gdt_system_code - gdt
gdt_data_sel equ gdt_system_data - gdt


;----------------------------------------------
;
;		VBE RETURN STATUS
;
;          AL ==     4Fh: Function is supported
;          AL !=     4Fh: Function is not supported
;          AH ==     00h: Function call successful
;          AH ==     01h: Function call failed
;          AH ==     02h: Software supports this function, but the hardware
;                         does not
;          AH ==     03h: Function call invalid in current video mode
;------------------------------------------------


get_bootdriver:
	push	ax
	push	es
	push	dx
	
	mov	ax, BOOTSEC_SEG
	mov	es, ax
	mov	dl, byte [es:BOOTSEC_OFFSET+24h] ;24h�μ�boot.asm
	mov	ax, BOOTINFO_SEG
	mov	es, ax
	mov	byte [es:BOOTDRIVER], dl
	
	pop	dx
	pop	es
	pop 	ax
	ret



;---------------------------------------------
;ȡ�ø���ʾ��������Ϣ
;---------------------------------------------
get_svga_info:
	push	ax
	push	fs
	
	;;ȡ�����б�֧�ֵ�ģʽ
	mov               bx , 0x8000
	mov               es , bx  

	mov               di , 0x0  
	
	mov			eax,'VBE2'
	mov			[es:0],eax
	
        mov eax, 0x0200
        mov [es:4], eax
	
	mov               ax , 0x4f00

	int               0x10  	
	
	pop		fs
	pop		ax
	ret
;---------------------------------------------
; �õ�һ��ģʽ����Ϣ
;---------------------------------------------
set_svga_mode:                                             ;;�����Կ�ģʽ
  push              es
  push              ax    
  
  
  ;;�����Կ�ģʽ
  ;mov               ax , 0x4f02
  ;mov               bx , 0xc140                             ;;800 * 600 ( 8:8:8 )  
  ;int               0x10
  
  ;;ȡ�ø�ģʽ���Դ����Ե�ַ
  mov               bx , 0x8000
  mov               es , bx  

 
  
  mov               di , 0x200  
  mov				ax,0x0bb
  mov				[es:di],ax
  mov               ax , 0x4f01
  mov               cx , 0x140
  int               0x10  
  
    
  pop               ax
  pop               es
  ret


;---------------------------------------------
; ��ȡ���ڴ��С
;---------------------------------------------
get_mem_ext:
  ;��ȡ��չ�ڴ�
  mov	ax,	0xe801
  int	0x15
  jc	erre801		;��ȡ�ɹ������cfλ
  cmp	cx,	0x0
  jne	ok_cxdx
  cmp	dx,	0x0
  jne	ok_cxdx
  ;���ax,bx���涼û��,�϶���cx,dx����
  mov	cx,	ax
  mov	dx,	bx

ok_cxdx:
  ;������չ�ڴ�
  mov	ax,	BOOTINFO_SEG
  mov	es,	ax
  and	edx,	0xffff
  shl	edx,	6
  mov	[es:MEM_E801_MEM_LEN_DW],	edx
  and	ecx,	0xffff
  add	[es:MEM_E801_MEM_LEN_DW],	ecx

  
erre801:

ret

;---------------------------------------------
; ��ȡ�ڴ�map
;---------------------------------------------
get_mem_map:
  ;��ȡ��չ�ڴ�ֲ�
  mov	ax,	BOOTINFO_SEG
  mov	es,	ax
  mov	dword [es:MEM_E820_ITEM_NUM_DW],	0
  mov	di,	MEM_E820_ITEM_OFFSET
jmpe820:
clean20byte:
  mov	dword [es:di],		0
  mov	dword [es:di+4],	0
  mov	dword [es:di+8],	0
  mov	dword [es:di+12],	0
  mov	dword [es:di+16],	0

  mov	ax,	0xe820
  mov	edx,	0x534d4150  ;"SMAP"
  mov	ecx,	MEM_E820_ITEM_SIZE

  int	0x15
  jc	erre820

  cmp	eax,	0x534d4150
  jne	erre820
oke820:
  inc	dword [es:MEM_E820_ITEM_NUM_DW]
  add	di,	MEM_E820_ITEM_SIZE
  cmp	ebx,	0

  jne	jmpe820
erre820:

ret


; --------------------------------------------
;  Here is the operating system entry point
; --------------------------------------------
begin:
	mov	ax, cs		; Get the current segment
	mov	ds, ax		; The data is in this segment
	cli			; disable interrupts while changing stack
	mov	ss, ax		; We'll use this segment for the stack too
	mov	sp, 0xfffe	; Start the stack at the top of the segment
	sti			; Reenable interrupts

	mov	si, msg		; load address of our message
	call	putstr		; print the message

        call get_bootdriver

        ;svga�������ʼ��
	;call set_svga_mode
	;call get_svga_info
	call get_mem_ext
	call get_mem_map
	

	;���濪ʼΪ���뱣��ģʽ�����г�ʼ������
	
	cli                                                 ;���ж�
	lgdt              [gdt_addr]                        ;����gdt��������
	;�������ý���32λ����ģʽ����
	mov               eax , cr0
	or                eax , 1
	mov               cr0 , eax

	;���潫�����system���Ƶ�0x00000λ��
	cld

	mov               ax , gdt_data_sel
	mov               ds , ax
	mov               es , ax
	mov               fs , ax
	mov               gs , ax
	mov               ss , ax

	jmp dword gdt_code_sel:NTLDR16_LINEADDR+NTLDR16_SIZE

        ;�ߵ��������ı�����
; --------------------------------------------
; data for our program

msg	db	'STARTING...!', 0

; ---------------------------------------------
; ��ӡ�ַ�������
; ---------------------------------------------
putstr:
	lodsb		; AL = [DS:SI]
	or al, al	; Set zero flag if al=0
	jz putstrd	; jump to putstrd if zero flag is set
	mov ah, 0x0e	; video function 0Eh (print char)
	mov bx, 0x0007	; color
	int 0x10
	jmp putstr
putstrd:
	retn

;------------------------------------------------
;ȡ��vesa��Ϣ
;------------------------------------------------
;vbeerr_msg :
;        db "setupvga.asm : can't get vbe info !"
;        db 13,10,0

;vbe_Info:
;        pushf
;        pusha
;        push ds
;        push es
              


;       	;;ȡ�����б�֧�ֵ�ģʽ
;		mov               bx , VBEINFOSEG
;		mov               es , bx  
  
	
;		mov			eax,'VBE2'
;		mov			[es:0],eax
;	
;        mov eax, 0x0200
;        mov [es:4], eax
;	
;		mov               ax , 0x4f00
;
;		int               0x10  	

 

;        cmp ax,0x4f 
;        je  near .return
;		;������ɹ���Ҳ���ǲ�֧��vbe2)����
;        mov si,vbeerr_msg
;        call putstr
;           .dead :
;              jmp near .dead
;.return:
;        pop es
;        pop ds
;        popa
;        popf
;       ret
;----------------------------------------------------------------------------
;
;
;������Щģʽ
;-------------------------------------------------------------------------
;VideoModePtr	equ		0x8000e
;
;vbe_moe_test:

              
	; ��ȡmode list ��far pointer ��ƫ��
;        mov SI, [VideoModePtr]
;        ;; ��ȡmode list far pointer �Ķε�ַ
;        mov AX, [VideoModePtr + 2]
;        mov ES, AX
           
;        ;; ES:SI ָ��һ����FFFFh ��β��mode list
;        ;; ѭ����ȡ֧�ֵ�����ģʽ
;        ;mov DI,0  ; DI---ģʽ�б���������
;	modeLoop:
	
;           ;; �õ�ģʽ��
;           mov DX, word [ES:SI]
;                     
;           ;; Is it the end of the mode number list?
;           cmp DX, 0FFFFh
;           je  done
           
;           ;; Increment the pointer for the next loop
;           ;add SI, 2
;
;			;��������ģʽ
















 







times 1024-($-$$) db 0



