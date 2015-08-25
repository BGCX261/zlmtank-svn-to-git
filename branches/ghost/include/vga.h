#ifndef _VGA_H
#define _VGA_H


//�ı�ģʽ80*25*16ɫ

#define VGAMEM 0xB8000
#define COLS 80
#define LINES 25
#define TABSPACE 8
// ��ʾ���˿�
#define VGA_CRTIDX	0x3D4
#define VGA_CRTDATA	0x3D5
// VGA CRT �Ĵ������λ�üĴ���
#define VGA_CRT_CURHIGH 14
#define VGA_CRT_CURLOW 15
#define VGA_CRT_CURBEGIN 10
#define VGA_CRT_CUREND 11


//Ĭ�ϵ���ɫ��������0�����ְ�7
STATIC unsigned short color=0x0700;
//�������
STATIC int cx=0,cy=0;


typedef struct{
	int mode;//ģʽ��
	int width,height,colors;//��Ļ���ߣ���ɫ��
	char *modename;//ģʽ����
}VGA_INFO;


STATIC VGA_INFO vi;




// vga_init(): ��ʼ�� VGA ��ʾ
int vga_init(void);
// vga_putchar(): ����Ļ���һ���ַ� ch
void vga_putc(unsigned char ch);
// vga_putstr(): ����� '\0' ��β���ַ��� s
// ֮���Բ��� puts ���� putstr ��Ϊ�� puts() ����Ӧ���� s ���������һ�� '\n'
void vga_puts(unsigned char *s);
// vga_gotoxy(): ���ù��λ��
void vga_gotoxy(int x, int y);
// vga_getcx(): ���ع�������
int vga_getcx(void);
// vga_getcy(): ���ع��������
int vga_getcy(void);
// vga_clrscr(): ���������Ļ
void vga_clrscr(void);

int vga_getvmemitem(int x,int y);

enum {
	BLACK = 0,
	BLUE = 1,
	GREEN = 2,
	CYAN = 3,
	RED = 4,
	MAGENTA = 5,
	BROWN = 6,
	LIGHTGRAY = 7,
	DARKGRAY = 8,
	LIGHTBLUE = 9,
	LIGHTGREEN = 10,
	LIGHTCYAN = 11,
	LIGHTRED = 12,
	LIGHTMAGENTA = 13,
	YELLOW = 14,
	WHITE = 15,
};

// vga_textcolor(): �����ַ���ʾ��ɫΪ c ������ԭ������
// ���� -1 ���ý���������ʾ��ɫ
EXTERN int vga_textcolor(int c);
// vga_textbgcolor(): �����ַ���ʾ����ɫΪ c ������ԭ������
// ���� -1 ���ý���������ʾ��ɫ
EXTERN int vga_textbgcolor(int c);
//vga_bgcolor():������Ļ����ɫΪc,������ԭ������
int vga_bgcolor(int c);



typedef enum{
		VGA_CURSOR_HIDE = 0,
		VGA_CURSOR_NORMAL = 1,
		VGA_CURSOR_HALF = 2,
		VGA_CURSOR_FULL = 3,
		VGA_CURSOR_ACK = -1,
}VGA_CURSOR;

//vga_getinfo():�õ���ǰ��ʾ��Ϣ
EXTERN VGA_INFO* vga_getinfo(void);

// vga_showcursor(): ��ʾ�������
// 0 �رչ����ʾ 1 ��ͨ�����ʾ
// 2 һ���ַ���ʾ 3 ȫ�ַ���ʾ
// -1 �������ã������ص�ǰ����
EXTERN VGA_CURSOR vga_setcursor(VGA_CURSOR nshow);




#endif

