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
 * Register analysis
 */
#include        <stdio.h>
#include        "expr.h"
#include        "c.h"
#include        "gen386.h"
#include 				"diag.h"

/* pc-relative expressions not optimized */
extern AMODE    push[], pop[];
extern OCODE    *peep_tail;
extern SYM *currentfunc;
extern int prm_stackcheck,prm_linkreg;
extern int floatregs,dataregs,addrregs;
extern long framedepth,stackdepth;
extern int cf_maxfloat, cf_maxaddress,cf_maxdata,cf_freeaddress,cf_freedata,cf_freefloat;
extern CSE       *olist;         /* list of optimizable expressions */

long lc_maxauto;
int  fsave_mask,save_mask;
void reserveregs(int *datareg, int *addreg, int *floatreg)
/*
 * Reserve regs goes through and reserves a register for variables with
 * the REGISTER keyword.  Note that it currently does register allocation
 * backwards...  NOT the standard I suspect.
 */
{
	CSE *csp = olist;

	while (csp) {
		switch (csp->exp->nodetype) {
								case en_floatref:
								case en_doubleref:
								case en_longdoubleref:
										break;
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
											}
            			    else if( (csp->duses <= csp->uses / 4) && (*datareg < cf_maxdata) &&dataregs)
                        csp->reg = (*datareg)++;
			                else if(!(csp->size ==-1 || csp->size ==1) && (*addreg < cf_maxaddress) && addrregs) {
                        csp->reg = (*addreg)++;
											}
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
 *      a high enough desirability.  It also puts the function
 * header, consisting of saved registers and stack decrments for local
 * variables
 */
{       CSE      *csp;
        ENODE    *exptr;
        unsigned      mask, rmask,i,fmask,frmask,size;
        AMODE    *ap, *ap2;
				framedepth = 4;
        mask = 0;
				rmask = 0;
				fmask = frmask = 0;
				for (i=cf_freedata; i < datareg; i++) {
						rmask = rmask | (1 << (15 - i));
                        mask = mask | (1 << i);
				}
				for (i=cf_freeaddress+16; i < addreg; i++) {
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
									}
            			else if( (csp->duses <= csp->uses / 4) && (datareg < cf_maxdata) &&dataregs)
                  	csp->reg = (datareg)++;
	                else if( (!(csp->size == 1 || csp->size == -1) || csp->exp->nodetype == en_icon) && (addreg < cf_maxaddress) && addrregs) {
  	                csp->reg = (addreg)++;
									}
								}
						}
            if( csp->reg != -1 )
				{
						if (lvalue(csp->exp) && !((SYM *)csp->exp->v.p[0]->v.p[0])->funcparm) {
							((SYM *)csp->exp->v.p[0]->v.p[0])->inreg = TRUE;
							((SYM *)csp->exp->v.p[0]->v.p[0])->value.i = -csp->reg;
						}
						if (csp->reg < 16) {
							rmask = rmask | (1 << (15 - csp->reg));
                        mask = mask | (1 << csp->reg);
						}
						if (csp->reg < 32) {
							rmask = rmask | (1 << (23 - csp->reg));
                        mask = mask | (1 << (csp->reg-8));
						}
						else {
							frmask = frmask | (1 << (39 - csp->reg));
              fmask = fmask | (1 << (csp->reg-32));
						}
				}
                csp = csp->next;
                }
				allocstack();								/* Allocate stack space for the local vars */
				if (prm_linkreg && (lc_maxauto || currentfunc->tp->lst.head && currentfunc->tp->lst.head != (SYM *)-1)) {
					gen_code(op_enter,4,make_immed(lc_maxauto),make_immed(0));
				}
				if (currentfunc->intflag) {
					gen_code(op_pushad,0,0,0);
					stackdepth = 4*8;
				}
        else
				  if( mask != 0 ) 
							pushregs(rmask);
        save_mask = mask;
				if (fmask!=0)
				fsave_mask = fmask;
				
				if (!prm_linkreg && lc_maxauto) {
					gen_code(op_sub,4,makedreg(ESP), make_immed(lc_maxauto));
				}
				stackdepth +=lc_maxauto;
				framedepth +=stackdepth;
				stackdepth = 0;
				if (prm_stackcheck) {
					AMODE *ap1;
					ap = set_symbol("_stackerror",1);
					ap1 = set_symbol("_stackbottom",0);
					ap1->mode = am_direct;
					gen_code(op_cmp,4,makedreg(ESP),ap1);
					gen_code(op_jb,0,ap,0);
				}
}
void loadregs(void)
/*
 * initialize allocated registers
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
                                if( csp->reg < 16 ) {
																	if (ap->mode == am_dreg)
																		peep_tail->oper1->preg = csp->reg;
																	else {
                                        ap2 = makedreg(csp->reg);
																				if (ap->mode == am_immed || sz == 4 || sz == -4)
                                					gen_code(op_mov,4,ap2,ap);
																				else
																					if (sz < 0)
																						gen_code2(op_movsx,-4,sz,ap2,ap);
																					else
																						gen_code2(op_movzx,4,sz,ap2,ap);
																	}
																}
                                else
																	if (csp->reg < 32) {
																		if (ap->mode == am_dreg)
																			peep_tail->oper1->preg = csp->reg - 12;
																		else {
  	                                      ap2 = makedreg(csp->reg - 12);
																					if (ap->mode == am_immed || sz == 4 || sz == -4)
  	                              					gen_code(op_mov,4,ap2,ap);
																					else
																						if (sz < 0)
																							gen_code2(op_movsx,-4,sz,ap2,ap);
																						else
																							gen_code2(op_movzx,4,sz,ap2,ap);
																		}
																	}
																	else {
																		/* Should never get here */
																		DIAG("float reg assigned in analyze");
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
void asm_scannode (ENODE *node)
{
	CSE *csp;
	if (node->nodetype == en_add) {
		asm_scannode(node->v.p[0]);
		asm_scannode(node->v.p[1]);
	}
	else {
		switch (node->nodetype) {
                case en_icon:
								case en_lcon:
								case en_iucon:
								case en_lucon:
								case en_ccon:
                        break;
                case en_napccon:
                case en_nacon:
								case en_absacon:
                case en_autocon:
                case en_autoreg:
												csp = enternode(node,0,0);
												csp->voidf = TRUE;
                        break;
								case en_labcon: case en_nalabcon:
												break;
								default:
												DIAG("Invalid node in assembler line");
												break;
		}
	}
}
void asm_scan1(AMODE *ap)
{
	if (!ap || !ap->offset)
		return;
	asm_scannode(ap->offset);
}
void asm_scan(OCODE *cd)
{
	asm_scan1(cd->oper1);
	asm_scan1(cd->oper2);
	asm_scan1(cd->oper3);
}
void asm_repnode(ENODE **node)
{
	if ((*node)->nodetype == en_add) {
		asm_repnode(&(*node)->v.p[0]);
		asm_repnode(&(*node)->v.p[1]);
	}
	else
        if( (*node)->nodetype == en_autocon  || (*node)->nodetype == en_autoreg)
                {
								if (prm_linkreg) {
      			    	*node = makenode(en_icon,(char *)((SYM *)(*node)->v.p[0])->value.i,0);
								}
								else if (((SYM *)(*node)->v.p[0])->funcparm)
                	*node = makenode(en_icon,(char *)(((SYM *)(*node)->v.p[0])->value.i+framedepth+stackdepth),0);
                else
									*node = makenode(en_icon,(char *)(((SYM *)(*node)->v.p[0])->value.i+stackdepth+lc_maxauto),0);
                }
        else if( (*node)->nodetype == en_nacon || (*node)->nodetype == en_napccon)
                {
                *node = makenode((*node)->nodetype,(char *)((SYM *)(*node)->v.p[0])->name,0);
                }
        else if( (*node)->nodetype == en_nalabcon)
                {
                *node = makenode((*node)->nodetype,(char *)(*node)->v.i,0);
                }
        else if( (*node)->nodetype == en_labcon)
                {
                *node = makenode((*node)->nodetype,(char *)(*node)->v.i,0);
                }
				else if ((*node)->nodetype == en_absacon) {
								*node = makenode(en_absacon,(char *)((SYM *)(*node)->v.p[0])->value.i,0);

				}
}
int voidexpr(ENODE *node)
{       CSE      *csp;
        if( node == 0 )
                return 0;
        switch( node->nodetype ) {
                case en_rcon: case en_lrcon: case en_fcon:
											return 1;
                case en_icon:
								case en_lcon:
								case en_iucon:
								case en_lucon:
								case en_ccon:
                case en_nacon:
                case en_napccon:
								case en_absacon:
                case en_autocon:
								case en_autoreg:
												return 0;
								case en_floatref:
								case en_doubleref:
								case en_longdoubleref:
												return 1;
                case en_ub_ref:
                case en_uw_ref:
                case en_b_ref:
                case en_w_ref:
                case en_l_ref:
                case en_ul_ref:
												return 0;
                case en_uminus: case en_bits:
                case en_not:    case en_compl:
                case en_ainc:   case en_adec:
                        return voidexpr(node->v.p[0]);
								case en_cb: case en_cub:
								case en_cw: case en_cuw:
								case en_cl: case en_cul:
								case en_cf: case en_cd: case en_cp: case en_cld:
                        return voidexpr(node->v.p[0]);
                case en_add:    case en_sub:
                case en_umul:    case en_udiv: case en_umod:
                case en_mul:    case en_div:
                case en_mod:    case en_lsh:
								case en_asalsh: case en_asarsh: case en_alsh: case en_arsh:
                case en_rsh:    case en_and:
                case en_or:     case en_xor:
                case en_land:   case en_lor:
                case en_eq:     case en_ne:
                case en_lt:     case en_le:
								case en_ugt:	case en_uge: case en_ult: case en_ule:
                case en_gt:     case en_ge:
                case en_cond:   case en_void:
								case en_pmul:
                case en_fcall: case en_trapcall: case en_pdiv:
								case en_pfcall: case en_pfcallb:
                case en_intcall: case en_fcallb:
                case en_moveblock: case en_stackblock: case en_callblock:
								case en_pcallblock:
                        return voidexpr(node->v.p[0]) || voidexpr(node->v.p[1]);
                case en_asadd:  case en_assub:
                case en_asmul:  case en_asdiv:
                case en_asor:   case en_asand:   case en_asxor:
                case en_asmod:  case en_aslsh:
								case en_asumod: case en_asudiv: case en_asumul:
                case en_asrsh: case en_assign: case en_refassign:
												if (voidexpr(node->v.p[1])) {
													csp = searchnode(node->v.p[0]);
													if (csp)
														csp->voidf = 1;
												}
												return voidexpr(node->v.p[0]);
								default:
											return 0;
                }
	
}
void voidfloat(SNODE *block)
/*
 * Scan through a block and void all CSEs which do asadd, asmul, asmodiv
 * of float to int
 */
{       while( block != 0 ) {
                switch( block->stype ) {
                        case st_return:
                        case st_expr:
                                voidexpr(block->exp);
                                break;
                        case st_while:
                        case st_do:
                                voidexpr(block->exp);
                                voidfloat(block->s1);
                                break;
                        case st_for:
                                voidexpr(block->label);
                                voidexpr(block->exp);
                                voidfloat(block->s1);
                                voidexpr(block->s2);
                                break;
                        case st_if:
                                voidexpr(block->exp);
                                voidfloat(block->s1);
                                voidfloat(block->s2);
                                break;
                        case st_switch:
                                voidexpr(block->exp);
                                voidfloat(block->s1);
                                break;
                        case st_case:
                                voidfloat(block->s1);
                                break;
												case st_block:
																voidfloat(block->exp);
																break;
                        }
                block = block->next;
                }
}
void asm_repcse1(AMODE *ap)
{
	if (!ap || !ap->offset)
		return;
	asm_repnode(&ap->offset);
}
void asm_repcse(OCODE *cd)
{
	asm_repcse1(cd->oper1);
	asm_repcse1(cd->oper2);
	asm_repcse1(cd->oper3);
}