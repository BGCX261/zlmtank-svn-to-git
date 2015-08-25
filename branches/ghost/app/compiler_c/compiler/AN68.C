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
#include 				"diag.h"

/* pc-relative expressions not optimized */
extern int linkreg,basereg;
extern AMODE    push[], pop[];
extern OCODE    *peep_tail;
extern SYM *currentfunc;
extern int prm_stackcheck,prm_phiform,prm_linkreg,prm_rel, prm_smalldata;
extern int floatregs,dataregs,addrregs;
extern long framedepth,stackdepth;
extern int cf_maxfloat, cf_maxaddress,cf_maxdata;
extern int cf_freedata,cf_freeaddress;
extern CSE       *olist;         /* list of optimizable expressions */
long lc_maxauto;

int save_mask, fsave_mask;
void reserveregs(int *datareg, int *addreg, int *floatreg)
/*
 * Reserve regs goes through and reserves a register for variables with
 * the REGISTER keyword.  Note that it currently does register allocation
 * backwards...
 */
{
	CSE *csp = olist;

	while (csp) {
		switch (csp->exp->nodetype) {
								case en_floatref:
								case en_doubleref:
								case en_longdoubleref:
                case en_b_ref:
                case en_w_ref:
                case en_l_ref:
                case en_ub_ref:
                case en_uw_ref:
								case en_ul_ref:
										if (csp->exp->v.p[0]->nodetype != en_autoreg)		
											break;
								  		if (csp->exp->nodetype == en_floatref || csp->exp->nodetype == en_doubleref 
												|| csp->exp->nodetype == en_longdoubleref) {
												if (*floatreg <cf_maxfloat && floatregs)
													csp->reg = (*floatreg)++;
											}
			                else if( (*datareg < cf_maxdata) && (csp->duses <= csp->uses/4) && dataregs)
                        csp->reg = (*datareg)++;
            			    else if(!(csp->size == 1 || csp->size == -1) && (*addreg < cf_maxaddress) &&addrregs)
                        csp->reg = (*addreg)++;
											if (csp->reg != -1) {
												((SYM *)csp->exp->v.p[0]->v.p[0])->inreg = TRUE;
												((SYM *)csp->exp->v.p[0]->v.p[0])->value.i = -csp->reg;
											}
											break;
		}
		csp = csp->next;
	}
}	

