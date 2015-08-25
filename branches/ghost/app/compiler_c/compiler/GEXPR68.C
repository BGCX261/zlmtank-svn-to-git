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

/*
 *      this module contains all of the code generation routines
 *      for evaluating expressions and conditions.
 */
extern int stdinttype,stdunstype,stdintsize, stdldoublesize,stdaddrsize;
extern int cf_freeaddress, cf_freedata;
extern int linkreg,basereg;
extern long stackdepth,framedepth;
extern int prm_largedata, prm_68020,prm_phiform,prm_linkreg;
extern int prm_smallcode, prm_rel, prm_smalldata;
extern AMODE     push[], pop[];
extern int prm_68020;
extern SYM *currentfunc;
extern long lc_maxauto;
extern long nextlabel;
extern char regstack[], rsold[], rsodepth,rsdepth;

long bittab[32] = { 1,0x3,0x7,0xf,0x1f,0x3f,0x7f,0xff,
			0x1ff,0x3ff,0x7ff,0xfff,0x1fff,0x3fff,0x7fff,0xffff,
			0x1ffffL,0x3ffffL,0x7ffffL,0xfffffL,0x1fffffL,0x3fffffL,0x7fffffL,0xffffffL,
			0x1ffffffL,0x3ffffffL,0x7ffffffL,0xfffffffL,0x1fffffffL,0x3fffffffL,0x7fffffffL,0xffffffffL
};

