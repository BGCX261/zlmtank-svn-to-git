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
#include 				"diag.h"

/*
 *      this module contains all of the code generation routines
 *      for evaluating expressions and conditions.
 */
extern int stdinttype,stdunstype,stdintsize, stdldoublesize,stdaddrsize;
extern int stackadd,stackmod;
extern int prm_largedata, prm_linkreg;
extern AMODE     push[], pop[];
extern int prm_68020;
extern long framedepth, stackdepth;
extern int regs[3];
extern long nextlabel;
extern long lc_maxauto;
extern char regstack[],rsold[],rsodepth,rsdepth;
AMODE freg0[] = { { am_freg, 0 } };
AMODE sreg[] = { { am_dreg,4 } };

long bittab[32] = { 1,0x3,0x7,0xf,0x1f,0x3f,0x7f,0xff,
			0x1ff,0x3ff,0x7ff,0xfff,0x1fff,0x3fff,0x7fff,0xffff,
			0x1ffffL,0x3ffffL,0x7ffffL,0xfffffL,0x1fffffL,0x3fffffL,0x7fffffL,0xffffffL,
			0x1ffffffL,0x3ffffffL,0x7ffffffL,0xfffffffL,0x1fffffffL,0x3fffffffL,0x7fffffffL,0xffffffffL
};

void gen_f10code(int op, int size, AMODE *ap1, AMODE *ap2)
{
	if (size != 10) 
		gen_code(op,size,ap1,ap2);
	else {
		gen_code(op_fld,size,ap1,ap2);
		gen_code(op,size,0,0);
	}
}
int chksize(int lsize, int rsize)
{
	int l,r;
	l = lsize;
	r = rsize;
	if (l < 0) l = - l;
	if (r < 0) r = - r;
	return(l > r);
}
AMODE *fstack(void)
{
	AMODE *ap = xalloc(sizeof(AMODE));
	ap->mode = am_freg;
	ap->preg = 0;
	ap->sreg = 0;
	ap->offset = 0;
	ap->tempflag = TRUE;
	return(ap);
}
AMODE		*make_muldivval(AMODE *ap)
{
			int temp;
			AMODE *ap1 = make_label(temp = nextlabel++);
			queue_muldivval(temp,ap->offset->v.i);
			ap1->mode = am_direct;
			return(ap1);
}
void make_floatconst(AMODE *ap, int size)
{
	int temp;
	AMODE *ap1 = make_label(temp = nextlabel++);
	if (isintconst(ap->offset->nodetype))
		ap->offset->v.f = ap->offset->v.i;
	queue_floatval(temp,ap->offset->v.f,size);
	ap->mode = am_direct;
	ap->length = 8;
	ap->offset = ap1->offset;
}
AMODE    *make_label(int lab)
/*
 *      construct a reference node for an internal label number.
 */
{       ENODE    *lnode;
        AMODE    *ap;
        lnode = xalloc(sizeof(ENODE));
        lnode->nodetype = en_labcon;
        lnode->v.i = lab;
        ap = xalloc(sizeof(AMODE));
        ap->mode = am_immed;
        ap->offset = lnode;
        return ap;
}

AMODE    *make_immed(long i)
/*
 *      make a node to reference an immediate value i.
 */
{       AMODE    *ap;
        ENODE    *ep;
        ep = xalloc(sizeof(ENODE));
        ep->nodetype = en_icon;
        ep->v.i = i;
        ap = xalloc(sizeof(AMODE));
        ap->mode = am_immed;
        ap->offset = ep;
        return ap;
}
AMODE    *make_immedt(long i, int size)
/*
 *      make a node to reference an immediate value i.
 */
{
				switch (size) {
					case 1:
					case -1:
						i &= 0xff;
						break;
					case 2:
					case -2:
						i &= 0xffff;
						break;
				}
				return make_immed(i);
}

AMODE    *make_offset(ENODE *node)
/*
 *      make a direct reference to a node.
 */
{       AMODE    *ap;
        ap = xalloc(sizeof(AMODE));
        ap->mode = am_direct;
        ap->offset = node;
        return ap;
}
AMODE *make_stack(int number)
{
	AMODE *ap = xalloc(sizeof(AMODE));        
	ENODE *ep = xalloc(sizeof(ENODE));
	ep->nodetype = en_icon;
	ep->v.i = -number;
	ap->mode = am_indisp;
	ap->preg = ESP;
	ap->offset = ep;
	return(ap);
}
void make_legal(AMODE *ap,int flags,int size)
/*
 *      make_legal will coerce the addressing mode in ap1 into a
 *      mode that is satisfactory for the flag word.
 */
{       AMODE    *ap2,*ap1;
        if( ((flags & F_VOL) == 0) || ap->tempflag )
                {
                switch( ap->mode )
                        {
												case am_freg:
																if (flags & F_FREG && size > 4)
																		return;
																break;
                        case am_immed:
																if (size > 4) {
																	make_floatconst(ap,size);
								                  if (flags & F_MEM)
																		return;
																}
																else
                                	if( flags & F_IMMED )
                                        return;         /* mode ok */
																
                                break;
                        case am_dreg:
                                if( flags & F_DREG) {
                                  return;
																}
                                break;
												case am_indisp: case am_indispscale: 
												case am_direct:
																if (flags & F_INDX)
																	return;
                                if( flags & F_MEM)
                                        return;
                                break;
                        }
                }
				if (!(flags & F_DREG)) {
					if (flags & F_FREG && ap->mode != am_freg && !(flags & F_MEM)) {
						freeop(ap);
						if (size <=4) 
							if (size == 1 || size == -1) {
								ap1 = temp_data();
								if (size < 0)
									gen_code2(op_movsx,4,1,ap1,ap);
								else
									gen_code2(op_movzx,4,1,ap1,ap);
								gen_code(op_push,4,ap1,0);
								gen_code(op_fild,2,make_stack(0),0);
								gen_code(op_add,4,sreg,make_immed(4));
								freeop(ap1);
							}
							else
								gen_code(op_fild,size,ap,0);
						else
							gen_code(op_fld,size,ap,0);
						ap->mode = am_freg;
						ap->preg = 0;
						return;
					}
				}
				else if (size > 4) {
					freeop(ap);
					gen_code(op_fld,size,ap,0);
					if (flags & F_FREG) {
						ap->mode = am_freg;
						ap->preg = 0;
						return;
					}
					else {
						gen_code(op_push,4,makedreg(3),0);
						gen_code(op_fistp,4,make_stack(0),0);
						ap1 = temp_data();
						gen_code(op_pop,4,ap1,make_stack(0));
						ap->mode = ap1->mode;
						ap->preg = ap1->preg;
						return;
					}
				}
        if( size == -1 )
                {
                freeop(ap);
                ap2 = temp_data();
								if (ap->mode == am_immed)
                	gen_code(op_mov,1,ap2,ap);
								else if (ap->mode == am_dreg && ap->preg > 3)
                	gen_code(op_mov,4,ap2,ap);
								else
                	gen_code2(op_movsx,4,1,ap2,ap);
                ap->mode = ap2->mode;
                ap->preg = ap2->preg;
								ap->tempflag = TRUE;
                size = -2;
                }
				
        else if( size == 1 )
                {
                freeop(ap);
                ap2 = temp_data();
								if (ap->mode == am_immed)
                	gen_code(op_mov,1,ap2,ap);
								else if (ap->mode == am_dreg && ap->preg > 3)
                	gen_code(op_mov,4,ap2,ap);
								else
                	gen_code2(op_movzx,4,1,ap2,ap);
                ap->mode = ap2->mode;
                ap->preg = ap2->preg;
								ap->tempflag = TRUE;
                size = 2;
                }
        else if( size == -2 )
                {
                freeop(ap);
                ap2 = temp_data();
								if (ap->mode == am_immed)
                	gen_code(op_mov,2,ap2,ap);
								else if (ap->mode == am_dreg && ap->preg > 3)
                	gen_code(op_mov,4,ap2,ap);
								else
                	gen_code2(op_movsx,4,2,ap2,ap);
                ap->mode = ap2->mode;
                ap->preg = ap2->preg;
								ap->tempflag = TRUE;
                size = -4;
                }
				
        else if( size == 2 )
                {
                freeop(ap);
                ap2 = temp_data();
								if (ap->mode == am_immed)
                	gen_code(op_mov,2,ap2,ap);
								else if (ap->mode == am_dreg && ap->preg > 3)
                	gen_code(op_mov,4,ap2,ap);
								else
                	gen_code2(op_movzx,4,2,ap2,ap);
                ap->mode = ap2->mode;
                ap->preg = ap2->preg;
								ap->tempflag = TRUE;
                size = 4;
                }
				else if (size == 4 || size == -4) {
        	freeop(ap);
        	ap2 = temp_data();
        	gen_code(op_mov,size,ap2,ap);
        	ap->mode = am_dreg;
        	ap->preg = ap2->preg;
        	ap->tempflag = 1;
				}
}
void bit_legal(AMODE *ap,ENODE *node, int size)
{
	if (node->bits != -1) {
			make_legal(ap,F_DREG | F_VOL,size);
			if (node->startbit)
				gen_code(op_shr,size,ap,make_immed(node->startbit));
			gen_code(op_and,4,ap,make_immed(bittab[node->bits-1]));
	}
}
AMODE *get_bitval(AMODE *ap,ENODE *node, int size)
{
	AMODE *ap1 = temp_data();
	ap1->tempflag = TRUE;
		gen_code(op_mov,size,ap1,ap);
		if (node->startbit)
			gen_code(op_shr,size,ap1,make_immed(node->startbit));
		gen_code(op_and,4,ap1,make_immed(bittab[node->bits-1]));
		return ap1;
}
void bit_move(AMODE *ap2, AMODE *ap1, ENODE *node, int flags, int sizein, int sizeout)
{
		gen_code(op_and,sizeout,ap2,make_immed(~(bittab[node->bits-1]<<node->startbit)));
		if (ap1->mode == am_immed) {
			ap1->offset->v.i &= bittab[node->bits-1];
			if (ap1->offset->v.i) {
				ap1->offset->v.i <<= node->startbit;
				gen_code(op_or,sizeout,ap2,ap1);
				if (!(flags & F_NOVALUE)) {
					make_legal(ap2,flags,sizeout);
					ap1->offset->v.i >>= node->startbit;
					gen_code(op_mov,sizeout,ap2,ap1);
				}
			}
		}
		else {
			make_legal(ap1,F_DREG | F_VOL,sizein);
			gen_code(op_and,sizein,ap1,make_immed(bittab[node->bits-1]));
			if (node->startbit)
				gen_code(op_shl,sizein,ap1,make_immed(node->startbit));
			gen_code(op_or,sizeout,ap2,ap1);
			if (!(flags & F_NOVALUE)) {
				if (node->startbit)
					gen_code(op_shr,sizein,ap1,make_immed(node->startbit));
			}
		}
}
void do_extend(AMODE *ap,int isize,int osize,int flags)
/*
 *      if isize is not equal to osize then the operand ap will be
 *      loaded into a register (if not already) and if osize is
 *      greater than isize it will be extended to match.
 */
{				AMODE *ap2;
				if (isize == 0)
					return;
       	if( isize == osize || isize == -osize)
                return;
        if( ap->mode != am_dreg && osize <=4) {
									if (flags & F_DEST)
                		make_legal(ap,flags,isize);
									else
                		make_legal(ap,F_DREG | F_FREG,isize);
									if (flags & F_DEST)
										return;
									if (chksize(osize,isize))
										return;
				}
				switch(isize)
								{
doextend:
								case -1:
								case 1:
                case -2:
								case 2:
												if (osize < isize) {
														gen_code2(op_movsx,osize,isize,ap,ap);
												}
												else if (osize <=4 && osize > isize) {
														gen_code2(op_movzx,osize,isize,ap,ap);
												}
								case 4:
								case -4:
do4:
												if (osize <= 4)
													return;
												else {
													if (ap->mode != am_freg) {
														do_extend(ap,isize,4,F_ALL | F_VOL);
														ap2 = make_stack(0);
														gen_code(op_push,4,ap,0);
														freeop(ap);
														gen_codef(op_fild,4,ap2,0);
														gen_code(op_add,4,sreg,make_immed(4));
														freeop(ap);
													}
													ap->mode = am_freg;
													ap->preg = 0;
													ap->tempflag = TRUE;
												}
												break;
								case 6:
								case 8:
												if (osize > isize) {
													if (ap->mode != am_freg) {
														gen_codef(op_fld,osize,ap,0);	
														freeop(ap);
														ap->mode = am_freg;
														ap->preg = 0;
														ap->tempflag = TRUE;
													}
													return;
												}
												
								case 10: 
												if (ap->mode != am_freg) {
													freeop(ap);
													gen_codef(op_fld,isize,ap,0);
													ap->mode = am_freg;
													ap->preg = 0;
													ap->tempflag = TRUE;
												}
												switch(osize) {
													case 1:
													case -1:
													case 2:
													case -2:
													case 4:
													case -4:
														freeop(ap);
														ap2 = temp_data();
														ap->mode = ap2->mode;
														ap->preg = ap2->preg;
														gen_code(op_push,4,makedreg(3),0);
														gen_codef(op_fistp,4,ap2 = make_stack(0),0);
														gen_codef(op_fwait,0,0,0);
														gen_code(op_mov,osize,ap,ap2);
														gen_code(op_add,4,sreg,make_immed(4));
														goto doextend;
													case 6:
													case 8:
														break;
												}
				}
}

