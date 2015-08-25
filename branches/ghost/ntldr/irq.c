#include <types.h>
#include <stdlib.h>
#include <irq.h>
#include <io.h>
#include <ntldr.h>
#include <idt.h>

// Ӳ���ж� IDT ��ʼ������
#define IRQBASE 0x20
// ÿ�� 8259 �� 8 �� IRQ
#define I8259NUM 8


// 8259 �жϿ�����
#define IRQ_MASTERBASE    0x20
#define IRQ_MASTERCTLMASK 0x21
#define IRQ_SLAVEBASE     0xA0
#define IRQ_SLAVECTLMASK  0xA1


// init8259(): ��ʼ�� 8259 �жϿ�����
// ��ѧ΢�ͼ����������������
// �������Ĵ�������Ǳ���ģ�������Ψһ��
// IO ����������ת CPU �ȴ� 8259 ����������
//
// ICW Initialization Command Words ��ʼ��������
// OCW Operation Command Words ����������
//
// ��Ƭ ��Ƭ ����
// 20h  A0h  д�� ICW1 OCW2 OCW3
// д�� ICW1 ʱ bit4 = 1
// д�� OCW2 �� OCW3 bit4 = 0
// ���� bit3 = 0 Ϊ OCW2 ���� = 1 Ϊ OCW3
//
// ��Ƭ ��Ƭ ����
// 21h  A1h  д�� ICW2 ICW3 ICW4 �� OCW1
// д�� ICW2 ICW3 ICW4 �� ICW1 ѡ�������д��
// û�в��� ICW1 Ĭ��Ϊд�� OCW1
static void init8259(void)
{
  // �� ICW1 �����ֵ� 20h(��) �� A0h(��) �˿�
  // ICW1 �� 8259 ��ʼ����ǰ����ģ������ֽ����£�
  // bit0 = 1 ָ���� 21h/A1h Ҫ���� ICW4 ����(ICW2 ICW3 ICW4 ˳����)
  // bit1 = 0 ������ʽ����Ƭ 8259 ���������ǵ�Ƭ(8088)
  // bit3 = 0 ���ش�����ʽ��= 1 Ϊ��ƽ����
  // bit4 = 1 ������� 1�� = 0 ʱΪд�� OCW2 �� OCW3
  // ����λ����Ϊ 0
  io_writebyte_p(IRQ_MASTERBASE, 0x11);
  io_writebyte_p(IRQ_SLAVEBASE, 0x11);
  
  // ���� ICW1 bit0 = 1 ���η��� ICW2
  // ICW2 Ϊ�ж������֣�����Ӳ���жϵ� IDT ���ж�������
  // bit3-7 Ϊ�����Ÿ� 5 λ���� 3 λ(bit0-2)�� 8259 �Զ���д
  // Ҳ����˵ ICW2 ֻ��ָ�� 8259 ��һ������ IRQ0
  // �����ƶ�������Ϊ 8 �ı���
  // ���� 8259 ����ݵ�һ�����ŵ��������Զ��������� 7 ������
  // ICW2 ICW3 ICW4 ʹ�ö˿� 21h(��) �� A1h(��)
  // ��Ƭ����Ϊ 20h �� 27h
  io_writebyte_p(IRQ_MASTERCTLMASK, IRQBASE);
  // ��Ƭ��������Ϊ 28h �� 2Fh
  io_writebyte_p(IRQ_SLAVECTLMASK, IRQBASE + I8259NUM);
  
  // ���� ICW1 bit0 = 1 ������ ICW2 �󣬱���������� ICW3
  // ICW3 �Ǽ��������֡�����Ƭ����д�룬���岻ͬ
  // ��Ƭ bit0-7 = 1 �Ǳ�����Ӧ�� IRQ0-7 ���д�Ƭ
  // 04h: bit2 = 1 IRQ2 �Ӵ�Ƭ
  io_writebyte_p(IRQ_MASTERCTLMASK, 0x04);
  // ��Ƭ bit0-2 ��Ч��˵���伶������Ƭ�Ǹ�����
  // bit3-7 ����������Ϊ 0
  // 02h = 2 ������Ƭ IRQ2 ��
  io_writebyte_p(IRQ_SLAVECTLMASK, 0x02);
  
  // ���� ICW1 bit0 = 1 ������ ICW3 �󣬱���������� ICW4
  // ICW4 �жϷ�ʽ�֣����� 8259 �Ļ���������ʽ
  // bit0 = 1 CPU Ϊ 16 λ 80x86��= 0 �� 8 λ 8080/8085
  // bit1 = 1 �Զ��жϽ����� = 0 ���Զ�
  // bit2 = 1 ��Ƭ = 0 ��Ƭ
  // bit3 = 1 �����߻��� = 0 �ǻ��巽ʽ
  // bit4 = 1 ����ȫǶ�׷�ʽ = 0 ��ͨȫǶ�׷�ʽ
  // bit5-7 ����������Ϊ 0
  // ��ͨȫǶ�׷�ʽ�£��ж�����Ȩ�Ӹߵ���˳��Ϊ
  // IRQ0 IRQ1 IRQ2  IRQ8 IRQ9 IRQ10 IRQ11 IRQ12
  // IRQ13 IRQ14 IRQ15  IRQ3 IRQ4 IRQ5 IRQ6 IRQ7
  io_writebyte_p(IRQ_MASTERCTLMASK, 0x01);
  io_writebyte_p(IRQ_SLAVECTLMASK, 0x01);
  ///? ���Ϲ��� ICW3 ���ԡ�16/32 λ΢��ԭ��������Լ��ӿڼ�����p181
  ///? Ǯ���ݡ����α�������е��ҵ������ 2001.7 ISBN 7-111-08912-X
  ///? �ƺ�������������� 01h Ҫ��һ��(����Ƭһ����������)
  
  // ���� ICW1 bit0 = 1 ���� ICW2 ICW3 ICW4 ���
  
  // Ĭ�Ϸ���Ϊ OCW1 ����
  // OCW1 ���������֣�����д���ж����μĴ��� IMR
  // bit0-7 ��Ӧ 8259 ���� IRQ0-7
  // = 1 ���θ��жϣ�= 0 ������ж�
  // FFh ��ֹ����
  io_writebyte_p(IRQ_MASTERCTLMASK, 0xFF);
  io_writebyte_p(IRQ_SLAVECTLMASK, 0xFF);
  return;
}

