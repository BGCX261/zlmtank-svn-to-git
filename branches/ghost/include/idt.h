#ifndef _IDT_H
#define _IDT_H

#include <gdt.h>

// ���ݽṹ���ֽڶ��룬�����Զ���չ����
//#pragma pack(push, 1)

// IDTGATE: �жϷ����ţ�8 ���ֽ�
// i386 �£����ݽṹ����ַ������ 4 �ı���(32 λ DWORD)
// i386 �������ţ�Ӳ���ж�/ϵͳ���á��쳣/���塢�����л�
// ��ν��ֻ���л�����˼����һ���ط��л���һ���ط���ͨ����
// �л���ԭ����Ӳ���豸��������ʱ�뱨�浽 CPU (�ж�)
// ����ִ�е�ָ����쳣���޷������������� (����)
// �û�����Ҫʹ��ϵͳ�ں���ʵ�ֵĹ��� (����)
// ��Щ������ CPU ������ָ��ִ������(��ָ��ִ��״̬)
// ��ô��i386 ʹ��һ��ȫ�ֵ�����Ϊÿһ������ƶ�����ʽ
// ÿһ��������Ŀ����һ���ţ�����ʲô��ʲô��
// ��������Щ������CPU ֹͣ��ǰ�Ķ����������ֳ�״̬
// �����ŵ���Ϣ������ CPU ��״̬����ת���ŵ���һͷ��ʼִ��
// �ȵ�������ָ��ִ����Ϻ�ת���ŵ���һͷ������ԭ�ȵ�ָ��
// ʹ���Ŷ�����ֱ����ָ����ת���������Ϊ�����ǲ�ȷ����
// �޷�Ԥ�Ȼ�֪ʲôʱ��Ӳ��������Ϣ�Լ�ָ��ִ���쳣
// �����������Ҫ CPU ���û�̬ת���ں�̬(ring3 -> ring0)
// ����ͨ����ʱ��CPU Ҳ��Ҫ����һϵ�еļȶ�����
#if 0
typedef struct {
	// �ж�ִ�д���ƫ�����ĵ� 16 λ
	unsigned short offset_low;
	// ѡ������Ҳ���ǼĴ���
	// ������ GDT �е�ƫ��ֵ
	// �����Ŵ˴�Ϊ TSS ��ѡ���
	unsigned short selector;
	// ����λ��ʼ��Ϊ��
	unsigned char reserved;
	// IDT �е��ŵ�����
	// ��һ��Ŀ��ѡ����û����ǣ�浽��ȷ������������ʱ���ͻ������쳣 13
	unsigned char type:4;
	// �α�ʶλ = 0
	unsigned char segflag:1;
	// �ж��ŵ�Ȩ�޵ȼ� DPL
	unsigned char dpl:2;
	// ���ֱ�־λ P
	// �ж������������Ƿ���Ч = 1 ��Ч
	unsigned char present:1;
	// �ж�ִ�д���ƫ�����ĸ� 16 λ
	unsigned short offset_high;
} IDTGATE;
#else
typedef struct {
  WORD offset_low;
  WORD selector;
  BYTE dcount;     //ϵͳ����Ϊreserved��
  BYTE attr;
  WORD offset_high;
}IDTGATE;

#endif

//#pragma pack(pop)



#if 0
// idt ���������������
// 0101b i386 ������ ���������л�
#define TYPE_TASK 0x05

// 1100b i386 ������ �ı���Ȩ����(��������Ų��� IDT �� GDT/LDT)
// 1110b i386 �ж��� ����Ӳ���жϷ����ӳ���
#define TYPE_INTERRUPT 0x0E
// 1111b i386 ������ ȷ���쳣�����ӳ���
#define TYPE_TRAP 0x0F

// idt ���������Ȩ��
// 00b ring0 ��ʾ�ں˼�(ring������windows��˵��,��)
#define DPL_KERNEL 0
// 11b ring3 ��ʾ�û���
#define DPL_USER 3
//����ƺ�������ring1,ring2,źҲ����
#endif

// idt_init(): ��ʼ�� idt(���� IDT λ��)
void idt_init(void);

// idt_setgate(): ���� idt ����
// ָ�� IDT ������ n �ķ�������ַ addr
// ������Ϊ type ��Ȩ��Ϊ dpl
void Idt_SetEntry(DWORD vector, BYTE desc_type, void* handler);


// ���������ţ���Ȩ�� 0
//#define idt_settaskgate(n, addr) \
//	idt_setgate((n), TYPE_TASK, DPL_KERNEL, (unsigned)(addr))
#define idt_settaskgate(n, addr) \
	Idt_SetEntry((n), DA_TaskGate, (unsigned)(addr))


// �����ж��ţ���Ȩ�� 0
//#define idt_setintrgate(n, addr) \
//	idt_setgate((n), TYPE_INTERRUPT, DPL_KERNEL, (unsigned)(addr))
#define idt_setintrgate(n, addr) \
	Idt_SetEntry((n), DA_386IGate, (addr))

// ����ϵͳ���ã���Ȩ�� 3

#define idt_setusergate(n, addr) \
	Idt_SetEntry((n), DA_386IGate, (addr))


// ���������ţ���Ȩ�� 0
//#define idt_settrapgate(n, addr) \
//	idt_setgate((n), TYPE_TRAP, DPL_KERNEL, (unsigned)(addr))
#define idt_settrapgate(n, addr) \
	Idt_SetEntry((n), DA_386TGate, (addr))


// �����û��������ţ���Ȩ�� 3
//#define idt_setusergate(n, addr) \
//	idt_setgate((n), TYPE_TRAP, DPL_USER, (unsigned)(addr))

// IDT_REGS: �жϴ���Ĵ�����ջ�ṹ
typedef struct {
	// pusha 8 ��ͨ�üĴ���
	unsigned edi, esi, ebp, esp, ebx, edx, ecx, eax;
	// �ֱ� push �� 4 ���μĴ���
	unsigned ds, es, fs, gs;
	// �ж������źʹ����
	unsigned idx, errno;
	// �����ַ�ͶΣ���־�Ĵ������û� ESP �� SS
	unsigned eip, cs, eflags, uesp, uss;
} IDT_REGS;

typedef struct _idtr{
  // ���޳����ֽ�
  unsigned short limit;
  // ����ַ��ʵ�������ַ
  unsigned long base;
}IDTR;


#endif
