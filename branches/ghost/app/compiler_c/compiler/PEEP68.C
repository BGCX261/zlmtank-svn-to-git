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
 * peepcode optimizations
 */
#include        <stdio.h>
#include        "expr.h"
#include        "c.h"
#include        "gen68.h"
#include 				"diag.h"

extern SYM *currentfunc;

OCODE    *peep_head = 0,
                *peep_tail = 0,
					*peep_insert;

extern int prm_buggyclr;

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
				newap->scale = ap->scale;
        newap->tempflag = ap->tempflag;
        newap->offset = ap->offset;
        return newap;
}

void gen_code(int op,int len,AMODE *ap1,AMODE *ap2)
/*
 *      generate a code sequence into the peep list.
 */
{       OCODE    *new;
        new = xalloc(sizeof(OCODE));
        new->opcode = op;
				if (len < 0)
        	new->length = -len;
				else
        	new->length = len;
        new->oper1 = copy_addr(ap1);
        new->oper2 = copy_addr(ap2);
				new->oper3 = 0;
        add_peep(new);
}
void gen_lea(int size, AMODE *ap1, AMODE * ap2)
{
				AMODE *ap3,*ap4;
				if (ap1->mode== am_ainc || ap1->mode == am_adec) {
					enum e_am om = ap1->mode;
					if (!size) {
						size = 1;
						DIAG("Illegal len in autoinc lea");
					}
					if (size < 0) size = - size;
				  ap1->mode = am_ind;
					ap3 = copy_addr(ap1);
					ap3->mode = am_areg;
					ap4 = make_immed(size);
					gen_code(op_move,4,ap3,ap2);
					if (om == am_ainc)
						gen_code(op_add,4,ap4,ap3);
					else
						gen_code(op_sub,4,ap4,ap3);
					ap1->mode = om;
				}
				else
					if (ap2->mode == am_areg) {
						if (ap1->mode == am_baseindxaddr && !ap1->scale) {
							if (ap1->preg == ap2->preg && !ap1->offset->v.i) {
								ap3 = xalloc(sizeof(AMODE));
								ap3->mode = am_areg;
								ap3->preg = ap1->sreg;
								gen_code(op_add,4,ap3,ap2);
							}
							else if (ap1->sreg == ap2->preg && !ap1->offset->v.i) {
								ap3 = xalloc(sizeof(AMODE));
								ap3->mode = am_areg;
								ap3->preg = ap1->preg;
								gen_code(op_add,4,ap3,ap2);
							}
							else 
								gen_code(op_lea,size,ap1,ap2);
						}
						else if (ap1->mode == am_baseindxdata && !ap1->offset->v.i && !ap1->scale) {
							if (ap1->preg == ap2->preg) {
								ap3 = xalloc(sizeof(AMODE));
								ap3->mode = am_dreg;
								ap3->preg = ap1->sreg;
								gen_code(op_add,4,ap3,ap2);
							}
							else
								gen_code(op_lea,0,ap1,ap2);
						}
						else
							gen_code(op_lea,0,ap1,ap2);
					}
					else	 
						gen_code(op_lea,0,ap1,ap2);
}
void gen_codelab(SYM *lab)
/*
 *      generate a code sequence into the peep list.
 */
{       OCODE    *new;
        new = xalloc(sizeof(OCODE));
        new->opcode = op_funclabel;
        new->length = 0;
        new->oper1 = lab;
        new->oper2 = 0;
				new->oper3 = 0;
        add_peep(new);
}
void gen_line(SNODE *stmt)
{
				OCODE *new = xalloc(sizeof(OCODE));
				new->opcode = op_line;
				new->length = (int)stmt->exp;
				new->oper1 = (AMODE *)stmt->label;
				new->oper2 = 0;
				new->oper3= 0;
				add_peep(new);
}
void gen_codef(int op, int len, AMODE *ap1, AMODE *ap2)
{
  if (ap1->mode == am_freg) {
		if (!ap2 || ap2->mode == am_freg)
			len=10;
	}
	else if (ap2 && ap2->mode == am_freg) {
		if (ap1->mode == am_immed)
			len = 10;
	}
	gen_code(op,len,ap1,ap2);
}
void gen_code3(int op, int len, AMODE *ap1, AMODE *ap2, AMODE *ap3)
{       OCODE    *new;
        new = xalloc(sizeof(OCODE));
        new->opcode = op;
				if (len < 0)
        	new->length = -len;
				else
        	new->length = len;
        new->oper1 = copy_addr(ap1);
        new->oper2 = copy_addr(ap2);
				new->oper3 = copy_addr(ap3);
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
                        put_label(peep_head);
                else
									if (peep_head->opcode == op_funclabel)
												gen_strlab(peep_head->oper1);
									else
                        put_code(peep_head);
                peep_head = peep_head->fwd;
                }
				/* oc_reorgfunc(); */
				peep_head = 0;
}

