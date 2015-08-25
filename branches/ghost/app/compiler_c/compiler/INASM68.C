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
/*
 * inline assembler (68k)  not implemented
 */
#include        <stdio.h> 
#include        "expr.h" 
#include        "c.h" 
#include        "errors.h"


extern int asmline;

ASMNAME *keyimage;

void inasmini(void)
{
}
SNODE *asm_statement (int shortfin)
{
	asmline = shortfin;
	getsym();
	return 0;
}