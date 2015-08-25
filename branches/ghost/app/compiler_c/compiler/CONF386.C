/*
 * 68K/386 32-bit C compiler.
 *
 * copyright (c) 1997, David Lindauer
 * 
 * This compiler is intended for educational use.  It may not be used
 * for profit without the express written consent of the author.
 *
 * It may be freely redistributed, as long as this notice remains intact
 * and either the original sources or derived sources 
 * are distributed along with any executables derived from the originals.
 *
 * The author is not responsible for any damages that may arise from use
 * of this software, either idirect or consequential.
 *
 * v1.35 March 1997
 * David Lindauer, gclind01@starbase.spd.louisville.edu
 *
 * Credits to Mathew Brandt for original K&R C compiler
 *
 */
#include <stdio.h>
#include	"expr.h"
#include 	"c.h"
extern int prm_packing, prm_68020;
extern long framedepth, lc_maxauto;
extern int prm_linkreg;

char PROGNAME[]="CC386";
char ENVNAME[]="CC386";
char SOURCEXT[]=".ASM";
char GLBDEFINE[]="_i386_";

#ifdef BRIEFHELP
char *usage_text = "[+e/+i/f+l/w+A/C/D/E/I/O] file list";
#else
char *usage_text = "[options] files\n"
"+e     - dump errors to file        /fname - specify parameter file\n"
"+i     - dump preprocessed file     +l     - dump listing file\n"
"/fname - specify parameter file     +l     - dump listing file\n"
"/w-xxx - disable a warning          +A     - disable extensions\n"
"/C     - codegen parameters         /Dxxx  - define something\n"
"/Enn   - max number of errors       /Ipath - specify include path\n"
"/O     - optimzer parameters\n"
"Codegen parameters: (/C[+][-][params])\n"
"  +d   - display diagnostics        -b     - no BSS\n"
"  -l   - no C source in ASM file    -m     - no leading underscores\n"
"  +r   - reverse order of bit ops   +F     - FLAT model\n"
"  +N   - generate NASM code         -R     - no link register\n"
"Optimizer parameters (/O[+][-][params])\n"
"  -A   - no address register optimizations\n"
"  -D   - no data register optimizations\n"
"  -F   - no fp register optimizations\n";
#endif                                    
int prm_flat = FALSE;
int prm_nasm = 0;
int cf_maxaddress =20;
int cf_maxdata=4;
int cf_freeaddress =1;
int cf_freedata=3;
int cf_freefloat=3;
int stackadd = 3;
int stackmod = -4;
int strucadd = 3;
int strucmod = -4;
int stdretblocksize = 8;
int stdinttype = bt_long;
int stdunstype = bt_unsigned;
int stdintsize = 4;
int stdldoublesize = 10;
int stdaddrsize = 4;
int regdsize = 4;
int regasize = 4;
int regfsize = 10;

extern TYP stdchar;
TYP             stdconst = { bt_long, 1, UF_DEFINED, 0, 0, -1, -1, 4, {0, 0}, 0, "stdconst",0};
TYP             stdstring = {bt_pointer, 0, 0,0,0,-1, -1, 4, {0, 0}, &stdchar, 0,0};
TYP             stdint = { bt_long, 0, UF_DEFINED | UF_USED,0, 0,-1, -1, 4, {0, 0}, 0, 0,0 };
TYP							stdlongdouble = {bt_longdouble,0,0,0,0,-1,-1,10,{0,0},0,0,0 };
TYP             stduns = { bt_unsigned, 0, 0,0, 0,-1, -1, 4, {0, 0}, 0, 0,0 };
KEYWORDS prockeywords[] = {
				{0,"_absolute", kw__abs}, { 0, "pascal", kw__pascal },
				{0,"_interrupt", kw__interrupt},{0,"_genbyte", kw__genword },
				{0,"_EAX", kw_D0}, {0,"_ECX", kw_D1},{0,"_EDX", kw_D2},
				{0,"_EBX", kw_D3},{0,"_ESP", kw_D4},{0,"_EBP", kw_D5},
				{0,"_ESI", kw_D6},{0,"_EDI", kw_D7},
        {0, 0, 0} };

char *registers[] = { "EAX","ECX","EDX","EBX","ESP","EBP","ESI","EDI",
"","","","","","","","",
"ESP","EBP","ESI","EDI" };

int confcodegen(char s, int bool)
{
	switch (s) {
					case 'N':
						prm_nasm = bool;
						break;
					case 'F':
						prm_flat = bool;
						break;
					default:
						return 0;
	}
	return 1;
}
void confsetup(void)
{
	if (prm_linkreg)
		cf_freeaddress = 2;
	else
		cf_freeaddress = 1;
}
int     alignment(int type, TYP *tp)
{       switch(tp->type) {
                case bt_char: case bt_unsignedchar:  return 1;
                case bt_short: case bt_unsignedshort: return 2;
                case bt_long: case bt_unsigned: return 4;
                case bt_enum:           return 2;
                case bt_pointer:
								case bt_matchall:
                        if(tp->val_flag)
                                return alignment(type,tp->btp);
                        else
                                return 4;
                case bt_float:          return 4;
                case bt_double:         return 4;
								case bt_longdouble:				return 4;
                case bt_struct:
                case bt_union:          return 4;
                default:                return 1;
                }
}
int getalign(int sc, TYP *tp)
{
   int align = alignment(sc,tp);
												if (sc != sc_auto) 
													if (prm_packing)
														align = 1;
	return align;
}

long getautoval(long val)
{
	if (prm_linkreg)
		return val;
	if (val >= 0) 
		return framedepth+val;
	else
		return lc_maxauto + val;
}
funcvaluesize(int size)
{
		return 0;
}