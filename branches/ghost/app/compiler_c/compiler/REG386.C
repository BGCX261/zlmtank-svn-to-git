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
#include        <stdio.h>
#include        "expr.h"
#include        "c.h"
#include        "gen386.h"

/*
 *      this module handles the allocation and de-allocation of
 *      temporary registers. when a temporary register is allocated
 *      the stack depth is saved in the field deep of the address
 *      mode structure. when validate is called on an address mode
 *      structure the stack is popped until the register is restored
 *      to it's pre-push value.
 */
extern int cf_freedata,cf_freeaddress,cf_freefloat;

	AMODE push[1],pop[1];
int             next_data,		/* Next available */
                next_addr;
int             max_data,			/* Max available */
                max_addr;
int							next_float, max_float;
long stackdepth,framedepth;
int regs[3];

char regstack[20],rsold[30],rsdepth = 0,rsodepth=0;

void regini(void)
{
	rsdepth = rsodepth = 0;
}
void gen_push(int reg, int rmode, int flag)
/*
 *      this routine generates code to push a register onto the stack
 */
{       AMODE    *ap1;
        ap1 = xalloc(sizeof(AMODE));
        ap1->preg = reg;
        ap1->mode = rmode;
				if (rmode == am_freg) {
			FLOAT;
				}
				else {
          gen_code(op_push,4,ap1,0);
					if (flag)
						framedepth+=4;
					else
						stackdepth +=4;
				}
}

void gen_pop(int reg, int rmode, int flag)
/*
 *      generate code to pop the primary register in ap from the
 *      stack.
 */
{       AMODE    *ap1;
        ap1 = xalloc(sizeof(AMODE));
        ap1->preg = reg;
        ap1->mode = rmode;
				if (rmode == am_freg) {
			FLOAT;
				}
				else {
        	gen_code(op_pop,4,ap1,0);
					stackdepth -=4;
				}
}
void pushregs(unsigned mask)
{
	int i;
	int umask = 0x08000;
	for (i=0; i < 4; i++) {
		if (umask & mask)
			gen_push(i,am_dreg,1);
		umask >>=1;
	}
	umask = 0x080;
	for (i=4; i < 8; i++) {
		if (umask & mask)
			gen_push(i,am_dreg,1);
		umask >>=1;
	}
}
/* This is ONLY called from the return.  Calling from any other place
 * will leave the stack depth unpredictable... */
void popregs(unsigned mask)
{
	int i;
	int umask = 0x800;
	for (i=7; i >= 4; i--) {
		if (umask & mask) {
			gen_pop(i,am_dreg,1);
			stackdepth+=4;
		}
		umask >>=1;
	}
	umask = 0x8;
	for (i=3; i >= 0; i--) {
		if (umask & mask) {
			gen_pop(i,am_dreg,1);
			stackdepth+=4;
		}
		umask >>=1;
	}
}
void initstack(void)
/*
 *      this routine should be called before each expression is
 *      evaluated to make sure the stack is balanced and all of
 *      the registers are marked free.
 */
{
			rsdepth = rsodepth = 0;
       next_data = 0;
        next_addr = 0;
        max_data = cf_freedata-1;
        max_addr = cf_freeaddress-1;
			next_float = 0;
			max_float = cf_freefloat-1;
			regs[0]=regs[1]=regs[2]=0;
}

void mark(void)
{
	rsold[rsodepth++] = rsdepth;
}
void release(void)
{
	if (!rsodepth)
		return;
	rsodepth--;
	while (rsdepth > rsold[rsodepth]) {
		int data = regstack[--rsdepth];
	  gen_pop(data,am_dreg,0);
	}
}
AMODE    *temp_data(void)
/*
 *      allocate a temporary data register and return it's
 *      addressing mode.
 */
{       AMODE    *ap;
        ap = xalloc(sizeof(AMODE));
        ap->mode = am_dreg;
        ap->preg = next_data % cf_freedata;
        if( next_data > max_data )
                {
                gen_push(ap->preg,am_dreg,0);
								regstack[rsdepth++] = ap->preg;
								regs[ap->preg]--;
                max_data = next_data;
                }
				regs[ap->preg]++;
        ++next_data;
        return ap;
}

void freedata(int dreg)
{
	if (dreg < cf_freedata && next_data > 0) {
		--next_data;
		regs[dreg]--;
	}
}
void freeaddr(int areg)
{
	if (areg < cf_freeaddress && next_addr > 0) 
		--next_addr;
}
		
void freeop(AMODE *ap)
/*
 *      release any temporary registers used in an addressing mode.
 */
{       if( ap->mode == am_immed || ap->mode == am_direct )
                return;         /* no registers used */
        if( ap->mode == am_dreg)
					freedata(ap->preg);
		 		else if (ap->mode == am_indisp)		
					freedata(ap->preg);
		 		else if (ap->mode == am_indispscale) {
					freedata(ap->preg);
					freedata(ap->sreg);
				}
}