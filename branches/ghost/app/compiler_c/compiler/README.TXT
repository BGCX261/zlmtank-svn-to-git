]Hi,
	This is the source for a C compiler that generates 386 or
m68K code.  Code generation is fairly good although the optimizer is
a little naive... the code itself should be highly portable although
there are some issues involved with porting from MSDOS that have to
be resolved; for example the size of LONG on unix machines is so big
some of the code generated gets messed up.

Code generation requires TASM to assemble the compiler output;
you can use the borland TLINK to produce executables, or the 
Watcom WLINK can also be used.  PMODE systems written by TRAN are used
for the DPMI interface.

This compiler source is a companion file to the two files:

ccdl139e.zip - C compiler executables with DOS libraries (for DOS)
ccdl139l.zip - C compiler run-time libraries

Which should also be available on the distribution site you got this
from.  Read the LICENSE.TXT file for redistribution information.	

See license.txt for licensing information.

Notes:
	This version has an inline assembler
	This version compiles itself and the resulting image runs!
	do NOT use tasm 4.1!!! (it is buggy)

This program is being distributed through a variety of media; on the
internet new releases may be found on:

ftp.std.com:/pub/os-code  ( my site, updates get here quicker )
ftp.simtel.net
garbo.uwasa.fi


David Lindauer
1428 Hepburn Ln. Apt #1
Louisville, KY 40204

email:  gclind01@starbase.spd.louisville.edu