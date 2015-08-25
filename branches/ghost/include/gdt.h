#ifndef _GDT_H
#define _GDT_H



/* 描述符类型值说明 */
#define	DA_32			0x4000	/* 32 位段				*/
#define	DA_LIMIT_4K		0x8000	/* 段界限粒度为 4K 字节			*/
#define	DA_DPL0			0x00	/* DPL = 0				*/
#define	DA_DPL1			0x20	/* DPL = 1				*/
#define	DA_DPL2			0x40	/* DPL = 2				*/
#define	DA_DPL3			0x60	/* DPL = 3				*/
/* 存储段描述符类型值说明 */
#define	DA_DR			0x90	/* 存在的只读数据段类型值		*/
#define	DA_DRW			0x92	/* 存在的可读写数据段属性值		*/
#define	DA_DRWA			0x93	/* 存在的已访问可读写数据段类型值	*/
#define	DA_C			0x98	/* 存在的只执行代码段属性值		*/
#define	DA_CR			0x9A	/* 存在的可执行可读代码段属性值		*/
#define	DA_CCO			0x9C	/* 存在的只执行一致代码段属性值		*/
#define	DA_CCOR			0x9E	/* 存在的可执行可读一致代码段属性值	*/
/* 系统段描述符类型值说明 */
#define	DA_LDT			0x82	/* 局部描述符表段类型值			*/
#define	DA_TaskGate		0x85	/* 任务门类型值				*/
#define	DA_386TSS		0x89	/* 可用 386 任务状态段类型值		*/
#define	DA_386CGate		0x8C	/* 386 调用门类型值			*/
#define	DA_386IGate		0x8E	/* 386 中断门类型值			*/
#define	DA_386TGate		0x8F	/* 386 陷阱门类型值			*/
/* 选择子类型值说明 */
/* 其中, SA_ : Selector Attribute */
#define	SA_RPL_MASK	0xFFFC
#define	SA_RPL0		0
#define	SA_RPL1		1
#define	SA_RPL2		2
#define	SA_RPL3		3

#define	SA_TI_MASK	0xFFFB
#define	SA_TIG		0
#define	SA_TIL		4



/*******************************
定义一些系统初始化结构
codeby:zlm
*******************************/

// 定义任务状态段（TSS）结构  
struct _Tss{ 
  WORD Link; // 链接字段 
  WORD UnUsed0; //未用 
 
  DWORD Esp0; // 零级堆栈指针 
  WORD SS0; // 零级堆栈基址 
  WORD UnUsed1; 
 
  DWORD Esp1; 
  WORD SS1; 
  WORD UnUsed2; 
 
  DWORD Esp2; 
  WORD SS2; 
  WORD UnUsed3; 
 
  DWORD Cr3; 
  DWORD Eip; 
  DWORD Eflags; // 在进程初始化时，需要将此标志中的“IF（中断许可位）”置 1 ，否则切换后新进程不能响应中断 
  DWORD Eax; 
  DWORD Ecx; 
  DWORD Edx; 
  DWORD Ebx; 
  DWORD Esp; 
  DWORD Ebp; 
  DWORD Esi; 
  DWORD Edi; 

  //...ri,这个结构太复杂了..
  WORD Es; 
  WORD UnUsed4; 
 
  WORD Cs; 
  WORD UnUsed5; 
 
  WORD Ss; 
  WORD UnUsed6; 
 
  WORD Ds; 
  WORD UnUsed7; 
 
  WORD Fs; 
  WORD UnUsed8; 
 
  WORD Gs; 
  WORD UnUsed9; 
 
  WORD Ldt; 
  WORD UnUsed10; 
 
  BYTE T :1; 
  WORD UnUsed11 :15; 
 
  WORD EnableIO; // I/O 许可位图 
}TSS; 
 