int chksize(int lsize, int rsize)
{
	int l,r;
	l = lsize;
	r = rsize;
	if (l < 0) l = - l;
	if (r < 0) r = - r;
	return(l > r);
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
        ap->mode = am_direct;
        ap->offset = lnode;
        return ap;
}
AMODE    *makebf(ENODE *node, AMODE *ap1, int size)
/*
 *      construct a bit field reference for 68020 bit field instructions
 */
{
        AMODE    *ap;
				if (node->startbit == -1)
					DIAG("Illegal bit field");
        ap = xalloc(sizeof(AMODE));
        ap->mode = am_bf;
				ap->preg = node->startbit;
				ap->sreg = node->bits;
				switch (size) {
					case 1:
					case -1:
				 		ap->preg =8-node->startbit-node->bits;
						break;
					case 2:
					case -2:
				 		ap->preg =16-node->startbit-node->bits;
						break;
					case 4:
					case -4:
				 		ap->preg =32-node->startbit-node->bits;
						break;
				}
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
        
void tofloat(AMODE *ap,int size)
{
	AMODE *ap2;
								freeop(ap);
								ap2 = temp_float();
                gen_codef(op_fmove,size,ap,ap2);
                ap->mode = am_freg;
                ap->preg = ap2->preg;
                ap->tempflag = 1;
}
void make_legal(AMODE *ap,int flags,int size)
/*
 *      make_legal will coerce the addressing mode in ap1 into a
 *      mode that is satisfactory for the flag word.
 */
{       AMODE    *ap2;
        if( ((flags & F_VOL) == 0) || ap->tempflag )
                {
                switch( ap->mode )
                        {
												case am_freg:
																if (flags & F_FREG)
																		return;
																break;
                        case am_immed:
                                if( flags & F_IMMED )
                                        return;         /* mode ok */
                                break;
                        case am_areg:
                                if( flags & F_AREG )
                                        return;
                                break;
                        case am_dreg:
                                if( flags & F_DREG )
                                        return;
                                break;
												case am_indx:
												case am_ind:
                        case am_baseindxdata: 
                        case am_baseindxaddr:
												case am_adirect:
												case am_ainc: case am_adec:
																if (flags & F_INDX)
																				return;
												case am_direct:
                        case am_pcindx:
                                if( flags & F_INDX )
                                        return;
                                break;
                        }
                }
				if (size > 4) {
						tofloat(ap,size);
						return;
                }
        if( flags & F_DREG )
                {
								if (ap->mode == am_dreg && ap->tempflag)
									return;
                freeop(ap);             /* maybe we can use it... */
                ap2 = temp_data();      /* allocate to dreg */
                gen_code(op_move,size,ap,ap2);
                ap->mode = am_dreg;
                ap->preg = ap2->preg;
                ap->tempflag = 1;
                return;
                }
        if( size == -1 )
                {
                freeop(ap);
                ap2 = temp_data();
                gen_code(op_move,1,ap,ap2);
                gen_code(op_ext,2,ap2,0);
                ap->mode = ap2->mode;
                ap->preg = ap2->preg;
                size = -2;
                }
        if( size == 1 )
                {
                freeop(ap);
                ap2 = temp_data();
                gen_code(op_move,1,ap,ap2);
                gen_code(op_and,2,make_immed(0xff),ap2);
                ap->mode = ap2->mode;
                ap->preg = ap2->preg;
                size = 2;
                }
        	freeop(ap);
        	ap2 = temp_addr();
        	gen_code(op_move,size,ap,ap2);
        	ap->mode = am_areg;
        	ap->preg = ap2->preg;
        	ap->tempflag = 1;
}
void doshift(int op, AMODE *ap2, AMODE *ap1, int size)
{
				if (ap2->mode == am_immed) {
					int temp = ap2->offset->v.i;
					while (temp >8) {
						temp = temp-8;
        		gen_code(op,size,make_immed(8),ap1);
					}
					if (temp != 0) {
						ap2->offset->v.i = temp;
        		gen_code(op,size,ap2,ap1);
					}
				}
				else
        	gen_code(op,size,ap2,ap1);
}
void bit_legal(AMODE *ap,ENODE *node, int size)
{
	if (node->bits != -1) {
		if (prm_68020) {
			AMODE *ap1;
			if (ap->mode != am_dreg || !ap->tempflag) {
				ap1 = temp_data();
				ap1->tempflag = 1;
			}
			else
				ap1 = copy_addr(ap);
			gen_code3(op_bfextu,0,ap,makebf(node,ap1,size),ap1);	
			ap->mode = ap1->mode;
			ap->preg = ap1->preg;
			ap->tempflag = ap1->tempflag;
		}
		else {
			make_legal(ap,F_DREG | F_VOL,size);
			if (node->startbit)
				doshift(op_asr,make_immed(node->startbit),ap,size);
				gen_code(op_asr,size,make_immed(node->startbit),ap);
			gen_code(op_andi,4,make_immed(bittab[node->bits-1]),ap);
		}
	}
}
AMODE *get_bitval(AMODE *ap,ENODE *node, int size)
{
	AMODE *ap1 = temp_data();
	ap1->tempflag = TRUE;
	if (prm_68020) {
		gen_code3(op_bfextu,0,ap,makebf(node,ap,size),ap1);
		return ap1;
	}
	else {
		gen_code(op_move,size,ap,ap1);
		if (node->startbit)
			doshift(op_asr,make_immed(node->startbit),ap1,size);
		gen_code(op_andi,4,make_immed(bittab[node->bits-1]),ap1);
		return ap1;
	}
}
void bit_move(AMODE *ap1, AMODE *ap2, ENODE *node, int flags, int sizein, int sizeout)
{
	if (prm_68020) {
		make_legal(ap1,F_DREG,sizein);
		gen_code3(op_bfins,0,ap1,ap2,makebf(node,ap2,sizeout));
	}
	else {
		gen_code(op_andi,sizeout,make_immed(~(bittab[node->bits-1]<<node->startbit)), ap2);
		if (ap1->mode == am_immed) {
			ap1->offset->v.i &= bittab[node->bits-1];
			if (ap1->offset->v.i) {
				ap1->offset->v.i <<= node->startbit;
				gen_code(op_ori,sizeout,ap1,ap2);
				if (!(flags & F_NOVALUE)) {
					make_legal(ap2,flags,sizeout);
					ap1->offset->v.i >>= node->startbit;
					gen_code(op_move,sizeout,ap1,ap2);
				}
			}
		}
		else {
			make_legal(ap1,F_DREG | F_VOL,sizein);
			gen_code(op_andi,sizein,make_immed(bittab[node->bits-1]), ap1);
			if (node->startbit)
				doshift(op_asl,make_immed(node->startbit),ap1,sizein);
			gen_code(op_or,sizeout,ap2,ap1);
			if (!(flags & F_NOVALUE)) {
				if (node->startbit)
					doshift(op_asr,make_immed(node->startbit),ap1,sizein);
			}
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
			  if (osize == 0 || isize == 0)
					return;
       if( isize == osize || isize == -osize)
                return;
				if (chksize(osize,isize) && osize <=4)
					if (isize ^ osize < 0)
						osize = -osize;
        if( ap->mode != am_areg && ap->mode != am_dreg && isize <= 4) {
								if (flags&F_DREG) {
									freeop(ap);
									gen_code(op_move,isize,ap,ap2=temp_data());
									ap->mode = am_dreg;
									ap->preg = ap2->preg;
								}
								else if (isize ==1 || isize == -1) {
									freeop(ap);
									gen_code(op_move,isize,ap,ap2=temp_data());
									ap->mode = am_dreg;
									ap->preg = ap2->preg;
									freeop(ap);
									gen_code(op_ext,2,ap,0);
									gen_code(op_move,2,ap,ap2=temp_addr());
									ap->mode = am_areg;
									ap->preg = ap2->preg;
								}
								else {
									freeop(ap);
									gen_code(op_move,isize,ap,ap2=temp_addr());
									ap = ap2;
								}
				}
        if( ap->mode == am_areg )
                return;         /* extend is automagic */
					
				switch(isize)
								{
                case -1:
								case 1:
												if (osize < 0)
													if (osize <= -4 && prm_68020)
														gen_code(op_extb,4,ap,0);
													else
                        		gen_code(op_ext,2,ap,0);
												else {
													int t = osize;
													if (osize > 4) t = 4;
													gen_code(op_andi,t,make_immed(0xff),ap);
												}
                case -2:
								case 2:
													if (osize == 1 || osize == -1)
														break;
                        	if( osize <-2 && !(prm_68020 && (isize == -1 || isize == 1))) {
                          	gen_code(op_ext,4,ap,0);
													}
													else
														if (isize == 2 || isize == -2) {
															gen_code(op_andi,4,make_immed(0xffff),ap);
														}
								case 4:
								case -4:
												if  (osize <=4)
													break;
												isize = -4;
								case 6:
								case 8: 
												if (osize > 4) {
													if (ap->mode != am_freg)
														tofloat(ap,isize);
												}
												else {
													freeop(ap);
													ap2 = temp_data();
													gen_codef(op_fmove,osize,ap,ap2);
													ap->mode = am_dreg;
													ap->preg = ap2->preg;
												}
												break;
				}
}

int     isshort(ENODE *node)
/*
 *      return true if the node passed can be generated as a short
 *      offset.
 */
{       return (isintconst(node->nodetype) || node->nodetype == en_absacon)&&
                (node->v.i >= -32768L && node->v.i <= 32767L);
}

int     isbyte(ENODE *node)
/*
 *      return true if the node passed can be evaluated as a byte
 *      offset.
 */
{       return isintconst(node->nodetype) &&
                (-128 <= node->v.i && node->v.i <= 127);
}

int isamshort(AMODE *ap)
{
	long v;
	if (ap->offset->nodetype != en_icon)
		return TRUE;
	v = ap->offset->v.i;
	return (v >=-32768L && v < 32767);
}
int isamshort2(AMODE *ap, AMODE *ap2)
{
	long v;
	if (ap->offset->nodetype != en_icon || ap2->offset->nodetype != en_icon)
		return TRUE;
	v = ap->offset->v.i + ap2->offset->v.i;
	return (v >=-32768L && v < 32767);
}
int isambyte(AMODE *ap)
{
	long v;
	if (ap->offset->nodetype != en_icon)
		return FALSE;
	v = ap->offset->v.i;
	return (v >=-128 && v < 128);
}
int isambyte2(AMODE *ap, AMODE *ap2)
{
	long v;
	if (ap->offset->nodetype != en_icon || ap2->offset->nodetype != en_icon)
		return FALSE;
	v = ap->offset->v.i + ap2->offset->v.i;
	return (v >=-128 && v < 128);
}                            

AMODE    *gen_index(int siz1,ENODE *node)
/*
 *      generate code to evaluate an index node (^+) and return
 *      the addressing mode of the result. This routine takes no
 *      flags since it always returns either am_ind or am_indx.
 */
{       AMODE    *ap1,*ap2, *ap3, *ap;
				ENODE node2;
				int scale;

				switch (node->v.p[0]->nodetype) {
					case en_icon:
						ap1 = gen_expr(node->v.p[0],F_IMMED,4);
						break;
					case en_lsh:
						if (prm_68020 && (scale = node->v.p[0]->v.p[1]->v.i) < 4 && scale) {
							ap1 = gen_expr(node->v.p[0]->v.p[0],F_IMMED | F_DREG | F_AREG,4);
							if (ap1->mode == am_immed) {
								while (--scale)
									ap1->offset->v.i <<=1;
							}
							else {
								if (ap1->mode == am_dreg)
									ap1->mode = am_baseindxdata;
								else
									ap1->mode = am_baseindxaddr;
								ap1->sreg = ap1->preg;
								ap1->preg = -1;
								ap1->scale = scale;
								ap1->offset = makenode(en_icon,0,0);
							}
							break;
						}
					default:
						mark();
						ap1 = gen_deref(node,F_INDX | F_DREG | F_AREG,4,FALSE);
						switch (ap1->mode) {
							case am_ainc:
							case am_adec:
								ap2 = temp_addr();
								gen_lea(siz1,ap1,ap2);
								freeop(ap1);
								ap1 = ap2;
							default:
								rsold[rsodepth-1] = rsdepth;
								break;
							case am_baseindxdata:
							case am_baseindxaddr:
								if (ap1->sreg >=0 && ap1->preg >= 0) {
									int t = rsold[rsodepth-1];
									freeop(ap1);
									ap3 = temp_addr();
									gen_lea(siz1,ap1,ap3);
									ap3->mode = am_ind;
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
						if (prm_68020 && (scale = node->v.p[1]->v.p[1]->v.i) < 4 && scale) {
							if (node->v.p[1]->v.p[0]->nodetype == en_tempref)
							ap2 = gen_expr(node->v.p[1]->v.p[0],F_IMMED | F_DREG | F_AREG,4);
							if (ap2->mode == am_immed) {
								while (--scale)
									ap2->offset->v.i <<=1;
							}
							else {
								if (ap2->mode == am_dreg)
									ap2->mode = am_baseindxdata;
								else
									ap2->mode = am_baseindxaddr;
								ap2->sreg = ap1->preg;
								ap2->preg = -1;
								ap2->scale = scale;
								ap2->offset = makenode(en_icon,0,0);
							}
							break;
						}
					default:      {
							int flag = ap1->mode == am_areg || ap1->mode == am_ind ||
										ap1->mode == am_indx || ap1->mode == am_baseindxaddr;
							node2.v.p[0] = node->v.p[1];
							node2.nodetype = node->nodetype;
							mark();
							ap2 = gen_deref(&node2,F_INDX | F_DREG | F_AREG,4,flag);
							switch (ap1->mode) {
								case am_ainc:
								case am_adec:
									ap3 = temp_addr();
									gen_lea(siz1,ap2,ap3);
									freeop(ap2);
									ap2 = ap3;
								default:
									rsold[rsodepth-1] = rsdepth;
									break;
								case am_baseindxdata:
								case am_baseindxaddr:
									if (ap2->sreg >=0 && ap2->preg >= 0) {
										int t = rsold[rsodepth-1];
										freeop(ap2);
										ap3 = temp_addr();
										gen_lea(siz1,ap2,ap3);
										ap3->mode = am_ind;
										if (t <rsdepth-1 && ap3->preg + 8 == regstack[t+1])
											t+=2;
										if (t <rsdepth && ap3->preg + 8 == regstack[t])
											t+=1;
										ap2 = ap3;
									}
							}
							release();
						}
						break;
				}
tryagain:
				switch(ap1->mode) {
					case am_areg:
						switch (ap2->mode) {
							case am_areg:
								ap1->sreg = ap2->preg;
								ap1->mode = am_baseindxaddr;
								ap1->offset = makenode(en_icon,0,0);
								ap1->scale = 0;
								return ap1;
							case am_dreg:
								ap1->sreg = ap2->preg;
								ap1->mode = am_baseindxdata;
								ap1->offset = makenode(en_icon,0,0);
								ap1->scale = 0;
								return ap1;
							case am_adirect:
								if ((!prm_largedata && (prm_rel || !prm_smalldata)) || prm_68020) {
									ap1->mode = am_indx;
									ap1->offset = ap2->offset;
									return ap1;
								}
								ap = temp_addr();
								gen_lea(0,ap2,ap);
								ap1->mode = am_baseindxaddr;
								ap1->offset = makenode(en_icon,0,0);
								ap1->scale = 0;
								ap1->sreg = ap->preg;
								return ap1;
							case am_immed:
								if ((!prm_largedata && (prm_rel || !prm_smalldata)) || prm_68020) {
									ap1->mode = am_indx;
									ap1->offset = ap2->offset;
									return ap1;
								}
								ap = temp_data();
								gen_code(op_move,4,ap2,ap);
								ap1->mode = am_baseindxdata;
								ap1->offset = makenode(en_icon,0,0);
								ap1->scale = 0;
								ap1->sreg = ap->preg;
								return ap1;
							case am_indx:
								if (prm_68020 || isambyte(ap2)) {
									ap1->mode = am_baseindxaddr;
									ap1->offset = ap2->offset;
									ap1->scale = 0;
									ap1->sreg = ap2->preg;
									return ap1;
								}
								freeop(ap2);
								ap = temp_addr();
								gen_lea(0,ap2,ap);
								ap->mode = am_baseindxaddr;
								ap->sreg = ap1->preg;
								ap->scale = 0;
								ap->offset = makenode(en_icon,0,0);
								return ap;
							case am_ind:
								ap1->mode = am_baseindxaddr;
								ap1->offset = makenode(en_icon,0,0);
								ap1->scale = 0;
								ap1->sreg = ap2->preg;
								return ap1;
							case am_baseindxaddr:
							case am_baseindxdata:
								if (ap2->preg == -1) {
									ap2->preg = ap1->preg;
									return ap2;
								}
								freeop(ap2);
								ap = temp_addr();
								gen_lea(0,ap2,ap);
								ap1->mode = am_baseindxaddr;
								ap1->scale = 0;
								ap1->offset = makenode(en_icon,0,0);
								ap1->sreg = ap->preg;
								return ap1;
						}
						break;
					case am_dreg:
						switch (ap2->mode) {
							case am_areg:
								ap2->sreg = ap1->preg;
								ap2->mode = am_baseindxdata;
								ap2->offset = makenode(en_icon,0,0);
								ap2->scale = 0;
								return ap2;
							case am_dreg:
								freeop(ap2);
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								ap->sreg = ap1->preg;
								ap->mode = am_baseindxdata;
								ap->offset = makenode(en_icon,0,0);
								ap->scale = 0;
								return ap;
							case am_adirect:
								if ((!prm_largedata && (prm_rel || !prm_smalldata)) || prm_68020) {
									freeop(ap1);
									ap = temp_addr();
									gen_code(op_move,4,ap1,ap);
									ap->mode = am_indx;
									ap->offset = ap2->offset;
									return ap;
								}
								ap = temp_addr();
								gen_lea(0,ap2,ap);
								ap->mode = am_baseindxdata;
								ap->offset = makenode(en_icon,0,0);
								ap->scale = 0;
								ap->sreg = ap1->preg;
								return ap;
							case am_immed:
								if (prm_68020 || isamshort(ap2)) {
									freeop(ap1);
									ap = temp_addr();
									gen_code(op_move,4,ap1,ap);
									ap->mode = am_indx;
									ap->offset = ap2->offset;
									return ap;
								}
								ap = temp_addr();
								gen_code(op_move,0,ap2,ap);
								ap->mode = am_baseindxdata;
								ap->offset = makenode(en_icon,0,0);
								ap->scale = 0;
								ap->sreg = ap1->preg;
								return ap;
							case am_indx:
								if (prm_68020 || isambyte(ap2)) {
									ap2->mode = am_baseindxdata;
									ap2->scale = 0;
									ap2->sreg = ap1->preg;
									return ap2;
								}
								freeop(ap2);
								ap = temp_addr();
								gen_lea(0,ap2,ap);
								ap->mode = am_baseindxdata;
								ap->sreg = ap1->preg;
								ap->scale = 0;
								ap->offset = makenode(en_icon,0,0);
								return ap;
							case am_ind:
								ap2->mode = am_baseindxdata;
								ap2->offset = makenode(en_icon,0,0);
								ap2->scale = 0;
								ap2->sreg = ap1->preg;
								return ap2;
							case am_baseindxaddr:
							case am_baseindxdata:
								if (ap2->preg == -1) {
									ap = temp_addr();
									gen_code(op_move,4,ap1,ap);
									ap2->preg = ap->preg;
									return ap2;
								}
								freeop(ap2);
								ap = temp_addr();
								gen_lea(0,ap2,ap);
								ap->mode = am_baseindxdata;
								ap->scale = 0;
								ap->offset = makenode(en_icon,0,0);
								ap->sreg = ap1->preg;
								return ap;
						}
						break;
					case am_adirect:
						switch (ap2->mode) {
							case am_areg:
								if ((!prm_largedata && (prm_rel || !prm_smalldata)) || prm_68020) {
									ap2->mode = am_indx;
									ap2->offset = ap1->offset;
									return ap2;
								}
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								ap2->mode = am_baseindxaddr;
								ap2->offset = makenode(en_icon,0,0);
								ap2->scale = 0;
								ap2->sreg = ap->preg;
								return ap2;
							case am_dreg:
								if ((!prm_largedata && (prm_rel || !prm_smalldata)) || prm_68020) {
									freeop(ap1);
									ap = temp_addr();
									gen_code(op_move,4,ap2,ap);
									ap->mode = am_indx;
									ap->offset = ap1->offset;
									return ap;
								}
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								ap->mode = am_baseindxdata;
								ap->offset = makenode(en_icon,0,0);
								ap->scale = 0;
								ap->sreg = ap2->preg;
								return ap;
							case am_adirect:
								ap1->offset = makenode(en_add,ap1->offset,ap2->offset);
								return ap1;
							case am_immed:
								ap1->offset = makenode(en_add,ap1->offset,ap2->offset);
								return ap1;
							case am_indx:
								if ((!prm_largedata && (prm_rel || !prm_smalldata)) || prm_68020) {
									ap2->offset = makenode(en_add,ap2->offset,ap1->offset);
									return ap2;
								}
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								ap2->mode = am_baseindxaddr;
								ap2->sreg = ap->preg;
								ap2->scale = 0;
								return ap2;
							case am_ind:
								if ((!prm_largedata && (prm_rel || !prm_smalldata)) || prm_68020) {
									ap2->offset = ap1->offset;
									ap2->mode = am_indx;
									return ap2;
								}
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								ap2->mode = am_baseindxaddr;
								ap2->sreg = ap->preg;
								ap2->scale = 0;
								ap2->offset = makenode(en_icon,0,0);
								return ap2;
							case am_baseindxaddr:
							case am_baseindxdata:
								if (prm_68020) {
									ap2->offset = makenode(en_add,ap2->offset,ap1->offset);
									return ap2;
								}
								if (ap2->preg == -1) {
									ap = temp_addr();
									ap2->preg = ap->preg;
									gen_lea(0,ap1,ap);
									return ap2;
								}
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								ap2->mode = am_baseindxaddr;
								ap2->sreg = ap->preg;
								ap2->scale = 0;
								ap2->offset = makenode(en_icon,0,0);
								return ap2;
						}
						break;
					case am_immed:
						switch (ap2->mode) {
							case am_areg:
								if (prm_68020 || isamshort(ap1)) {
									ap2->mode = am_indx;
									ap2->offset = ap1->offset;
									return ap2;
								}
								ap = temp_data();
								gen_code(op_move,4,ap1,ap);
								ap2->mode = am_baseindxdata;
								ap2->offset = makenode(en_icon,0,0);
								ap2->scale = 0;
								ap2->sreg = ap->preg;
								return ap2;
							case am_dreg:
								if (prm_68020 || isamshort(ap1)) {
									freeop(ap2);
									ap = temp_addr();
									gen_code(op_move,4,ap2,ap);
									ap->mode = am_indx;
									ap->offset = ap1->offset;
									return ap;
								}
								ap = temp_addr();
								gen_code(op_move,0,ap1,ap);
								ap->mode = am_baseindxdata;
								ap->offset = makenode(en_icon,0,0);
								ap->scale = 0;
								ap->sreg = ap2->preg;
								return ap;
							case am_adirect:
								ap2->offset = makenode(en_add,ap2->offset,ap1->offset);
								return ap2;
							case am_immed:
								if (prm_68020 || isamshort2(ap1,ap2)) {
									ap1->offset->v.i += ap2->offset->v.i;
									return ap1;
								}
								if (isamshort(ap1)) {
									ap = temp_addr();
									gen_code(op_move,4,ap2,ap);
									ap->mode = am_indx;
									ap->offset = ap1->offset;
									return ap;
								}
								if (isamshort(ap2)) {
									ap = temp_addr();
									gen_code(op_move,4,ap1,ap);
									ap->mode = am_indx;
									ap->offset = ap2->offset;
									return ap;
								}
								ap = temp_addr();
								ap3 = temp_data();
								gen_code(op_move,4,ap1,ap);
								gen_code(op_move,4,ap2,ap3);
								ap->mode = am_baseindxdata;
								ap->sreg = ap3->preg;
								ap->scale = 0;
								ap->offset = makenode(en_icon,0,0);
								return ap;
							case am_indx:
								if (prm_68020 || isamshort2(ap1,ap2)) {
									if (ap2->offset->nodetype == en_icon)
										ap2->offset->v.i += ap1->offset->v.i;
									else
										ap2->offset = makenode(en_add,ap2->offset,ap1->offset);
									return ap2;
								}
								ap = temp_data();
								gen_code(op_move,4,ap2,ap);
								ap1->mode = am_baseindxdata;
								ap1->scale = 0;
								ap1->sreg = ap->preg;
								return ap1;
							case am_baseindxaddr:
							case am_baseindxdata:
								if (ap2->preg == -1 && !prm_68020) {
									ap = temp_addr();
									gen_code(op_move,4,ap1,ap);
									ap2->preg = ap->preg;
									return ap2;
								}
								if (prm_68020 || isambyte2(ap1,ap2)) {
									if (ap2->offset->nodetype == am_immed)
										ap2->offset->v.i += ap1->offset->v.i;
									else
										ap2->offset = makenode(en_add,ap2->offset,ap1->offset);
									return ap2;
								}
								freeop(ap2);
								ap = temp_addr();
								gen_lea(0,ap2,ap);
								ap->mode = am_ind;
								ap2 = ap;
								/* drop through */
							case am_ind:
								if (prm_68020 || isamshort(ap1)) {
									ap2->offset = ap1->offset;
									ap2->mode = am_indx;
									return ap2;
								}
								ap = temp_data();
								gen_code(op_move,4,ap1,ap);
								ap2->mode = am_baseindxdata;
								ap2->scale = 0;
								ap2->sreg = ap->preg;
								ap2->offset = makenode(en_icon,0,0);
								return ap2;
						}
						break;
					case am_indx:
						switch (ap2->mode) {
							case am_areg:
								if (prm_68020 || isambyte(ap1)) {
									ap2->mode = am_baseindxaddr;
									ap2->offset = ap1->offset;
									ap2->scale = 0;
									ap2->sreg = ap2->preg;
									return ap2;
								}
								freeop(ap1);
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								ap->mode = am_baseindxaddr;
								ap->sreg = ap2->preg;
								ap->scale = 0;
								ap->offset = makenode(en_icon,0,0);
								return ap;
							case am_dreg:
								if (prm_68020 || isambyte(ap1)) {
									ap1->mode = am_baseindxdata;
									ap1->scale = 0;
									ap1->sreg = ap2->preg;
									return ap1;
								}
								freeop(ap1);
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								ap->mode = am_baseindxdata;
								ap->sreg = ap2->preg;
								ap->scale = 0;
								ap->offset = makenode(en_icon,0,0);
								return ap;
							case am_adirect:
								if ((!prm_largedata && (prm_rel || !prm_smalldata)) || prm_68020) {
									ap1->offset = makenode(en_add,ap1->offset,ap2->offset);
									return ap1;
								}
								ap = temp_addr();
								gen_lea(0,ap2,ap);
								ap1->mode = am_baseindxaddr;
								ap1->sreg = ap->preg;
								ap1->scale = 0;
								return ap1;
							case am_immed:
								if (prm_68020 || isamshort2(ap1,ap2)) {
									if (ap1->offset->nodetype == am_immed)
										ap1->offset->v.i += ap2->offset->v.i;
									else
										ap1->offset = makenode(en_add,ap1->offset,ap2->offset);
									return ap1;
								}
								ap = temp_data();
								gen_code(op_move,4,ap2,ap);
								ap1->mode = am_baseindxdata;
								ap1->scale = 0;
								ap1->sreg = ap->preg;
								return ap1;
							case am_indx:
								if (isambyte2(ap1,ap2) || prm_68020) {
									if (ap1->offset->nodetype == am_immed && ap2->offset->nodetype)
										ap1->offset->v.i += ap2->offset->v.i;
									else
										ap1->offset = makenode(en_add,ap1->offset,ap2->offset);
									ap1->mode = am_baseindxaddr;
									ap1->sreg = ap2->preg;
									ap1->scale = 0;
									ap1->offset = makenode(en_icon,0,0);
									return ap1;
								}
								if (isambyte(ap1)) {
									freeop(ap2);
									ap = temp_addr();
							    gen_lea(0,ap2,ap);
									ap1->mode = am_baseindxaddr;
									ap1->sreg = ap->preg;
									ap1->scale = 0;
									ap1->offset = makenode(en_icon,0,0);
									return ap1;
								}
								if (isambyte(ap2)) {
									freeop(ap1);
									ap = temp_addr();
							    gen_lea(0,ap1,ap);
									ap2->mode = am_baseindxaddr;
									ap2->sreg = ap->preg;
									ap2->scale = 0;
									ap2->offset = makenode(en_icon,0,0);
									return ap2;
								}
								freeop(ap1);
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								freeop(ap2);
								ap1 = temp_addr();
								gen_lea(0,ap2,ap1);
								ap1->sreg = ap->preg;
								ap1->mode = am_baseindxaddr;
								ap1->scale = 0;
								ap1->offset = makenode(en_icon,0,0);
								return ap1;
							case am_ind:
								if (isambyte(ap1) || prm_68020) {
									ap1->mode = am_baseindxaddr;
									ap1->sreg = ap2->preg;
									ap1->scale = 0;
									return ap1;
								}
								freeop(ap1);
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								ap2->sreg = ap->preg;
								ap2->mode = am_baseindxaddr;
								ap2->scale = 0;
								ap2->offset = makenode(en_icon,0,0);
								return ap2;
							case am_baseindxaddr:
							case am_baseindxdata:
								if (prm_68020 || isambyte2(ap1,ap2)) {
									if (ap2->preg == -1) {
										ap2->preg = ap1->preg;
										if (ap2->offset->nodetype == am_immed && ap1->offset->nodetype == am_immed)
											ap2->offset->v.i+= ap1->offset->v.i;
										else
											ap2->offset = makenode(en_add,ap2->offset, ap1->offset);
										return ap2;
									}
									freeop(ap2);
									ap = temp_addr();
									gen_lea(0,ap2,ap);
									ap->mode = am_baseindxaddr;
									ap->sreg = ap1->preg;
									ap->scale = 0;
									ap->offset = ap1->offset;
									return ap;
								}
								if (ap2->preg == -1) {
									ap3 = xalloc(sizeof(AMODE));
									ap3->preg = ap2->sreg;
									if (ap2->mode == am_baseindxdata) {
										ap3->mode = F_DREG;
										if (ap3->preg < cf_freedata)
											ap3->tempflag = 1;
									}
									else {
										ap3->mode = F_AREG;
										if (ap3->preg < cf_freeaddress)
											ap3->tempflag = 1;
									}
									if (ap2->scale) {
										make_legal(ap3,F_DREG | F_VOL,4);
										gen_code(op_asl,4,ap3,make_immed(ap2->scale));
										make_legal(ap3,F_AREG | F_VOL,4);
									}
									else
										make_legal(ap3,F_AREG,4);
									ap2->preg = ap3->preg;
									ap2->mode = am_indx;
									return ap2;
								}
								freeop(ap2);
								ap = temp_addr();
								gen_lea(0,ap2,ap);
								ap2 = temp_data();
								gen_code(op_move,4,ap,ap2);
								freeop(ap1);
								gen_lea(0,ap1,ap);
								ap->mode = am_baseindxdata;
								ap->sreg = ap2->preg;
								ap->scale = 0;
								ap->offset = makenode(en_icon,0,0);
								return ap;
						}
						break;
					case am_ind:
						switch (ap2->mode) {
							case am_areg:
								ap2->mode = am_baseindxaddr;
								ap2->offset = makenode(en_icon,0,0);
								ap2->scale = 0;
								ap2->sreg = ap1->preg;
								return ap2;
							case am_dreg:
								ap1->mode = am_baseindxdata;
								ap1->offset = makenode(en_icon,0,0);
								ap1->scale = 0;
								ap1->sreg = ap2->preg;
								return ap1;
							case am_adirect:
								if ((!prm_largedata && (prm_rel || !prm_smalldata)) || prm_68020) {
									ap1->offset = ap2->offset;
									ap1->mode = am_indx;
									return ap1;
								}
								ap = temp_addr();
								gen_lea(0,ap2,ap);
								ap1->mode = am_baseindxaddr;
								ap1->sreg = ap->preg;
								ap1->scale = 0;
								ap1->offset = makenode(en_icon,0,0);
								return ap1;
							case am_immed:
								if (prm_68020 || isamshort(ap2)) {
									ap1->offset = ap2->offset;
									ap1->mode = am_indx;
									return ap1;
								}
								ap = temp_data();
								gen_code(op_move,4,ap2,ap);
								ap1->mode = am_baseindxdata;
								ap1->scale = 0;
								ap1->sreg = ap->preg;
								ap1->offset = makenode(en_icon,0,0);
								return ap1;
							case am_indx:
								if (isambyte(ap2) || prm_68020) {
									ap2->mode = am_baseindxaddr;
									ap2->sreg = ap1->preg;
									return ap2;
								}
								freeop(ap2);
								ap = temp_addr();
								gen_lea(0,ap2,ap);
								ap1->sreg = ap->preg;
								ap1->mode = am_baseindxaddr;
								ap1->scale = 0;
								ap1->offset = makenode(en_icon,0,0);
								return ap1;
							case am_ind:
								ap1->mode = am_baseindxaddr;
								ap1->sreg= ap2->preg;
								ap1->scale = 0;
								ap1->offset = makenode(en_icon,0,0);
								return ap1;
							case am_baseindxaddr:
							case am_baseindxdata:
								if (ap2->preg == -1) {
									ap2->preg = ap1->preg;
									return ap2;
								}
								freeop(ap2);
								ap = temp_addr();
								gen_lea(0,ap2,ap);
								ap->mode = am_baseindxaddr;
								ap->sreg = ap1->preg;
								ap->scale = 0;
								ap->offset = makenode(en_icon,0,0);
								return ap;
						}
						break;
					case am_baseindxaddr:
						switch (ap2->mode) {
							case am_areg:
								if (ap1->preg == -1) {
									ap1->preg = ap2->preg;
									return ap1;
								}
								freeop(ap1);
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								ap2->mode = am_baseindxaddr;
								ap2->scale = 0;
								ap2->offset = makenode(en_icon,0,0);
								ap2->sreg = ap->preg;
								return ap2;
							case am_dreg:
								if (ap1->preg == -1) {
									ap = temp_addr();
									gen_code(op_move,4,ap2,ap);
									ap1->preg = ap->preg;
									return ap1;
								}
								freeop(ap1);
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								ap->mode = am_baseindxdata;
								ap->scale = 0;
								ap->offset = makenode(en_icon,0,0);
								ap->sreg = ap2->preg;
								return ap;
							case am_adirect:
								if (prm_68020) {
									ap1->offset = makenode(en_add,ap1->offset,ap2->offset);
									return ap1;
								}
								if (ap1->preg == -1) {
									ap = temp_addr();
									ap1->preg = ap->preg;
									gen_lea(0,ap2,ap);
									return ap1;
								}
								ap = temp_addr();
								gen_lea(0,ap2,ap);
								ap1->mode = am_baseindxaddr;
								ap1->sreg = ap->preg;
								ap1->scale = 0;
								ap1->offset = makenode(en_icon,0,0);
								return ap1;
							case am_immed:
								if (!prm_68020 && ap1->preg == -1) {
									ap = temp_addr();
									gen_code(op_move,4,ap2,ap);
									ap1->preg = ap->preg;
									return ap1;
								}
								if (prm_68020 || isambyte2(ap1,ap2)) {
									if (ap1->offset->nodetype == am_immed)
										ap1->offset->v.i += ap2->offset->v.i;
									else
										ap1->offset = makenode(en_add,ap1->offset,ap2->offset);
									return ap1;
								}
								freeop(ap1);
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								if ((!prm_largedata && (prm_rel || !prm_smalldata)) || prm_68020) {
									ap->mode = am_indx;
									ap->offset = ap2->offset;
									return ap;
								}
								ap1 = temp_data();
								ap1->mode = am_baseindxdata;
								ap1->sreg = ap->preg;
								ap1->scale = 0;
								ap1->offset = makenode(en_icon,0,0);
								return ap1;
							case am_indx:
								if (prm_68020 || isambyte2(ap1,ap2)) {
									if (ap1->preg == -1) {
										ap1->preg = ap2->preg;
										if (ap2->offset->nodetype == am_immed && ap1->offset->nodetype == am_immed)
											ap1->offset->v.i+= ap2->offset->v.i;
										else
											ap1->offset = makenode(en_add,ap1->offset, ap2->offset);
										return ap1;
									}
									freeop(ap1);
									ap = temp_addr();
									gen_lea(0,ap1,ap);
									ap->mode = am_baseindxaddr;
									ap->sreg = ap2->preg;
									ap->scale = 0;
									ap->offset = ap2->offset;
									return ap;
								}
								if (ap1->preg == -1) {
									ap3 = xalloc(sizeof(AMODE));
									ap3->preg = ap1->sreg;
									if (ap1->mode == am_baseindxdata) {
										ap3->mode = F_DREG;
										if (ap3->preg < cf_freedata)
											ap3->tempflag = 1;
									}
									else {
										ap3->mode = F_AREG;
										if (ap3->preg < cf_freeaddress)
											ap3->tempflag = 1;
									}
									if (ap1->scale) {
										make_legal(ap3,F_DREG | F_VOL,4);
										gen_code(op_asl,4,ap3,make_immed(ap1->scale));
										make_legal(ap3,F_AREG | F_VOL,4);
									}
									else
										make_legal(ap3,F_AREG,4);
									ap1->preg = ap3->preg;
									ap1->mode = am_indx;
									return ap1;
								}
								freeop(ap1);
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								ap1 = temp_data();
								gen_code(op_move,4,ap,ap1);
								freeop(ap2);
								ap = temp_addr();
								gen_lea(0,ap2,ap);
								ap->mode = am_baseindxdata;
								ap->sreg = ap1->preg;
								ap->scale = 0;
								ap->offset = makenode(en_icon,0,0);
								return ap;
							case am_ind:
								if (ap1->preg == -1) {
									ap1->preg = ap2->preg;
									return ap1;
								}
								freeop(ap1);
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								ap->mode = am_baseindxaddr;
								ap->sreg = ap2->preg;
								ap->scale = 0;
								ap->offset = ap1->offset;
								return ap;
							case am_baseindxaddr:
							case am_baseindxdata:
								if (prm_68020 || (ap1->preg != -1 && ap2->preg != -1)) {
									freeop(ap1);
									ap = temp_addr();
									gen_lea(0,ap1,ap);
									ap1 = temp_data();
									gen_code(op_move,4,ap,ap1);
									freeop(ap2);
									gen_lea(0,ap2,ap);
									ap->mode = am_baseindxdata;
									ap->sreg = ap1->preg;
									ap->scale = 0;
									ap->offset = makenode(en_icon,0,0);
									return ap;
								}
								if (ap1->preg == -1) {
									ap3 = xalloc(sizeof(AMODE));
									ap3->preg = ap1->sreg;
									if (ap1->mode == am_baseindxdata) {
										ap3->mode = F_DREG;
										if (ap3->preg < cf_freedata)
											ap3->tempflag = 1;
									}
									else {
										ap3->mode = F_AREG;
										if (ap3->preg < cf_freeaddress)
											ap3->tempflag = 1;
									}
									if (ap1->scale) {
										make_legal(ap3,F_DREG | F_VOL,4);
										gen_code(op_asl,4,ap3,make_immed(ap1->scale));
										make_legal(ap3,F_AREG | F_VOL,4);
									}
									else
										make_legal(ap3,F_AREG,4);
									ap1->preg = ap3->preg;
									ap1->mode = am_indx;
								}
								if (ap2->preg == -1) {
									ap3 = xalloc(sizeof(AMODE));
									ap3->preg = ap2->sreg;
									if (ap2->mode == am_baseindxdata) {
										ap3->mode = F_DREG;
										if (ap3->preg < cf_freedata)
											ap3->tempflag = 1;
									}
									else {
										ap3->mode = F_AREG;
										if (ap3->preg < cf_freeaddress)
											ap3->tempflag = 1;
									}
									if (ap2->scale) {
										make_legal(ap3,F_DREG | F_VOL,4);
										gen_code(op_asl,4,ap3,make_immed(ap2->scale));
										make_legal(ap3,F_AREG | F_VOL,4);
									}
									else
										make_legal(ap3,F_AREG,4);
									ap2->preg = ap3->preg;
									ap2->mode = am_indx;
								}
								goto tryagain;
						}
						break;
					case am_baseindxdata:
						switch (ap2->mode) {
							case am_areg:
								if (ap1->preg == -1) {
									ap1->preg = ap2->preg;
									return ap1;
								}
								freeop(ap1);
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								ap2->mode = am_baseindxaddr;
								ap2->scale = 0;
								ap2->offset = makenode(en_icon,0,0);
								ap2->sreg = ap->preg;
								return ap2;
							case am_dreg:
								if (ap1->preg == -1) {
									ap = temp_addr();
									gen_code(op_move,4,ap2,ap);
									ap1->preg = ap->preg;
									return ap1;
								}
								freeop(ap1);
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								ap->mode = am_baseindxdata;
								ap->scale = 0;
								ap->offset = makenode(en_icon,0,0);
								ap->sreg = ap2->preg;
								return ap;
							case am_adirect:
								if (prm_68020) {
									ap1->offset = makenode(en_add,ap1->offset,ap2->offset);
									return ap1;
								}
								if (ap1->preg == -1) {
									ap = temp_addr();
									ap1->preg = ap->preg;
									gen_lea(0,ap2,ap);
									return ap1;
								}
								ap = temp_addr();
								gen_lea(0,ap2,ap);
								ap1->mode = am_baseindxaddr;
								ap1->sreg = ap->preg;
								ap1->scale = 0;
								ap1->offset = makenode(en_icon,0,0);
								return ap1;
							case am_immed:
								if (!prm_68020 && ap1->preg == -1) {
									ap = temp_addr();
									gen_code(op_move,4,ap2,ap);
									ap1->preg = ap->preg;
									return ap1;
								}
								if (prm_68020 || isambyte2(ap1,ap2)) {
									if (ap1->offset->nodetype == am_immed)
										ap1->offset->v.i += ap2->offset->v.i;
									else
										ap1->offset = makenode(en_add,ap1->offset,ap2->offset);
									return ap1;
								}
								freeop(ap1);
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								if ((!prm_largedata && (prm_rel || !prm_smalldata)) || prm_68020) {
									ap->mode = am_indx;
									ap->offset = ap2->offset;
									return ap;
								}
								ap1 = temp_data();
								ap1->mode = am_baseindxdata;
								ap1->sreg = ap->preg;
								ap1->scale = 0;
								ap1->offset = makenode(en_icon,0,0);
								return ap1;
							case am_indx:
								if (prm_68020 || isambyte2(ap1,ap2)) {
									if (ap1->preg == -1) {
										ap1->preg = ap2->preg;
										if (ap2->offset->nodetype == am_immed && ap1->offset->nodetype == am_immed)
											ap1->offset->v.i+= ap2->offset->v.i;
										else
											ap1->offset = makenode(en_add,ap1->offset, ap2->offset);
										return ap1;
									}
									freeop(ap1);
									ap = temp_addr();
									gen_lea(0,ap1,ap);
									ap->mode = am_baseindxaddr;
									ap->sreg = ap2->preg;
									ap->scale = 0;
									ap->offset = ap2->offset;
									return ap;
								}
								if (ap1->preg == -1) {
									ap3 = xalloc(sizeof(AMODE));
									ap3->preg = ap1->sreg;
									if (ap1->mode == am_baseindxdata) {
										ap3->mode = F_DREG;
										if (ap3->preg < cf_freedata)
											ap3->tempflag = 1;
									}
									else {
										ap3->mode = F_AREG;
										if (ap3->preg < cf_freeaddress)
											ap3->tempflag = 1;
									}
									if (ap3->preg < cf_freedata)
										ap3->tempflag = 1;
									if (ap1->scale) {
										make_legal(ap3,F_DREG | F_VOL,4);
										gen_code(op_asl,4,ap3,make_immed(ap1->scale));
										make_legal(ap3,F_AREG | F_VOL,4);
									}
									else
										make_legal(ap3,F_AREG,4);
									ap1->preg = ap3->preg;
									ap1->mode = am_indx;
									return ap1;
								}
								freeop(ap1);
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								ap1 = temp_data();
								gen_code(op_move,4,ap,ap1);
								freeop(ap2);
								gen_lea(0,ap2,ap);
								ap->mode = am_baseindxdata;
								ap->sreg = ap1->preg;
								ap->scale = 0;
								ap->offset = makenode(en_icon,0,0);
								return ap;
							case am_ind:
								if (ap1->preg == -1) {
									ap1->preg = ap2->preg;
									return ap1;
								}
								freeop(ap1);
								ap = temp_addr();
								gen_lea(0,ap1,ap);
								ap->mode = am_baseindxaddr;
								ap->sreg = ap2->preg;
								ap->scale = 0;
								ap->offset = ap1->offset;
								return ap;
							case am_baseindxaddr:
							case am_baseindxdata:
								if (prm_68020 || (ap1->preg != -1 && ap2->preg != -1)) {
									freeop(ap1);
									ap = temp_addr();
									gen_lea(0,ap1,ap);
									ap1 = temp_data();
									gen_code(op_move,4,ap,ap1);
									freeop(ap2);
									gen_lea(0,ap2,ap);
									ap->mode = am_baseindxdata;
									ap->sreg = ap->preg;
									ap->scale = 0;
									ap->offset = makenode(en_icon,0,0);
									return ap;
								}
								if (ap1->preg == -1) {
									ap3 = xalloc(sizeof(AMODE));
									ap3->preg = ap1->sreg;
									if (ap1->mode == am_baseindxdata) {
										ap3->mode = F_DREG;
										if (ap3->preg < cf_freedata)
											ap3->tempflag = 1;
									}
									else {
										ap3->mode = F_AREG;
										if (ap3->preg < cf_freeaddress)
											ap3->tempflag = 1;
									}
									if (ap1->scale) {
										make_legal(ap3,F_DREG | F_VOL,4);
										gen_code(op_asl,4,ap3,make_immed(ap1->scale));
										make_legal(ap3,F_AREG | F_VOL,4);
									}
									else
										make_legal(ap3,F_AREG,4);
									ap1->preg = ap3->preg;
									ap1->mode = am_indx;
								}
								if (ap2->preg == -1) {
									ap3 = xalloc(sizeof(AMODE));
									ap3->preg = ap2->sreg;
									if (ap2->mode == am_baseindxdata) {
										ap3->mode = F_DREG;
										if (ap3->preg < cf_freedata)
											ap3->tempflag = 1;
									}
									else {
										ap3->mode = F_AREG;
										if (ap3->preg < cf_freeaddress)
											ap3->tempflag = 1;
									}
									if (ap2->scale) {
										make_legal(ap3,F_DREG | F_VOL,4);
										gen_code(op_asl,4,ap3,make_immed(ap2->scale));
										make_legal(ap3,F_AREG | F_VOL,4);
									}
									else
										make_legal(ap3,F_AREG,4);
									ap2->preg = ap3->preg;
									ap2->mode = am_indx;
								}
								goto tryagain;
						}
						break;
				}
				DIAG("invalid index conversion");
}
AMODE    *gen_deref(ENODE *node, int flags,int size, int flag)
/*
 *      return the addressing mode of a dereferenced node.
 */
{       AMODE    *ap1,*ap2;
        int             siz1,psize;
				psize = size;
				if (psize < 0)
					psize = - psize;
        switch( node->nodetype )        /* get load size */
                {
                case en_ub_ref:
                        siz1 = 1;
                        break;
                case en_b_ref:
                        siz1 = -1;
                        break;
                case en_uw_ref:
                        siz1 = 2;
                        break;
                case en_w_ref:
                        siz1 = -2;
                        break;
                case en_l_ref:
												siz1 = -4;
												break;
								case en_add:
								case en_ul_ref:
                        siz1 = 4;
                        break;
								case en_floatref:
												siz1 = 6;
												break;
								case en_doubleref:
												siz1 = 8;
												break;
								case en_longdoubleref:
												siz1 = 10;
												break;
								default:
												siz1 = 4;
                }
        if( node->v.p[0]->nodetype == en_add )
                {
                ap1 = gen_index(siz1,node->v.p[0]);
                do_extend(ap1,siz1,size,flags);
                make_legal(ap1,flags,psize);
                return ap1;
                }
        else if( node->v.p[0]->nodetype == en_autocon  || node->v.p[0]->nodetype == en_autoreg)
                {
                ap1 = xalloc(sizeof(AMODE));
                ap1->mode = am_indx;
								if (prm_linkreg && !currentfunc->intflag) {
                		ap1->preg = linkreg;
                		ap1->offset = makenode(en_icon,(char *)((SYM *)node->v.p[0]->v.p[0])->value.i,0);
								}
								else if (((SYM *)node->v.p[0]->v.p[0])->funcparm) {
									if (prm_phiform || currentfunc->intflag) {
                		ap1->preg = linkreg;
                		ap1->offset = makenode(en_icon,(char *)((SYM *)node->v.p[0]->v.p[0])->value.i,0);
									}
									else {
                		ap1->preg = 7;
                		ap1->offset = makenode(en_icon,(char *)(((SYM *)node->v.p[0]->v.p[0])->value.i+framedepth+stackdepth),0);
									}
								}
								else {
                	ap1->preg = 7;
                	ap1->offset = makenode(en_icon,(char *)(((SYM *)node->v.p[0]->v.p[0])->value.i+stackdepth+lc_maxauto),0);
								}
                do_extend(ap1,siz1,size,flags);
                make_legal(ap1,flags,psize);
                return ap1;
                }
        else if( node->v.p[0]->nodetype == en_nacon)
                {
      	        ap1 = xalloc(sizeof(AMODE));
								if (prm_rel) {
        	        ap1->preg = basereg;
          	      ap1->offset = makenode(node->v.p[0]->nodetype,((SYM *)node->v.p[0]->v.p[0])->name,0);
					        if (prm_largedata) {
										ap2 = temp_addr();
										ap1->mode = am_areg;
										gen_code(op_move,4,ap1,ap2);
										ap1->mode = am_immed;
										gen_code(op_add,4,ap1,ap2);
										ap1 = ap2;
										ap1->mode = am_ind;
									}
									else {
  	              	ap1->mode = am_indx;
									}
								}
								else {
									ap1->mode = am_adirect;
          	      ap1->offset = makenode(node->v.p[0]->nodetype,((SYM *)node->v.p[0]->v.p[0])->name,0);
									if (prm_smalldata)
										ap1->preg = 2;
									else
										ap1->preg = 4;
								}
                do_extend(ap1,siz1,size,flags);
                make_legal(ap1,flags,psize);
                return ap1;
                }
        else if( node->v.p[0]->nodetype == en_nalabcon)
                {
      	        ap1 = xalloc(sizeof(AMODE));
								if (prm_rel) {
        	        ap1->preg = basereg;
          	      ap1->offset = makenode(node->v.p[0]->nodetype,(char *)node->v.p[0]->v.i,0);
					        if (prm_largedata) {
										ap2 = temp_addr();
										ap1->mode = am_areg;
										gen_code(op_move,4,ap1,ap2);
										ap1->mode = am_immed;
										gen_code(op_add,4,ap1,ap2);
										ap1 = ap2;
										ap1->mode = am_ind;
									}
									else {
  	              	ap1->mode = am_indx;
									}
								}
								else {
									ap1->mode = am_adirect;
          	      ap1->offset = makenode(node->v.p[0]->nodetype,(char *)node->v.p[0]->v.i,0);
									if (prm_smalldata)
										ap1->preg = 2;
									else
										ap1->preg = 4;
								}
                do_extend(ap1,siz1,size,flags);
                make_legal(ap1,flags,psize);
                return ap1;
                }
        else if( node->v.p[0]->nodetype == en_labcon || node->v.p[0]->nodetype == en_napccon)
                {
	                ap1 = xalloc(sizeof(AMODE));
									if (prm_rel)
  	              	ap1->mode = am_pcindx;
									else {
										ap1->mode = am_adirect;
										if (prm_smallcode)
											ap1->preg = 2;
										else
											ap1->preg = 4;
									}
									if (node->v.p[0]->nodetype == en_labcon)
    	            	ap1->offset = makenode(node->v.p[0]->nodetype,(char *)node->v.p[0]->v.i,0);
									else
    	            	ap1->offset = makenode(node->v.p[0]->nodetype,((SYM *)node->v.p[0]->v.p[0])->name,0);
                do_extend(ap1,siz1,size,flags);
                make_legal(ap1,flags,psize);
                return ap1;
                }
				else if (node->v.p[0]->nodetype == en_absacon) {
								ap1 = xalloc(sizeof(AMODE));
								ap1->mode = am_adirect;
								ap1->preg = isshort(node->v.p[0]) ? 2 : 4;
								ap1->offset = makenode(en_absacon,(char *)((SYM *)node->v.p[0]->v.p[0])->value.i,0);
                do_extend(ap1,siz1,size,flags);
                make_legal(ap1,flags,psize);
                return ap1;

				}
				else if (node->v.p[0]->nodetype == en_regref) {
        	ap1 = gen_expr(node->v.p[0],F_ALL,4);
          do_extend(ap1,siz1,size,flags);
          make_legal(ap1,flags,psize);
					return ap1;
				}
				if (flag) {
        	ap1 = gen_expr(node->v.p[0],F_AREG | F_DREG | F_IMMED,4); /* generate address */
					if (ap1->mode == am_dreg)
						return ap1;
				}
				else {
        	ap1 = gen_expr(node->v.p[0],F_AREG | F_IMMED,4); /* generate address */
				}

				/* AINCDEC for example may return an indirect mode already */
				if (ap1->mode == am_areg || ap1->mode == am_immed) {
        	if( ap1->mode == am_areg )
          	      {
            	    ap1->mode = am_ind;
              	  do_extend(ap1,siz1,size,flags);
                	make_legal(ap1,flags,psize);
                	return ap1;
                	}
        	ap1->mode = am_direct;
				}
        do_extend(ap1,siz1,size,flags);
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
        	gen_codef(fop,size,ap,0);
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
        ap1 = gen_expr(node->v.p[0],F_VOL | F_DREG | F_AREG | F_FREG,size);
				mark();
        ap2 = gen_expr(node->v.p[1],F_ALL,size);
				if (size > 4)
        	gen_codef(fop,size,ap2,ap1);
				else
        	gen_code(op,size,ap2,ap1);
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
        ap1 = gen_expr(node->v.p[0],F_VOL | F_DREG | F_FREG,size);
				mark();
        ap2 = gen_expr(node->v.p[1],F_DREG| F_FREG, size);
				if (size > 4)
        	gen_codef(fop,size,ap2,ap1);
				else
        	gen_code(op,size,ap2,ap1);
        freeop(ap2);
				release();
        make_legal(ap1,flags,size);
        return ap1;
}
AMODE    *gen_shift(ENODE *node, int flags, int size, int op)
/*
 *      generate code to evaluate a shift node and return the
 *      address mode of the result.
 */
{       AMODE    *ap1, *ap2;
        ap1 = gen_expr(node->v.p[0],F_DREG | F_VOL,size);
				mark();
        ap2 = gen_expr(node->v.p[1],F_DREG | F_IMMED,size);
				doshift(op,ap2,ap1,size);
        freeop(ap2);
				release();
        make_legal(ap1,flags,size);
        return ap1;
}

AMODE    *gen_modiv(ENODE *node, int flags, int size, int op, int modflag)
/*
 *      generate code to evaluate a mod operator or a divide
 *      operator. these operations are done on only long
 *      divisors and word dividends so that the 68000 div
 *      instruction can be used.
 */
{       AMODE    *ap1, *ap2,*ap3;
				int temp;
				int sz1 = natural_size(node->v.p[0]);
				int sz2 = natural_size(node->v.p[0]);
				int psize;

				if (sz1 < 0)  sz1 = - sz1;
				if (sz2 < 0)  sz2 = - sz2;
				psize = sz2 > sz1 ? sz2 : sz1;
				if (psize < 2) psize = 2;
				if (op == op_divs)  psize = -psize;
				
				if (size > 4) {
					ap1 = gen_expr(node->v.p[0],F_FREG | F_VOL,size);
					mark();
					ap2 = gen_expr(node->v.p[1],F_ALL&~F_AREG,size);
					gen_codef(op_fdiv,size,ap2,ap1);
					freeop(ap2);
					release();
					make_legal(ap1,flags,size);
					return(ap1);

				}
				if (prm_68020 || psize ==2 || psize == -2) {
					if (psize != 2 && psize != -2)
						if (op == op_divs)
							op = op_divsl;
						else
							op = op_divul;
  	      ap1 = gen_expr(node->v.p[0],F_DREG | F_VOL,psize == 2 ? 4 : -4);
					mark();
    	    ap2 = gen_expr(node->v.p[1],F_ALL & ~F_AREG,psize);
					if (psize !=2 && psize != -2) {
						ap3 = temp_data();
						temp = ap3->preg;
						ap3->mode = am_divsl;
        		if( modflag ) {
							ap3->sreg = ap1->sreg;
						}
						else {
							ap3->sreg = ap3->preg = ap1->sreg;
						}
						gen_code(op,psize,ap2,ap3);
						ap3->mode = am_dreg;
						ap3->preg = temp;
						if (modflag)
							gen_code(op_exg,0,ap1,ap3);
        		freeop(ap3);
					}
					else {
						gen_code(op,2,ap2,ap1);
						if (modflag) {
							gen_code(op_swap,2,ap1,0);
						}
					}
					do_extend(ap1,psize,size,flags);
        	make_legal(ap1,flags,size);
        	freeop(ap2);
        	release();
        	return ap1;
					
				}		
				flush_for_libcall();
				if (op == op_divs) {
        	ap1 = gen_expr(node->v.p[0],F_ALL,-4);
				}
				else {
        	ap1 = gen_expr(node->v.p[0],F_ALL,4);
				}
				mark();
				if (op == op_divs) {
        	ap2 = gen_expr(node->v.p[1],F_ALL,-4);
				}
				else {
        	ap2 = gen_expr(node->v.p[1],F_ALL,4);
				}
				gen_code(op_move,4,ap1,push);
        freeop(ap2);
				gen_code(op_move,4,ap2,push);
				
				freeop(ap1);
				if (op == op_divs)
					if (modflag) {
						ap1 = call_library("__mods",8);
					}
					else {
						ap1 = call_library("__divs",8);
					}
				else
					if (modflag) {
						ap1 = call_library("__modu",8);
					}
					else {
						ap1 = call_library("__divu",8);
					}
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
				AMODE *ap1, *ap2;
				ap1 = gen_expr(node->v.p[0],F_DREG | F_VOL,4);
				mark();
				ap2 = gen_expr(node->v.p[1],F_ALL,2);
				gen_code(op_divs,2,ap2,ap1);
				freeop(ap2);
				release();
				make_legal(ap1,flags,size);
        return ap1;
}			
AMODE * gen_pmul(ENODE *node, int flags, int size)
{
				AMODE *ap1, *ap2;
				if (isintconst(node->v.p[0]->nodetype))
					swap_nodes(node);
				ap1 = gen_expr(node->v.p[0],F_DREG | F_VOL,4);
				mark();
				ap2 = gen_expr(node->v.p[1],F_ALL,4);
				gen_code(op_muls,0,ap2,ap1);
				freeop(ap2);
				release();
				make_legal(ap1,flags,size);
        return ap1;
}			
AMODE    *gen_mul(ENODE *node, int flags, int size, int op)
/*
 *      generate code to evaluate a multiply node. both operands
 *      are treated as words and the result is long and is always
 *      in a register so that the 68000 mul instruction can be used.
 */
{       AMODE    *ap1, *ap2;
				int sz1 = natural_size(node->v.p[0]);
				int sz2 = natural_size(node->v.p[0]);
				int psize;

				if (sz1 < 0)  sz1 = - sz1;
				if (sz2 < 0)  sz2 = - sz2;
				psize = sz2 > sz1 ? sz2 : sz1;
				if (psize < 2) psize = 2;
				if (op == op_muls)  psize = -psize;
				
				if (size > 4) {
					ap1 = gen_expr(node->v.p[0],F_FREG | F_VOL,size);
					mark();
					ap2 = gen_expr(node->v.p[1],F_ALL&~F_AREG,size);
					gen_codef(op_fmul,size,ap2,ap1);
					freeop(ap2);
					release();
					make_legal(ap1,flags,size);
					return(ap1);

				}
				if (prm_68020 || psize == -2 || psize == 2) {
					if (isintconst(node->v.p[0]->nodetype))
						swap_nodes(node);
      	  ap1 = gen_expr(node->v.p[0],F_DREG | F_VOL,psize);
					mark();
    	    ap2 = gen_expr(node->v.p[1],F_ALL&~F_AREG,psize);
					gen_code(op,psize,ap2,ap1);
					freeop(ap2);
					release();
					do_extend(ap1,psize,size,flags);
					make_legal(ap1,flags,size);
        	return ap1;
				}
				flush_for_libcall();
				if (isintconst(node->v.p[0]->nodetype))
					swap_nodes(node);
				if (op == op_muls) {
        	ap1 = gen_expr(node->v.p[0],F_ALL,-4);
				}
				else {
        	ap1 = gen_expr(node->v.p[0],F_ALL,4);
				}
				mark();
				if (op == op_muls) {
        	ap2 = gen_expr(node->v.p[1],F_ALL,-4);
				}
				else {
        	ap2 = gen_expr(node->v.p[1],F_ALL,4);
				}
				gen_code(op_move,4,ap1,push);
        freeop(ap2);
				gen_code(op_move,4,ap2,push);
				freeop(ap1);
				if (op == op_muls) {
					ap1 = call_library("__muls",8);
				}
				else {
					ap1 = call_library("__mulu",8);
				}
        release();
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
        gen_code(op_bra,0,make_label(end_label),0);
        gen_label(false_label);
        ap2 = gen_expr(node->v.p[1],flags,size);
        if( !equal_address(ap1,ap2) )
                {
                freeop(ap2);
                if( ap1->mode == am_dreg )
                        temp_data();
                else
                        temp_addr();
                gen_code(op_move,size,ap2,ap1);
                }
        gen_label(end_label);
        return ap1;
}

AMODE    *gen_asadd(ENODE *node, int flags, int size, int op, int fop)
/*
 *      generate a plus equal or a minus equal node.
 */
{       AMODE    *ap1, *ap2, *ap3;
        int             ssize,rsize;
        ssize = natural_size(node->v.p[0]);
        rsize = natural_size(node->v.p[1]);
				if (ssize > 4) {
					ap1 = gen_expr(node->v.p[0],F_ALL,ssize);
					ap3 = xalloc(sizeof(AMODE));
					ap3->mode = ap1->mode;
					ap3->preg = ap1->preg;
					ap3->sreg = ap1->sreg;
					ap3->offset = ap1->offset;
					make_legal(ap1,F_FREG,ssize);
					mark();
					ap2 = gen_expr(node->v.p[1],F_ALL,size);
					gen_codef(fop,ssize,ap2,ap1);
					make_legal(ap1,flags,size);
					if (ap3->mode != am_freg)
						gen_codef(op_fmove,ssize,ap1,ap3);
					freeop(ap2);
					release();
					return(ap1);

				}
        if (chksize( size , rsize ))
                rsize = size;
        ap1 = gen_expr(node->v.p[0],F_ALL | F_NOBIT,ssize);
				mark();
				if (node->v.p[0]->nodetype == en_bits) {
					ap3= get_bitval(ap1,node->v.p[0],ssize);
				}
        ap2 = gen_expr(node->v.p[1],F_DREG | F_AREG | F_FREG | F_IMMED,rsize);
				if (ssize > 4)
        	gen_codef(fop,ssize,ap2,ap1);
				else 
					if (node->v.p[0]->nodetype == en_bits) {
						gen_code(op,ssize,ap2,ap3);
						bit_move(ap3,ap1,node->v.p[0],flags, ssize,rsize);
						freeop(ap3);
					}
					else
	        	gen_code(op,ssize,ap2,ap1);
  	    freeop(ap2);
				release();
				if (flags & F_NOVALUE)
					freeop(ap1);
				else {
					if (node->v.p[0]->nodetype == en_bits) {
  	  	    do_extend(ap3,ssize,size,0);
    	  	  make_legal(ap3,flags,size);
      	  	return ap3;
	      	}
	    	  do_extend(ap1,ssize,size,0);
  	    	make_legal(ap1,flags,size);
				}
        return ap1;
}

AMODE    *gen_aslogic(ENODE *node, int flags, int size, int op)
/*
 *      generate a and equal or a or equal node.
 */
{       AMODE    *ap1, *ap2, *ap3;
        int             ssize,rsize;
        ssize = natural_size(node->v.p[0]);
        rsize = natural_size(node->v.p[1]);
        if (chksize( size , rsize ))
                rsize = size;
        ap1 = gen_expr(node->v.p[0],F_ALL | F_NOBIT,ssize);
				mark();
        ap2 = gen_expr(node->v.p[1],F_DREG | F_IMMED,rsize);
				if (node->v.p[0]->nodetype == en_bits) {
					if (ap2->mode == am_immed) {
						ap2->offset->v.i &= bittab[node->v.p[0]->bits-1];
						ap2->offset->v.i <<= node->v.p[0]->startbit;
						gen_code(op,ssize,ap2,ap1);
					}
					else {
					  gen_code(op_and,ssize,make_immed(bittab[node->v.p[0]->bits-1]),ap2);
						if (node->v.p[0]->startbit)
					  	gen_code(op_asl,ssize,make_immed(node->v.p[0]->startbit),ap2);
						gen_code(op,ssize,ap2,ap1);
						if (!(flags & F_NOVALUE)) {
							freeop(ap2);
							if (!(ap1->mode == am_dreg && ap1->preg < cf_freedata)) {
								freeop(ap1);
								ap2 = temp_data();
								gen_code(op_move,ssize,ap1,ap2);
								ap1 = ap2;
							}
							if (node->v.p[0]->startbit)
					  		gen_code(op_asr,ssize,make_immed(node->v.p[0]->startbit),ap1);
				  		gen_code(op_and,ssize,make_immed(bittab[node->v.p[0]->bits-1]),ap1);
						}
					}
				}
				else
        	if( ap1->mode != am_areg )
          	      gen_code(op,ssize,ap2,ap1);
	        else
  	              {
    	            ap3 = temp_data();
      	          gen_code(op_move,4,ap1,ap3);
        	        gen_code(op,size,ap2,ap3);
          	      gen_code(op_move,size,ap3,ap1);
            	    freeop(ap3);
              	  }
        freeop(ap2);
				release();
				if (flags & F_NOVALUE)
					freeop(ap1);
				else {
        	do_extend(ap1,ssize,size,0);
        	make_legal(ap1,flags,size);
				}
        return ap1;
}

AMODE *gen_asshift(ENODE *node, int flags, int size, int op)
/*
 *      generate shift equals operators.
 */
{       AMODE    *ap1, *ap2, *ap3,*ap4;
        int ssize = natural_size(node->v.p[0]);
        int rsize = natural_size(node->v.p[1]);
        if (chksize( size , rsize ))
                rsize = size;
        ap1 = gen_expr(node->v.p[0],F_ALL | F_NOBIT,ssize);
				mark();
				if (node->v.p[0]->nodetype == en_bits)
					ap4 = get_bitval(ap1,node->v.p[0],ssize);
				else
					ap4 = ap1;
        if( ap4->mode != am_dreg )
                {
                ap3 = temp_data();
								ap3->tempflag = TRUE;
                gen_code(op_move,ssize,ap4,ap3);
                }
        else
                ap3 = ap4;
        ap2 = gen_expr(node->v.p[1],F_DREG | F_IMMED,rsize);
				doshift(op,ap2,ap3,size);
				if (node->v.p[0]->nodetype == en_bits) {
				  bit_move(ap3,ap1,node->v.p[0],flags,ssize,ssize);
        } else if( ap3 != ap1 )
                {
                gen_code(op_move,ssize,ap3,ap1);
                }
				freeop(ap2);
				release();
				if (ap4 != ap1)
					freeop(ap4);
				if (flags & F_NOVALUE)
					freeop(ap2);
				else {
	    	  do_extend(ap3,ssize,size,0);
  	      make_legal(ap3,flags,size);
				}
        return ap3;
}

AMODE    *gen_asmul(ENODE *node, int flags, int size,int op)
/*
 *      generate a *= node.
 */
{       AMODE    *ap1, *ap2, *ap3, *ap4;
        int             siz1;
				int sz1 = natural_size(node->v.p[0]);
				int sz2 = natural_size(node->v.p[0]);
				int psize;

				if (sz1 < 0)  sz1 = - sz1;
				if (sz2 < 0)  sz2 = - sz2;
				psize = sz2 > sz1 ? sz2 : sz1;
				if (psize < 2) psize = 2;
				if (op == op_muls)  psize = -psize;
        siz1 = natural_size(node->v.p[0]);
				if (siz1 > 4) {
					ap1 = gen_expr(node->v.p[0],F_ALL&~F_AREG,siz1);
					ap3 = xalloc(sizeof(AMODE));
					ap3->mode = ap1->mode;
					ap3->preg = ap1->preg;
					ap3->sreg = ap1->sreg;
					ap3->offset = ap1->offset;
					make_legal(ap1,F_FREG,siz1);
					mark();
					ap2 = gen_expr(node->v.p[1],F_ALL&~F_AREG,siz1);
					gen_codef(op_fmul,siz1,ap2,ap1);
					make_legal(ap1,flags,size);
					if (ap3->mode != am_freg)
						gen_codef(op_fmove,siz1,ap1,ap3);
					freeop(ap2);
					release();
					return(ap1);

				}
				if (prm_68020 || psize == 2 || psize == -2) {
        	ap1 = gen_expr(node->v.p[1],F_DREG | F_VOL,psize);
					ap4 = xalloc(sizeof(AMODE));
  	      ap2 = gen_expr(node->v.p[0],F_ALL | F_NOBIT,0);
					ap4->mode = ap2->mode;
					ap4->preg = ap2->preg;
					ap4->sreg = ap2->sreg;
					ap4->tempflag = ap2->tempflag;
					ap4->offset = ap2->offset;
					if (node->v.p[0]->nodetype == en_bits)
						ap2 = get_bitval(ap2,node->v.p[0],psize);
					do_extend(ap2,siz1, psize,F_ALL);
      	  gen_code(op,psize,ap2,ap1);
					if (node->v.p[0]->nodetype == en_bits) 
						bit_move(ap1,ap4,node->v.p[0],flags,siz1,psize);
					else 
        		gen_code(op_move,siz1,ap1,ap4);
					if (flags & F_NOVALUE)
						freeop(ap1);
					else {
	    	  	do_extend(ap1,psize,size,flags);
  	      	make_legal(ap1,flags,size);
					}
        	freeop(ap2);
					return(ap1);
				}
				flush_for_libcall();
					
				if (op == op_muls) {
        	ap1 = gen_expr(node->v.p[1],F_ALL,-4);
				}
				else {
        	ap1 = gen_expr(node->v.p[1],F_ALL,4);
				}
				ap4 = xalloc(sizeof(AMODE));
        ap2 = gen_expr(node->v.p[0],F_ALL | F_NOBIT,siz1);
				ap4->mode = ap2->mode;
				ap4->preg = ap2->preg;
				ap4->sreg = ap2->sreg;
				ap4->tempflag = ap2->tempflag;
				ap4->offset = ap2->offset;
				if (node->v.p[0]->nodetype == en_bits)
					ap2 = get_bitval(ap2,node->v.p[0],siz1);

				if (op == op_muls) {
					do_extend(ap2,siz1, - 4,F_ALL);
				}
				else {
					do_extend(ap2,siz1, 4,F_ALL);
				}
				gen_code(op_move,4,ap2,push);
				gen_code(op_move,4,ap1,push);
        freeop(ap2);
				freeop(ap1);
				if (op == op_muls) {
					ap1 = call_library("__muls",8);
				}
				else {
					ap1 = call_library("__mulu",8);
				}
				if (node->v.p[0]->nodetype == en_bits)
					bit_move(ap1,ap4,node->v.p[0],flags,siz1,size);
				else
        	gen_code(op_move,siz1,ap1,ap4);
				freeop(ap4);
				if (flags & F_NOVALUE)
					freeop(ap1);
				else {
	    	 	do_extend(ap1,siz1,size,0);
  	     	make_legal(ap1,flags,size);
				}
        return ap1;
}

AMODE    *gen_asmodiv(ENODE *node, int flags, int size, int op, int modflag)
/*
 *      generate /= and %= nodes.
 */
{       AMODE    *ap1, *ap2, *ap3 = 0, *ap4,*ap5;
        int             siz1,temp;
				int sz1 = natural_size(node->v.p[0]);
				int sz2 = natural_size(node->v.p[0]);
				int psize;

				if (sz1 < 0)  sz1 = - sz1;
				if (sz2 < 0)  sz2 = - sz2;
				psize = sz2 > sz1 ? sz2 : sz1;
				if (psize < 2) psize = 2;
				if (op == op_muls)  psize = -psize;
        siz1 = natural_size(node->v.p[0]);
				if (siz1 > 4) {
					ap1 = gen_expr(node->v.p[0],F_ALL&~F_AREG,siz1);
					ap3 = xalloc(sizeof(AMODE));
					ap3->mode = ap1->mode;
					ap3->preg = ap1->preg;
					ap3->sreg = ap1->sreg;
					ap3->offset = ap1->offset;
					make_legal(ap1,F_FREG,siz1);
					mark();
					ap2 = gen_expr(node->v.p[1],F_ALL&~F_AREG,siz1);
					gen_codef(op_fdiv,siz1,ap2,ap1);
					make_legal(ap1,flags,size);
					if (ap3->mode != am_freg)
						gen_codef(op_fmove,siz1,ap1,ap3);
					freeop(ap2);
					release();
					return(ap1);
				}
				if (prm_68020 || psize == 2 || psize == -2) {
					if (psize !=2 && psize != -2)
						if (op == op_divs) 
							op = op_divsl;
						else
							op = op_divul;
  	      ap1 = gen_expr(node->v.p[0],F_ALL | F_NOBIT,0);
					ap5 = xalloc(sizeof(AMODE));
					ap5->mode = ap1->mode;
					ap5->preg = ap1->preg;
					ap5->sreg = ap1->sreg;
					ap5->offset = ap1->offset;
					if (node->v.p[0]->nodetype == en_bits)
						ap1 = get_bitval(ap1,node->v.p[0],siz1);

					do_extend(ap1,siz1, psize == 2 ? 4 : -4,F_DREG | F_VOL);
					make_legal(ap1,F_DREG | F_VOL,psize == 2 ? 4 : -4);
					mark();
	        ap3 = gen_expr(node->v.p[1],F_ALL & ~F_AREG,psize);
					if (psize != 2 && psize != -2) {
						ap4 = temp_data();
						ap4->tempflag = TRUE;
						temp = ap4->preg;
      	  	if( modflag ) {
							gen_code(op_move,4,ap1,ap4);
							ap4->sreg = ap1->preg;
						}
						else {
							ap4->sreg = ap4->preg = ap1->preg;
						}
						ap4->mode = am_divsl;
						gen_code(op,psize,ap3,ap4);
						ap4->mode = am_dreg;
						ap4->preg = temp;
						freeop(ap4);
      	  	freeop(ap3);
    	    release();
					}
					else {
						gen_code(op,psize,ap3,ap1);
						if (modflag) {
							gen_code(op_swap,2,ap1,0);
						}
						freeop(ap3);
					}
					if (node->v.p[0]->nodetype == en_bits)
						bit_move(ap1,ap5,node->v.p[0],flags,psize,size);
					else
  	      	gen_code(op_move,siz1,ap1,ap5);
					if (flags & F_NOVALUE)
						freeop(ap1);
					else {
		    	 	do_extend(ap1,psize,size,0);
  	     		make_legal(ap1,flags,size);
					}
      	  freeop(ap2);
	        return ap1;
				}
				flush_for_libcall();
				if (op == op_divs) {
        	ap1 = gen_expr(node->v.p[1],F_ALL,-4);
				}
				else {
        	ap1 = gen_expr(node->v.p[1],F_ALL,4);
				}
				ap4 = xalloc(sizeof(AMODE));
        ap2 = gen_expr(node->v.p[0],F_ALL | F_NOBIT,siz1);
				ap4->mode = ap2->mode;
				ap4->preg = ap2->preg;
				ap4->sreg = ap2->sreg;
				ap4->tempflag = ap2->tempflag;
				ap4->offset = ap2->offset;
				if (node->v.p[0]->nodetype == en_bits)
					ap2 = get_bitval(ap2,node->v.p[0],siz1);

				if (op == op_divs) {
					do_extend(ap2,siz1, - 4,F_ALL);
				}
				else {
					do_extend(ap2,siz1, 4,F_ALL);
				}
				gen_code(op_move,4,ap2,push);
				gen_code(op_move,4,ap1,push);
        freeop(ap2);
				freeop(ap1);
				if (modflag)
					if (op == op_divs)
						ap1=call_library("__mods",8);
					else
						ap1=call_library("__modu",8);
				else
					if (op == op_divs)
						ap1=call_library("__divs",8);
					else
						ap1=call_library("__divu",8);
				make_legal(ap1,flags,size);
				if (node->v.p[0]->nodetype == en_bits)
					bit_move(ap1,ap4,node->v.p[0],flags,siz1,size);
				else
  	     	gen_code(op_move,siz1,ap1,ap4);
				if (flags & F_NOVALUE)
					freeop(ap1);
				else {
		   	 	do_extend(ap1,siz1,size,0);
  	    	make_legal(ap1,flags,size);
				}
				freeop(ap4);
        return ap1;
}
AMODE *gen_moveblock(ENODE *node, int flags, int size)
{
	AMODE *ap1, *ap2, *ap3;
	int lbl;
	long v,tp,sz;
	if (!(sz=node->size))
		return(0);
	if (sz & 1) {
		tp = 1;
	}
	else if (sz & 2) {
		tp = 2;
	}
	else {
		tp = 4;
	}
	v = sz/tp;
	if (v < 65536)
		v--;
	lbl = nextlabel++;
	ap2 = gen_expr(node->v.p[0], F_AREG | F_VOL, 4);
	mark();
	ap3 = gen_expr(node->v.p[1], F_AREG | F_VOL,4);
	ap1 = temp_data();
	ap2->mode = am_ainc;
	ap3->mode = am_ainc;
	gen_code(op_move,v < 65536 ? 2 : 4,make_immed(v),ap1);
	gen_label(lbl);
	gen_code(op_move,tp,ap3,ap2);
	if (v < 65536) 
		gen_code(op_dbra,0,ap1,make_label(lbl));
	else {
		gen_code(op_sub,4,make_immed(1),ap1);
		gen_code(op_bne,0,make_label(lbl),0);
	}
	freeop(ap3);
	freeop(ap1);
	freeop(ap2);
	return(ap2);
	release();
}
int count_regs(AMODE *ap1, AMODE *ap2)
{
	int r = 0;
	switch(ap1->mode) {
		case am_baseindxaddr:
			if (ap1->sreg < cf_freeaddress && ap1->sreg != -1)
				r++;
		case am_baseindxdata:
		case am_ind:
		case am_indx:
		case am_areg:
			if (ap1->preg < cf_freeaddress && ap1->preg != -1)
				r++;
			break;
	}
	switch(ap2->mode) {
		case am_baseindxaddr:
			if (ap2->sreg < cf_freeaddress && ap2->sreg != -1)
				r++;
		case am_baseindxdata:
		case am_ind:
		case am_indx:
		case am_areg:
			if (ap2->preg < cf_freeaddress && ap2->preg != -1)
				r++;
			break;
	}
	return r;
}
AMODE    *gen_assign(ENODE *node, int flags, int size)
/*
 *      generate code for an assignment node. if the size of the
 *      assignment destination is larger than the size passed then
 *      everything below this node will be evaluated with the
 *      assignment size.
 */
{       AMODE    *ap1, *ap2,*ap3;
        int             ssize,rsize;
				int q = node->v.p[0]->nodetype;
				rsize = natural_size(node->v.p[1]);
				if (q == en_bits)
					q = node->v.p[0]->v.p[0]->nodetype;
        switch( q )        
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
                }
        if (chksize( size , rsize ))
                rsize = size;
        ap2 = gen_expr(node->v.p[1],F_DREG | F_FREG | F_IMMED,rsize);
				do_extend(ap2,rsize,ssize,flags);
				mark();
        ap1 = gen_expr(node->v.p[0],F_ALL | F_NOBIT,ssize);
				if (ssize > 4) {
					gen_codef(op_fmove,ssize,ap2,ap1);	
				}
				else	{
					if (node->v.p[0]->nodetype ==en_bits)
						bit_move(ap2,ap1,node->v.p[0],flags,ssize,size);
					else
        		gen_code(op_move,ssize,ap2,ap1);
				}
				freeop(ap1);
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
{       AMODE    *ap1, *ap2,*ap4;
        int             ssize,rsize;
				int q = node->v.p[0]->nodetype;
				rsize = natural_size(node->v.p[1]);
				if (q == en_bits)
					q = node->v.p[0]->v.p[0]->nodetype;
        switch( q )        
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
                }
        if (chksize( size , rsize ))
                rsize = size;
        ap1 = gen_expr(node->v.p[0],F_ALL | F_NOBIT,ssize);
				mark();
        ap2 = gen_expr(node->v.p[1],F_ALL,rsize);
				ap4 = xalloc(sizeof(AMODE));
				ap4->mode = am_ind;
				ap4->preg = ap1->preg;
				make_legal(ap2,flags,ssize);
				if (ssize > 4) {
					if (ap1->mode != am_freg && ap2->mode != am_freg) {
						AMODE *ap3 = temp_float();
						gen_codef(op_fmove,ssize,ap2,ap3);
        		gen_codef(op_fmove,ssize,ap3,ap4);
						freeop(ap1);
						freeop(ap2);
						return(ap3);
					}
					else 
						gen_codef(op_fmove,ssize,ap2,ap4);	
				}
				else	
					if (node->v.p[0]->nodetype ==en_bits)
						bit_move(ap2,ap4,node->v.p[0],flags,ssize,size);
					else
        		gen_code(op_move,ssize,ap2,ap4);
				freeop(ap2);
				release();
				do_extend(ap1,ssize,size,flags);
				make_legal(ap1,flags,size);
        return ap1;
}

AMODE    *gen_aincdec(ENODE *node, int flags, int size, int op)
/*
 *      generate an auto increment or decrement node. op should be
 *      either op_add (for increment) or op_sub (for decrement).
 */
{       AMODE    *ap1, *ap2, *ap3;
        int             siz1;
        siz1 = natural_size(node->v.p[0]);
        if( flags & F_NOVALUE )         /* dont need result */
                {
        				ap1 = gen_expr(node->v.p[0],F_ALL | F_NOBIT,siz1);
								if (node->v.p[0]->nodetype == en_bits) {
									ap2 = get_bitval(ap1,node->v.p[0],siz1);
									gen_code(op,siz1,make_immed(1),ap2);
									bit_move(ap2,ap1,node->v.p[0],flags,siz1,size);
								}
								else {
                	gen_code(op,siz1,make_immed((long)node->v.p[1]),ap1);
								}
                freeop(ap1);
									
                return ap1;
                }
				if (flags & F_DREG)
					ap3 = temp_data();
				else if (flags & F_AREG)
					ap3 = temp_addr();
				ap3->tempflag = 1;
        ap1 = gen_expr(node->v.p[0],F_ALL | F_NOBIT,siz1);
				if (node->v.p[0]->nodetype == en_bits) {
					ap2 = get_bitval(ap1,node->v.p[0],siz1);
					gen_code(op_move,siz1,ap2,ap3);
					gen_code(op,siz1,make_immed(1),ap2);
					bit_move(ap2,ap1,node->v.p[0],flags,siz1,size);
        	do_extend(ap3,siz1,size,0);
					make_legal(ap3,flags,size);
					freeop(ap2);
				}
				else {
						if (siz1 <=4 && ap1->mode == am_areg && ap1->preg >= cf_freeaddress && op == op_add) {
							if (op == op_add) {
								freeop(ap3);
								ap3 = ap1;
								ap3->mode = am_ainc;
								flags |= F_INDX;
							}
						}
						else {
							gen_code(op_move,siz1,ap1,ap3);
            	gen_code(op,siz1,make_immed((long)node->v.p[1]),ap1);
							freeop(ap1);
						}
				}
        do_extend(ap3,siz1,size,0);
				make_legal(ap3,flags,size);
        return ap3;
}

int push_param(ENODE *ep,int size)
/*
 *      push the operand expression onto the stack.
 */
{       AMODE    *ap,*ap1;
				int rv;
				switch (ep->nodetype) {
                case en_napccon:
												ep->v.p[0] = ((SYM *)ep->v.p[0])->name;
                case en_labcon:
                        ap = xalloc(sizeof(AMODE));
												if (prm_rel)
			  	              	ap->mode = am_pcindx;
												else {
													ap->mode = am_adirect;
													if (prm_smallcode)
														ap->preg = 2;
													else
														ap->preg = 4;
												}
                        ap->offset = ep;     /* use as constant node */
                        gen_code(op_pea,0,ap,0);
												rv = 4;
												break;
								case en_absacon:
                        ap = xalloc(sizeof(AMODE));
                        ap->mode = am_adirect;
												ep->v.i = ((SYM *)ep->v.p[0])->value.i;
												ap->preg = isshort(ep) ? 2 : 4;
                        ap->offset = ep;     /* use as constant node */
                        gen_code(op_pea,0,ap,0);
												rv = 4;
												break;
                case en_nacon:
												ep->v.p[0] = ((SYM *)ep->v.p[0])->name;
								case en_nalabcon:
                        ap = xalloc(sizeof(AMODE));
												if (prm_rel) {
	                        ap->preg = basereg;          /* frame pointer */
													if (prm_largedata) {
														ap1 = temp_addr();
														ap->mode = am_areg;
														gen_code(op_move,4,ap,ap1);
														ap->mode = am_immed;
														ap->offset = ep;
														gen_code(op_add,4,ap,ap1);
														ap = ap1;
														ap->mode = am_ind;
                    	    	gen_code(op_move,4,ap,push);
												    ap->mode = am_areg;
														freeop(ap);
													}
													else {
                        		ap->mode = am_indx;
	                        	ap->offset = ep;     /* use as constant node */
														gen_code(op_pea,0,ap,0);
													}
												}
												else {
													ap->mode = am_adirect;
					          	    ap->offset = ep;
													if (prm_smalldata) {
														ap->preg = 2;
													}
													else {
														ap->preg = 4;
													}
												  gen_code(op_pea,0,ap,0);
												}
												rv = 4;
												break;
								case en_cf:
								case en_floatref:
												ap = gen_expr(ep,F_FREG|F_INDX,6);
												if (ap->mode == am_freg)
													gen_codef(op_fmove,6,ap,push);
												else
													gen_code(op_move,4,ap,push);
												rv = 4;
												break;
								case en_cd:
								case en_doubleref:
												ap = gen_expr(ep,F_FREG,8);
												gen_codef(op_fmove,8,ap,push);
												rv = 8;
												break;
								case en_cld:
								case en_longdoubleref:
												ap = gen_expr(ep,F_FREG,8);
												gen_codef(op_fmove,10,ap,push);
												rv = 8;
												break;
								case en_rcon:
								case en_fcon:
								case en_lrcon:
												gen_codef(op_fmove,size,ap,push);
												rv = size;
												break;
								default:
												rv = 4;
      			  					ap = gen_expr(ep,F_ALL,4);
												if (ap->mode == am_freg)
													gen_codef(op_fmove,4,ap,push);
												else
													gen_code(op_move,4,ap,push);
												break;
				}
        freeop(ap);
	stackdepth += rv;
	return(rv);
}
int push_stackblock(ENODE *ep)
{
	AMODE *ap, *ap1, *ap2;
	int lbl;
	long sz = ep->size, tp;
	long v;
	if (!sz)
		return(0);
	if (sz & 1) {
		tp = 1;
	}
	else if (sz & 2) {
		tp = 2;
	}
	else {
		tp = 4;
	}
				switch (ep->nodetype) {
                case en_napccon:
												ep->v.p[0] = ((SYM *)ep->v.p[0])->name;
                case en_labcon:
                        ap = xalloc(sizeof(AMODE));
												if (prm_rel)
			  	              	ap->mode = am_pcindx;
												else {
													ap->mode = am_adirect;
													if (prm_smallcode)
														ap->preg = 2;
													else
														ap->preg = 4;
												}
                        ap->offset = makenode(en_add,ep,makenode(en_icon,(char *)sz,0));     /* use as constant node */
                        gen_lea(0,ap,ap2 = temp_addr());
												break;
								case en_absacon:
												ep->v.i = ((SYM *)ep->v.p[0])->value.i;
                        ap = xalloc(sizeof(AMODE));
                        ap->mode = am_adirect;
												ap->preg = isshort(ep) ? 2 : 4;
                        ap->offset = makenode(en_add,ep,makenode(en_icon,(char *)sz,0));     /* use as constant node */
                        gen_lea(0,ap,ap2 = temp_addr());
												break;
                case en_nacon:
												ep->v.p[0] = ((SYM *)ep->v.p[0])->name;
								case en_nalabcon:
												ap1 = 0;
                        ap = xalloc(sizeof(AMODE));
												if (prm_rel) {
	                        ap->preg = basereg;          /* frame pointer */
							
													if (prm_largedata) {
														ap1 = temp_addr();
														ap->mode = am_areg;
														gen_code(op_move,4,ap,ap1);
														ap->mode = am_immed;
														ap->offset = ep;
														gen_code(op_add,4,ap,ap1);
														ap = ap1;
														ap->mode = am_ind;
													}
													else {
                        		ap->mode = am_indx;
	                        	ap->offset = ep;     /* use as constant node */
													}
												}
												else {
													ap->mode = am_adirect;
					          	    ap->offset = ep;
													if (prm_smalldata) {
														ap->preg = 2;
													}
													else {
														ap->preg = 4;
													}
												}
                        ap->offset = makenode(en_add,ep,makenode(en_icon,(char *)sz,0));     /* use as constant node */
                        gen_lea(0,ap,ap2=temp_addr());
												if (ap1) {
													ap1->mode = am_areg;
													freeop(ap1);
												}
												break;
                case en_autocon:
                case en_autoreg:
												ep->v.i = ((SYM *)ep->v.p[0])->value.i;
                        ap = xalloc(sizeof(AMODE));
                        ap->mode = am_indx;
												if (prm_linkreg && !currentfunc->intflag) {
			                		ap->preg = linkreg;
                        	ap->offset = makenode(en_add,ep,makenode(en_icon,(char *)sz,0));     /* use as constant node */
												}
					 							else if (((SYM *)ep->v.p[0])->funcparm ) {
													if (prm_phiform || currentfunc->intflag) {
                        		ap->preg = linkreg;          /* frame pointer */
                        		ap->offset = makenode(en_add,ep,makenode(en_icon,(char *)sz,0));     /* use as constant node */
													}
													else {
														ap->preg = 7;
                        		ap->offset = makenode(en_add,ep,makenode(en_icon,(char *)(sz+stackdepth+framedepth),0));     /* use as constant node */
													}
                				}
												else {
													ap->preg = 7;
                        	ap->offset = makenode(en_add,ep,makenode(en_icon,(char *)(sz+stackdepth+lc_maxauto),0));     /* use as constant node */
												}
                        gen_lea(0,ap,ap2 = temp_addr());
												break;
								default:
												ap = 0;
      			  					ap2 = gen_expr(ep,F_AREG | F_VOL,4);
												gen_code(op_add,4,make_immed(sz),ap2);
												break;
				}
	lbl = nextlabel++;
	ap1 = temp_data();
  v = sz/tp;
	if (v < 65536)
		v--;
	gen_code(op_move,v < 65536 ? 2 : 4,make_immed(v),ap1);
	gen_label(lbl);
	ap2->mode = am_adec;
	gen_code(op_move,tp,ap2,push);
	if (v < 65536) {
		gen_code(op_dbra,0,ap1,make_label(lbl));
	}
	else {
		gen_code(op_sub,4,make_immed(1),ap1);
		gen_code(op_bne,0,make_label(lbl),0);
	}
	freeop(ap1);
	freeop(ap2);
	if (ap)
  	freeop(ap);

	stackdepth += sz;
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
{       AMODE    *ap, *result, *ap1;
        int             i,siz1;
        result = temp_addr();
        temp_addr();                    /* push any used addr temps */
        freeop(result); freeop(result);
        result = temp_data();
        temp_data(); temp_data();      /* push any used data registers */
        freeop(result); freeop(result); freeop(result);
				result = temp_float();  temp_float(); temp_float();
        freeop(result); freeop(result); freeop(result);
				if (node->nodetype == en_callblock) {
					i = gen_parms(node->v.p[1]->v.p[1]->v.p[1]->v.p[0],size);
      		ap = gen_expr(node->v.p[0],F_ALL,4);
					gen_code(op_move,4,ap,push);
					i+=4;
					stackdepth+=4;
					node=node->v.p[1];
					freeop(ap);
					siz1 = 4;
				}
				else {
	        i = gen_parms(node->v.p[1]->v.p[1]->v.p[0],size);    /* generate parameters */
					siz1 = node->v.p[0]->v.i;
				}
				if ((prm_phiform || node->nodetype == en_trapcall || node->nodetype == en_intcall) && i)
					gen_code(op_move,4,makeareg(7),makeareg(0));
				if (node->nodetype == en_trapcall) {
					gen_code(op_trap,0,make_immed(node->v.p[1]->v.p[0]->v.i),0);
				}
				else if (node->nodetype == en_intcall) {
					/* This is one case where we can generate code that will run
					 * on a 68000 but not on a 68020 or vice versa*/
					int curlabel = nextlabel++;
					DIAG("Direct calls to interrups not portable across processors");
					if (prm_68020) {
						gen_code(op_clr,2,push,0);
					}
					else
					ap1 = make_label(curlabel);
					if (prm_rel)
			  	 	ap1->mode = am_pcindx;
					else {
						ap1->mode = am_adirect;
					  if (prm_smallcode)
							ap1->preg = 2;
						else
							ap1->preg = 4;
					}
					gen_code(op_pea,4,ap1,0);
					ap1 = xalloc(sizeof(AMODE));
					ap1->mode = am_sr;
					gen_code(op_move,2,ap1,push);	/* May cause an exception on an 020 */
					gen_code(op_bra,0,make_offset(node->v.p[1]->v.p[0]),0);
					gen_label(curlabel);
					freeop(ap1);
				}
     	  else if( node->v.p[1]->v.p[0]->nodetype == en_nacon || node->v.p[1]->v.p[0]->nodetype == en_napccon ) {
					SYM * sp = node->v.p[1]->v.p[0]->v.p[0];
					if (sp->inreg) {
						if (sp->value.i < 8)
							ap1 = makedreg(sp->value.i);
						else
							ap1 = makeareg(sp->value.i & 7);
						make_legal(ap1,F_AREG,size);
						ap1->mode = am_ind;
						gen_code(op_jsr,0,ap1,0);
					}
					else {
						node->v.p[1]->v.p[0]->v.p[0] = sp->name;
						if (prm_rel)
       	    	gen_code(op_bsr,0,make_offset(node->v.p[1]->v.p[0]),0);
						else {
							ap1 = xalloc(sizeof(AMODE));
							ap1->mode = am_adirect;
          		ap1->offset = node->v.p[1]->v.p[0];
							if (prm_smallcode) {
								ap1->preg = 2;
							}
							else {
								ap1->preg = 4;
							}
							gen_code(op_jsr,0,ap1,0);
						}
					}
		    }
        else
                {
                ap = gen_expr(node->v.p[1]->v.p[0],F_AREG,4);
								ap->mode = am_ind;
                freeop(ap);
                gen_code(op_jsr,0,ap,0);
                }
        if( i != 0 ) {
								if (i > 8) {
									AMODE *ap = xalloc(sizeof(AMODE)); 
									ap->mode = am_indx;
									ap->offset = makenode(en_icon,(char *)i,0);
									ap->preg = 7;
                	gen_lea(0,ap,makeareg(7));
								}
								else
                	gen_code(op_add,4,make_immed(i),makeareg(7));
								stackdepth -= i;
				}
				if (siz1 > 4) {
						result = temp_float();
						if (result->preg != 0)
							gen_codef(op_fmove,8,makefreg(0),result);
				}
				else {
          result = temp_data();
        	if( result->preg != 0)
                gen_code(op_move,4,makedreg(0),result);
				}
				result->tempflag = 1;
				do_extend(result,siz1,size,flags);
				make_legal(result,flags,size);
        return result;
}
AMODE    *gen_pfcall(ENODE *node,int flags, int size)
/*
 *      generate a function call node for pascal function calls
	*			and return the address mode of the result.
 */
{       AMODE    *ap, *result, *ap1;
        int             i,siz1;
				ENODE * invnode = 0,*anode;
				
				/* invert the parameter list */
				if (node->nodetype == en_pcallblock)
					anode = node->v.p[1]->v.p[1]->v.p[1]->v.p[0];
				else
					anode = node->v.p[1]->v.p[1]->v.p[0];
				while (anode) {
					invnode = makenode(anode->nodetype,anode->v.p[0],invnode);
					anode = anode->v.p[1];
				}
        result = temp_addr();
        temp_addr();                    /* push any used addr temps */
        freeop(result); freeop(result);
        result = temp_data();
        temp_data(); temp_data();      /* push any used data registers */
        freeop(result); freeop(result); freeop(result);
				result = temp_float();  temp_float(); temp_float();
        freeop(result); freeop(result); freeop(result);
				if (node->nodetype == en_pcallblock) {
      		ap = gen_expr(node->v.p[0],F_ALL,4);
					gen_code(op_move,4,ap,push);
					freeop(ap);
					i =4;
					stackdepth+=4;
					i += gen_parms(invnode,size);
					node=node->v.p[1];
						siz1 = 4;
				}
				else {
	        i = gen_parms(invnode,size);    /* generate parameters */
					siz1 = node->v.p[0]->v.i;
				}
				if ((prm_phiform || node->nodetype == en_trapcall || node->nodetype == en_intcall) && i)
					gen_code(op_move,4,makeareg(7),makeareg(0));
     	  if( node->v.p[1]->v.p[0]->nodetype == en_nacon || node->v.p[1]->v.p[0]->nodetype == en_napccon ) {
					SYM * sp = node->v.p[1]->v.p[0]->v.p[0];
					if (sp->inreg) {
						if (sp->value.i < 8)
							ap1 = makedreg(sp->value.i);
						else
							ap1 = makeareg(sp->value.i & 7);
						make_legal(ap1,F_AREG,size);
						ap1->mode = am_ind;
						gen_code(op_jsr,0,ap1,0);
					}
					else {
						node->v.p[1]->v.p[0]->v.p[0] = sp->name;
						if (prm_rel)
       	    	gen_code(op_bsr,0,make_offset(node->v.p[1]->v.p[0]),0);
						else {
							ap1 = xalloc(sizeof(AMODE));
							ap1->mode = am_adirect;
          		ap1->offset = node->v.p[1]->v.p[0];
							if (prm_smallcode) {
								ap1->preg = 2;
							}
							else {
								ap1->preg = 4;
							}
							gen_code(op_jsr,0,ap1,0);
						}
					}
		    }
        else
                {
                ap = gen_expr(node->v.p[1]->v.p[0],F_AREG,4);
								ap->mode = am_ind;
                freeop(ap);
                gen_code(op_jsr,0,ap,0);
                }
				stackdepth -= i;
				if (siz1 > 4) {
						result = temp_float();
						if (result->preg != 0)
							gen_codef(op_fmove,8,makefreg(0),result);
				}
				else {
          result = temp_data();
        	if( result->preg != 0)
                gen_code(op_move,4,makedreg(0),result);
				}
				result->tempflag = 1;
				do_extend(result,siz1,size,flags);
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
												ap1 = gen_expr(node->v.p[0],flags | F_INDX | F_DREG | F_FREG,natural_size(node->v.p[0]));
                				do_extend(ap1,natural_size(node->v.p[0]),size,flags);
												make_legal(ap1,flags,size);
												return ap1;
                case en_napccon:
												node->v.p[0] = ((SYM *)node->v.p[0])->name;
                case en_labcon:
                        ap1 = temp_addr();
												ap1->tempflag = TRUE;
                        ap2 = xalloc(sizeof(AMODE));
												if (prm_rel)
										  	 	ap2->mode = am_pcindx;
												else {
													ap2->mode = am_adirect;
												  if (prm_smallcode)
														ap2->preg = 2;
													else
														ap2->preg = 4;
												}
                        ap2->offset = node;     /* use as constant node */
                        gen_lea(0,ap2,ap1);
                        make_legal(ap1,flags,size);
                        return ap1;             /* return reg */
								case en_absacon:
												node->v.i = ((SYM *)node->v.p[0])->value.i;
                        ap1 = temp_addr();
												ap1->tempflag = TRUE;
                        ap2 = xalloc(sizeof(AMODE));
                        ap2->mode = am_adirect;
												ap2->preg = isshort(node) ? 2 : 4;
                        ap2->offset = node;     /* use as constant node */
                        gen_lea(0,ap2,ap1);
                        make_legal(ap1,flags,size);
                        return ap1;             /* return reg */
                case en_nacon:
												node->v.p[0] = ((SYM *)node->v.p[0])->name;
								case en_nalabcon:
	                      ap1 = temp_addr();
												ap1->tempflag = TRUE;
    	                  ap2 = xalloc(sizeof(AMODE));
												if (prm_rel) {
  	  	                  ap2->preg = basereg;          /* frame pointer */
													if (prm_largedata) {
														ap2->mode = am_areg;
														gen_code(op_move,0,ap2,ap1);
														ap2->mode = am_immed;
														ap2->offset = node;
														gen_code(op_add,0,ap2,ap1);
													}
													else {
	                      	  ap2->mode = am_indx;
    	                    	ap2->offset = node;     /* use as constant node */
	      	                  gen_lea(0,ap2,ap1);
													}
												}
												else {
													ap2->mode = am_adirect;
					          	    ap2->offset = node;
													if (prm_smalldata) {
														ap2->preg = 2;
													}
													else {
														ap2->preg = 4;
													}
	      	                gen_lea(0,ap2,ap1);
												}
                        make_legal(ap1,flags,size);
                        return ap1;             /* return reg */
                case en_icon:
								case en_lcon: 
								case en_lucon: 
								case en_iucon: 
								case en_ccon:                 
								case en_rcon:
								case en_lrcon: case en_fcon:
                        ap1 = xalloc(sizeof(AMODE));
                        ap1->mode = am_immed;
                        ap1->offset = node;
                        make_legal(ap1,flags,size);
                        return ap1;
                case en_autocon:
                case en_autoreg:
                        ap1 = temp_addr();
												ap1->tempflag = TRUE;
                        ap2 = xalloc(sizeof(AMODE));
                        ap2->mode = am_indx;
												if (prm_linkreg && !currentfunc->intflag) {
			                		ap2->preg = linkreg;
      			          		ap2->offset = makenode(en_icon,(char *)((SYM *)node->v.p[0])->value.i,0);
												}
					 							else if (((SYM *)node->v.p[0])->funcparm ) {
													if (prm_phiform || currentfunc->intflag) {
                        		ap2->preg = linkreg;          /* frame pointer */
														ap2->offset = makenode(en_icon,(char *)(((SYM *)node->v.p[0])->value.i),0);
													}
													else {
														ap2->preg = 7;
														ap2->offset = makenode(en_icon,(char *)(((SYM *)node->v.p[0])->value.i+stackdepth+framedepth),0);
													}
                				}
												else {
													ap2->preg = 7;
													ap2->offset = makenode(en_icon,(char *)(((SYM *)node->v.p[0])->value.i+stackdepth+lc_maxauto),0);
												}
                        gen_lea(0,ap2,ap1);
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
                       	ap1 = gen_deref(node,flags,size,FALSE);
												return ap1;
                case en_tempref:
                case en_regref:
                        ap1 = xalloc(sizeof(AMODE));
                        if( (node->v.i & 0xff) < 16 )
                                {
                                ap1->mode = am_dreg;
                                ap1->preg = node->v.i & 0xff;
                                }
                        else
													if ((node->v.i &0xff) < 32)
                                {
                                ap1->mode = am_areg;
                                ap1->preg = (node->v.i & 0xff) - 16;
                                }
													else
                                {
                                ap1->mode = am_freg;
                                ap1->preg = (node->v.i &0xff)- 32;
                                }
                        ap1->tempflag = 0;      /* not a temporary */
                				do_extend(ap1,node->v.i >> 8,size,flags);
                        make_legal(ap1,flags,size);
                        return ap1;
								case en_bits:
												size = natural_size(node->v.p[0]);
												ap1 = gen_expr(node->v.p[0],F_ALL,size);
												if (!(flags & F_NOBIT))
													bit_legal(ap1,node,size);
												return ap1;
                case en_uminus:
                        return gen_unary(node,flags,size,op_neg, op_fneg);
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
						return gen_xbin(node,flags,size,op_eor,op_eor);
								case en_pmul:
												return gen_pmul(node,flags,size);
								case en_pdiv:
												return gen_pdiv(node,flags,size);
                case en_mul:
                        return gen_mul(node,flags,size,op_muls);
                case en_umul:
                        return gen_mul(node,flags,size,op_mulu);
                case en_div:
                        return gen_modiv(node,flags,size,op_divs,0);
                case en_udiv:
                        return gen_modiv(node,flags,size,op_divu,0);
                case en_mod:
                        return gen_modiv(node,flags,size,op_divs,1);
                case en_umod:
                        return gen_modiv(node,flags,size,op_divu,1);
                case en_alsh:
                        return gen_shift(node,flags,size,op_asl);
                case en_arsh:
                        return gen_shift(node,flags,size,op_asr);
                case en_lsh:
                        return gen_shift(node,flags,size,op_lsl);
                case en_rsh:
                        return gen_shift(node,flags,size,op_lsr);
                case en_asadd:
                        return gen_asadd(node,flags,size,op_add,op_fadd);
                case en_assub:
                        return gen_asadd(node,flags,size,op_sub,op_fsub);
                case en_asand:
                        return gen_aslogic(node,flags,size,op_and);
                case en_asor:
                        return gen_aslogic(node,flags,size,op_or);
                case en_asxor:
                        return gen_aslogic(node,flags,size,op_eor);
                case en_aslsh:
                        return gen_asshift(node,flags,size,op_lsl);
                case en_asrsh:
                        return gen_asshift(node,flags,size,op_lsr);
                case en_asalsh:
                        return gen_asshift(node,flags,size,op_asl);
                case en_asarsh:
                        return gen_asshift(node,flags,size,op_asr);
                case en_asmul:
                        return gen_asmul(node,flags,size, op_muls);
                case en_asumul:
                        return gen_asmul(node,flags,size,op_mulu);
                case en_asdiv:
                        return gen_asmodiv(node,flags,size,op_divs,FALSE);
                case en_asudiv:
                        return gen_asmodiv(node,flags,size,op_divu,FALSE);
                case en_asmod:
                        return gen_asmodiv(node,flags,size,op_divs,TRUE);
                case en_asumod:
                        return gen_asmodiv(node,flags,size,op_divu,TRUE);
                case en_assign:
                        return gen_assign(node,flags,size);
                case en_refassign:
                        return gen_refassign(node,flags | F_NOVALUE,size);
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
                        gen_code(op_moveq,0,make_immed(1),ap1);
                        gen_code(op_bra,0,make_label(lab1),0);
                        gen_label(lab0);
                        gen_code(op_clr,4,ap1,0);
                        gen_label(lab1);
                        return ap1;
                case en_cond:
                        return gen_hook(node,flags,size);
                case en_void:
                        natsize = natural_size(node->v.p[0]);
                        freeop(gen_expr(node->v.p[0],F_ALL | F_NOVALUE,natsize));
                        return gen_expr(node->v.p[1],flags,size);
								case en_pfcall: case en_pfcallb:
								case en_pcallblock:
                        return gen_pfcall(node,flags,size);
                case en_fcall:  case en_callblock: case en_fcallb:
                case en_intcall: 
                case en_trapcall:
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
{       int     siz0, siz1;
        if( node == 0 )
                return 0;
        switch( node->nodetype )
                {
								case en_bits:
												return stdintsize;
                case en_icon:
								case en_lcon: 
								case en_lucon: 
                case en_iucon:
												return 0;
								case en_ccon:                 
												return 0;
								case en_rcon:
								case en_doubleref:
												return 8;
								case en_lrcon:
								case en_longdoubleref:
												return stdldoublesize;
								case en_fcon:
								case en_floatref:
												return 6;
								case en_trapcall:
                case en_labcon:
                case en_nacon:  case en_autocon:  case en_autoreg:
                case en_napccon:  case en_absacon: case en_nalabcon:
												return stdaddrsize;
								case en_l_ref:
								case en_cl:
												return -4;
								case en_pfcall: case en_pfcallb: /* ignore pascal style now */
								case en_pcallblock:
								case en_fcall: case en_callblock: case en_fcallb:
								case en_intcall:
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
												return 10;
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
                        siz1 = natural_size(node->v.p[1]);
                        if( chksize(siz1,siz0))
                                return siz1;
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
        	ap1 = gen_expr(node->v.p[0],F_FREG,size);
					mark();
        	ap2 = gen_expr(node->v.p[1],F_ALL,size);
        	gen_code(op_fcmp,size,ap2,ap1);
        	gen_code(btype3,0,make_label(label),0);
					freeop(ap2);
					freeop(ap1);
					release();
					return;
				}
        ap1 = gen_expr(node->v.p[0],F_ALL,size);
				mark();
        ap2 = gen_expr(node->v.p[1],F_ALL,size);
				if (ap1->mode != am_dreg && ap1->mode != am_areg) {
					if (ap2->mode == am_immed)  {
						if (ap1->mode == am_immed)
							goto cmp2;
					}
					else {
						if (ap2->mode == am_dreg || ap2->mode == am_areg) {
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
								gen_code(op_move,size,ap3,ap1);
      	        make_legal(ap1,F_DREG,size);
							}
					}
				}
        gen_code(op_cmp,size,ap2,ap1);
        gen_code(btype,0,make_label(label),0);
				if (ap3) {
					freeop(ap1);
					freeop(ap2);
					freeop(ap3);
				}
				else {	
        	freeop(ap2);
        	freeop(ap1);
				}
				release();
}

void truejp(ENODE *node, int label)
/*
 *      generate a jump to label if the node passed evaluates to
 *      a true condition.
 */
{       AMODE    *ap1;
        int             siz1;
        int             lab0;
        if( node == 0 )
                return;
        switch( node->nodetype )
                {
                case en_eq:
                        gen_compare(node,op_beq,op_beq,op_fbeq,op_fbeq,label);
                        break;
                case en_ne:
                        gen_compare(node,op_bne,op_bne,op_fbne,op_fbne,label);
                        break;
                case en_lt:
                        gen_compare(node,op_blt,op_bgt,op_fblt,op_fbgt,label);
                        break;
                case en_le:
                        gen_compare(node,op_ble,op_bge,op_fble,op_fbge,label);
                        break;
                case en_gt:
                        gen_compare(node,op_bgt,op_blt,op_fbgt,op_fblt,label);
                        break;
                case en_ge:
                        gen_compare(node,op_bge,op_ble,op_fbge,op_fble,label);
                        break;
                case en_ult:
                        gen_compare(node,op_blo,op_bhi,op_fblt,op_fbgt,label);
                        break;
                case en_ule:
                        gen_compare(node,op_bls,op_bhs,op_fble,op_fbge,label);
                        break;
                case en_ugt:
                        gen_compare(node,op_bhi,op_blo,op_fbgt,op_fblt,label);
                        break;
                case en_uge:
                        gen_compare(node,op_bhs,op_bls,op_fbge,op_fble,label);
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
                        siz1 = natural_size(node);    
												if (isintconst(node->nodetype)) {
													if (node->v.i != 0)
                        		gen_code(op_bra,0,make_label(label),0);
						              break;
												}
												else {
													if (siz1 > 4)
                        		ap1 = gen_expr(node,F_FREG,siz1);
													else
                        		ap1 = gen_expr(node,F_DREG | F_INDX,siz1);
												}
												if (siz1 > 4) {
                        	gen_codef(op_ftst,siz1,ap1,0);
                        	freeop(ap1);
                        	gen_code(op_fbne,0,make_label(label),0);
												}
												else {
                        	gen_code(op_tst,siz1,ap1,0);
                        	freeop(ap1);
                        	gen_code(op_bne,0,make_label(label),0);
												}
                        break;
                }
}

void falsejp(ENODE *node, int label)
/*
 *      generate code to execute a jump to label if the expression
 *      passed is false.
 */
{       AMODE    *ap;
        int             siz1;
        int             lab0;
        if( node == 0 )
                return;
        switch( node->nodetype )
                {
                case en_eq:
                        gen_compare(node,op_bne,op_bne,op_fbne,op_fbne,label);
                        break;
                case en_ne:
                        gen_compare(node,op_beq,op_beq,op_fbeq,op_fbeq,label);
                        break;
                case en_lt:
                        gen_compare(node,op_bge,op_ble,op_fbge,op_fble,label);
                        break;
                case en_le:
                        gen_compare(node,op_bgt,op_blt,op_fbgt,op_fblt,label);
                        break;
                case en_gt:
                        gen_compare(node,op_ble,op_bge,op_fble,op_fbge,label);
                        break;
                case en_ge:
                        gen_compare(node,op_blt,op_bgt,op_fblt,op_fbgt,label);
                        break;
                case en_ult:
                        gen_compare(node,op_bhs,op_bls,op_fbge,op_fble,label);
                        break;
                case en_ule:
                        gen_compare(node,op_bhi,op_blo,op_fbgt,op_fblt,label);
                        break;
                case en_ugt:
                        gen_compare(node,op_bls,op_bhs,op_fble,op_fbge,label);
                        break;
                case en_uge:
                        gen_compare(node,op_blo,op_bhi,op_fblt,op_fbgt,label);
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
                        siz1 = natural_size(node);    
												if (isintconst(node->nodetype)) {
													if (node->v.i == 0)
                        		gen_code(op_bra,0,make_label(label),0);
						              break;
												}
												else{
													if (siz1 > 4)
                        		ap = gen_expr(node,F_FREG,siz1);
													else
                        		ap = gen_expr(node,F_DREG | F_INDX,siz1);
												}
												if (siz1 > 4) {
                        	gen_codef(op_ftst,siz1,ap,0);
                        	freeop(ap);
                        	gen_code(op_fbeq,0,make_label(label),0);
												}
												else {
                        	gen_code(op_tst,siz1,ap,0);
                        	freeop(ap);
                        	gen_code(op_beq,0,make_label(label),0);
												}
                        break;
                }
}