int     isshort(ENODE *node)
/*
 *      return true if the node passed can be generated as a short
 *      offset.
 */
{       return (isintconst(node->nodetype) &&
                (node->v.i >= -32768L && node->v.i <= 32767L));
}

int     isbyte(ENODE *node)
/*
 *      return true if the node passed can be evaluated as a byte
 *      offset.
 */
{       return isintconst(node->nodetype) &&
                (-128 <= node->v.i && node->v.i <= 127);
}

AMODE    *gen_index(ENODE *node)
/*
 *      generate code to evaluate an index node (^+) and return
 *      the addressing mode of the result. This routine takes no
 *      flags since it always returns either am_ind or am_indx.
 */
{       AMODE    *ap1,*ap2, *ap, *ap3;
				ENODE node2;
				int scale;

				switch (node->v.p[0]->nodetype) {
					case en_icon:
						ap1 = gen_expr(node->v.p[0],F_IMMED,4);
						break;
					case en_lsh:
						if ((scale = node->v.p[0]->v.p[1]->v.i) < 4 && scale) {
							ap1 = gen_expr(node->v.p[0]->v.p[0],F_IMMED | F_DREG,4);
							if (ap1->mode == am_immed) {
								while (--scale)
									ap1->offset->v.i <<=1;
							}
							else {
								ap1->mode = am_indispscale;
								ap1->sreg = ap1->preg;
								ap1->preg = -1;
								ap1->scale = scale;
								ap1->offset = makenode(en_icon,0,0);
							}
							break;
						}
					default:
						mark();
						ap1 = gen_deref(node,F_MEM | F_DREG,4);
						switch (ap1->mode) {
							default:
								rsold[rsodepth-1] = rsdepth;
								break;
							case am_indispscale:
								if (ap1->sreg >=0 && ap1->preg >= 0) {
									int t = rsold[rsodepth-1];
									freeop(ap1);
									ap3 = temp_data();
									gen_code(op_lea,4,ap3,ap1);
									ap3->mode = am_indisp;
									ap3->offset = makenode(en_icon,0,0);
									if (t <rsdepth-1 && ap3->preg + 8 == regstack[t+1])
										t+=2;
									if (t <rsdepth && ap3->preg + 8 == regstack[t])
										t+=1;
									ap1 = ap3;
								}
						
						}
						release();
						break;
				}
				switch (node->v.p[1]->nodetype) {
					case en_icon:
						ap2 = gen_expr(node->v.p[1],F_IMMED,4);
						break;
					case en_lsh:
						if ((scale = node->v.p[1]->v.p[1]->v.i) < 4 && scale) {
							ap2 = gen_expr(node->v.p[1]->v.p[0],F_IMMED | F_DREG,4);
							if (ap2->mode == am_immed) {
								while (--scale)
									ap2->offset->v.i <<=1;
							}
							else {
								ap2->mode = am_indispscale;
								ap2->sreg = ap2->preg;
								ap2->preg = -1;
								ap2->scale = scale;
								ap2->offset = makenode(en_icon,0,0);
							}
							break;
						}
					default:
						node2.v.p[0] = node->v.p[1];
						node2.nodetype = node->nodetype;
						mark();
						ap2 = gen_deref(&node2,F_MEM | F_DREG,4);
						switch (ap1->mode) {
							default:
								rsold[rsodepth-1] = rsdepth;
								break;
							case am_indispscale:
								if (ap1->sreg >=0 && ap1->preg >= 0) {
									int t = rsold[rsodepth-1];
									freeop(ap1);
									ap3 = temp_data();
									gen_code(op_lea,4,ap3,ap1);
									ap3->mode = am_indisp;
									ap3->offset = makenode(en_icon,0,0);
									if (t <rsdepth-1 && ap3->preg + 8 == regstack[t+1])
										t+=2;
									if (t <rsdepth && ap3->preg + 8 == regstack[t])
										t+=1;
									ap1 = ap3;
								}
						
						}
						release();
						break;
				}
				switch(ap1->mode) {
					case am_dreg:
						switch (ap2->mode) {
							case am_dreg:
								ap1->sreg = ap2->preg;
								ap1->scale = 0;
								ap1->mode = am_indispscale;
								ap1->offset = makenode(en_icon,(char *)0,0);
								return ap1;
							case am_immed:
							case am_direct:
								ap2->preg = ap1->preg;
								ap2->mode = am_indisp;
								return ap2;	
							case am_indisp:
								ap2->sreg = ap2->preg;
								ap2->preg = ap1->preg;
								ap2->mode = am_indispscale;
								ap2->offset = ap1->offset;
								ap2->scale = 0;
								return ap2;
							case am_indispscale:
								if (ap2->preg == -1) {
									ap2->preg = ap1->preg;
									return ap2;
								}
								freeop(ap2);
								ap = temp_data();
								gen_code(op_lea,4,ap,ap2);
								ap->sreg = ap1->preg;
								ap->mode = am_indispscale;
								ap->scale = 0;
								ap->offset = makenode(en_icon,0,0);
								return ap;
						}
						break;
					case am_direct:
					case am_immed:
						switch (ap2->mode) {
							case am_dreg:
								ap2->mode = am_indisp;
								ap2->offset = ap1->offset;
								return ap2;
							case am_immed:
							case am_direct:
								if (ap1->offset->nodetype == en_icon && ap2->offset->nodetype == en_icon)
									ap1->offset->v.i += ap2->offset->v.i;
								else
									ap1->offset = makenode(en_add,ap1->offset,ap2->offset);
							  ap1->mode = am_direct;
								return ap1;
							case am_indisp:
							case am_indispscale:
								if (ap1->offset->nodetype == en_icon && ap2->offset->nodetype == en_icon)
									ap2->offset->v.i += ap1->offset->v.i;
								else
									ap2->offset = makenode(en_add,ap1->offset,ap2->offset);
								return ap2;
						}
						break;
					case am_indisp:
						switch (ap2->mode) {
							case am_dreg:
								ap1->mode = am_indispscale;
								ap1->sreg = ap2->preg;
								ap1->scale = 0;
								return ap1;
							case am_immed:
							case am_direct:
								if (ap1->offset->nodetype == en_icon && ap2->offset->nodetype == en_icon)
									ap1->offset->v.i += ap2->offset->v.i;
								else
									ap1->offset = makenode(en_add,ap1->offset,ap2->offset);
								return ap1;	
							case am_indisp:
								ap1->mode = am_indispscale;
								ap1->sreg = ap2->preg;
								ap1->scale = 0;
								if (ap1->offset->nodetype == en_icon && ap2->offset->nodetype == en_icon)
									ap1->offset->v.i += ap2->offset->v.i;
								else
									ap1->offset = makenode(en_add,ap1->offset,ap2->offset);
								return ap1;
							case am_indispscale:
								if (ap2->preg == -1) {
									ap2->preg = ap1->preg;
									if (ap1->offset->nodetype == en_icon && ap2->offset->nodetype == en_icon)
										ap2->offset->v.i += ap1->offset->v.i;
									else
										ap2->offset = makenode(en_add,ap1->offset,ap2->offset);
									return ap2;
								}
								freeop(ap2);
								ap = temp_data();
								gen_code(op_lea,4,ap,ap2);
								ap1->sreg = ap->preg;
								ap1->scale = 0;
								ap1->mode = am_indispscale;
								if (ap1->offset->nodetype == en_icon && ap2->offset->nodetype == en_icon)
									ap1->offset->v.i += ap2->offset->v.i;
								else
									ap1->offset = makenode(en_add,ap1->offset,ap2->offset);
								return ap1;
						}
						break;
					case am_indispscale:
						switch (ap2->mode) {
							case am_dreg:
								if (ap1->preg == -1) {
									ap1->preg = ap2->preg;
									return ap1;
								}
								freeop(ap1);
								ap = temp_data();
								gen_code(op_lea,4,ap,ap1);
								ap->sreg = ap2->preg;
								ap->scale = 0;
								ap->offset = ap1->offset;
								return ap;
							case am_immed:
							case am_direct:
								if (ap1->offset->nodetype == en_icon && ap2->offset->nodetype == en_icon)
									ap1->offset->v.i += ap2->offset->v.i;
								else
									ap1->offset = makenode(en_add,ap1->offset,ap2->offset);
								return ap1;
							case am_indisp:
								if (ap1->preg == -1) {
									ap1->preg = ap2->preg;
									if (ap1->offset->nodetype == en_icon && ap2->offset->nodetype == en_icon)
										ap1->offset->v.i += ap2->offset->v.i;
									else
										ap1->offset = makenode(en_add,ap1->offset,ap2->offset);
									return ap1;
								}
								freeop(ap1);
								ap = temp_data();
								gen_code(op_lea,4,ap,ap1);
								ap->sreg = ap2->preg;
								ap->scale = 0;
								ap->mode = am_indispscale;
								if (ap1->offset->nodetype == en_icon && ap2->offset->nodetype == en_icon)
									ap->offset->v.i = ap2->offset->v.i +ap1->offset->v.i;
								else
									ap->offset = makenode(en_add,ap1->offset,ap2->offset);
								return ap;
							case am_indispscale:
								if (ap1->preg == -1 && ap2->preg == -1) {
									if (ap1->scale == 0) {
										ap2->preg = ap1->sreg;
										if (ap1->offset->nodetype == en_icon && ap2->offset->nodetype == en_icon)
											ap2->offset->v.i += ap1->offset->v.i;
										else
											ap2->offset = makenode(en_add,ap1->offset,ap2->offset);
										return ap2;
									} else if (ap2->scale == 0) {
										ap1->preg = ap2->sreg;
										if (ap1->offset->nodetype == en_icon && ap2->offset->nodetype == en_icon)
											ap1->offset->v.i += ap2->offset->v.i;
										else
											ap1->offset = makenode(en_add,ap1->offset,ap2->offset);
										return ap1;
									}
								}
								if (ap1->preg == -1) {
									freeop(ap2);
									ap = temp_data();
									gen_code(op_lea,4,ap,ap2);
									ap1->preg = ap->preg;
									return ap1;
								}
								else if (ap2->preg == -1) {
									freeop(ap1);
									ap = temp_data();
									gen_code(op_lea,4,ap,ap1);
									ap2->preg = ap->preg;
									return ap2;
								}
								freeop(ap1);
								ap = temp_data();
								gen_code(op_lea,4,ap,ap1);
								freeop(ap2);
								ap1 = temp_data();
								gen_code(op_lea,4,ap1,ap2);
								ap->mode = am_indispscale;
								ap->sreg = ap1->preg;
								ap->scale = 0;
								ap->offset = makenode(en_icon,0,0);
								return ap;
						}
						break;           
				}
				DIAG("invalid index conversion");
}

