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
#include        "gen68.h"

/*
 *      this module handles the allocation and de-allocation of
 *      temporary registers. when a temporary register is allocated
 *      the stack depth is saved in the field deep of the address
 *      mode structure. when validate is called on an address mode
 *      structure the stack is popped until the register is restored
 *      to it's pre-push value.
 */
extern int cf_freedata,cf_freeaddress,cf_freefloat,maxdata,maxaddress;

AMODE    push[] = { {am_adec,7} },
                pop[] = { {am_ainc,7} };
int             next_data,		/* Next available */
                next_addr;
int             max_data,			/* Max available */
                max_addr;
int							next_float, max_float;
long stackdepth,framedepth;

char regstack[20],rsold[30],rsdepth = 0,rsodepth=0;
char aregs[8],dregs[8],fregs[8];

void regini(void)
{
	int i;
	for (i=0; i < 8; i++) {
		aregs[0] = dregs[0] = fregs[0] = 0;
	}
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
          gen_code(op_fmove,10,ap1,push);
					stackdepth +=8;
				}
				else {
          gen_code(op_move,4,ap1,push);
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
        	gen_code(op_fmove,10,pop,ap1);
					stackdepth -=8;
				}
				else {
        	gen_code(op_move,4,pop,ap1);
					stackdepth -=4;
				}
}
void initstack(void)
/*
 *      this routine should be called before each expression is
 *      evaluated to make sure the stack is balanced and all of
 *      the registers are marked free.
 */
{
	int i;
	for (i=0; i < 8; i++) 
		aregs[0] = dregs[0] = fregs[0] = 0;
			rsdepth = rsodepth = 0;
       next_data = 0;
        next_addr = 0;
        max_data = cf_freedata-1;
        max_addr = cf_freeaddress-1;
			next_float = 0;
			max_float = cf_freefloat-1;
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
		if (data <8) {
			gen_pop(data,am_dreg,0);
			dregs[data] = 1;
		}
		else
			if (data < 16) {
				gen_pop(data & 7,am_areg,0);
				aregs[data-8] = 1;
			}
			else {
				gen_pop(data & 7,am_freg,0);
				fregs[data-16] = 1;
			}
	}
 	max_addr = next_addr < cf_freeaddress ? cf_freeaddress-1 : next_addr-1;
	max_data = next_data < cf_freedata ? cf_freedata-1 : next_data-1;
	max_float = next_float < cf_freefloat ? cf_freefloat-1 : next_float-1;
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
								dregs[ap->preg] = 1;
								regstack[rsdepth++] = ap->preg;
                max_data = next_data;
                }
        ++next_data;
        return ap;
}

AMODE    *temp_addr(void)
/*
 *      allocate a temporary address register and return it's
 *      addressing mode.
 */
{       AMODE    *ap;
        ap = xalloc(sizeof(AMODE));
        ap->mode = am_areg;
        ap->preg = next_addr % cf_freeaddress;
        if( next_addr > max_addr )
                {
                gen_push(ap->preg,am_areg,0);
								aregs[ap->preg] = 1;
								regstack[rsdepth++] = ap->preg+8;
                max_addr = next_addr;
                }
        ++next_addr;
        return ap;
}
AMODE    *temp_float(void)
/*
 *      allocate a temporary address register and return it's
 *      addressing mode.
 */
{       AMODE    *ap;
        ap = xalloc(sizeof(AMODE));
        ap->mode = am_freg;
        ap->preg = next_float % cf_freefloat;
        if( next_float > max_float )
                {
                gen_push(ap->preg,am_freg,0);
								fregs[ap->preg] = 1;
								regstack[rsdepth++] = ap->preg+16;
                max_addr = next_float;
                }
        ++next_float;
        return ap;
}
void freedata(int dreg)
{
	if (dreg < cf_freedata && next_data > 0) {
		dregs[dreg] = 0;
		--next_data;
	}
}
void freeaddr(int areg)
{
	if (areg < cf_freeaddress && next_addr > 0) {
		aregs[areg] = 0;
		--next_addr;
	}
}
		
void freeop(AMODE *ap)
/*
 *      release any temporary registers used in an addressing mode.
 */
{       if( ap->mode == am_immed || ap->mode == am_direct )
                return;         /* no registers used */
        if( ap->mode == am_dreg)
					freedata(ap->preg);
        else if( ap->mode == am_areg || ap->mode == am_ind || ap->mode == am_indx || ap->mode == am_adec || ap->mode == am_ainc)
					freeaddr(ap->preg);
        else if( ap->mode == am_freg && ap->preg < cf_freefloat && next_float>0) {
									fregs[ap->preg] = 0;
                	--next_float;
				}
				else if (ap->mode == am_baseindxdata) {
					freeaddr(ap->preg);
					freedata(ap->sreg);
				}
				else if (ap->mode == am_baseindxaddr) {
					freeaddr(ap->preg);
					freeaddr(ap->sreg);
				}
}