/* 定义任务状态段描述符 */ 
typedef struct _TSSItem{ 
  WORD LimitLength_0_15; // TSS 的限长的 0~ 15 位 
  WORD Base_0_15; // 基址的 0~15 位 
  BYTE Base_16_23; // 基址的 16~23 位 
  BYTE Reserved_1 : 1; // 保留，需置为 1 
  BYTE B :1; // B 位（忙位），在进程初始化时，需置为 0 ，表示此进程可用 
  BYTE Reserved_010 :3; // 保留需置为 010 ; 
  BYTE DPL :2; // 特权位 
  BYTE P : 1; // P位（存在位） 
  BYTE LimitLength_16_19 : 4; // 限长的 16~19 位 
  BYTE AVL : 1; //AVL（软件可自行利用）位 
  BYTE Reserved_00 : 2; // 保留，需置为 00 
  BYTE G : 1; // G位（粒度位） 
  BYTE Base_24_31; // 基址的 24~31 位 
}TSSITEM; 
 
// 定义任务门 
typedef struct _TSSGate{ 
  WORD UnUsed0; // 未用，置为0 
  WORD TSSSelector; // 任务状态段的段选择符 
  BYTE UnUsed1; // 未用，置为0 
  BYTE Type_0101 : 4; // 类型 
  BYTE Reserved0_0 : 1; // 置为 0 ; 
  BYTE DPL : 2; // 特权级 
  BYTE P : 1; // 存在位 
  WORD UnUsed2; // 未用，置为0 
}TSSGATE;


/* 调用门结构 */ 
typedef struct _InvokeGate{ 
  WORD Offset_0_15; /* 偏移量的 0~15 位 */ 
  WORD SegSelector; /* 段选择符 */ 
  BYTE DWordCount : 5; /* 双字计数字段 */ 
  BYTE Reserved_0 : 3; /* 保留，需为 0 */ 
  BYTE Type_1100 : 4; /* 类型字段，调用门需为 1100 ( 0xC ) */  
  BYTE DT_0 : 1; /* 需为 0 , 以表明这是一个系统用的描述符 */ 
  BYTE DPL : 2; /* 特权级 */ 
  BYTE P : 1; /* 存在位 */ 
  WORD Offset_16_31; 
}INVOKEGATE; 

#if 0
typedef struct _GdtItem{  
  WORD SegLimit_0_15; // 段限0~15位
  WORD SegAddr_0_15; // 基址的 0~15　位
  BYTE SegAddr_16_23; //基址的16~23位
  BYTE Type : 4;  //type位
  BYTE S : 1; //S位
  BYTE DPL : 2; // 特权位
  BYTE P : 1; //P位
  BYTE SegLimit_16_19 : 4 ; //段限的16~19位
  BYTE AVL : 1;//AVL位
  BYTE Reserved : 1; //保留位，必须为0
  BYTE DorB : 1; //D/B位
  BYTE G : 1; //G位
  BYTE SegAddr_24_31; //基址的24~31位
}GDTITEM;
#else
typedef struct _GdtItem{ 
  WORD limit_low;
  WORD base_low;
  BYTE base_mid;
  BYTE attr1;
  BYTE limit_high_attr2;
  BYTE base_high;
}GDTITEM;

#endif

#if 0

/* GDT表 */
// 先暂时放这些东西,以后要用再扩展
typedef struct _Gdt{ 
  GDTITEM gdtNull;       //空段，Intel保留
  GDTITEM gdtSystemCode; //系统代码段
  GDTITEM gdtSystemDate; //系统数据段

  /* 系统调用门 */
  INVOKEGATE InvokeGate[2]; 

  // 任务段（TSS）描述符
  TSSITEM TSS[4];  //0(0x28)号给 0 号进程用 1
						//(0x30) 号给 A 进程用 
                        //2(0x38) 号给 B 进程用 
						//3(0x40) 号给时钟中断用。

  // 任务门
  TSSGATE TssGate; //0x48
}GDT;
#endif

/* GDTR寄存器所用描述符 */
typedef struct _Gdtr{ 
  unsigned short GdtLengthLimit;
  GDTITEM *GdtAddr;
}GDTR;

void gdt_init(void);
void Gdt_SetEntry(DWORD vector, DWORD base, DWORD limit, DWORD attribute);


#endif