// irq_enableirq(): ���� IRQn Ӳ���ж�
// IRQn ��Ƭ      n+  ��Ƭ
// 0    ��ʱ��     8 ʵʱʱ��
// 1    ����       9 ���� 0Ah �ж�
// 2    ������Ƭ  10 ����
// 3    ���п� 2  11 ����
// 4    ���п� 1  12 PS/2 ���(��ǰ����)
// 5    ���п� 2  13 ��ѧЭ������
// 6    ����      14 Ӳ��(��ǰ����)
// 7    ���п� 1  15 ����
void irq_enableirq(IRQ_NUM n)
{
	unsigned char mask;
	unsigned int port;

	// ѡ������˿�
	port = n < 8 ? IRQ_MASTERCTLMASK : IRQ_SLAVECTLMASK;
	// read: xxxx xxxx xxxx xxxx
	// 1<<n: 1111 1111 11n1 1111 : [n] = 0
	// mask: xxxx xxxx xx0x xxxx
	// ��λ����ѭ����λ
	mask = io_readbyte_p(port) & (~(1 << n));
	io_writebyte_p(port, mask);
	return;
}

// irq_disableirq(): ���� IRQn Ӳ���ж�
void irq_disableirq(IRQ_NUM n)
{
	unsigned char mask;
	unsigned int port;

	// ѡ������˿�
	port = n < 8 ? IRQ_MASTERCTLMASK : IRQ_SLAVECTLMASK;
	// read: xxxx xxxx xxxx xxxx
	// 1<<n: 0000 0000 00n0 0000 : [n] = 1
	// mask: xxxx xxxx xx1x xxxx
	// ��λ����ѭ����λ
	mask = io_readbyte_p(port) | (1 << n);
	io_writebyte_p(port, mask);
	return;
}

// �����жϴ�������� irq_s.asm �ж���
// ��Щ�����ʵ�ֲ��ܱ� C ���Եȸ߼����Եĺ���ʵ��
// ԭ���Ǹ߼��������˴�������Ĺ���
// �����жϷ������ʹ��ָ�� iret ����
// ������ C �������� ret ����ָ��
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

