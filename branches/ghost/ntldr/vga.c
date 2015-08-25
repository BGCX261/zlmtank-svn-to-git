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

	// ��������������ڸ����棬ʵ�����Ͼ�
	memcpy(dest, src, (LINES - 1) * COLS * 2);

	// �������µ�һ��
	for (i = 0; i < COLS; i++)
		vgamem[i] = ' ' | color;
	return;
}

// vga_putc(): ����Ļ���һ���ַ� ch
void vga_putc(unsigned char ch)
{
	unsigned short *vgamem = (unsigned short *)VGAMEM;
	int x, y, i, ntab;

	// û��ֱ�Ӳ����ڲ��� cx �� cy
	x = cx;
	y = cy;
	// �����ַ� ch �ж����
	switch (ch) {
	case '\0':
		// ���ַ�������
		return;
	case '\b':
		// �˸�
		// �����˸������Ϊ������� kbd ����
		// kbd Ӧ������һ������λ�������� '\b' ɾ��ԭ���ַ�
		if (x > 0)
			x--;
		else if (y > 0) {
			x = COLS - 1;
			y--;
		}
		vgamem[y * COLS + x] = ' ' | color;
		break;
	case '\n':
		// ���в��س�
		///??? FIXME
		///? ����ַ���ʾ����βʱ����� '\n' ������һ��
		///? ��ʱ '\n' �ֻ�����һ��
		x = 0;
		if (++y >= LINES) {
			vga_scrollup();
			y--;
		}
		break;
	case '\r':
		// �س�ֻ�ǻ��е�һ���ֲ���
		// ���� DOS/WIN ��˵ '\n' ֻ���в��س���
		x = 0;
		break;
	case '\t':
		// TAB �Ʊ��
		// ���㲹 8 ��Ҫ�Ŀո�
		ntab = TABSPACE - x % TABSPACE;
		// Ȼ�����������Щ�ո�
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
		// Ĭ�ϵ����
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
	// ���λ��꣬���� cx, cy
	vga_gotoxy(x, y);
	return;
}

// vga_puts(): ����� '\0' ��β���ַ��� s
// ֮���Բ��� puts ���� puts ��Ϊ�� puts() ����Ӧ���� s ���������һ�� '\n'
void vga_puts(unsigned char *s)
{
	while (*s != 0)
		vga_putc(*s++);
	return;
}

// vga_gotoxy(): ���ù��λ��
void vga_gotoxy(int x, int y)
{
	unsigned pos, flags;

	// ���� cx, cy ������������ pos
	cx = x;
	cy = y;
	pos = cy * COLS + cx;

	// ���� I/O ����
	io_saveflag(flags);
	io_writebyte(VGA_CRTIDX, VGA_CRT_CURHIGH);
	io_writebyte(VGA_CRTDATA, (pos >> 8) & 0xFF);
	io_writebyte(VGA_CRTIDX, VGA_CRT_CURLOW);
	io_writebyte(VGA_CRTDATA, pos & 0xFF);
	io_restoreflag(flags);
	return;
}



// vga_getcx(): ���ع�������
int vga_getcx(void)
{
	return cx;
}

// vga_getcy(): ���ع��������
int vga_getcy(void)
{
	return cy;
}

// vga_clrscr(): ���������Ļ
void vga_clrscr(void)
{
	
	int i;
	unsigned short *vgamem = (unsigned short *)VGAMEM;

	for (i = 0; i < COLS * LINES; i++)
		vgamem[i] = ' ' | color;
	vga_gotoxy(0, 0);
	return;
}

// vga_textcolor(): �����ַ���ʾ��ɫΪ c ������ԭ������
// ���� -1 ���ý���������ʾ��ɫ
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

// vga_textbgcolor(): �����ַ���ʾ����ɫΪ c ������ԭ������
// ���� -1 ���ý���������ʾ��ɫ
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

// vga_bgcolor(): ����������ʾ����ɫΪ c ������ԭ������
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


// ����С��ʼ�Ĵ��� 0Ah
// bit0-4 ��ʼɨ���ߣ�����С����ֹɨ���ߴ�С
// bit5 = 1 ����ʾ��꣬= 0 ��ʾ
// bit6-7 ����Ϊ 0
// ����С��ֹ�Ĵ��� 0Bh
// bit0-4 ��ֹɨ���ߣ����������ʼɨ���ߴ�С
// bit5-6 �����ʾ(��˸)ʱ��
// bit7 ����Ϊ 0
// ���� vga 03h ��˵ÿ���ַ��� 16 ��ɨ���߸�
// �ַ���� 0 ��ɨ���ߣ������ 15 ��ɨ����
// ָ���Ĺ����ʼɨ���߲��ܴ�����ֹɨ���ߣ�������ʾ���

// vga_showcursor(): ��ʾ�������
// 0 �رչ����ʾ 1 ��ͨ�����ʾ
// 2 һ���ַ���ʾ 3 ȫ�ַ���ʾ
// -1 �������ã������ص�ǰ����
VGA_CURSOR vga_setcursor(VGA_CURSOR nshow)
{
	VGA_CURSOR ret;
	static VGA_CURSOR old = VGA_CURSOR_NORMAL;

	ret = old;
	switch (nshow) {
	case VGA_CURSOR_HIDE:
		// �رչ����ʾ
		io_writebyte(VGA_CRTIDX, VGA_CRT_CURBEGIN);
		io_writebyte(VGA_CRTDATA, 0x2F);
		old = nshow;
		break;
	case VGA_CURSOR_NORMAL:
		// �����ʾ����ɨ���ߣ��� 13 ֹ 15
		io_writebyte(VGA_CRTIDX, VGA_CRT_CURBEGIN);
		io_writebyte(VGA_CRTDATA, 0x0D);
		io_writebyte(VGA_CRTIDX, VGA_CRT_CUREND);
		io_writebyte(VGA_CRTDATA, 0x2F);
		old = nshow;
		break;
	case VGA_CURSOR_HALF:
		// �����ʾһ���ַ�ɨ���ߣ��� 7 ֹ 15
		io_writebyte(VGA_CRTIDX, VGA_CRT_CURBEGIN);
		io_writebyte(VGA_CRTDATA, 0x07);
		io_writebyte(VGA_CRTIDX, VGA_CRT_CUREND);
		io_writebyte(VGA_CRTDATA, 0x2F);
		old = nshow;
		break;
	case VGA_CURSOR_FULL:
		// �����ʾȫ�ַ�ɨ���ߣ��� 0 ֹ 15
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





