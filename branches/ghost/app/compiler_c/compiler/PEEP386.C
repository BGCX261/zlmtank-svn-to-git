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

extern SYM *currentfunc;
extern int funcfloat;
extern int nextlabel;

OCODE    *peep_head = 0,
                *peep_tail = 0,
				*peep_insert;

void peepini(void)
{
	peep_head = peep_tail = 0;
}
AMODE    *copy_addr(AMODE *ap)
/*
 *      copy an address mode structure (these things dont last).
 */
{       AMODE    *newap;
        if( ap == 0 )
                return 0;
        newap = xalloc(sizeof(AMODE));
        newap->mode = ap->mode;
        newap->preg = ap->preg;
        newap->sreg = ap->sreg;
        newap->tempflag = ap->tempflag;
        newap->offset = ap->offset;
				newap->scale = ap->scale;
        return newap;
}

void gen_code(int op,int len,AMODE *ap1,AMODE *ap2)
/*
 *      generate a code sequence into the peep list.
 */
{       OCODE    *new;
        new = xalloc(sizeof(OCODE));
        new->opcode = op;
        new->oper1 = copy_addr(ap1);
        new->oper2 = copy_addr(ap2);
				if (len < 0)
        	len = -len;
				if (new->oper1)
					new->oper1->length = len;
				if (new->oper2)
				  new->oper2->length = len;
        add_peep(new);
}
void gen_codelab(SYM *lab)
/*
 *      generate a code sequence into the peep list.
 */
{       OCODE    *new;
        new = xalloc(sizeof(OCODE));
        new->opcode = op_funclabel;
        new->oper1 = lab;
        new->oper2 = 0;
        add_peep(new);
}
void gen_branch(int op,int len, AMODE *ap1)
{
/*#ifndef NASM
*/	gen_code(op,len,ap1,0);
/*#else
*/
#ifdef HIII
  int label = nextlabel++;
  int op1;
  AMODE *ap = make_label(label);
  switch(op) {
		case op_jne:
			op1 = op_je;
			break;
		case op_je:
			op1 = op_jne;
			break;
		case op_jge:
			op1 = op_jl;
			break;
		case op_jl:
			op1 = op_jge;
			break;
		case op_jle:
			op1 = op_jg;
			break;
		case op_jg:
			op1 = op_jle;
			break;
		case op_jnc:
			op1 = op_jb;
			break;
		case op_jb:
			op1 = op_jnc;
			break;
		case op_ja:
			op1 = op_jbe;
			break;
		case op_jbe:
			op1 = op_ja;
			break;
	}
  gen_code(op1,len,ap,0);
	gen_code(op_jmp,len,ap1,0);
  gen_label(label);
#endif
}

void gen_line(SNODE *stmt)
{
				OCODE *new = xalloc(sizeof(OCODE));
				new->opcode = op_line;
				new->oper2 = (AMODE *)((int)stmt->exp);
				new->oper1 = (AMODE *)stmt->label;
				add_peep(new);
}
void gen_codef(int op, int len, AMODE *ap1, AMODE *ap2)
{
  
  if (ap1 && ap2 && ap1->mode == am_freg && ap2->mode == am_freg)
		len=10;
	funcfloat++;
	gen_code(op,len,ap1,ap2);
}
void gen_code2(int op, int len, int len2, AMODE *ap1, AMODE *ap2)
{
        OCODE    *new;
        new = xalloc(sizeof(OCODE));
        new->opcode = op;
        new->oper1 = copy_addr(ap1);
        new->oper2 = copy_addr(ap2);
				if (len < 0)
        	len = -len;
				if (new->oper1)
					new->oper1->length = len;
				if (len2 < 0)
        	len2 = -len2;
				if (new->oper2)
					new->oper2->length = len2;
        add_peep(new);
}


void add_peep(OCODE *new)
/*
 *      add the ocoderuction pointed to by new to the peep list.
 */
{       if( peep_head == 0 )
                {
                peep_head = peep_tail = new;
                new->fwd = 0;
                new->back = 0;
                }
        else
                {
                new->fwd = 0;
                new->back = peep_tail;
                peep_tail->fwd = new;
                peep_tail = new;
                }
}

void gen_label(int labno)
/*
 *      add a compiler generated label to the peep list.
 */
{       OCODE    *new;
        new = xalloc(sizeof(OCODE));
        new->opcode = op_label;
        new->oper1 = (AMODE *)labno;
        add_peep(new);
}

void flush_peep(void)
/*
 *      output all code and labels in the peep list.
 */
{       opt3();         /* do the peephole optimizations */
        while( peep_head != 0 )
                {
                if( peep_head->opcode == op_label )
                        put_label((int)peep_head->oper1);
                else
									if (peep_head->opcode == op_funclabel)
												gen_strlab(peep_head->oper1);
									else
                        put_code(peep_head);
                peep_head = peep_head->fwd;
                }
				dump_muldivval();
				peep_head = 0;
}

void peep_add(OCODE *ip)
/*
 * Turn add,1 into inc
 */
{
	if (ip->noopt)
		return;
	if (ip->oper2->mode != am_immed || !isintconst(ip->oper2->offset->nodetype))
		return;
	if (ip->oper2->offset->v.i == 1) {
		ip->opcode = op_inc;
		ip->oper2 = 0;
	}
	return;
}
/*
 * Turn sub,1 into dec
 */