AMODE    *gen_deref(ENODE *node, int flags,int size)
/*
 *      return the addressing mode of a dereferenced node.
 */
{       AMODE    *ap1;
        int             ssize,psize;
				psize = size;
				if (psize < 0)
					psize = - psize;
        switch( node->nodetype )        /* get load size */
                {
                case en_ub_ref:
                        ssize = 1;
                        break;
                case en_b_ref:
                        ssize = -1;
                        break;
                case en_uw_ref:
                        ssize = 2;
                        break;
                case en_w_ref:
                        ssize = -2;
                        break;
                case en_l_ref:
												ssize = -4;
												break;
								case en_ul_ref:
                        ssize = 4;
                        break;
								case en_floatref:
												ssize = 6;
												break;
								case en_doubleref:
												ssize = 8;
												break;
								case en_longdoubleref:
												ssize = 10;
												break;
								default:
												ssize = 4;
                }
        if( node->v.p[0]->nodetype == en_add )
                {
                ap1 = gen_index(node->v.p[0]);
                do_extend(ap1,ssize,psize,flags);
                make_legal(ap1,flags,psize);
                return ap1;
                }
        else if( node->v.p[0]->nodetype == en_autocon  || node->v.p[0]->nodetype == en_autoreg)
                {
                ap1 = xalloc(sizeof(AMODE));
                ap1->mode = am_indisp;
								ap1->preg = ESP;
								if (prm_linkreg) {
			          	ap1->preg = EBP;
      			    	ap1->offset = makenode(en_icon,(char *)((SYM *)node->v.p[0]->v.p[0])->value.i,0);
								}
								else if (((SYM *)node->v.p[0]->v.p[0])->funcparm)
                	ap1->offset = makenode(en_icon,(char *)(((SYM *)node->v.p[0]->v.p[0])->value.i+framedepth+stackdepth),0);
                else
									ap1->offset = makenode(en_icon,(char *)(((SYM *)node->v.p[0]->v.p[0])->value.i+stackdepth+lc_maxauto),0);
                do_extend(ap1,ssize,size,flags);
                make_legal(ap1,flags,psize);
                return ap1;
                }
        else if( node->v.p[0]->nodetype == en_nacon || node->v.p[0]->nodetype == en_napccon)
                {
                ap1 = xalloc(sizeof(AMODE));
                ap1->offset = makenode(node->v.p[0]->nodetype,(char *)((SYM *)node->v.p[0]->v.p[0])->name,0);
                ap1->mode = am_direct;
                do_extend(ap1,ssize,size,flags);
                make_legal(ap1,flags,psize);
                return ap1;
                }
        else if( node->v.p[0]->nodetype == en_nalabcon)
                {
                ap1 = xalloc(sizeof(AMODE));
                ap1->offset = makenode(node->v.p[0]->nodetype,(char *)node->v.p[0]->v.i,0);
                ap1->mode = am_direct;
                do_extend(ap1,ssize,size,flags);
                make_legal(ap1,flags,psize);
                return ap1;
                }
        else if( node->v.p[0]->nodetype == en_labcon)
                {
                ap1 = xalloc(sizeof(AMODE));
                ap1->mode = am_direct;
                ap1->offset = makenode(node->v.p[0]->nodetype,(char *)node->v.p[0]->v.i,0);
                do_extend(ap1,ssize,size,flags);
                make_legal(ap1,flags,psize);
                return ap1;
                }
				else if (node->v.p[0]->nodetype == en_absacon) {
								ap1 = xalloc(sizeof(AMODE));
								ap1->mode = am_direct;
								ap1->offset = makenode(en_absacon,(char *)((SYM *)node->v.p[0]->v.p[0])->value.i,0);
                do_extend(ap1,ssize,size,flags);
                make_legal(ap1,flags,psize);
                return ap1;

				}
				else if (node->v.p[0]->nodetype == en_regref) {
        	ap1 = gen_expr(node->v.p[0],F_ALL,4);
          do_extend(ap1,ssize,size,flags);
          make_legal(ap1,flags,psize);
					return ap1;
				}
        ap1 = gen_expr(node->v.p[0],F_DREG | F_IMMED,4); /* generate address */
        if( ap1->mode == am_dreg )
                {
                ap1->mode = am_indisp;
								ap1->offset = makenode(en_icon,0,0);
          			do_extend(ap1,ssize,size,flags);
        				make_legal(ap1,flags,psize);
                return ap1;
                }
        ap1->mode = am_direct;
        do_extend(ap1,ssize,size,flags);
        make_legal(ap1,flags,psize);
        return ap1;
}

AMODE    *gen_unary(ENODE *node,int flags,int size,int op, int fop)
/*
 *      generate code to evaluate a unary minus or complement.
 */
{       AMODE    *ap;
        ap = gen_expr(node->v.p[0],F_FREG | F_DREG | F_VOL,size);
				if (ap->mode == am_freg) {
					gen_code(fop,0,0,0);
					ap = fstack();
				}
				else {
        	gen_code(op,size,ap,0);
				}
        make_legal(ap,flags,size);
        return ap;
}

AMODE    *gen_binary(ENODE *node,int flags,int size,int op, int fop)
/*
 *      generate code to evaluate a binary node and return 
 *      the addressing mode of the result.
 */
{       AMODE    *ap1, *ap2;
				if (size > 4) {
        	ap1 = gen_expr(node->v.p[0],F_VOL | F_FREG,size);
					mark();
        	ap2 = gen_expr(node->v.p[1],F_MEM | F_FREG,size);
					if (ap2->mode == am_freg)
						gen_code(fop,0,0,0);
					else {
						int t;
						t = natural_size(node->v.p[1]);
						if (t== 0) t = size;
						if (t <=4)
							if (fop == op_fadd)
								fop = op_fiadd;
							else
								fop = op_fisub;
						gen_f10code(fop,t,ap2,0);
					}
					ap1 = fstack();
				}
				else {
        	ap1 = gen_expr(node->v.p[0],F_VOL | F_DREG,size);
					mark();
        	ap2 = gen_expr(node->v.p[1],F_ALL,size);
        	gen_code(op,size,ap1,ap2);
				}
        freeop(ap2);
				release();
        make_legal(ap1,flags,size);
        return ap1;
}

