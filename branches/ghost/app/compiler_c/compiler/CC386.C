#include <stdio.h>
void main() {
	static unsigned long mem = 0;
	asm {
		mov bx, 0xFFFF
		mov ah, 0x48
		int 0x21
		mov word ptr [mem], bx
	}
	printf( "Memory %lu\n", mem*16UL );
}