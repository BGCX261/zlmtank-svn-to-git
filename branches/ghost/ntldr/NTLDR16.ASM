;----------------------------------------------------------------------
; 
; 
;
; 
;   
;----------------------------------------------------------------------
%include "def.inc"

;------------------------------------------------------------------------
 jmp begin                  ;入口
;-----------------------------------------------------------------------
gdt_addr:
                    dw                  gdtend - gdt - 1        ;GDT表的大小
                    dw                  gdt           ;GDT表的位置
                    dw                  0x0009
gdt:
  gdt_null:
                    dw                  0x0000
                    dw                  0x0000
                    dw                  0x0000
                    dw                  0x0000
  gdt_system_code:
                    dw                  0xffff        ;段限4G
                    dw                  0x0000
                    dw                  0x9a00
                    dw                  0x00cf
  gdt_system_data:
                    dw                  0xffff			;段限4G
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
	mov	dl, byte [es:BOOTSEC_OFFSET+24h] ;24h参见boot.asm
	mov	ax, BOOTINFO_SEG
	mov	es, ax
	mov	byte [es:BOOTDRIVER], dl
	
	pop	dx
	pop	es
	pop 	ax
	ret



;---------------------------------------------
;取得该显示适配器信息
;---------------------------------------------
get_svga_info:
	push	ax
	push	fs
	
	;;取得所有被支持的模式
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
; 得到一种模式的信息
;---------------------------------------------
set_svga_mode:                                             ;;设置显卡模式
  push              es
  push              ax    
  
  
  ;;设置显卡模式
  ;mov               ax , 0x4f02
  ;mov               bx , 0xc140                             ;;800 * 600 ( 8:8:8 )  
  ;int               0x10
  
  ;;取得该模式下显存线性地址
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
; 获取总内存大小
;---------------------------------------------
get_mem_ext:
  ;获取扩展内存
  mov	ax,	0xe801
  int	0x15
  jc	erre801		;获取成功会清除cf位
  cmp	cx,	0x0
  jne	ok_cxdx
  cmp	dx,	0x0
  jne	ok_cxdx
  ;如果ax,bx里面都没有,肯定在cx,dx里面
  mov	cx,	ax
  mov	dx,	bx

ok_cxdx:
  ;计算扩展内存
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
; 获取内存map
;---------------------------------------------
get_mem_map:
  ;获取扩展内存分布
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

        ;svga到后面初始化
	;call set_svga_mode
	;call get_svga_info
	call get_mem_ext
	call get_mem_map
	

	;下面开始为进入保护模式而进行初始化工作
	
	cli                                                 ;关中断
	lgdt              [gdt_addr]                        ;载入gdt的描述符
	;下面设置进入32位保护模式运行
	mov               eax , cr0
	or                eax , 1
	mov               cr0 , eax

	;下面将读入的system搬移到0x00000位置
	cld

	mov               ax , gdt_data_sel
	mov               ds , ax
	mov               es , ax
	mov               fs , ax
	mov               gs , ax
	mov               ss , ax

	jmp dword gdt_code_sel:NTLDR16_LINEADDR+NTLDR16_SIZE

        ;走到这里就真的杯具了
; --------------------------------------------
; data for our program

msg	db	'STARTING...!', 0

; ---------------------------------------------
; 打印字符串过程
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
;取得vesa信息
;------------------------------------------------
;vbeerr_msg :
;        db "setupvga.asm : can't get vbe info !"
;        db 13,10,0

;vbe_Info:
;        pushf
;        pusha
;        push ds
;        push es
              


;       	;;取得所有被支持的模式
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
;		;如果不成功（也就是不支持vbe2)死机
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
;测试这些模式
;-------------------------------------------------------------------------
;VideoModePtr	equ		0x8000e
;
;vbe_moe_test:

              
	; 获取mode list 的far pointer 的偏移
;        mov SI, [VideoModePtr]
;        ;; 获取mode list far pointer 的段地址
;        mov AX, [VideoModePtr + 2]
;        mov ES, AX
           
;        ;; ES:SI 指向一个以FFFFh 结尾的mode list
;        ;; 循环获取支持的所有模式
;        ;mov DI,0  ; DI---模式列表数组索引
;	modeLoop:
	
;           ;; 得到模式号
;           mov DX, word [ES:SI]
;                     
;           ;; Is it the end of the mode number list?
;           cmp DX, 0FFFFh
;           je  done
           
;           ;; Increment the pointer for the next loop
;           ;add SI, 2
;
;			;测试这种模式
















 







times 1024-($-$$) db 0