AMODE    *gen_xbin(ENODE *node,int flags,int size,int op, int fop)
/*
 *      generate code to evaluate a restricted binary node and return 
 *      the addressing mode of the result.
 */
{       AMODE    *ap1, *ap2;
				if (size > 4) {
        	ap1 = gen_expr(node->v.p[0],F_VOL | F_FREG,size);
					mark();
        	ap2 = gen_expr(node->v.p[1],F_MEM | F_FREG,size);
					if (ap2->mode == am_freg)
						gen_code(fop,0,0,0);
					else
						gen_code(fop,size,ap2,0);
					ap1 = fstack();
				}
				else {
        	ap1 = gen_expr(node->v.p[0],F_VOL | F_DREG,size);
					mark();
        	ap2 = gen_expr(node->v.p[1],F_ALL,size);
        	gen_code(op,size,ap2,ap1);
				}
				release();
        freeop(ap2);
        make_legal(ap1,flags,size);
        return ap1;
}
void doshift(AMODE *ap1, AMODE *ap2, int size, int op)
{       AMODE   *ecx = makedreg(ECX), *eax = makedreg(EAX);
				if (ap2->mode == am_immed) {
					gen_code2(op,size,1,ap1,ap2);
				}
				else
					if (ap1->mode == am_dreg && ap1->preg == ECX) {
							if (ap2->mode == am_dreg) {
								gen_code(op_xchg,4,ap2,ap1);
								gen_code2(op,size,1,ap2,ecx);
								gen_code(op_xchg,4,ap2,ap1);
							}
							else {
								if (regs[0])
									gen_push(EAX,am_dreg,0);
								gen_code(op_xchg,4,eax,ecx);
								gen_code(op_mov,1,ap2,ecx);
								gen_code2(op,size,1,eax,ecx);
								gen_code(op_xchg,4,eax,ecx);
								if (regs[0])
									gen_pop(EAX,am_dreg,0);
							}
					}
					else
						if (ap2->mode == am_dreg) {
							if (ap2->preg != ECX)
								gen_code(op_xchg,4,ap2,ecx);
							gen_code2(op,size,1,ap1,ecx);
							if (ap2->preg != ECX)
								gen_code(op_xchg,4,ap2,ecx);
						}
						else {
							if (regs[1])
								gen_push(ECX,am_dreg,0);
							gen_code(op_mov,4,ap2,ecx);
							gen_code2(op,size,1,ap1,ecx);
							if (regs[1])
								gen_pop(ECX,am_dreg,0);
						}
}
AMODE    *gen_shift(ENODE *node, int flags, int size, int op)
/*
 *      generate code to evaluate a shift node and return the
 *      address mode of the result.
 */
{       AMODE    *ap1, *ap2;
        ap1 = gen_expr(node->v.p[0],F_DREG | F_VOL,size);
				mark();
        ap2 = gen_expr(node->v.p[1],F_DREG | F_IMMED,natural_size(node->v.p[1]));
				doshift(ap1,ap2,size,op);
        freeop(ap2);
				release();
        make_legal(ap1,flags,size);
        return ap1;
}
void dodiv(AMODE *ap1, AMODE *ap2, int size, int op,int modflag)
{
	AMODE *eax = makedreg(EAX), *edx = makedreg(EDX), *ecx = makedreg(ECX);
	if (ap2->mode == am_immed) {
		ap2 = make_muldivval(ap2);
	}
	if (ap1->preg != EAX) {
		int temp;
		gen_code(op_xchg,4,eax,ap1);
		if (ap1->mode == am_dreg && ap2->mode == am_dreg && ap1->preg == ap2->preg) {
			gen_code(op,4,eax,0);
			gen_code(op_xchg,4,eax,ap1);
			return;
		}
		temp = regs[0];
		regs[0] = regs[ap1->preg];
		regs[ap1->preg] = temp;
	}
	if (ap2->preg == EDX) {
		if (regs[1])
			gen_push(ECX,am_dreg,0);
		if (regs[2])
			gen_push(EDX,am_dreg,0);
		gen_code(op_mov,4,ecx,edx);
		gen_code(op_sub,4,edx,edx);
		gen_code(op,4,ecx,0);
		if (modflag)
			gen_code(op_xchg,4,eax,edx);
		if (regs[2])
			gen_pop(EDX,am_dreg,0);
		if (regs[1])
			gen_pop(ECX,am_dreg,0);
	}
	else {
		if (regs[2])
			gen_push(EDX,am_dreg,0);
		gen_code(op_sub,4,edx,edx);
		gen_code(op,4,ap2,0);
		if (modflag)
			gen_code(op_xchg,4,eax,edx);
		if (regs[2])
			gen_pop(EDX,am_dreg,0);
	}
	if (ap1->preg != EAX) {
		int temp;
		gen_code(op_xchg,4,eax,ap1);
		temp = regs[0];
		regs[0] = regs[ap1->preg];
		regs[ap1->preg] = temp;
	}
}
void domul(AMODE *ap1, AMODE *ap2, int size, int op)
{
	AMODE *eax = makedreg(EAX), *edx = makedreg(EDX), *ecx = makedreg(ECX);
	if (ap2->mode == am_immed) {
		ap2 = make_muldivval(ap2);
	}
	if (ap1->preg != EAX) {
		int temp;
		gen_code(op_xchg,4,eax,ap1);
		if (ap1->mode == am_dreg && ap2->mode == am_dreg && ap1->preg == ap2->preg && ap1->preg != EDX) {
			gen_code(op,4,eax,0);
			gen_code(op_xchg,4,eax,ap1);
			return;
		}
		temp = regs[0];
		regs[0] = regs[ap1->preg];
		regs[ap1->preg] = temp;
	}
	if (ap2->preg == EDX) {
		if (regs[1])
			gen_push(ECX,am_dreg,0);
		if (regs[2])
			gen_push(EDX,am_dreg,0);
		gen_code(op_mov,4,ecx,edx);
		gen_code(op_sub,4,edx,edx);
		gen_code(op,4,ecx,0);
		if (regs[2])
			gen_pop(EDX,am_dreg,0);
		if (regs[1])
			gen_pop(ECX,am_dreg,0);
	}
	else {
		if (regs[2])
			gen_push(EDX,am_dreg,0);
		gen_code(op,4,ap2,0);
		if (regs[2])
			gen_pop(EDX,am_dreg,0);
	}
	if (ap1->preg != EAX) {
		int temp;
		gen_code(op_xchg,4,eax,ap1);
		temp = regs[0];
		regs[0] = regs[ap1->preg];
		regs[ap1->preg] = temp;
	}
}
AMODE    *gen_modiv(ENODE *node, int flags, int size, int op, int modflag)
/*
 *      generate code to evaluate a mod operator or a divide
 *      operator. these operations are done on only long
 *      divisors and word dividends so that the 68000 div
 *      instruction can be used.
 */
{       AMODE    *ap1, *ap2;
				
				if (size > 4) {
					ap1 = gen_expr(node->v.p[0],F_FREG | F_VOL,size);
					ap2 = gen_expr(node->v.p[1],F_ALL,size);
					op = op_fdiv;
					if (ap2->mode == am_freg)
						gen_code(op,0,0,0);
					else {
						int t;
						t = natural_size(node->v.p[1]);
						if (t== 0) t = size;
						if (t <=4)
							op = op_fidiv;
						gen_f10code(op,t,ap2,0);
					}
					ap1 = fstack();
					do_extend(ap1,10,size,flags);
					make_legal(ap1,flags,size);
					return ap1;
				}
					if (op == op_idiv) {
  	      	ap1 = gen_expr(node->v.p[0],F_DREG | F_VOL,-4);
					}
					else {
        		ap1 = gen_expr(node->v.p[0],F_DREG | F_VOL,4);
					}
					mark();
					if (op == op_idiv) {
    	    	ap2 = gen_expr(node->v.p[1],F_ALL,-4);
					}
					else {
        		ap2 = gen_expr(node->v.p[1],F_ALL,4);
					}
					dodiv(ap1,ap2,size,op,modflag);
					freeop(ap2);
					release();
        	make_legal(ap1,flags,size);
        	return ap1;
					
}


void swap_nodes(ENODE *node)
/*
 *      exchange the two operands in a node.
 */
{       ENODE    *temp;
        temp = node->v.p[0];
        node->v.p[0] = node->v.p[1];
        node->v.p[1] = temp;
}

AMODE * gen_pdiv(ENODE *node, int flags, int size)
{
				return gen_modiv(node,flags,size,op_div,FALSE);
}			
AMODE * gen_pmul(ENODE *node, int flags, int size)
{
				return gen_mul(node,flags,size,op_mul);
}			
AMODE    *gen_mul(ENODE *node, int flags, int size, int op)
/*
 *      generate code to evaluate a multiply node. both operands
 *      are treated as words and the result is long and is always
 *      in a register so that the 68000 mul instruction can be used.
 */
{       AMODE    *ap1, *ap2;

				if (isintconst(node->v.p[0]->nodetype))
					swap_nodes(node);				
				if (size > 4) {
					ap1 = gen_expr(node->v.p[0],F_FREG | F_VOL,size);
					ap2 = gen_expr(node->v.p[1],F_ALL,size);
					op = op_fmul;
					if (ap2->mode == am_freg)
						gen_code(op,0,0,0);
					else {
						int t;
						t = natural_size(node->v.p[1]);
						if (t== 0) t = size;
						if (t <=4)
							op = op_fimul;
						gen_f10code(op,t,ap2,0);
					}
					ap1 = fstack();
					do_extend(ap1,10,size,flags);
					make_legal(ap1,flags,size);
					return ap1;
				}
					if (op == op_imul) {
      	  	ap1 = gen_expr(node->v.p[0],F_DREG | F_VOL,-4);
					}
					else {
	        	ap1 = gen_expr(node->v.p[0],F_DREG | F_VOL,4);
					}
					mark();
					if (op == op_imul) {
    	    	ap2 = gen_expr(node->v.p[1],F_ALL,-4);
					}
					else {
        		ap2 = gen_expr(node->v.p[1],F_ALL,4);
					}
					domul(ap1,ap2,size,op);
					freeop(ap2);
					release();
					do_extend(ap1,4,size,flags);
        	make_legal(ap1,flags,size);
        	return ap1;
}
AMODE    *gen_hook(ENODE *node, int flags, int size)
/*
 *      generate code to evaluate a condition operator node (?:)
 */
{       AMODE    *ap1, *ap2;
        int             false_label, end_label;
        false_label = nextlabel++;
        end_label = nextlabel++;
        flags = (flags & (F_AREG | F_DREG)) | F_VOL;
        falsejp(node->v.p[0],false_label);
        node = node->v.p[1];
        ap1 = gen_expr(node->v.p[0],flags,size);
        freeop(ap1);
        gen_code(op_jmp,0,make_label(end_label),0);
        gen_label(false_label);
        ap2 = gen_expr(node->v.p[1],flags,size);
        if( !equal_address(ap1,ap2) )
                {
                freeop(ap2);
                temp_data();
                gen_code(op_mov,size,ap2,ap1);
                }
        gen_label(end_label);
        return ap1;
}

void floatstore(AMODE *ap, int size, int flags)
{
	if (size <= 4)
						if (flags & F_NOVALUE)
						  gen_codef(op_fistp,size,ap,0);
						else
						  gen_codef(op_fist,size,ap,0);
	else
						if (flags & F_NOVALUE)
						  gen_codef(op_fstp,size,ap,0);
						else
						  gen_codef(op_fst,size,ap,0);
}
AMODE    *gen_asadd(ENODE *node, int flags, int size, int op, int fop)
/*
 *      generate a plus equal or a minus equal node.
 */
{       AMODE    *ap1, *ap2, *ap3;
        int             ssize,rsize;
        ssize = natural_size(node->v.p[0]);
        rsize = natural_size(node->v.p[1]);
				if (rsize == 0)
					rsize = ssize;
				if (rsize > 4) {
					ap2 = gen_expr(node->v.p[0],F_FREG,rsize);
					ap3 = gen_expr(node->v.p[0],F_MEM,ssize);
					ap1 = gen_expr(node->v.p[1],F_FREG | F_MEM,rsize);
					if (ap1->mode == am_freg)
						gen_code(fop,0,0,0);
					else {
						if (rsize <= 4)
							if (fop == op_fadd)
								fop = op_fiadd;
							else
								fop = op_fisub;
						gen_f10code(fop,rsize,ap1,0);
					}
					floatstore(ap3,ssize,flags);
					gen_codef(op_fwait,0,0,0);
					if (!(flags & F_NOVALUE)) {
						ap2 = fstack();
						do_extend(ap2,ssize,size,flags);
						make_legal(ap2,flags,size);
					}
					return ap2;
				}
        if (chksize( ssize ,rsize ))
                rsize = ssize;
      	  ap2 = gen_expr(node->v.p[0],F_ALL | F_NOBIT | F_DEST,ssize);
				mark();
        ap1 = gen_expr(node->v.p[1],F_DREG | F_IMMED,rsize);
				if (node->v.p[0]->nodetype == en_bits)
					ap3= get_bitval(ap2,node->v.p[0],ssize);
				if (node->v.p[0]->nodetype == en_bits) {
					gen_code(op,ssize,ap3,ap1);
					bit_move(ap2,ap3,node->v.p[0],flags, ssize,rsize);
					freeop(ap3);
				}
				else
       		gen_code(op,ssize,ap2,ap1);
				freeop(ap1);
				release();
				if (flags & F_NOVALUE)
					freeop(ap2);
				else {
					do_extend(ap2,ssize,size,flags);
       		make_legal(ap2,flags,4);
				}
        return ap2;
}

AMODE    *gen_aslogic(ENODE *node, int flags, int size, int op)
/*
 *      generate a and equal or a or equal node.
 */
{       AMODE    *ap1, *ap2;
        int             ssize,rsize;
        ssize = natural_size(node->v.p[0]);
        rsize = natural_size(node->v.p[1]);
        if (chksize( ssize , rsize ))
                rsize = ssize;
			
      	  ap2 = gen_expr(node->v.p[0],F_ALL | F_NOBIT | F_DEST,ssize);
				mark();
        ap1 = gen_expr(node->v.p[1],F_DREG | F_IMMED,rsize);
				if (node->v.p[0]->nodetype == en_bits) {
					if (ap1->mode == am_immed) {
						ap1->offset->v.i &= bittab[node->v.p[0]->bits-1];
						ap1->offset->v.i <<= node->v.p[0]->startbit;
						gen_code(op,ssize,ap2,ap1);
					}
					else {
					  gen_code(op_and,ssize,ap1,make_immed(bittab[node->v.p[0]->bits-1]));
						if (node->v.p[0]->startbit)
					  	gen_code(op_shl,ssize,ap1,make_immed(node->v.p[0]->startbit));
						gen_code(op,ssize,ap2,ap1);
						if (!(flags & F_NOVALUE)) {
							freeop(ap1);
							release();
							if (node->v.p[0]->startbit)
					  		gen_code(op_shr,ssize,ap2,make_immed(node->v.p[0]->startbit));
        			do_extend(ap2,ssize,size,0);
							make_legal(ap2,F_DREG,size);
							return(ap2);
						}
					}
				}
				else
        	gen_code(op,ssize,ap2,ap1);
				freeop(ap1);
				release();
				if (flags & F_NOVALUE)
					freeop(ap2);
				else {
					do_extend(ap2,ssize,size,flags);
        	make_legal(ap2,flags,4);
				}
        return ap2;
}