void allocate(int datareg, int addreg, int floatreg, SNODE *block )
/*
 *      allocate will allocate registers for the expressions that have
 *      a high enough desirability.
 */
{       CSE      *csp;
        ENODE    *exptr;
        unsigned      mask, rmask,i,fmask,frmask,size;
        AMODE    *ap, *ap2;
				framedepth = 4+lc_maxauto;
        mask = 0;
				rmask = 0;
				fmask = frmask = 0;
				for (i=cf_freedata; i < datareg; i++) {
						framedepth+=4;
						rmask = rmask | (1 << (15 - i));
                        mask = mask | (1 << i);
				}
				for (i=cf_freeaddress+16; i < addreg; i++) {
						framedepth+=4;
						rmask = rmask | (1 << (23 - i));
                        mask = mask | (1 << (i-8));
				}
        while( bsort(&olist) );         /* sort the expression list */
        csp = olist;
        while( csp != 0 ) {
						if (csp->reg == -1 && !(csp->exp->cflags & DF_VOL) && !csp->voidf) {
                if( desire(csp) < 3 )
                        csp->reg = -1;
								else {
									if (csp->exp->nodetype == en_rcon || csp->exp->nodetype == en_fcon || csp->exp->nodetype == en_lrcon
								  			|| csp->exp->nodetype == en_floatref || csp->exp->nodetype ==en_doubleref
												|| csp->exp->nodetype == en_longdoubleref) {
										if (floatreg <24 && floatregs)
											csp->reg = floatreg++;
									}
			            else if( (datareg < cf_maxdata) && (csp->duses <= csp->uses/4) && dataregs)
                    csp->reg = (datareg)++;
      	      		else if( !(csp->size == 1 || csp->size == -1) && (addreg < cf_maxaddress) &&addrregs)
        	          csp->reg = (addreg)++;
								}
						}
            if( csp->reg != -1 )
				{
						if (lvalue(csp->exp) && !((SYM *)csp->exp->v.p[0]->v.p[0])->funcparm) {
							((SYM *)csp->exp->v.p[0]->v.p[0])->inreg = TRUE;
							((SYM *)csp->exp->v.p[0]->v.p[0])->value.i = -csp->reg;
						}
						if (csp->reg < 16) {
							framedepth+=4;
							rmask = rmask | (1 << (15 - csp->reg));
                        mask = mask | (1 << csp->reg);
						}
						else if (csp->reg < 32) {
							framedepth+=4;
							rmask = rmask | (1 << (23 - csp->reg));
                        mask = mask | (1 << (csp->reg-8));
						}
						else {
							framedepth+=12;
							frmask = frmask | (1 << (39 - csp->reg));
              fmask = fmask | (1 << (csp->reg-32));
						}
				}
                csp = csp->next;
                }
				allocstack();								/* Allocate stack space for the local vars */
				if (currentfunc->tp->lst.head !=0 && currentfunc->tp->lst.head != (SYM *)-1) {
					if (prm_phiform || currentfunc->intflag) {
						mask |= (1 << (linkreg +8));
						rmask |= (1 << (15 - linkreg -8));
						framedepth+=4;
					}
					if (currentfunc->intflag) {
						mask |= 0xffff;
						rmask |= 0xffff;
						framedepth = lc_maxauto;
					}
				}
				if (prm_linkreg && !currentfunc->intflag && (currentfunc->tp->lst.head && currentfunc->tp->lst.head != (SYM *)-1 || lc_maxauto)) {
					gen_code(op_link,0,makeareg(linkreg),make_immed(-lc_maxauto));
				}				
			  if( mask != 0 ) 
              gen_code(op_movem,4,make_mask(rmask,0,0),push);
        save_mask = mask;
				if (fmask!=0)
                gen_code(op_fmovem,10,make_mask(frmask,0,1),push);
				fsave_mask = fmask;
				if ((prm_phiform || currentfunc->intflag) && currentfunc->tp->lst.head && currentfunc->tp->lst.head != (SYM *) -1) {
					gen_code(op_move,4,makeareg(0), makeareg(linkreg));
				}
				if ((!prm_linkreg || currentfunc->intflag) && lc_maxauto) {
					AMODE *ap = xalloc(sizeof(AMODE)); 
					ap->mode = am_indx;
					ap->offset = makenode(en_icon,(char *)-lc_maxauto,0);
					ap->preg = 7;
          gen_code(op_lea,0,ap,makeareg(7));
				}
				
				if (prm_stackcheck) {
					AMODE *ap1;
					ap = set_symbol("_stackerror",1);
					ap1 = set_symbol("_stackbottom",0);
					if (prm_rel) {
						ap1->mode = am_indx;
						ap1->preg = basereg;
					}
					else {
						ap1->mode = am_adirect;
						if (prm_smalldata)
							ap1->preg = 2;
						else
							ap1->preg = 4;
					}
					gen_code(op_cmp,4,ap1,makeareg(7));
					gen_code(op_bhi,0,ap,0);
				}
}
void loadregs(void)
/*
 * Initailze allocated regs
 *
 */
{       CSE      *csp;
        ENODE    *exptr;
        unsigned      mask, rmask,i,fmask,frmask,size;
        AMODE    *ap, *ap2;
        csp = olist;
        while( csp != 0 ) {
								int sz;
                if( csp->reg != -1 )
                        {               /* see if preload needed */
                        exptr = csp->exp;
                        if( !lvalue(exptr) || ((SYM *)exptr->v.p[0]->v.p[0])->funcparm )
                                {
																exptr = csp->exp;
                                initstack();
																sz = csp->size;
                                ap = gen_expr(exptr,F_ALL,sz);
							                  if (sz == 0 && ap->mode == am_immed)
																	sz = 4;
                                if( csp->reg < 16 ) {
							                  	if (sz == 0 && ap->mode == am_immed)
																		sz = 4;
																	if (ap->mode == am_dreg)
																		peep_tail->oper2->preg = csp->reg;
																	else {
                                        ap2 = makedreg(csp->reg);
                                				gen_code(op_move,sz,ap,ap2);
																				do_extend(ap2,sz,4,F_DREG);
																	}
																}
                                else
																	if (csp->reg < 32) {
							                  		if (sz == 0 && ap->mode == am_immed)
																			sz = 4;
																		if (ap->mode == am_areg)
																			peep_tail->oper2->preg = csp->reg - 16;
																		else {
  	                                      ap2 = makeareg(csp->reg - 16);
    	                            				gen_code(op_move,4,ap,ap2);
																		}
																	}
																	else {
									                  if (sz == 0 && ap->mode == am_immed)
																			sz = 8;
																		if (ap->mode == am_freg)
																			peep_tail->oper2->preg = csp->reg - 32;
																		else {
  	                                      ap2 = makefreg(csp->reg - 32);
																					size = 8;
																					if (exptr->nodetype == en_floatref)
																						size = 6;
    	                            				gen_code(op_fmove,size,ap,ap2);
																		}
																	}
                                freeop(ap);
																if (lvalue(exptr) && ((SYM *)exptr->v.p[0]->v.p[0])->funcparm) {
																	((SYM *)exptr->v.p[0]->v.p[0])->inreg = TRUE;
																	((SYM *)exptr->v.p[0]->v.p[0])->value.i = -csp->reg;
																}
                                }
                        }
                csp = csp->next;
                }
}
void voidfloat(SNODE *block)
{
}
void asm_scan(OCODE *cd)
{
}
void asm_repcse(OCODE *cd)
{
}