void peep_sub(OCODE *ip)
{
	if (ip->noopt)
		return;
	if (ip->oper2->mode != am_immed || !isintconst(ip->oper2->offset->nodetype))
		return;
	if (ip->oper2->offset->v.i == 1) {
		ip->opcode = op_dec;
		ip->oper2 = 0;
	}
	return;
}
/*
 * Turn move reg,0 into sub reg,reg
 */
void peep_move(OCODE *ip)
{
	if (ip->noopt)
		return;
	if (ip->oper1->mode != am_dreg || 
				ip->oper2->mode != am_immed || !isintconst(ip->oper2->offset->nodetype))
		return;
	if (ip->oper2->offset->v.i == 0) {
		ip->opcode = op_sub;
		ip->oper2 = ip->oper1;
	}
	return;
}
/*
 * delete or reg,reg preceded by an instruction that already sets flags
 */
void peep_or(OCODE *ip)
{
	OCODE *ip2;
	if (ip->noopt)
		return;
	if ((ip->oper1->mode != ip->oper2->mode) || ip->oper1->mode != am_dreg ||
				(ip->oper1->preg != ip->oper2->preg))
		return;
	ip2 = ip->back;
	if (!ip2->oper1 || ip2->opcode == op_label || ip2->oper1->mode != am_dreg || ip2->oper1->preg != ip->oper1->preg)
		return;
	if (ip2->opcode != op_sub && ip2->opcode != op_add && ip2->opcode != op_inc
			&& ip2->opcode != op_dec && ip2->opcode != op_and && ip2->opcode != op_or
			&& ip2->opcode != op_xor)
		return;
	ip2->fwd = ip->fwd;
	ip->fwd->back = ip->back;
	/* A quick pentium optimization */
	if (ip2->opcode == op_and)
		ip2->opcode = op_test;
}
void peep_uctran(OCODE *ip)
/*
 *      peephole optimization for unconditional transfers.
 *      deletes instructions which have no path.
 *      applies to bra, jmp, and rts instructions.
 */
{       while( ip->fwd != 0 && ip->fwd->opcode != op_label )
                {
                ip->fwd = ip->fwd->fwd;
                if( ip->fwd != 0 )
                        ip->fwd->back = ip;
                }
}
void peep_label(OCODE *ip)
/*
 *		peephole optimization for labels
 *		deletes relbranches that jump to the next instruction
 */
{
			OCODE *curpos, *index;
			curpos = ip;
			do {
				curpos = curpos->back;
			} while(curpos->opcode == op_label || curpos->opcode == op_line);
			while ((curpos->opcode == op_jmp) || curpos->opcode == op_cmp
						|| curpos->opcode == op_test 
						|| (curpos->opcode == op_jne) || (curpos->opcode == op_je) 
						|| (curpos->opcode == op_jge) || (curpos->opcode == op_jle) 
						|| (curpos->opcode == op_jg) || (curpos->opcode == op_jl) 
						|| (curpos->opcode == op_jnc) || (curpos->opcode == op_jbe) 
						|| (curpos->opcode == op_ja) || (curpos->opcode == op_jb) ) {
				index = curpos->fwd;
				if (curpos->noopt)
					return;
				if (curpos->opcode == op_cmp
								|| curpos->opcode == op_test) {
						curpos->back->fwd = curpos->fwd;
						curpos->fwd->back = curpos->back;
						curpos = curpos->back;
				}
				else {
					do {
						if ((index->opcode == op_label) && (curpos->oper1->mode == am_immed) && ((int)index->oper1 == curpos->oper1->offset->v.i)) {
							curpos->back->fwd = curpos->fwd;
							curpos->fwd->back = curpos->back;
							curpos = curpos->back;
							break;
						}
						index = index->fwd;
					} while (index != ip->fwd);
					if (index == ip->fwd)
						break;
				}
				while(curpos->opcode == op_label || curpos->opcode == op_line)
					curpos = curpos->back;
			}
}
int equal_address(AMODE *ap1,AMODE *ap2)
{
	if (ap1->mode != ap2->mode)
		return(FALSE);
	switch (ap1->mode) {
		case am_indispscale:
						if (ap1->scale != ap2->scale || ap1->sreg != ap2->sreg)
							return(FALSE);
		case am_indisp:
						if (ap1->offset)
							if (ap2->offset) {
								if (ap1->offset->v.i != ap2->offset->v.i)
									return(FALSE);
							}
							else
								return(FALSE);
						else
							if (ap2->offset)
								return(FALSE);
		case am_dreg:
		case am_freg:
						if (ap1->preg != ap2->preg)
							return(FALSE);
						break;
					
		case am_direct:
						return FALSE;
	}
	return(TRUE);
}

void opt3(void)
/*
 *      peephole optimizer. This routine calls the instruction
 *      specific optimization routines above for each instruction
 *      in the peep list.
 */
{       OCODE    *ip;
        ip = peep_head;
        while( ip != 0 )
                {
                switch( ip->opcode )
                        {
												case op_label:
																peep_label(ip);
																break;
												case op_add:
																peep_add(ip);
																break;
												case op_sub:
																peep_sub(ip);
																break;
												case op_mov:
																peep_move(ip);
																break;
												case op_or:
																peep_or(ip);
																break;
                        case op_jmp:
                        case op_ret:
                                peep_uctran(ip);
                        }
                ip = ip->fwd;
                }
}