AMODE *gen_asshift(ENODE *node, int flags, int size, int op)
/*
 *      generate shift equals operators.
 */
{       
        AMODE    *ap1, *ap2, *ap3;
        int ssize = natural_size(node->v.p[0]);
        int rsize = natural_size(node->v.p[1]);
        if (chksize( ssize , rsize ))
                rsize = rsize;
      	  ap2 = gen_expr(node->v.p[0],F_ALL | F_NOBIT | F_DEST,ssize);
				mark();
        ap1 = gen_expr(node->v.p[1],F_DREG | F_IMMED,rsize);
				if (node->v.p[0]->nodetype == en_bits)
					ap3 = get_bitval(ap2,node->v.p[0],ssize);
				else
					ap3 = ap2;

				doshift(ap3,ap1,ssize,op);
				if (node->v.p[0]->nodetype == en_bits)
					bit_move(ap2,ap3,node->v.p[0],flags,ssize,rsize);
				freeop(ap1);
				release();
				if (ap3 != ap1)
					freeop(ap3);
				if (flags & F_NOVALUE)
					freeop(ap2);
				else {
					do_extend(ap2,ssize,size,flags);
       		make_legal(ap2,flags,4);
				}
        	return ap2;
}

AMODE    *gen_asmul(ENODE *node, int flags, int size,int op)
/*
 *      generate a *= node.
 */
{       AMODE    *ap1, *ap2,*ap3;
        int             ssize, lsize,rsize;
        ssize = natural_size(node->v.p[0]);
        rsize = natural_size(node->v.p[1]);
				if (rsize == 0)
					rsize = ssize;
				if (rsize > 4) {
					int fop = op_fmul;
					ap2 = gen_expr(node->v.p[0],F_FREG,rsize);
					ap3 = gen_expr(node->v.p[0],F_MEM,ssize);
					ap1 = gen_expr(node->v.p[1],F_FREG | F_MEM,rsize);
					if (ap1->mode == am_freg)
						gen_code(fop,0,0,0);
					else {
						if (rsize <= 4)
							fop = op_fimul;
						gen_f10code(fop,ssize,ap1,0);
					}
					floatstore(ap3,ssize,flags);
					gen_codef(op_fwait,0,0,0);
					if (!(flags & F_NOVALUE)) {
						ap2 = fstack();
						do_extend(ap2,ssize,size,flags);
						make_legal(ap2,flags,size);
					}
					return ap2;
				}
				if (op == op_imul)
					lsize= -4;
				else
					lsize = 4;
      	ap2 = gen_expr(node->v.p[0],F_ALL | F_NOBIT | F_DEST,ssize);
				mark();
    	  ap1 = gen_expr(node->v.p[1],F_ALL,lsize);
				ap3 = xalloc(sizeof(AMODE));
				ap3->mode = ap2->mode;
				ap3->preg = ap2->preg;
				ap3->sreg = ap2->sreg;
				ap3->scale = ap2->scale;
				ap3->offset = ap2->offset;
				if (node->v.p[0]->nodetype == en_bits)
					ap2 = get_bitval(ap2,node->v.p[0],ssize);
				else {
        	make_legal(ap2,F_DREG | F_VOL,4);
					do_extend(ap2,ssize,lsize,F_DREG);
				}
				domul(ap2,ap1,lsize,op);
				freeop(ap1);
				release();
				if (!equal_address(ap2,ap3))
					if (node->v.p[0]->nodetype == en_bits)
						bit_move(ap3,ap2,node->v.p[0],flags,ssize,rsize);
					else
						gen_code(op_mov,ssize,ap3,ap2);
				if (flags & F_NOVALUE)
					freeop(ap2);
				else {
					do_extend(ap2,ssize,size,flags);
       		make_legal(ap2,flags,4);
				}
        return ap2;
}

AMODE    *gen_asmodiv(ENODE *node, int flags, int size, int op, int modflag)
/*
 *      generate /= and %= nodes.
 */
{       AMODE    *ap1, *ap2,*ap3;
        int             ssize,lsize,rsize;
        ssize = natural_size(node->v.p[0]);
        rsize = natural_size(node->v.p[1]);
				if (rsize == 0)
					rsize = ssize;
				if (rsize > 4) {
					int fop = op_fdiv;
					ap2 = gen_expr(node->v.p[0],F_FREG,rsize);
					ap3 = gen_expr(node->v.p[0],F_MEM,ssize);
					ap1 = gen_expr(node->v.p[1],F_FREG | F_MEM,rsize);
					if (ap1->mode == am_freg)
						gen_code(fop,0,0,0);
					else {
						if (rsize <= 4)
							fop = op_fidiv;
						gen_f10code(fop,ssize,ap1,0);
					}
					floatstore(ap3,ssize,flags);
					gen_codef(op_fwait,0,0,0);
					if (!(flags & F_NOVALUE)) {
						ap2 = fstack();
						do_extend(ap2,ssize,size,flags);
						make_legal(ap2,flags,size);
					}
					return ap2;
				}                                                   
				if (op == op_idiv)                                  
					lsize= -4;                                        
				else                                                
					lsize = 4;                                        
			                                                      
     	  ap2 = gen_expr(node->v.p[0],F_ALL | F_NOBIT | F_DEST, ssize);         
				mark();
   	    ap1 = gen_expr(node->v.p[1],F_ALL,lsize);         
				ap3 = xalloc(sizeof(AMODE));                      
				ap3->mode = ap2->mode;                            
				ap3->preg = ap2->preg;                            
				ap3->sreg = ap2->sreg;                            
				ap3->scale = ap2->scale;                          
				ap3->offset = ap2->offset;
				if (node->v.p[0]->nodetype == en_bits)
					ap2 = get_bitval(ap2,node->v.p[0],ssize);
				else {
        	make_legal(ap2,F_DREG | F_VOL,4);
					do_extend(ap2,ssize,lsize,F_DREG);
				}
				dodiv(ap2,ap1,ssize,op,modflag);                  
				freeop(ap1);
				release();
				if (!equal_address(ap2,ap3))
					if (node->v.p[0]->nodetype == en_bits)
						bit_move(ap3,ap2,node->v.p[0],flags,ssize,rsize);
					else
						gen_code(op_mov,ssize,ap3,ap2);
				if (flags & F_NOVALUE)                              
					freeop(ap2);                                      
				else {
					do_extend(ap2,ssize,size,flags);
       		make_legal(ap2,flags,4);
				}
       	return ap2;                                       
}
AMODE *gen_moveblock(ENODE *node, int flags, int size)      
{                                                           
	AMODE *ap1, *ap2;                                         
	if (!node->size)                                          
		return(0);
	ap2 = gen_expr(node->v.p[1],F_DREG | F_VOL,4);                     
	mark();
	ap1 = gen_expr(node->v.p[0],F_DREG | F_VOL,4);                     
	gen_push(ESI,am_dreg,0);                                  
	gen_push(EDI,am_dreg,0);                                  
	if (regs[1])                                              
		gen_push(ECX,am_dreg,0);                                
	gen_code(op_mov,4,makedreg(ESI),ap2);		                  
	gen_code(op_mov,4,makedreg(EDI),ap1);                     
	gen_code(op_mov,4,makedreg(ECX),make_immed(node->size));  
	gen_code(op_cld,0,0,0);
	gen_code(op_rep,1,0,0);                                   
	gen_code(op_movsb,1,0,0);		                              
	if (regs[1])                                              
		gen_pop(ECX,am_dreg,0);
	gen_pop(EDI,am_dreg,0);
	gen_pop(ESI,am_dreg,0);
	freeop(ap2);
	freeop(ap1);
	release();
	return(ap2);
}
AMODE    *gen_assign(ENODE *node, int flags, int size)
/*
 *      generate code for an assignment node. if the size of the
 *      assignment destination is larger than the size passed then
 *      everything below this node will be evaluated with the
 *      assignment size.
 */
{       AMODE    *ap1, *ap2, *ap3,*ap4 = 0;
        int             ssize,rsize;
				rsize = natural_size(node->v.p[1]);
        switch( node->v.p[0]->nodetype )
                {
								case en_bits:
												ssize = natural_size(node->v.p[0]);
												break;
                case en_ub_ref:
								case en_cub:
                        ssize = 1;
                        break;
                case en_b_ref:
								case en_cb:
                        ssize = -1;
                        break;
                case en_uw_ref:
								case en_cuw:
                        ssize = 2;
                        break;
                case en_w_ref:
								case en_cw:
                        ssize = -2;
                        break;
                case en_l_ref:
								case en_cl:
												ssize = -4;
												break;
                case en_ul_ref:
								case en_cul:
								case en_cp:
                        ssize = 4;
                        break;
                case en_tempref:
                case en_regref:
												ssize = node->v.p[0]->v.i >> 8;
												break;
								case en_floatref:
								case en_cf:
												ssize = 6;
												break;
								case en_doubleref:
								case en_cd:
												ssize = 8;
												break;
								case en_longdoubleref:
								case en_cld:
												ssize = 10;
												break;
								default:
												ssize = -4;
                }
        if (chksize( ssize , rsize ))
                rsize = ssize;
        ap2 = gen_expr(node->v.p[1],F_DREG | F_FREG | F_IMMED,rsize);
				mark();
        ap1 = gen_expr(node->v.p[0],F_ALL | F_NOBIT | F_DEST,ssize);
				if (!equal_address(ap1,ap2)	) {
					if (rsize > 4) {
						floatstore(ap1,ssize,flags);
						gen_codef(op_fwait,0,0,0);
					}
					else {
						if (node->v.p[0]->nodetype == en_bits) {
							bit_move(ap1,ap2,node->v.p[0],flags,ssize,rsize);
						}
						else {
							if (ap2->mode != am_dreg && ap2->mode != am_immed
										&& ap1->mode != am_dreg) {
								freeop(ap2);
								ap3 = temp_data();
								gen_code(op_mov,ssize,ap3,ap2);
								gen_code(op_mov,ssize,ap1,ap3);
							}
							else {
  	      			gen_code(op_mov,ssize,ap1,ap2);
							}
						}
					}
				}
				release();
				if (flags & F_NOVALUE)
					freeop(ap2);
				else {
					do_extend(ap2,ssize,size,flags);
					make_legal(ap2,flags,size);
				}
        return ap2;
}
AMODE    *gen_refassign(ENODE *node, int flags, int size)
/*
 *      generate code for an assignment node. if the size of the
 *      assignment destination is larger than the size passed then
 *      everything below this node will be evaluated with the
 *      assignment size.
 */
{       AMODE    *ap1, *ap2, *ap3,*ap4;
        int             ssize,rsize;
				rsize = natural_size(node->v.p[1]);
        switch( node->v.p[0]->nodetype )
                {
								case en_bits:
												ssize = natural_size(node->v.p[0]);
												break;
                case en_ub_ref:
								case en_cub:
                        ssize = 1;
                        break;
                case en_b_ref:
								case en_cb:
                        ssize = -1;
                        break;
                case en_uw_ref:
								case en_cuw:
                        ssize = 2;
                        break;
                case en_w_ref:
								case en_cw:
                        ssize = -2;
                        break;
                case en_l_ref:
								case en_cl:
												ssize = -4;
												break;
                case en_ul_ref:
								case en_cul:
								case en_cp:
                        ssize = 4;
                        break;
                case en_tempref:
                case en_regref:
												ssize = node->v.p[0]->v.i >> 8;
												break;
								case en_floatref:
								case en_cf:
												ssize = 6;
												break;
								case en_doubleref:
								case en_cd:
												ssize = 8;
												break;
								case en_longdoubleref:
								case en_cld:
												ssize = 10;
												break;
								default:
												ssize = -4;
                }
        if (chksize( ssize , rsize ))
                rsize = ssize;
        ap2 = gen_expr(node->v.p[1],F_DREG | F_FREG,rsize);
				mark();
        ap1 = gen_expr(node->v.p[0],F_ALL | F_NOBIT | F_DEST,ssize);
				ap4 = xalloc(sizeof(AMODE));
				ap4->preg = ap1->preg;
				ap4->mode = am_indisp;
				ap4->offset = makenode(en_icon,0,0);
				if (!equal_address(ap1,ap2)	)
					if (rsize > 4) {
						floatstore(ap4,ssize,flags);
						gen_codef(op_fwait,0,0,0);
					}
					else
						if (node->v.p[0]->nodetype == en_bits)
							bit_move(ap4,ap2,node->v.p[0],flags,ssize,rsize);
						else
							if (ap2->mode != am_dreg && ap2->mode != am_immed
										&& ap4->mode != am_dreg) {
								ap3 = temp_data();
  	  	    		gen_code(op_mov,rsize,ap3,ap2);
    	  	  		gen_code(op_mov,ssize,ap4,ap3);
								freeop(ap3);
							}
							else
  	      			gen_code(op_mov,ssize,ap4,ap2);
        freeop(ap1);
				do_extend(ap2,ssize,size,flags);
				make_legal(ap2,flags,size);
        return ap2;
}

