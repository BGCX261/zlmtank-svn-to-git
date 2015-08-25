del *.exe
del cc.cfg
make -Di386=-Di386 -fmakefile.32
ren cc.exe cc386.exe
del cc.cfg
make -fmakefile.32
ren cc.exe cc68k.exe