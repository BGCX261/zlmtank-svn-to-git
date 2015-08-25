#ifndef _GDT_H
#define _GDT_H



/* ����������ֵ˵�� */
#define	DA_32			0x4000	/* 32 λ��				*/
#define	DA_LIMIT_4K		0x8000	/* �ν�������Ϊ 4K �ֽ�			*/
#define	DA_DPL0			0x00	/* DPL = 0				*/
#define	DA_DPL1			0x20	/* DPL = 1				*/
#define	DA_DPL2			0x40	/* DPL = 2				*/
#define	DA_DPL3			0x60	/* DPL = 3				*/
/* �洢������������ֵ˵�� */
#define	DA_DR			0x90	/* ���ڵ�ֻ�����ݶ�����ֵ		*/
#define	DA_DRW			0x92	/* ���ڵĿɶ�д���ݶ�����ֵ		*/
#define	DA_DRWA			0x93	/* ���ڵ��ѷ��ʿɶ�д���ݶ�����ֵ	*/
#define	DA_C			0x98	/* ���ڵ�ִֻ�д��������ֵ		*/
#define	DA_CR			0x9A	/* ���ڵĿ�ִ�пɶ����������ֵ		*/
#define	DA_CCO			0x9C	/* ���ڵ�ִֻ��һ�´��������ֵ		*/
#define	DA_CCOR			0x9E	/* ���ڵĿ�ִ�пɶ�һ�´��������ֵ	*/
/* ϵͳ������������ֵ˵�� */
#define	DA_LDT			0x82	/* �ֲ��������������ֵ			*/
#define	DA_TaskGate		0x85	/* ����������ֵ				*/
#define	DA_386TSS		0x89	/* ���� 386 ����״̬������ֵ		*/
#define	DA_386CGate		0x8C	/* 386 ����������ֵ			*/
#define	DA_386IGate		0x8E	/* 386 �ж�������ֵ			*/
#define	DA_386TGate		0x8F	/* 386 ����������ֵ			*/
/* ѡ��������ֵ˵�� */
/* ����, SA_ : Selector Attribute */
#define	SA_RPL_MASK	0xFFFC
#define	SA_RPL0		0
#define	SA_RPL1		1
#define	SA_RPL2		2
#define	SA_RPL3		3

#define	SA_TI_MASK	0xFFFB
#define	SA_TIG		0
#define	SA_TIL		4



/*******************************
����һЩϵͳ��ʼ���ṹ
codeby:zlm
*******************************/

// ��������״̬�Σ�TSS���ṹ  
struct _Tss{ 
  WORD Link; // �����ֶ� 
  WORD UnUsed0; //δ�� 
 
  DWORD Esp0; // �㼶��ջָ�� 
  WORD SS0; // �㼶��ջ��ַ 
  WORD UnUsed1; 
 
  DWORD Esp1; 
  WORD SS1; 
  WORD UnUsed2; 
 
  DWORD Esp2; 
  WORD SS2; 
  WORD UnUsed3; 
 
  DWORD Cr3; 
  DWORD Eip; 
  DWORD Eflags; // �ڽ��̳�ʼ��ʱ����Ҫ���˱�־�еġ�IF���ж����λ������ 1 �������л����½��̲�����Ӧ�ж� 
  DWORD Eax; 
  DWORD Ecx; 
  DWORD Edx; 
  DWORD Ebx; 
  DWORD Esp; 
  DWORD Ebp; 
  DWORD Esi; 
  DWORD Edi; 

  //...ri,����ṹ̫������..
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
 
  WORD EnableIO; // I/O ���λͼ 
}TSS; 
 
