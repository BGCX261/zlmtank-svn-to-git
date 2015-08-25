set NTLDR_DIR=ntldr
set LIBS_DIR=libs
set FS_DIR=filesystem
set COMPILER_FLAG=-fpack-struct -std=c99 -c -fno-builtin

:clean

del /Q %NTLDR_DIR%\*.*


:build

nasm ..\%NTLDR_DIR%\ntldr16.asm -o %NTLDR_DIR%\ntldr16.s -i ..\%NTLDR_DIR%\

nasm -f elf ..\%NTLDR_DIR%\ntldr32pre.asm -o %NTLDR_DIR%\ntldr32pre.s -i ..\%NTLDR_DIR%\
objcopy -R .note -R .comment -S -O binary %NTLDR_DIR%\ntldr32pre.s %NTLDR_DIR%\ntldr32pre.out

nasm -f elf ..\%NTLDR_DIR%\ntldr32.asm -o %NTLDR_DIR%\ntldr32.s -i ..\%NTLDR_DIR%\



gcc %COMPILER_FLAG% ..\%NTLDR_DIR%\ntldr32.c -o %NTLDR_DIR%\ntldr32.o -I %CD%\..\include

gcc %COMPILER_FLAG% ..\%NTLDR_DIR%\vga.c -o %NTLDR_DIR%\vga.o -I %CD%\..\include

gcc %COMPILER_FLAG% ..\%NTLDR_DIR%\gdt.c -o %NTLDR_DIR%\gdt.o -I %CD%\..\include

gcc %COMPILER_FLAG% ..\%NTLDR_DIR%\idt.c -o %NTLDR_DIR%\idt.o -I %CD%\..\include

gcc %COMPILER_FLAG% ..\%NTLDR_DIR%\irq.c -o %NTLDR_DIR%\irq.o -I %CD%\..\include
nasm -f elf ..\%NTLDR_DIR%\irq_s.asm -o %NTLDR_DIR%\irq_s.s -i ..\%NTLDR_DIR%\

gcc %COMPILER_FLAG% ..\%NTLDR_DIR%\traps.c -o %NTLDR_DIR%\traps.o -I %CD%\..\include
nasm -f elf ..\%NTLDR_DIR%\traps_s.asm -o %NTLDR_DIR%\traps_s.s -i ..\%NTLDR_DIR%\

gcc %COMPILER_FLAG% ..\%NTLDR_DIR%\time.c -o %NTLDR_DIR%\time.o -I %CD%\..\include

gcc %COMPILER_FLAG% ..\%NTLDR_DIR%\serial.c -o %NTLDR_DIR%\serial.o -I %CD%\..\include

gcc %COMPILER_FLAG% ..\%NTLDR_DIR%\page.c -o %NTLDR_DIR%\page.o -I %CD%\..\include

gcc %COMPILER_FLAG% ..\%NTLDR_DIR%\floppy.c -o %NTLDR_DIR%\floppy.o -I %CD%\..\include

ld -o %NTLDR_DIR%\ntldr32.bin -Ttext 0x100000 -e ntldr_entry %NTLDR_DIR%\ntldr32.s %NTLDR_DIR%\ntldr32.o %LIBS_DIR%\stdlib.o %NTLDR_DIR%\vga.o %NTLDR_DIR%\gdt.o %NTLDR_DIR%\idt.o %NTLDR_DIR%\irq.o %NTLDR_DIR%\irq_s.s %NTLDR_DIR%\traps.o %NTLDR_DIR%\traps_s.s %NTLDR_DIR%\time.o %NTLDR_DIR%\serial.o %NTLDR_DIR%\page.o %LIBS_DIR%\syscall.o %NTLDR_DIR%\floppy.o %FS_DIR%\fat12.o

nm -A %NTLDR_DIR%\ntldr32.bin > sym1.txt
readelf -s %NTLDR_DIR%\ntldr32.bin > sym2.txt

objcopy -R .note -R .comment -S -O binary %NTLDR_DIR%\ntldr32.bin %NTLDR_DIR%\ntldr32.out

mkimg %NTLDR_DIR%\ntldr %NTLDR_DIR%\ntldr16.s %NTLDR_DIR%\ntldr32pre.out %NTLDR_DIR%\ntldr32.out