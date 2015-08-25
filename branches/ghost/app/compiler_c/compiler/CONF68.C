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
#include  <stdio.h>
#include	"expr.h"
#include 	"c.h"
extern int prm_packing;
extern long lc_maxauto, framedepth;
extern int prm_linkreg;
extern SYM *currentfunc;

char PROGNAME[]="CC68K";
char ENVNAME[]="CC68K";
char SOURCEXT[]=".SRC";
char GLBDEFINE[]="_m68k_";

#ifdef BRIEFHELP
char *usage_text = "[+e/+i/f+l/w+A/C/D/E/I/O] file list";
#else
char *usage_text = "[options] files\n"
"+e     - dump errors to file        /fname - specify parameter file\n"
"+i     - dump preprocessed file     +l     - dump listing file\n"
"/w-xxx - disable a warning          +A     - disable extensions\n"
"/C     - codegen parameters         /Dxxx  - define something\n"
"/Enn   - max number of errors       /Ipath - specify include path\n"
"/O     - optimzer parameters\n"
"Codegen parameters: (/C[+][-][params])\n"
"  -b   - no BSS\n"
"  -c   - don't optimize to CLR      +d     - display diagnostics\n"
"  +1   - generate 68010 code        +2     - generate 68020 code\n"
"  -l   - no C source in ASM file    -m     - no leading underscores\n"
"  +r   - reverse order of bit ops   +s     - small data model\n"
"  +A   - absolute addressing        +L     - large data model\n"
"  +P   - PHI system stack frames    -R     - no link register\n"
"Optimizer parameters (/O[+][-][params])\n"
"  -A   - no address register optimizations\n"
"  -D   - no data register optimizations\n"
"  -F   - no fp register optimizations\n";
#endif                                    

int prm_buggyclr = FALSE;
int prm_smalldata = FALSE;
int prm_smallcode = FALSE;
int prm_rel = TRUE;
int linkreg;
int basereg;
int prm_phiform=0;
int prm_largedata = FALSE;
int prm_68020 = FALSE;
int prm_68010 = FALSE;
int cf_maxaddress =21;
int cf_maxdata=8;
int cf_maxfloat = 40;
int cf_freeaddress =2;
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
int stdldoublesize = 12;
int stdaddrsize = 4;
int regdsize = 4;
int regasize = 4;
int regfsize = 12;

extern TYP stdchar;
TYP             stdconst = { bt_long, 1, UF_DEFINED, 0, 0, -1, -1, 4, {0, 0}, 0, "stdconst",0};
TYP             stdstring = {bt_pointer, 0, 0,0,0,-1, -1, 4, {0, 0}, &stdchar, 0,0};
TYP             stdint = { bt_long, 0, UF_DEFINED | UF_USED,0, 0,-1, -1, 4, {0, 0}, 0, 0,0 };
TYP							stdlongdouble = {bt_longdouble,0,0,0,0,-1,-1,12,{0,0},0,0,0 };
TYP             stduns = { bt_unsigned, 0, 0,0, 0,-1, -1, 4, {0, 0}, 0, 0,0 };
KEYWORDS prockeywords[] = {
				{0,"_trap", kw__trap}, {0,"_interrupt", kw__interrupt},
				{0,"_absolute", kw__abs }, {0,"_genword", kw__genword },
				{0,"pascal", kw__pascal },
				{0,"_D0",kw_D0},{0,"_D1",kw_D1},{0,"_D2",kw_D2},{0,"_D3",kw_D3},
				{0,"_D4",kw_D4},{0,"_D5",kw_D5},{0,"_D6",kw_D6},{0,"_D7",kw_D7},
				{0,"_A0",kw_A0},{0,"_A1",kw_A1},{0,"_A2",kw_A2},{0,"_A3",kw_A3},
				{0,"_A4",kw_A4},{0,"_A5",kw_A5},{0,"_A6",kw_A6},{0,"_A7",kw_A7},
				{0,"_FP0",kw_F0},{0,"_FP1",kw_F1},{0,"_FP2",kw_F2},{0,"_FP3",kw_F3},
				{0,"_FP4",kw_F4},{0,"_FP5",kw_F5},{0,"_FP6",kw_F6},{0,"_FP7",kw_F7},
        {0, 0, 0} };

char *registers[]  = { 
"D0","D1","D2","D3","D4","D5","D6","D7",
"","","","","","","","",
"A0","A1","A2","A3","A4","A5","A6","A7",
"","","","","","","","",
"FP0","FP1","FP2","FP3","FP4","FP5","FP6","FP7",
"","","","","","","","" } ;

int confcodegen(char s, int bool)
{
	switch (s) {
					case 'c':
						prm_buggyclr = !bool;
						break;
					case 'L':		/* 68000 specific */
						prm_largedata = bool;
						break;
					case '2':   /* 68020 specific */
						prm_68020 = bool;
						break;
					case '1':   /* 68020 specific */
						prm_68010 = bool;
						break;
					case 'P':
						prm_phiform = bool;
						break;
					case 's':
						prm_smalldata = prm_smallcode = bool;
						break;
					case 'A':
						prm_rel = !bool;
						break;
					default:
						return 0;
	}
	return 1;
}
void confsetup(void)
{
	if (prm_68020)
		prm_largedata = FALSE;
	if (prm_phiform)
		prm_linkreg = FALSE;
	linkreg = 6;
  basereg = 5;
	if (prm_phiform || prm_linkreg) {
		if (prm_rel) {
		  cf_maxaddress=21;
		}
		else
			cf_maxaddress = 22;
	}
	else {
		/* Note that the link reg may be used by trap calls even
		 * though nothing else uses it, so it can never be freed
		 */
		if (prm_rel) {
			cf_maxaddress=21;
		}
		else {
			cf_maxaddress=22;
		}
	}
}
static int     alignment(int type, TYP *tp)
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
														if (!prm_68020)
															align = 1;
														else
															if (align > 2) align = 2;
	return align;
}

long getautoval(long val)
{

	if (prm_linkreg && !currentfunc->intflag)
		return val;
	if (val >= 0) 
		if (prm_phiform || currentfunc->intflag)
			return framedepth+val;
		else
			return val;
	else
		return lc_maxauto + val;
}

int funcvaluesize(int size)
{
		return 4 - size;						
}