AMODE    *gen_aincdec(ENODE *node, int flags, int size, int op)
/*
 *      generate an auto increment or decrement node. op should be
 *      either op_add (for increment) or op_sub (for decrement).
 */
{       AMODE    *ap1,*ap2;
        int             ssize,rsize;
        ssize = natural_size(node->v.p[0]);
				if (!(flags & F_NOVALUE)) {
					ap2 = temp_data();
				}
				mark();
        ap1 = gen_expr(node->v.p[0],F_ALL,ssize);
				if (!(flags &F_NOVALUE)) {
					gen_code(op_mov,ssize,ap2,ap1);
				}
        gen_code(op,ssize,ap1,make_immed((int)node->v.p[1]));
				freeop(ap1);
				release();
				if (!(flags & F_NOVALUE)) {
					do_extend(ap2,ssize,size,flags);
					make_legal(ap2,flags,size);
				}
        return ap2;
}

int push_param(ENODE *ep, int size)
/*
 *      push the operand expression onto the stack.
 */
{       AMODE    *ap, *ap2;
				int rv,sz;
				switch (ep->nodetype) {
								case en_absacon:
												ep->v.i = (( SYM *)ep->v.p[0])->value.i;
                        ap = xalloc(sizeof(AMODE));
                        ap->mode = am_immed;
                        ap->offset = ep;     /* use as constant node */
                        gen_code(op_push,4,ap,0);
												rv = 4;
												break;
                case en_napccon:
                case en_nacon:
												ep->v.p[0] = ((SYM *)ep->v.p[0])->name;
                case en_labcon:
                case en_nalabcon:
                        ap = xalloc(sizeof(AMODE));
                        ap->mode = am_immed;
                        ap->offset = ep;     /* use as constant node */
                        gen_code(op_push,4,ap,0);
                        make_legal(ap,F_ALL,4);
												rv = 4;
												break;
								case en_cf:
								case en_floatref:
												ap = gen_expr(ep,F_FREG | F_VOL,6);
												gen_code(op_sub,4,makedreg(ESP),make_immed(4));
												ap2 = make_immed(0);
												ap2->preg = ESP;
												ap2->mode = am_indisp;
												gen_codef(op_fstp,6,ap2,0);
												gen_codef(op_fwait,0,0,0);
												rv = 4;
												break;
								case en_cd:
								case en_doubleref:
												ap = gen_expr(ep,F_FREG | F_VOL,8);
												gen_code(op_sub,4,makedreg(ESP),make_immed(8));
												ap2 = make_immed(0);
												ap2->preg = ESP;
												ap2->mode = am_indisp;
												gen_codef(op_fstp,8,ap2,0);
												gen_codef(op_fwait,0,0,0);
												rv = 8;
												break;
								case en_cld:
								case en_longdoubleref:
												ap = gen_expr(ep,F_FREG | F_VOL,8);
												gen_code(op_sub,4,makedreg(ESP),make_immed(10));
												ap2 = make_immed(0);
												ap2->preg = ESP;
												ap2->mode = am_indisp;
												gen_codef(op_fstp,10,ap2,0);
												gen_codef(op_fwait,0,0,0);
												rv = 8;
												break;
								case en_rcon: 
								case en_fcon:
								case en_lrcon:
												rv = size;
												if (rv == 6) rv= 4;
												if (rv == 10) rv= 12;
												ap = gen_expr(ep,F_FREG | F_VOL,size);
												make_floatconst(ap,size);
												gen_code(op_sub,4,makedreg(ESP),make_immed(rv));
												ap2 = make_immed(0);
												ap2->preg = ESP;
												ap2->mode = am_indisp;
												gen_codef(op_fstp,rv,ap2,0);
												gen_codef(op_fwait,0,0,0);
												break;
								default:
												rv = 4;
      			  					ap = gen_expr(ep,F_ALL,4);
												if (ap->mode != am_dreg &&ap->mode != am_immed&& ap->mode != am_freg) {
													sz = natural_size(ep->v.p[0]);
													if (sz < 4 && sz >-4) 
														do_extend(ap,sz,4,F_DREG);
												}
												if (ap->mode == am_freg) {
													gen_code(op_push,4,makedreg(3),0);
													ap2 = make_immed(0);
													ap2->preg = ESP;
													ap2->mode = am_indisp;
													gen_codef(op_fistp,4,ap2,0);
													gen_codef(op_fwait,0,0,0);
												}
												else
													gen_code(op_push,4,ap,0);
												break;
				}
        freeop(ap);
	stackdepth += rv;
	return(rv);
}
int push_stackblock(ENODE *ep)
{
	AMODE *ap;
	int sz = (ep->size + stackadd) &stackmod;
	if (!sz)
		return(0);
	gen_code(op_sub,4,makedreg(ESP),make_immed(sz));
	gen_code(op_push,4,makedreg(ESI),0);
	gen_code(op_push,4,makedreg(EDI),0);
	stackdepth+=sz+8;
	gen_code(op_lea,4,makedreg(EDI),make_stack(-8));
				switch (ep->nodetype) {
                case en_napccon:
                case en_nacon:
												ep->v.p[0] = ((SYM *)ep->v.p[0])->name;
                case en_nalabcon:
                case en_labcon:
                        ap = xalloc(sizeof(AMODE));
                        ap->mode = am_direct;
                        ap->offset = ep;     /* use as constant node */
                        gen_code(op_lea,4,makedreg(ESI),ap);
												break;
								case en_absacon:
												ep->v.i = (( SYM *)ep->v.p[0])->value.i;
                        ap = xalloc(sizeof(AMODE));
                        ap->mode = am_direct;
                        ap->offset = ep;     /* use as constant node */
                        gen_code(op_lea,4,makedreg(ESI),ap);
												break;
								default:
      			  					ap = gen_expr(ep,F_DREG | F_MEM | F_IMMED,4);
												gen_code(op_mov,4,makedreg(ESI),ap);
												break;
				}
	gen_code(op_mov,4,makedreg(ECX),make_immed(sz));
	gen_code(op_cld,0,0,0);
	gen_code(op_rep,0,0,0);
	gen_code(op_movsb,0,0,0);
	gen_code(op_pop,4,makedreg(EDI),0);
	gen_code(op_pop,4,makedreg(ESI),0);
	stackdepth-=8;
	freeop(ap);
	return(sz);
}

int     gen_parms(ENODE *plist,int size)
/*
 *      push a list of parameters onto the stack and return the
 *      size of parameters pushed.
 */
{       int     i;
        i = 0;
        while( plist != 0 )
                {         	
								if (plist->nodetype == en_stackblock)
									i+=push_stackblock(plist->v.p[0]);
								else
                	i+=push_param(plist->v.p[0],size);
                plist = plist->v.p[1];
                }
        return i;
}

AMODE    *gen_fcall(ENODE *node,int flags, int size)
/*
 *      generate a function call node and return the address mode
 *      of the result.
 */
{       AMODE    *ap, *result;
				ENODE *node2;
        int             i,ssize;
        result = temp_data();
        temp_data(); temp_data();       /* push any used data registers */
        freeop(result); freeop(result); freeop(result);
				if (node->nodetype == en_callblock) {
					i = gen_parms(node->v.p[1]->v.p[1]->v.p[1]->v.p[0],size);
      		ap = gen_expr(node->v.p[0],F_ALL,4);
					gen_code(op_push,4,ap,0);
					i+=4;
					stackdepth+=4;
					node = node->v.p[1];
					freeop(ap);
					ssize = 4;
				}
				else {
	        i = gen_parms(node->v.p[1]->v.p[1]->v.p[0],size);    /* generate parameters */
					ssize = node->v.p[0]->v.i;
				}
				if (node->nodetype == en_intcall) {
					AMODE *ap2 = xalloc(sizeof(AMODE));
					ap2->mode = am_seg;
					ap2->seg = e_cs;
					gen_code(op_pushfd,0,0,0);
					gen_code(op_push,0,ap2,0);
				}
									
   	    if( node->v.p[1]->v.p[0]->nodetype == en_nacon || node->v.p[1]->v.p[0]->nodetype == en_napccon ) {
								SYM *sp= node->v.p[1]->v.p[0]->v.p[0];
								if (sp->inreg) {
									ap = makedreg(sp->value.i);
								}
								else {
									node->v.p[1]->v.p[0]->v.p[0] = sp->name;
     	          	ap = make_offset(node->v.p[1]->v.p[0]);
									ap->mode = am_immed;
								}
         	      gen_code(op_call,0,ap,0);
				}
 	      else
   	            {
								if (node->v.p[1]->v.p[0]->nodetype == en_l_ref) {
									node2=node->v.p[1]->v.p[0]->v.p[0];
								}
								else {
									if (node->v.p[1]->v.p[0]->nodetype != en_tempref)
										DIAG("gen_fcall - questionable indirection");
									node2=node->v.p[1]->v.p[0];
								}
     	          ap = gen_expr(node2,F_ALL,4);
       	        freeop(ap);
         	      gen_code(op_call,4,ap,0);
                }
        if( i != 0 ) {
								if (i == 4)
									gen_code(op_pop,4,makedreg(ECX),0);
								else
                	gen_code(op_add,4,makedreg(4),make_immed(i));
								stackdepth -= i;
				}
				if (ssize > 4) {
						result = fstack();
				}
				else {
          result = temp_data();
        	if( result->preg != EAX)
                gen_code(op_mov,4,result,makedreg(EAX));
				}
				result->tempflag = 1;
				do_extend(result,ssize,size,flags);
				make_legal(result,flags,size);
        return result;
}
AMODE    *gen_pfcall(ENODE *node,int flags, int size)
/*
 *      generate a function call node to a pascal function
 *			and return the address mode of the result.
 */
{       AMODE    *ap, *result;
        int             i,ssize;
				ENODE * invnode = 0,*anode,*node2;
				
				/* invert the parameter list */
				if (node->nodetype == en_pcallblock)
					anode = node->v.p[1]->v.p[1]->v.p[1]->v.p[0];
				else
					anode = node->v.p[1]->v.p[1]->v.p[0];
				while (anode) {
					invnode = makenode(anode->nodetype,anode->v.p[0],invnode);
					anode = anode->v.p[1];
				}
        result = temp_data();
        temp_data(); temp_data();       /* push any used data registers */
        freeop(result); freeop(result); freeop(result);
				if (node->nodetype == en_pcallblock) {
      		ap = gen_expr(node->v.p[0],F_ALL,4);
					gen_code(op_push,4,ap,0);
					freeop(ap);
					i=4;
					stackdepth+=4;
					i += gen_parms(invnode,size);
					node = node->v.p[1];
					ssize = 4;
				}
				else {
	        i = gen_parms(invnode,size);    /* generate parameters */
					ssize = node->v.p[0]->v.i;
				}
									
   	    if( node->v.p[1]->v.p[0]->nodetype == en_nacon || node->v.p[1]->v.p[0]->nodetype == en_napccon ) {
								SYM *sp= node->v.p[1]->v.p[0]->v.p[0];
								if (sp->inreg) {
									ap = makedreg(sp->value.i);
								}
								else {
									node->v.p[0]->v.p[0]->v.p[0] = sp->name;
     	          	ap = make_offset(node->v.p[1]->v.p[0]);
									ap->mode = am_immed;
								}
         	      gen_code(op_call,0,ap,0);
				}
 	      else
   	            {
								if (node->v.p[1]->v.p[0]->nodetype == en_l_ref) {
									node2=node->v.p[1]->v.p[0]->v.p[0];
								}
								else {
									if (node->v.p[1]->v.p[0]->nodetype != en_tempref)
										DIAG("gen_fcall - questionable indirection");
									node2=node->v.p[1]->v.p[0];
								}
     	          ap = gen_expr(node2,F_ALL,4);
       	        freeop(ap);
         	      gen_code(op_call,4,ap,0);
                }
				stackdepth -= i;
				if (ssize > 4) {
						result = fstack();
				}
				else {
          result = temp_data();
        	if( result->preg != EAX)
                gen_code(op_mov,4,result,makedreg(EAX));
				}
				result->tempflag = 1;
				do_extend(result,ssize,size,flags);
				make_legal(result,flags,size);
        return result;
}

