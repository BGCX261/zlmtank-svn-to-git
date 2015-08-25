#ifndef _VGA_H
#define _VGA_H


//文本模式80*25*16色

#define VGAMEM 0xB8000
#define COLS 80
#define LINES 25
#define TABSPACE 8
// 显示卡端口
#define VGA_CRTIDX	0x3D4
#define VGA_CRTDATA	0x3D5
// VGA CRT 寄存器光标位置寄存器
#define VGA_CRT_CURHIGH 14
#define VGA_CRT_CURLOW 15
#define VGA_CRT_CURBEGIN 10
#define VGA_CRT_CUREND 11


//默认的颜色：背景黑0，文字白7
STATIC unsigned short color=0x0700;
//光标坐标
STATIC int cx=0,cy=0;


typedef struct{
	int mode;//模式号
	int width,height,colors;//屏幕宽，高，颜色数
	char *modename;//模式名称
}VGA_INFO;


STATIC VGA_INFO vi;




// vga_init(): 初始化 VGA 显示
int vga_init(void);
// vga_putchar(): 向屏幕输出一个字符 ch
void vga_putc(unsigned char ch);
// vga_putstr(): 输出以 '\0' 结尾的字符串 s
// 之所以不是 puts 而是 putstr 因为是 puts() 语义应该在 s 后面再输出一个 '\n'
void vga_puts(unsigned char *s);
// vga_gotoxy(): 设置光标位置
void vga_gotoxy(int x, int y);
// vga_getcx(): 返回光标横坐标
int vga_getcx(void);
// vga_getcy(): 返回光标纵坐标
int vga_getcy(void);
// vga_clrscr(): 清除整个屏幕
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

// vga_textcolor(): 设置字符显示颜色为 c 并返回原先设置
// 传递 -1 调用将不设置显示颜色
EXTERN int vga_textcolor(int c);
// vga_textbgcolor(): 设置字符显示背景色为 c 并返回原先设置
// 传递 -1 调用将不设置显示颜色
EXTERN int vga_textbgcolor(int c);
//vga_bgcolor():设置屏幕背景色为c,并返回原先设置
int vga_bgcolor(int c);



typedef enum{
		VGA_CURSOR_HIDE = 0,
		VGA_CURSOR_NORMAL = 1,
		VGA_CURSOR_HALF = 2,
		VGA_CURSOR_FULL = 3,
		VGA_CURSOR_ACK = -1,
}VGA_CURSOR;

//vga_getinfo():得到当前显示信息
EXTERN VGA_INFO* vga_getinfo(void);

// vga_showcursor(): 显示光标类型
// 0 关闭光标显示 1 普通光标显示
// 2 一半字符显示 3 全字符显示
// -1 不做设置，仅返回当前设置
EXTERN VGA_CURSOR vga_setcursor(VGA_CURSOR nshow);




#endif

