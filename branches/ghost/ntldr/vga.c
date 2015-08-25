#include <types.h>
#include <stdlib.h>
#include <io.h>
#include <vga.h>



int vga_init(void)
{
	vi.mode = 0x03;
	vi.width = COLS;
	vi.height = LINES;
	vi.colors = 16;
	vi.modename ="vga_03";

	
	
	cx=cy=0;
	vga_gotoxy(cx, cy);
	vga_textcolor(15);
	vga_setcursor(VGA_CURSOR_FULL);

	
	
	
	return 0;
}


static void vga_scrollup(void)
{
	int i;
	void *dest = (void *)VGAMEM;
	void *src = (void *)(VGAMEM + COLS * 2);
	unsigned short *vgamem = (unsigned short *)(VGAMEM + (LINES - 1) * COLS * 2);

	// 复制下面的内容掩盖上面，实现向上卷动
	memcpy(dest, src, (LINES - 1) * COLS * 2);

	// 清除最底下的一行
	for (i = 0; i < COLS; i++)
		vgamem[i] = ' ' | color;
	return;
}

// vga_putc(): 向屏幕输出一个字符 ch
void vga_putc(unsigned char ch)
{
	unsigned short *vgamem = (unsigned short *)VGAMEM;
	int x, y, i, ntab;

	// 没有直接操作内部的 cx 和 cy
	x = cx;
	y = cy;
	// 根据字符 ch 判定输出
	switch (ch) {
	case '\0':
		// 空字符不处理
		return;
	case '\b':
		// 退格
		// 定义退格操作是为了与键盘 kbd 互动
		// kbd 应该设置一个启动位置来避免 '\b' 删除原有字符
		if (x > 0)
			x--;
		else if (y > 0) {
			x = COLS - 1;
			y--;
		}
		vgamem[y * COLS + x] = ' ' | color;
		break;
	case '\n':
		// 换行并回车
		///??? FIXME
		///? 如果字符显示到行尾时，会把 '\n' 挤到下一行
		///? 这时 '\n' 又会重起一行
		x = 0;
		if (++y >= LINES) {
			vga_scrollup();
			y--;
		}
		break;
	case '\r':
		// 回车只是换行的一部分操作
		// 对于 DOS/WIN 来说 '\n' 只换行不回车的
		x = 0;
		break;
	case '\t':
		// TAB 制表键
		// 计算补 8 需要的空格
		ntab = TABSPACE - x % TABSPACE;
		// 然后依次输出这些空格
		for (i = 0; i < ntab; i++) {
			vgamem[y * COLS + x] = ' ' | color;
			if (x < COLS - 1)
				x++;
			else {
				x = 0;
				if (y < LINES - 1)
					y++;
				else {
					vga_scrollup();
					break;
				}
			}
		}
		break;
	default:
		// 默认的输出
		vgamem[y * COLS + x] = ch | color;
		if (++x >= COLS) {
			x = 0;
			if (++y >= LINES) {
				vga_scrollup();
				y--;
			}
		}
		break;
	}
	// 最后定位光标，更新 cx, cy
	vga_gotoxy(x, y);
	return;
}

// vga_puts(): 输出以 '\0' 结尾的字符串 s
// 之所以不是 puts 而是 puts 因为是 puts() 语义应该在 s 后面再输出一个 '\n'
void vga_puts(unsigned char *s)
{
	while (*s != 0)
		vga_putc(*s++);
	return;
}

// vga_gotoxy(): 设置光标位置
void vga_gotoxy(int x, int y)
{
	unsigned pos, flags;

	// 更新 cx, cy 并计算整屏的 pos
	cx = x;
	cy = y;
	pos = cy * COLS + cx;

	// 屏蔽 I/O 操作
	io_saveflag(flags);
	io_writebyte(VGA_CRTIDX, VGA_CRT_CURHIGH);
	io_writebyte(VGA_CRTDATA, (pos >> 8) & 0xFF);
	io_writebyte(VGA_CRTIDX, VGA_CRT_CURLOW);
	io_writebyte(VGA_CRTDATA, pos & 0xFF);
	io_restoreflag(flags);
	return;
}



// vga_getcx(): 返回光标横坐标
int vga_getcx(void)
{
	return cx;
}