void peep_move(OCODE *ip)
/*
 *      peephole optimization for move instructions.
 *      makes quick immediates when possible.
 *      changes move #0,d to clr d.
 *      changes long moves to address registers to short when
 *              possible.
 *      changes move immediate to stack to pea.
 */
{       ENODE    *ep;
				if (ip->noopt)
					return;
				if (ip->oper2->mode == am_areg)
					ip->opcode = op_movea;
        if( ip->oper1->mode != am_immed )
                return;
        ep = ip->oper1->offset;
        if( !isintconst(ep->nodetype))
                return;
        if( ip->oper2->mode == am_areg )
                {
                if( -32768L <= ep->v.i && ep->v.i <= 32768L )
                        ip->length = 2;
                }
        else if( ip->oper2->mode == am_dreg )
                {
                if( -128 <= ep->v.i && ep->v.i <= 127 )
                        {
                        ip->opcode = op_moveq;
                        ip->length = 0;
                        }
                }
        else
                {
                if( ep->v.i == 0 && !prm_buggyclr)
                        {
                        ip->opcode = op_clr;
                        ip->oper1 = ip->oper2;
                        ip->oper2 = 0;
                        }
                else if( ip->oper2->mode == am_adec && ip->oper2->preg == 7 )
                        {
                        ip->opcode = op_pea;       	
                        ip->length = 0;
                        ip->oper1->mode = am_direct;
                        ip->oper2 = 0;
                        }
                }
}

/*
 * get rid of a TST after any other instruction that sets flags if the
 * args match
 */
int peep_tst(OCODE *ip)
{
	if (ip->noopt)
		return;
	if (ip->back->opcode == op_move || ip->back->opcode == op_and || ip->back->opcode == op_or ||
			ip->back->opcode == op_andi || ip->back->opcode == op_ori || ip->back->opcode == op_add ||
			ip->back->opcode == op_addi || ip->back->opcode == op_addq || ip->back->opcode == op_sub ||
			ip->back->opcode == op_subi || ip->back->opcode == op_subq) {
		if (equal_address(ip->back->oper2,ip->oper1)) {
			ip->back->fwd = ip->fwd;
			ip->fwd->back = ip->back;
		}
	}
}
int     equal_address(AMODE *ap1, AMODE *ap2)
/*
 *      compare two address nodes and return true if they are
 *      equivalent.
 */
{       if( ap1 == 0 || ap2 == 0 )
                return 0;
        if( ap1->mode != ap2->mode )
                return 0;
        switch( ap1->mode )
                {
                case am_areg:   case am_dreg:
                case am_ainc:   case am_adec:
                        return ap1->preg == ap2->preg;
								case am_baseindxaddr:
								case am_baseindxdata:
								case am_indx:
												if (ap1->preg != ap2->preg)
													return FALSE;
												if (ap1->sreg != ap2->sreg)
													return FALSE;
												return equalnode(ap1->offset,ap2->offset);
								case am_immed:
								case am_direct:
								case am_adirect:
								case am_pcindx:
												return equalnode(ap1->offset,ap2->offset);
                }
        return 0;
}

