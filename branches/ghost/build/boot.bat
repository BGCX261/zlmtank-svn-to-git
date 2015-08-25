set BOOT_DIR=boot


:clean
del /Q %BOOT_DIR%\*.*


:build
nasm ..\%BOOT_DIR%\bootfloppy.asm -o %BOOT_DIR%\bootfloppy.sec
nasm ..\%BOOT_DIR%\bootCDROM.asm -o %BOOT_DIR%\bootCDROM.sec
nasm ..\%BOOT_DIR%\bootHD.asm -o %BOOT_DIR%\bootHD.sec
nasm ..\%BOOT_DIR%\boot12s.asm -o %BOOT_DIR%\boot12s.sec
nasm ..\%BOOT_DIR%\bootUboot.asm -o %BOOT_DIR%\bootUboot.sec