// vga_getcy(): 返回光标纵坐标
int vga_getcy(void)
{
	return cy;
}

// vga_clrscr(): 清除整个屏幕
void vga_clrscr(void)
{
	
	int i;
	unsigned short *vgamem = (unsigned short *)VGAMEM;

	for (i = 0; i < COLS * LINES; i++)
		vgamem[i] = ' ' | color;
	vga_gotoxy(0, 0);
	return;
}

// vga_textcolor(): 设置字符显示颜色为 c 并返回原先设置
// 传递 -1 调用将不设置显示颜色
int vga_textcolor(int c)
{
	int oldc;

	oldc = (color >> 8) & 0x0F;
	if (c != -1) {
		color &= 0xF000;
		color |= (c << 8) & 0x0F00;
	}
	return oldc;
}

// vga_textbgcolor(): 设置字符显示背景色为 c 并返回原先设置
// 传递 -1 调用将不设置显示颜色
int vga_textbgcolor(int c)
{
	int oldc;

	oldc = (color >> 12) & 0x0F;
	if (c != -1) {
		color &= 0x0F00;
		color |= (c << 12) & 0xF000;
	}
	return oldc;
}

// vga_bgcolor(): 设置整屏显示背景色为 c 并返回原先设置
int vga_bgcolor(int c)
{
	int i, oldc;
	unsigned short *vgamem = (unsigned short *)VGAMEM;

	oldc = vga_textbgcolor(c);
	for (i = 0; i < COLS * LINES; i++) {
		vgamem[i] &= 0x0FFF;
		vgamem[i] |= color & 0xF000;
	}

	return oldc;
}


// 光标大小起始寄存器 0Ah
// bit0-4 起始扫描线，必须小于终止扫描线大小
// bit5 = 1 不显示光标，= 0 显示
// bit6-7 保留为 0
// 光标大小终止寄存器 0Bh
// bit0-4 终止扫描线，必须大于起始扫描线大小
// bit5-6 光标显示(闪烁)时间
// bit7 保留为 0
// 对于 vga 03h 来说每个字符是 16 条扫描线高
// 字符最顶是 0 号扫描线，最底是 15 号扫描线
// 指定的光标起始扫描线不能大于终止扫描线，否则不显示光标

// vga_showcursor(): 显示光标类型
// 0 关闭光标显示 1 普通光标显示
// 2 一半字符显示 3 全字符显示
// -1 不做设置，仅返回当前设置
VGA_CURSOR vga_setcursor(VGA_CURSOR nshow)
{
	VGA_CURSOR ret;
	static VGA_CURSOR old = VGA_CURSOR_NORMAL;

	ret = old;
	switch (nshow) {
	case VGA_CURSOR_HIDE:
		// 关闭光标显示
		io_writebyte(VGA_CRTIDX, VGA_CRT_CURBEGIN);
		io_writebyte(VGA_CRTDATA, 0x2F);
		old = nshow;
		break;
	case VGA_CURSOR_NORMAL:
		// 光标显示两条扫描线，启 13 止 15
		io_writebyte(VGA_CRTIDX, VGA_CRT_CURBEGIN);
		io_writebyte(VGA_CRTDATA, 0x0D);
		io_writebyte(VGA_CRTIDX, VGA_CRT_CUREND);
		io_writebyte(VGA_CRTDATA, 0x2F);
		old = nshow;
		break;
	case VGA_CURSOR_HALF:
		// 光标显示一半字符扫描线，启 7 止 15
		io_writebyte(VGA_CRTIDX, VGA_CRT_CURBEGIN);
		io_writebyte(VGA_CRTDATA, 0x07);
		io_writebyte(VGA_CRTIDX, VGA_CRT_CUREND);
		io_writebyte(VGA_CRTDATA, 0x2F);
		old = nshow;
		break;
	case VGA_CURSOR_FULL:
		// 光标显示全字符扫描线，启 0 止 15
		io_writebyte(VGA_CRTIDX, VGA_CRT_CURBEGIN);
		io_writebyte(VGA_CRTDATA, 0x00);
		io_writebyte(VGA_CRTIDX, VGA_CRT_CUREND);
		io_writebyte(VGA_CRTDATA, 0x2F);
		old = nshow;
		break;
	case VGA_CURSOR_ACK:
		break;
	}
	return ret;
}