void peep_add(OCODE *ip)
/*
 *      peephole optimization for add instructions.
 *      makes quick immediates out of small constants.
 */
{       ENODE    *ep;
				if (ip->noopt)
					return;
				if (ip->oper2->mode == am_areg)
					ip->opcode = op_adda;
        if( ip->oper1->mode != am_immed )
                return;
        ep = ip->oper1->offset;
        if( ip->oper2->mode != am_areg )
                ip->opcode = op_addi;
        else
                {
                if( isshort(ep) )
                        ip->length = 2;
                }
        if( !(isintconst(ep->nodetype)))
                return;
        if( 1 <= ep->v.i && ep->v.i <= 8 )
                ip->opcode = op_addq;
        else if( -8 <= ep->v.i && ep->v.i <= -1 )
                {
                ip->opcode = op_subq;
                ep->v.i = -ep->v.i;
                }
}

void peep_sub(OCODE *ip)
/*
 *      peephole optimization for subtract instructions.
 *      makes quick immediates out of small constants.
 */
{       ENODE    *ep;
				if (ip->noopt)
					return;
				if (ip->oper2->mode == am_areg)
					ip->opcode = op_suba;
        if( ip->oper1->mode != am_immed )
                return;
        ep = ip->oper1->offset;
        if( ip->oper2->mode != am_areg )
                ip->opcode = op_subi;
        else
                {
                if( isshort(ep) )
                        ip->length = 2;
                }
        if(!isintconst( ep->nodetype) )
                return;
        if( 1 <= ep->v.i && ep->v.i <= 8 ) {
                ip->opcode = op_subq;
								if (ip->oper2->mode == am_areg && ip->length <=4 && ip->fwd->oper1->mode == am_ind && ip->oper2->preg == ip->fwd->oper1->preg) {
									int sz1 = ip->fwd->length;
									int sz2 = ep->v.i;
									if (sz1 < 0) sz1 = -sz1;
									if (sz2 < 0) sz2 = -sz2;
									if (sz2 == sz1) {
										ip->back->fwd = ip->fwd;
										ip->fwd->back = ip->back;
										ip->fwd->oper1->mode = am_adec;
									}
								}
				}
        else if( -8 <= ep->v.i && ep->v.i <= -1 )
                {
                ip->opcode = op_addq;
                ep->v.i = -ep->v.i;
                }
}
void     peep_cmp(OCODE *ip)
/*
 *      peephole optimization for compare instructions.
 *      changes compare #0 to tst and if previous instruction
 *      should have set the condition codes properly delete.
 *      return value is true if instruction was deleted.
 */
{       OCODE    *prev=0;
        ENODE    *ep;
				if (ip->noopt)
					return;
				if (ip->oper2->mode == am_areg)
					ip->opcode = op_cmpa;
        if( ip->oper1->mode == am_immed ) {
	        ep = ip->oper1->offset;
  	      if( ip->oper2->mode == am_areg )
    	            {
      	          if( isshort(ep) )
        	                ip->length = 2;
          	      return;
            	    }
	        ip->opcode = op_cmpi;
  	      if( isintconst(ep->nodetype) &&  ep->v.i == 0 ) {
						if (ip->fwd->opcode == op_bne || ip->fwd->opcode == op_beq ) {
			        ip->oper1 = ip->oper2;
  	  	  	  ip->oper2 = 0;
    	  	  	ip->opcode = op_tst;
		  	      prev = ip->back;
						}
					}
				}
    		if( prev == 0)
                return;
        if( (((prev->opcode == op_move || prev->opcode == op_moveq
							|| prev->opcode == op_add || prev->opcode == op_addi
							|| prev->opcode == op_sub || prev->opcode == op_subi
							|| prev->opcode == op_addq || prev->opcode == op_subq) &&
              equal_address(prev->oper1,ip->oper1)) &&
                prev->oper2->mode != am_areg) ||
                (prev->opcode != op_label &&
                equal_address(prev->oper2,ip->oper1)) )
                {
                prev->fwd = ip->fwd;
                if( prev->fwd != 0 )
                        prev->fwd->back = prev;
                }
}