AMODE    *gen_expr(ENODE *node, int flags, int size)
/*
 *      general expression evaluation. returns the addressing mode
 *      of the result.
 */
{
				AMODE    *ap1, *ap2;
        int             lab0, lab1;
        int             natsize;
        if( node == 0 )
                {
                DIAG("null node in gen_expr.");
                return 0;
                }
        switch( node->nodetype )
                {
								case en_bits:
												size = natural_size(node->v.p[0]);
												ap1 = gen_expr(node->v.p[0],F_ALL,size);
												if (!(flags & F_NOBIT))
													bit_legal(ap1,node,size);
												return ap1;
								case en_cb: 
								case en_cub:
								case en_cw: 
								case en_cuw:
								case en_cl: 
								case en_cul:
								case en_cf: 
								case en_cd: 
								case en_cld: 
								case en_cp:
												ap1 = gen_expr(node->v.p[0],flags | F_MEM | F_DREG | F_FREG,natural_size(node->v.p[0]));
                				do_extend(ap1,natural_size(node->v.p[0]),size,flags);
												make_legal(ap1,flags,size);
												return ap1;
                case en_napccon:
                case en_nacon:
												node->v.p[0] = ((SYM *)node->v.p[0])->name;
                case en_nalabcon:
                case en_labcon:
                        ap1 = temp_data();
												ap1->tempflag = TRUE;
                        ap2 = xalloc(sizeof(AMODE));
                        ap2->mode = am_direct;
                        ap2->offset = node;     /* use as constant node */
                        gen_code(op_lea,4,ap1,ap2);
                        make_legal(ap1,flags,size);
                        return ap1;             /* return reg */
                case en_icon:
								case en_lcon: case en_lucon: case en_iucon: case en_ccon:
								case en_rcon: case en_lrcon: case en_fcon:
                        ap1 = xalloc(sizeof(AMODE));
                        ap1->mode = am_immed;
                        ap1->offset = node;
                        make_legal(ap1,flags,size);
                        return ap1;
								case en_absacon:
												node->v.i = ((SYM *)node->v.p[0])->value.i;
                        ap1 = temp_data();
												ap1->tempflag = TRUE;
                        ap2 = xalloc(sizeof(AMODE));
                        ap2->mode = am_direct;
                        ap2->offset = node;     /* use as constant node */
                        gen_code(op_lea,4,ap1,ap2);
                        make_legal(ap1,flags,size);
                        return ap1;             /* return reg */
                case en_autocon:
                case en_autoreg:
                        ap1 = temp_data();
												ap1->tempflag = TRUE;
                        ap2 = xalloc(sizeof(AMODE));
				                ap2->mode = am_indisp;
												ap2->preg = ESP;
												if (prm_linkreg) {
			  				        	ap2->preg = EBP;
      			    					ap2->offset = makenode(en_icon,(char *)((SYM *)node->v.p[0])->value.i,0);
												}
												else if (((SYM *)node->v.p[0])->funcparm)
                					ap2->offset = makenode(en_icon,(char *)(((SYM *)node->v.p[0])->value.i+framedepth+stackdepth),0);
                				else
													ap2->offset = makenode(en_icon,(char *)(((SYM *)node->v.p[0])->value.i+stackdepth+lc_maxauto),0);
                        gen_code(op_lea,4,ap1,ap2);
                        make_legal(ap1,flags,size);
                        return ap1;             /* return reg */
                case en_b_ref:
                case en_w_ref:
                case en_ub_ref:
                case en_uw_ref:
                case en_l_ref:
                case en_ul_ref:
								case en_floatref:
								case en_doubleref:
								case en_longdoubleref:
                        return gen_deref(node,flags,size);
                case en_tempref:
                case en_regref:
                        ap1 = xalloc(sizeof(AMODE));
												ap1->tempflag = 0;
                        if( (node->v.i & 0xff) < 16 )
                                {
                                ap1->mode = am_dreg;
                                ap1->preg = node->v.i & 0xff;
																ap1->tempflag = 1;
                                }
                        else
													if ((node->v.i &0xff) < 32)
                                {
                                ap1->mode = am_dreg;
                                ap1->preg = (node->v.i & 0xff)- 12;
                                }
													else
                                {
                                ap1->mode = am_freg;
                                ap1->preg = (node->v.i & 0xff) - 32;
                                }
                        ap1->tempflag = 0;      /* not a temporary */
                				do_extend(ap1,node->v.i >> 8,size,flags);
                        make_legal(ap1,flags,size);
                        return ap1;
                case en_uminus:
                        return gen_unary(node,flags,size,op_neg, op_fchs);
                case en_compl:
                        return gen_unary(node,flags,size,op_not, op_not);
                case en_add:
                        return gen_binary(node,flags,size,op_add,op_fadd);
                case en_sub:
                        return gen_binary(node,flags,size,op_sub,op_fsub);
                case en_and:
                        return gen_binary(node,flags,size,op_and,op_and);
                case en_or:
                        return gen_binary(node,flags,size,op_or,op_or);
				case en_xor:
						return gen_xbin(node,flags,size,op_xor,op_xor);
								case en_pmul:
												return gen_pmul(node,flags,size);
								case en_pdiv:
												return gen_pdiv(node,flags,size);
                case en_mul:
                        return gen_mul(node,flags,size,op_imul);
                case en_umul:
                        return gen_mul(node,flags,size,op_mul);
                case en_div:
                        return gen_modiv(node,flags,size,op_idiv,0);
                case en_udiv:
                        return gen_modiv(node,flags,size,op_div,0);
                case en_mod:
                        return gen_modiv(node,flags,size,op_idiv,1);
                case en_umod:
                        return gen_modiv(node,flags,size,op_div,1);
                case en_alsh:
                        return gen_shift(node,flags,size,op_sal);
                case en_arsh:
                        return gen_shift(node,flags,size,op_sar);
                case en_lsh:
                        return gen_shift(node,flags,size,op_shl);
                case en_rsh:
                        return gen_shift(node,flags,size,op_shr);
                case en_asadd:
                        return gen_asadd(node,flags,size,op_add,op_fadd);
                case en_assub:
                        return gen_asadd(node,flags,size,op_sub,op_fsub);
                case en_asand:
                        return gen_aslogic(node,flags,size,op_and);
                case en_asor:
                        return gen_aslogic(node,flags,size,op_or);
                case en_asxor:
                        return gen_aslogic(node,flags,size,op_xor);
                case en_aslsh:
                        return gen_asshift(node,flags,size,op_shl);
                case en_asrsh:
                        return gen_asshift(node,flags,size,op_shr);
                case en_asalsh:
                        return gen_asshift(node,flags,size,op_sal);
                case en_asarsh:
                        return gen_asshift(node,flags,size,op_sar);
                case en_asmul:
                        return gen_asmul(node,flags,size, op_imul);
                case en_asumul:
                        return gen_asmul(node,flags,size,op_mul);
                case en_asdiv:
                        return gen_asmodiv(node,flags,size,op_idiv,FALSE);
                case en_asudiv:
                        return gen_asmodiv(node,flags,size,op_div,FALSE);
                case en_asmod:
                        return gen_asmodiv(node,flags,size,op_idiv,TRUE);
                case en_asumod:
                        return gen_asmodiv(node,flags,size,op_div,TRUE);
                case en_assign:
                        return gen_assign(node,flags,size);
                case en_refassign:
                        return gen_refassign(node,flags|F_NOVALUE,size);
                case en_moveblock:
                        return gen_moveblock(node,flags,size);
                case en_ainc:
                        return gen_aincdec(node,flags,size,op_add);
                case en_adec:
                        return gen_aincdec(node,flags,size,op_sub);
                case en_land:   case en_lor:
                case en_eq:     case en_ne:
                case en_lt:     case en_le:
                case en_gt:     case en_ge:
                case en_ult:    case en_ule:
                case en_ugt:    case en_uge:
                case en_not:
                        lab0 = nextlabel++;
                        lab1 = nextlabel++;
                        falsejp(node,lab0);
                        ap1 = temp_data();
                        gen_code(op_sub,4,ap1,ap1);
                        gen_code(op_inc,4,ap1,0);
                        gen_code(op_jmp,0,make_label(lab1),0);
                        gen_label(lab0);
                        gen_code(op_sub,4,ap1,ap1);
                        gen_label(lab1);
                        return ap1;
                case en_cond:
                        return gen_hook(node,flags,size);
                case en_void:
                        natsize = natural_size(node->v.p[0]);
                        freeop(gen_expr(node->v.p[0],F_ALL | F_NOVALUE,natsize));
                        return gen_expr(node->v.p[1],flags,size);
								case en_pfcall: case en_pfcallb:  case en_pcallblock:
												return gen_pfcall(node,flags,size);
                case en_fcall:  case en_callblock: case en_fcallb:
                case en_trapcall:
								case en_intcall:
                        return gen_fcall(node,flags,size);
                default:
                        DIAG("uncoded node in gen_expr.");
                        return 0;
                }
}

