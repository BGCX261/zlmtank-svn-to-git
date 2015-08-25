set FS_DIR=filesystem
set FS_INCLUDE=..\include
set COMPILER_FLAG=-fpack-struct -std=c99 -c -fno-builtin

:clean

del /Q %FS_DIR%\*.*


:build

gcc %COMPILER_FLAG% ..\%FS_DIR%\fat12.c -o %FS_DIR%\fat12.o -I %FS_INCLUDE%