void peep_muldiv(OCODE *ip, int op)
/*
 *      changes multiplies and divides by convienient values
 *      to shift operations. op should be either op_asl or
 *      op_asr (for divide).
 */
{       int     shcnt;
				if (ip->noopt)
					return;
        if( ip->oper1->mode != am_immed )
                return;
        if(!isintconst( ip->oper1->offset->nodetype ))
                return;
        shcnt = ip->oper1->offset->v.i;
/*      vax c doesn't do this type of switch well       */
        if( shcnt == 2) shcnt = 1;
        else if( shcnt == 4) shcnt = 2;
        else if( shcnt == 8) shcnt = 3;
        else if( shcnt == 16) shcnt = 4;
        else if( shcnt == 32) shcnt = 5;
        else if( shcnt == 64) shcnt = 6;
        else if( shcnt == 128) shcnt = 7;
        else if( shcnt == 256) shcnt = 8;
        else if( shcnt == 512) shcnt = 9;
        else if( shcnt == 1024) shcnt = 10;
        else if( shcnt == 2048) shcnt = 11;
        else if( shcnt == 4096) shcnt = 12;
        else if( shcnt == 8192) shcnt = 13;
        else if( shcnt == 16384) shcnt = 14;
        else return;
        ip->oper1->offset->v.i = shcnt;
        ip->opcode = op;
        ip->length = 4;
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

			if (!curpos->back)
				return;
			do {
				curpos = curpos->back;
			} while(curpos->opcode == op_label || curpos->opcode == op_line);
			while ((curpos->opcode == op_bra) || curpos->opcode == op_cmp
						|| curpos->opcode == op_cmpi || curpos->opcode == op_tst 
						|| (curpos->opcode == op_bne) || (curpos->opcode == op_beq) 
						|| (curpos->opcode == op_bge) || (curpos->opcode == op_ble) 
						|| (curpos->opcode == op_bgt) || (curpos->opcode == op_blt) 
						|| (curpos->opcode == op_bhs) || (curpos->opcode == op_bls) 
						|| (curpos->opcode == op_bhi) || (curpos->opcode == op_blo) ) {
				index = curpos->fwd;
				if (ip->noopt)
					return;
				if ((curpos->opcode == op_cmpi || curpos->opcode == op_cmp
								|| curpos->opcode == op_tst )) {
					if (curpos->fwd->opcode == op_label) {
						curpos->back->fwd = curpos->fwd;
						curpos->fwd->back = curpos->back;
						curpos = curpos->back;
					}
					else
						break;
				}
				else {
					do {
						if ((index->opcode == op_label) && (curpos->oper1->mode == am_direct) && ((int)index->oper1 == curpos->oper1->offset->v.i)) {
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

void opt3(void)
/*
 *      peephole optimizer. This routine calls the instruction
 *      specific optimization routines above for each instruction
 *      in the peep list.
 */
{       OCODE    *ip=peep_head;
        while( ip != 0 )
                {
								if (ip->opcode != op_line && ip->opcode != op_label && ip->opcode != op_slit) {
									if (ip->oper1 && ip->oper1->mode == am_indx && ip->oper1->offset->v.i == 0)
										ip->oper1->mode = am_ind;
									if (ip->oper2 && ip->oper2->mode == am_indx && ip->oper2->offset->v.i == 0)
										ip->oper2->mode = am_ind;
								}
                switch( ip->opcode )
                        {
                        case op_move:
                                peep_move(ip);
                                break;
                        case op_add:
                                peep_add(ip);
                                break;
                        case op_sub:
                                peep_sub(ip);
                                break;
												case op_tst:
																peep_tst(ip);
																break;
                        case op_cmp:
                                peep_cmp(ip);
                                break;
												case op_and:
																if (ip->oper1->mode == am_immed)
																	ip->opcode = op_andi;
																break;
												case op_or:
																if (ip->oper1->mode == am_immed)
																	ip->opcode = op_ori;
																break;
												case op_eor:
																if (ip->oper1->mode == am_immed)
																	ip->opcode = op_eori;
																break;
                        case op_muls:
                                peep_muldiv(ip,op_asl);
                                break;
												case op_label:
																peep_label(ip);
																break;
                        case op_bra:
                        case op_jmp:
                        case op_rts:
                                peep_uctran(ip);
                        }
                ip = ip->fwd;
                }
}