int     natural_size(ENODE *node)
/*
 *      return the natural evaluation size of a node.
 */
{       int     siz0, ssize;
        if( node == 0 )
                return 0;
        switch( node->nodetype )
                {
								case en_absacon:
												return stdaddrsize;
								case en_bits:
												return 4;
                case en_icon:
								case en_lcon: 
								case en_lucon: 
								case en_iucon: 
												return 0;
								case en_ccon:                 
												return 0;
								case en_rcon:
												return 0;
								case en_doubleref:
												return 8;
								case en_lrcon:
												return 0;
								case en_longdoubleref:
												return 10;
								case en_fcon:
												return 0;
								case en_floatref:
												return 6;
								case en_trapcall:
                case en_labcon:
                case en_nacon:  case en_autocon:  case en_autoreg:
                case en_napccon: case en_nalabcon:
												return stdaddrsize;
								case en_l_ref:
								case en_cl:
												return -4;
								case en_pfcall: case en_pfcallb: 
								case en_fcall: case en_intcall: case en_callblock: case en_fcallb:
								case en_pcallblock:
												return natural_size(node->v.p[1]);
								case en_tempref:
								case en_regref:
												return node->v.i >> 8;
                case en_ul_ref:
								case en_cul:
                        return 4;
								case en_cp:
												return stdaddrsize;
                case en_ub_ref:
								case en_cub:
												return 1;
                case en_b_ref:
								case en_cb:
                        return -1;
                case en_uw_ref:
								case en_cuw:
												return 2;
                case en_cw:
                case en_w_ref:
                        return -2;
								case en_cd:
												return 8;
								case en_cld:
												return stdldoublesize;
								case en_cf:
												return 6;
                case en_not:    case en_compl:
                case en_uminus: case en_assign: case en_refassign:
                case en_ainc:   case en_adec:
								case en_moveblock: case en_stackblock:
                        return natural_size(node->v.p[0]);
                case en_add:    case en_sub:
								case en_umul:		case en_udiv:	case en_umod: case en_pmul:
                case en_mul:    case en_div:  case en_pdiv:
                case en_mod:    case en_and:
                case en_or:     case en_xor:
								case en_asalsh: case en_asarsh: case en_alsh: case en_arsh:
                case en_lsh:    case en_rsh:
                case en_eq:     case en_ne:
                case en_lt:     case en_le:
                case en_gt:     case en_ge:
								case en_ugt: case en_uge: case en_ult: case en_ule:
                case en_land:   case en_lor:
                case en_asadd:  case en_assub:
                case en_asmul:  case en_asdiv:
                case en_asmod:  case en_asand:
								case en_asumod: case en_asudiv: case en_asumul:
                case en_asor:   case en_aslsh:  case en_asxor:
                case en_asrsh:
                        siz0 = natural_size(node->v.p[0]);
                        ssize = natural_size(node->v.p[1]);
                        if( chksize(ssize, siz0 ))
                                return ssize;
                        else
                                return siz0;
                case en_void:   case en_cond:
                        return natural_size(node->v.p[1]);
                default:
                        DIAG("natural size error.");
                        break;
                }
        return 0;
}

void gen_compare(ENODE *node, int btype1, int btype2, int btype3, int btype4, int label)
/*
 *      generate code to do a comparison of the two operands of
 *      node.
 */
{       AMODE    *ap1, *ap2,  *ap3;
        int             size;
				int btype = btype1;
				ap3 = 0;
        size = natural_size(node);
				if (size > 4) {
					ap1 = gen_expr(node->v.p[0],F_FREG | F_VOL, size);
					ap2 = gen_expr(node->v.p[1],F_ALL, size);
					if (ap2->mode == am_freg) {
						gen_codef(op_fcompp,0,0,0);
					}
					else
						gen_codef(op_fcomp,size,ap2,0);
					if (regs[0])
						gen_push(EAX,am_dreg,0);
					gen_codef(op_fstsw,2,makedreg(EAX),0);
					gen_codef(op_fwait,0,0,0);
					gen_code(op_sahf,0,0,0);
					if (regs[0])
						gen_pop(EAX,am_dreg,0);
					gen_branch(btype3,0,make_label(label));
					return;

				}
        ap1 = gen_expr(node->v.p[0],F_ALL,size);
				mark();
        ap2 = gen_expr(node->v.p[1],F_ALL,size);
				if (ap1->mode != am_dreg) {
					if (ap2->mode == am_immed)  {
						if (ap1->mode == am_immed)
							goto cmp2;
					}
					else {
						if (ap2->mode == am_dreg) {
swapit:
							ap3 = ap2;
							ap2 = ap1;
							ap1 = ap3;
							ap3 = 0;
							btype = btype2;
						}
						else  
							if (ap1->mode == am_immed) {
								goto swapit;
							}
							else {
cmp2:
								ap3 = ap1;
  	            ap1 = temp_data();
								gen_code(op_mov,size,ap1,ap3);
      	        make_legal(ap1,F_DREG,size);
							}
					}
				}
				if (ap1->mode == am_dreg && ap2->mode == am_immed && ap2->offset->v.i == 0)
        	gen_code(op_or,size,ap1,ap1);
        else
					gen_code(op_cmp,size,ap1,ap2);
				release();
        gen_branch(btype,0,make_label(label));
				if (ap3) {
					freeop(ap1);
					freeop(ap2);
					freeop(ap3);
				}
				else {	
        	freeop(ap2);
        	freeop(ap1);
				}
}

void truejp(ENODE *node, int label)
/*
 *      generate a jump to label if the node passed evaluates to
 *      a true condition.
 */
{       AMODE    *ap1;
        int             ssize;
        int             lab0;
        if( node == 0 )
                return;
        switch( node->nodetype )
                {
                case en_eq:
                        gen_compare(node,op_je,op_je,op_je,op_je,label);
                        break;
                case en_ne:
                        gen_compare(node,op_jne,op_jne,op_jne,op_jne,label);
                        break;
                case en_lt:
                        gen_compare(node,op_jl,op_jg,op_jb,op_ja,label);
                        break;
                case en_le:
                        gen_compare(node,op_jle,op_jge,op_jbe,op_jnc,label);
                        break;
                case en_gt:
                        gen_compare(node,op_jg,op_jl,op_ja,op_jb,label);
                        break;
                case en_ge:
                        gen_compare(node,op_jge,op_jle,op_jnc,op_jbe,label);
                        break;
                case en_ult:
                        gen_compare(node,op_jb,op_ja,op_jb,op_ja,label);
                        break;
                case en_ule:
                        gen_compare(node,op_jbe,op_jnc,op_jbe,op_jnc,label);
                        break;
                case en_ugt:
                        gen_compare(node,op_ja,op_jb,op_ja,op_jb,label);
                        break;
                case en_uge:
                        gen_compare(node,op_jnc,op_jbe,op_jnc,op_jbe,label);
                        break;
                case en_land:
                        lab0 = nextlabel++;
                        falsejp(node->v.p[0],lab0);
                        truejp(node->v.p[1],label);
                        gen_label(lab0);
                        break;
                case en_lor:
                        truejp(node->v.p[0],label);
                        truejp(node->v.p[1],label);
                        break;
                case en_not:
                        falsejp(node->v.p[0],label);
                        break;
                default:
                        ssize = natural_size(node);    
												if (isintconst(node->nodetype)) {
													if (node->v.i != 0) 
                        		gen_code(op_jmp,0,make_label(label),0);
													break;
												}
												else
													if (ssize > 4) {
														ap1 = gen_expr(node,F_FREG | F_VOL, ssize);
													}
													else
                        		ap1 = gen_expr(node,F_ALL,ssize);
												if (ssize > 4) {
													gen_codef(op_fldz,0,0,0);
													gen_codef(op_fcompp,0,0,0);
													if (regs[0])
														gen_push(EAX,am_dreg,0);
													gen_codef(op_fstsw,2,makedreg(EAX),0);
													gen_codef(op_fwait,0,0,0);
													gen_code(op_sahf,0,0,0);
													if (regs[0])
														gen_pop(EAX,am_dreg,0);
												}
												else {
													if (ap1->mode == am_dreg) 
                        		gen_code(op_or,ssize,ap1,ap1);
													else
                        		gen_code(op_test,ssize,ap1,make_immedt(0xffffffffL,ssize));
                        	freeop(ap1);
												}
                        gen_branch(op_jne,0,make_label(label));
                        break;
                }
}

void falsejp(ENODE *node, int label)
/*
 *      generate code to execute a jump to label if the expression
 *      passed is false.
 */
{       AMODE    *ap1;
        int             ssize;
        int             lab0;
        if( node == 0 )
                return;
        switch( node->nodetype )
                {
                case en_eq:
                        gen_compare(node,op_jne,op_jne,op_jne,op_jne,label);
                        break;
                case en_ne:
                        gen_compare(node,op_je,op_je,op_je,op_je,label);
                        break;
                case en_lt:
                        gen_compare(node,op_jge,op_jle,op_jnc,op_jbe,label);
                        break;
                case en_le:
                        gen_compare(node,op_jg,op_jl,op_ja,op_jb,label);
                        break;
                case en_gt:
                        gen_compare(node,op_jle,op_jge,op_jbe,op_jnc,label);
                        break;
                case en_ge:
                        gen_compare(node,op_jl,op_jg,op_jb,op_ja,label);
                        break;
                case en_ult:
                        gen_compare(node,op_jnc,op_jbe,op_jnc,op_jbe,label);
                        break;
                case en_ule:
                        gen_compare(node,op_ja,op_jb,op_ja,op_jb,label);
                        break;
                case en_ugt:
                        gen_compare(node,op_jbe,op_jnc,op_jbe,op_jnc,label);
                        break;
                case en_uge:
                        gen_compare(node,op_jb,op_ja,op_jb,op_ja,label);
                        break;
                case en_land:
                        falsejp(node->v.p[0],label);
                        falsejp(node->v.p[1],label);
                        break;
                case en_lor:
                        lab0 = nextlabel++;
                        truejp(node->v.p[0],lab0);
                        falsejp(node->v.p[1],label);
                        gen_label(lab0);
                        break;
                case en_not:
                        truejp(node->v.p[0],label);
                        break;
                default:
                        ssize = natural_size(node);    
												if (isintconst(node->nodetype)) {
													if (node->v.i == 0) 
                        		gen_code(op_jmp,0,make_label(label),0);
													break;
												}
												else
													if (ssize > 4) {
														ap1 = gen_expr(node,F_FREG | F_VOL, ssize);
													}
													else
                        		ap1 = gen_expr(node,F_ALL,ssize);
												if (ssize > 4) {
													gen_codef(op_fldz,0,0,0);
													gen_codef(op_fcompp,0,0,0);
													if (regs[0])
														gen_push(EAX,am_dreg,0);
													gen_codef(op_fstsw,2,makedreg(EAX),0);
													gen_codef(op_fwait,0,0,0);
													gen_code(op_sahf,0,0,0);
													if (regs[0])
														gen_pop(EAX,am_dreg,0);
												}
												else {
													if (ap1->mode == am_dreg) 
                        		gen_code(op_or,ssize,ap1,ap1);
													else
                        		gen_code(op_test,ssize,ap1,make_immedt(0xffffffffL,ssize));
                        	freeop(ap1);
												}
                        gen_branch(op_je,0,make_label(label));
                        break;
                }
}