/* ��������״̬�������� */ 
typedef struct _TSSItem{ 
  WORD LimitLength_0_15; // TSS ���޳��� 0~ 15 λ 
  WORD Base_0_15; // ��ַ�� 0~15 λ 
  BYTE Base_16_23; // ��ַ�� 16~23 λ 
  BYTE Reserved_1 : 1; // ����������Ϊ 1 
  BYTE B :1; // B λ��æλ�����ڽ��̳�ʼ��ʱ������Ϊ 0 ����ʾ�˽��̿��� 
  BYTE Reserved_010 :3; // ��������Ϊ 010 ; 
  BYTE DPL :2; // ��Ȩλ 
  BYTE P : 1; // Pλ������λ�� 
  BYTE LimitLength_16_19 : 4; // �޳��� 16~19 λ 
  BYTE AVL : 1; //AVL��������������ã�λ 
  BYTE Reserved_00 : 2; // ����������Ϊ 00 
  BYTE G : 1; // Gλ������λ�� 
  BYTE Base_24_31; // ��ַ�� 24~31 λ 
}TSSITEM; 
 
// ���������� 
typedef struct _TSSGate{ 
  WORD UnUsed0; // δ�ã���Ϊ0 
  WORD TSSSelector; // ����״̬�εĶ�ѡ��� 
  BYTE UnUsed1; // δ�ã���Ϊ0 
  BYTE Type_0101 : 4; // ���� 
  BYTE Reserved0_0 : 1; // ��Ϊ 0 ; 
  BYTE DPL : 2; // ��Ȩ�� 
  BYTE P : 1; // ����λ 
  WORD UnUsed2; // δ�ã���Ϊ0 
}TSSGATE;


/* �����Žṹ */ 
typedef struct _InvokeGate{ 
  WORD Offset_0_15; /* ƫ������ 0~15 λ */ 
  WORD SegSelector; /* ��ѡ��� */ 
  BYTE DWordCount : 5; /* ˫�ּ����ֶ� */ 
  BYTE Reserved_0 : 3; /* ��������Ϊ 0 */ 
  BYTE Type_1100 : 4; /* �����ֶΣ���������Ϊ 1100 ( 0xC ) */  
  BYTE DT_0 : 1; /* ��Ϊ 0 , �Ա�������һ��ϵͳ�õ������� */ 
  BYTE DPL : 2; /* ��Ȩ�� */ 
  BYTE P : 1; /* ����λ */ 
  WORD Offset_16_31; 
}INVOKEGATE; 

#if 0
typedef struct _GdtItem{  
  WORD SegLimit_0_15; // ����0~15λ
  WORD SegAddr_0_15; // ��ַ�� 0~15��λ
  BYTE SegAddr_16_23; //��ַ��16~23λ
  BYTE Type : 4;  //typeλ
  BYTE S : 1; //Sλ
  BYTE DPL : 2; // ��Ȩλ
  BYTE P : 1; //Pλ
  BYTE SegLimit_16_19 : 4 ; //���޵�16~19λ
  BYTE AVL : 1;//AVLλ
  BYTE Reserved : 1; //����λ������Ϊ0
  BYTE DorB : 1; //D/Bλ
  BYTE G : 1; //Gλ
  BYTE SegAddr_24_31; //��ַ��24~31λ
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

/* GDT�� */
// ����ʱ����Щ����,�Ժ�Ҫ������չ
typedef struct _Gdt{ 
  GDTITEM gdtNull;       //�նΣ�Intel����
  GDTITEM gdtSystemCode; //ϵͳ�����
  GDTITEM gdtSystemDate; //ϵͳ���ݶ�

  /* ϵͳ������ */
  INVOKEGATE InvokeGate[2]; 

  // ����Σ�TSS��������
  TSSITEM TSS[4];  //0(0x28)�Ÿ� 0 �Ž����� 1
						//(0x30) �Ÿ� A ������ 
                        //2(0x38) �Ÿ� B ������ 
						//3(0x40) �Ÿ�ʱ���ж��á�

  // ������
  TSSGATE TssGate; //0x48
}GDT;
#endif

/* GDTR�Ĵ������������� */
typedef struct _Gdtr{ 
  unsigned short GdtLengthLimit;
  GDTITEM *GdtAddr;
}GDTR;

void gdt_init(void);
void Gdt_SetEntry(DWORD vector, DWORD base, DWORD limit, DWORD attribute);


#endif