// IRQ ��Ҫ���� IDT ���жϷ��������
static IRQ_FUN irqfun[I8259NUM*2];

// unexpected(): ȱʡӲ���жϴ������
static void unexpected(int irq)
{
	// Ӳ���жϴ����������ԭ����û�в����ġ�����
	// "Unexpected IRQ #%d\n"
	NTLDR_PRINTF("Unexpected IRQ #%d\n", irq);
	return;
}

// irq_setirqfun(): �� #irq ָ��������� fn
int irq_setirqfun(IRQ_NUM irq, IRQ_FUN fn)
{
	// #irq �Ƿ���ȷ
	if (irq < 0 || irq >= sizeof(irqfun) / sizeof(irqfun[0])) {
		// "Invalid IRQ #%d\n"
		NTLDR_PRINTF("Invalid IRQ #%d\n", irq);
	}

	// ����Ҫ����
	if (irqfun[irq] == fn)
		return 0;

	// �ظ�����
	if( irqfun[irq] != (IRQ_FUN)unexpected) {
		// "That have set one handler on IRQ #%d\n"
		NTLDR_PRINTF("That have set one handler on IRQ #%d\n", irq);
	}

	// ����ǰ�Ƚ�ֹ��Ӳ���ж�
	// ��ʵ����֮ǰ��Ӳ���жϱ������ǽ�ֹ��
	irq_disableirq(irq);
	//����irq�Ĵ�����
	irqfun[irq] = fn;
	// ��Ҫ��Ϊ�˴�
	irq_enableirq(irq);

	return 0;
}

// irq_init(): ��ʼ��Ӳ���жϷ���
void irq_init(void)
{
	int i;
	// Ӳ���ж�оƬ��ʼ��
	init8259();
	// ���� idt �������
	idt_setintrgate(IRQBASE + 0, irq0);
	idt_setintrgate(IRQBASE + 1, irq1);
	idt_setintrgate(IRQBASE + 2, irq2);
	idt_setintrgate(IRQBASE + 3, irq3);
	idt_setintrgate(IRQBASE + 4, irq4);
	idt_setintrgate(IRQBASE + 5, irq5);
	idt_setintrgate(IRQBASE + 6, irq6);
	idt_setintrgate(IRQBASE + 7, irq7);
	idt_setintrgate(IRQBASE + 8, irq8);
	idt_setintrgate(IRQBASE + 9, irq9);
	idt_setintrgate(IRQBASE + 10, irq10);
	idt_setintrgate(IRQBASE + 11, irq11);
	idt_setintrgate(IRQBASE + 12, irq12);
	idt_setintrgate(IRQBASE + 13, irq13);
	idt_setintrgate(IRQBASE + 14, irq14);
	idt_setintrgate(IRQBASE + 15, irq15);
	// ��ʼ���������飬�������Լ���ֵ
	for (i = 0; i < sizeof(irqfun) / sizeof(irqfun[0]); i++)
		irqfun[i] = (IRQ_FUN)unexpected;
	//����ϵͳ����
extern int *systemcall;
extern void irq_syscall(void);
	idt_setusergate(SYSCALL_INT ,irq_syscall);
	return;
}

// ackicq(): �����ж�ѭ��
static void ackirq(int irq)
{
#define ENABLEINT 0x20
	// 0x20 = 0010 0000b
	// bit3-4 = 00b -> OCW2 �жϽ���������Ȩ������
	// bit0-3 = 000b ȡ������Ȩ�Զ�ѭ��
	io_writebyte(IRQ_MASTERBASE, ENABLEINT);
	if(irq > 8)
		io_writebyte(IRQ_SLAVEBASE, ENABLEINT);
	return;
}
extern void sc_null();

// doirq(): Ӳ���жϴ��� C �ӳ���
// �������� irq_s.asm �Ļ�����������
void doirq(IDT_REGS regs)
{
	irq_disableirq(regs.idx);
	ackirq(regs.idx);
	irq_disable();
	if (regs.idx == 0x80)
	{
		sc_null();
	}
	else
	{
		irqfun[regs.idx]();
	}
	irq_enable();
	irq_enableirq(regs.idx);
	return;
}





void* SystemCallTable[] = {
	//0
	sc_null
};

void sc_null()
{
	NTLDR_PRINTF("syscall: NULL");
}
