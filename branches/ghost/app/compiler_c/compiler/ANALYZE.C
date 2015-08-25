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
#include				"list.h"
#include        "expr.h"
#include        "c.h"
#include        "errors.h"
#include 				"diag.h"

extern int cf_freeaddress,cf_freedata,cf_freefloat;
extern long framedepth,stackdepth;
extern LIST *varlisthead;
extern int lc_maxauto;
extern int stackadd, stackmod;

int floatregs = 1,dataregs=1,addrregs = 1;
CSE       *olist;         /* list of optimizable expressions */
/*
 *      this module will step through the parse tree and find all
 *      optimizable expressions. at present these expressions are
 *      limited to expressions that are valid throughout the scope
 *      of the function. the list of optimizable expressions is:
 *
 *              constants
 *              global and static addresses
 *              auto addresses
 *              contents of auto addresses.
 *
 *      contents of auto addresses are valid only if the address is
 *      never referred to without dereferencing.
 *
 *      scan will build a list of optimizable expressions which
 *      opt1 will replace during the second optimization pass.
 */


int equalnode(ENODE *node1, ENODE *node2)
/*
 *      equalnode will return 1 if the expressions pointed to by
 *      node1 and node2 are equivalent.
 */
{       if( node1 == 0 || node2 == 0 )
                return 0;
        if( node1->nodetype != node2->nodetype )
                return 0;
				if (natural_size(node1) != natural_size(node2))
								return 0;
        if( (isintconst(node1->nodetype) || node1->nodetype == en_labcon ||
						node1->nodetype == en_autoreg || node1->nodetype == en_nalabcon ||
            node1->nodetype == en_nacon || node1->nodetype == en_napccon
						|| node1->nodetype == en_autocon || node1->nodetype == en_absacon) &&
            node1->v.i == node2->v.i )
                return 1;
				if ((node1->nodetype==en_rcon ||node1->nodetype==en_lrcon ||node1->nodetype==en_fcon) && node1->v.f == node2->v.f)
					return 1;
        if( lvalue(node1) && equalnode(node1->v.p[0], node2->v.p[0]) )
                return 1;
        return 0;
}

CSE *searchnode(ENODE *node)
/*
 *      searchnode will search the common expression table for an entry
 *      that matches the node passed and return a pointer to it.
 */
{       CSE     *csp;
        csp = olist;
        while( csp != 0 ) {
                if( equalnode(node,csp->exp) )
                        return csp;
                csp = csp->next;
                }
        return 0;
}

ENODE *copynode(ENODE *node)
/*
 *      copy the node passed into a new enode so it wont get
 *      corrupted during substitution.
 */
{       ENODE    *temp;
        if( node == 0 )
                return 0;
        temp = xalloc(sizeof(ENODE));
				temp->cflags = node->cflags;
        temp->nodetype = node->nodetype;
        temp->v.p[0] = node->v.p[0];
        temp->v.p[1] = node->v.p[1];
        return temp;
}

CSE *enternode(ENODE *node,int duse,int size)
/*
 *      enternode will enter a reference to an expression node into the
 *      common expression table. duse is a flag indicating whether or not
 *      this reference will be dereferenced.
 */
{       CSE      *csp;
				if (size == 0)
					size = natural_size(node);
        if( (csp = searchnode(node)) == 0 ) {   /* add to tree */
                csp = xalloc(sizeof(CSE));
                csp->next = olist;
                csp->uses = 1;
								csp->reg = -1;
                csp->duses = (duse != 0);
                csp->exp = copynode(node);
                csp->voidf = 0;
								csp->size = size;
                olist = csp;
                return csp;
                }
				else
					if (chksize(csp->size,size))
						csp->size = size;
        ++(csp->uses);
        if( duse )
                ++(csp->duses);
        return csp;
}

CSE *voidauto(ENODE *node)
/*
 *      voidauto will void an auto dereference node which points to
 *      the same auto constant as node.
 */
{       CSE      *csp;
        csp = olist;
        while( csp != 0 ) {
                if( lvalue(csp->exp) && equalnode(node,csp->exp->v.p[0]) ) {
                        if( csp->voidf )
                                return 0;
                        csp->voidf = 1;
                        return csp;
                        }
                csp = csp->next;
                }
        return 0;
}

void voidall(void)
/*
 * Go through the CSP list voiding any use of a variable that
 * has its address taken.
 */
{
				CSE *csp;
				csp = olist;
				while (csp) {
					if (csp->exp->nodetype == en_autocon || csp->exp->nodetype == en_autoreg) {
						CSE *two = voidauto(csp->exp);
						if (two) {
							csp->uses += two->duses;
						}
					}
					csp = csp->next;
				}
}
void scanexpr(ENODE *node, int duse,int size)
/*
 *      scanexpr will scan the expression pointed to by node for optimizable
 *      subexpressions. when an optimizable expression is found it is entered
 *      into the tree. if a reference to an autocon node is scanned the
 *      corresponding auto dereferenced node will be voided. duse should be
 *      set if the expression will be dereferenced.
 */
{       CSE      *csp, *csp1;
				int sz;
        if( node == 0 )
                return;
        switch( node->nodetype ) {
                case en_rcon: case en_lrcon: case en_fcon:
                case en_icon:
								case en_lcon:
								case en_iucon:
								case en_lucon:
								case en_ccon:
                        enternode(node,duse,size);
                        break;
                case en_napccon:
                case en_nacon:
								case en_absacon:
                case en_autocon:
                case en_autoreg:
                        enternode(node,duse,4);
                        break;
								case en_bits:
											scanexpr(node->v.p[0],duse,0);
											break;
								case en_floatref:
								case en_doubleref:
								case en_longdoubleref:
                case en_b_ref:
                case en_w_ref:
                case en_ul_ref:
                case en_l_ref:
                case en_ub_ref:
                case en_uw_ref:
												if (node->v.p[0]->nodetype == en_autocon ||
														node->v.p[0]->nodetype == en_autoreg)
															enternode(node,duse,size);
												else
                          scanexpr(node->v.p[0],1,natural_size(node));
                        break;
                case en_uminus:
                case en_compl:  case en_ainc:
                case en_adec:   case en_not:
                        scanexpr(node->v.p[0],duse,0);
                        break;
								case en_cb: case en_cub:
								case en_cw: case en_cuw:
								case en_cl: case en_cul:
								case en_cf: case en_cd: case en_cp: case en_cld:
                        scanexpr(node->v.p[0],duse,natural_size(node));
                        break;
                case en_asadd:  case en_assub:
												size = natural_size(node->v.p[0]);
                        scanexpr(node->v.p[0],duse,0);
                        scanexpr(node->v.p[1],duse,size);
                        break;
                case en_add:    case en_sub:
                        scanexpr(node->v.p[0],duse,0);
                        scanexpr(node->v.p[1],duse,0);
                        break;
								case en_asalsh: case en_asarsh: case en_alsh: case en_arsh:
                case en_asmul:  case en_asdiv:
                case en_asmod:  case en_aslsh:
								case en_asumod: case en_asudiv: case en_asumul:
                case en_asrsh:  case en_asand:
                case en_assign: case en_refassign:
								case en_asor:  case en_asxor:
												size = natural_size(node->v.p[0]);
                        scanexpr(node->v.p[0],0,0);
                        scanexpr(node->v.p[1],0,size);
                        break;
								case en_void:
								case en_pmul:		case en_pdiv:
                case en_mul:    case en_div:
                case en_umul:    case en_udiv: case en_umod:
                case en_lsh:    case en_rsh:
                case en_mod:    case en_and:
                case en_or:     case en_xor:
                case en_lor:    case en_land:
                case en_eq:     case en_ne:
                case en_gt:     case en_ge:
                case en_lt:     case en_le:
								case en_ugt:	case en_uge: case en_ult: case en_ule:
                case en_cond:
								case en_moveblock: case en_stackblock: case en_callblock:
								case en_pcallblock:
                        scanexpr(node->v.p[0],0,0);
                        scanexpr(node->v.p[1],0,0);
                        break;
                case en_fcall: case en_intcall: case en_fcallb:
								case en_pfcall: case en_pfcallb:
                case en_trapcall:
/*                        scanexpr(node->v.p[0],1,0);
*/                        scanexpr(node->v.p[1],0,0);
                        break;
                }
}

void scan(SNODE *block)
/*
 *      scan will gather all optimizable expressions into the expression
 *      list for a block of statements.
 */
{       while( block != 0 ) {
                switch( block->stype ) {
                        case st_return:
                        case st_expr:
                                opt4(&block->exp);
                                scanexpr(block->exp,0,0);
                                break;
                        case st_while:
                        case st_do:
                                opt4(&block->exp);
                                scanexpr(block->exp,0,0);
                                scan(block->s1);
                                break;
                        case st_for:
                                opt4(&block->label);
                                scanexpr(block->label,0,0);
                                opt4(&block->exp);
                                scanexpr(block->exp,0,0);
                                scan(block->s1);
                                opt4(&block->s2);
                                scanexpr(block->s2,0,0);
                                break;
                        case st_if:
                                opt4(&block->exp);
                                scanexpr(block->exp,0,0);
                                scan(block->s1);
                                scan(block->s2);
                                break;
                        case st_switch:
                                opt4(&block->exp);
                                scanexpr(block->exp,0,0);
                                scan(block->s1);
                                break;
                        case st_case:
                                scan(block->s1);
                                break;
												case st_block:
																scan(block->exp);
																break;
												case st_asm:
																if (block->exp)
																	asm_scan(block->exp);
																break;
                        }
                block = block->next;
                }
}

void exchange(CSE **c1)
/*
 *      exchange will exchange the order of two expression entries
 *      following c1 in the linked list.
 */
{       CSE      *csp1, *csp2;
        csp1 = *c1;
        csp2 = csp1->next;
        csp1->next = csp2->next;
        csp2->next = csp1;
        *c1 = csp2;
}

int     desire(CSE *csp)
/*
 *      returns the desirability of optimization for a subexpression.
 */
{       if( csp->voidf || (isintconst(csp->exp->nodetype) &&
                        csp->exp->v.i < 16 && csp->exp->v.i >= 0))
                return 0;
        if( lvalue(csp->exp) )
                return 2 * csp->uses;
        return csp->uses;
}

int     bsort(CSE **list)
/*
 *      bsort implements a bubble sort on the expression list.
 */
{       CSE      *csp1, *csp2;
        csp1 = *list;
        if( csp1 == 0 || csp1->next == 0 )
                return 0;
        bsort( &(csp1->next));
        csp2 = csp1->next;
        if( desire(csp1) < desire(csp2) ) {
                exchange(list);
                return 1;
                }
        return 0;
}

void repexpr(ENODE *node, int size)
/*
 *      repexpr will replace all allocated references within an expression
 *      with tempref nodes.
 */
{       CSE      *csp;
        if( node == 0 )
                return;
				if (size == 0)
					size = natural_size(node);
        switch( node->nodetype ) {
                case en_rcon: case en_lrcon: case en_fcon:
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
                        if( (csp = searchnode(node)) != 0 )
                                if( csp->reg > 0 ) {
                                        node->nodetype = en_tempref;
                                        node->v.i = csp->reg | (size << 8);
                                        }
                        break;
								case en_floatref:
								case en_doubleref:
								case en_longdoubleref:
                case en_ub_ref:
                case en_uw_ref:
                case en_b_ref:
                case en_w_ref:
                case en_l_ref:
                case en_ul_ref:
                        if( (csp = searchnode(node)) != 0 ) {
                                if( csp->reg > 0 ) {
                                        node->nodetype = en_tempref;
                                        node->v.i = csp->reg | (size << 8);
                                        }
                                else
                                        repexpr(node->v.p[0],0);
                                }
                        else
                                repexpr(node->v.p[0],0);
                        break;
                case en_uminus: case en_bits:
                case en_not:    case en_compl:
                case en_ainc:   case en_adec:
                        repexpr(node->v.p[0],0);
                        break;
								case en_cb: case en_cub:
								case en_cw: case en_cuw:
								case en_cl: case en_cul:
								case en_cf: case en_cd: case en_cp: case en_cld:
                        repexpr(node->v.p[0],natural_size(node));
                        break;
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
                case en_asadd:  case en_assub:
                case en_asmul:  case en_asdiv:
                case en_asor:   case en_asand:   case en_asxor:
                case en_asmod:  case en_aslsh:
								case en_asumod: case en_asudiv: case en_asumul: case en_pmul:
                case en_asrsh:  case en_fcall: case en_trapcall: case en_pdiv:
								case en_pfcall: case en_pfcallb:
                case en_assign: case en_intcall: case en_fcallb: case en_refassign:
                case en_moveblock: case en_stackblock: case en_callblock:
								case en_pcallblock:
                        repexpr(node->v.p[0],0);
                        repexpr(node->v.p[1],0);
                        break;
                }
}

void repcse(SNODE *block)
/*
 *      repcse will scan through a block of statements replacing the
 *      optimized expressions with their temporary references.
 */
{       while( block != 0 ) {
                switch( block->stype ) {
                        case st_return:
                        case st_expr:
                                repexpr(block->exp,0);
                                break;
                        case st_while:
                        case st_do:
                                repexpr(block->exp,0);
                                repcse(block->s1);
                                break;
                        case st_for:
                                repexpr(block->label,0);
                                repexpr(block->exp,0);
                                repcse(block->s1);
                                repexpr(block->s2,0);
                                break;
                        case st_if:
                                repexpr(block->exp,0);
                                repcse(block->s1);
                                repcse(block->s2);
                                break;
                        case st_switch:
                                repexpr(block->exp,0);
                                repcse(block->s1);
                                break;
                        case st_case:
                                repcse(block->s1);
                                break;
												case st_block:
																repcse(block->exp);
																break;
												case st_asm:
																if (block->exp)
																	asm_repcse(block->exp);
																break;
                        }
                block = block->next;
                }
}
void allocstack(void)
/*
 * This allocates space for local variables
 * we do this AFTER the register optimization so that we can
 * avoid allocating space on the stack twice
 */
{
	int lvl = 0;
	int again = TRUE;
	int oldauto, max;
	lc_maxauto = max = 0;
	while (again) {
		LIST *t = varlisthead;
		int align;
		lvl ++;
		oldauto = max;
		again = FALSE;
		while (t) {
			SYM *sp = t->data;
			while (sp && (sp->inreg || sp->funcparm || sp->storage_class == sc_static || sp->storage_class == sc_label || sp->storage_class == sc_ulabel))
				sp = sp->next;
			if (!sp) {
				t = t->link;
				continue;
			}
			if (sp->value.i >= lvl)
				again = TRUE;
			if (sp->value.i == lvl) {
				lc_maxauto = oldauto;
				while (sp) {
					if (!sp->inreg && !sp->funcparm && sp->storage_class != sc_static) {
						int val;
						align = getalign(sc_auto,sp->tp);
						val = lc_maxauto % align;
						if (val != 0)
							lc_maxauto += align - val;
						lc_maxauto += sp->tp->size;
						sp->value.i = -lc_maxauto;
					}
					sp = sp->next;
				}
			}
			if (lc_maxauto > max)
				max = lc_maxauto;
			t = t->link;
		}
	}
	lc_maxauto = max;
	lc_maxauto += stackadd;
	lc_maxauto &= stackmod;
}
void opt1(SNODE *block)
/*
 *      opt1 is the externally callable optimization routine. it will
 *      collect and allocate common subexpressions and substitute the
 *      tempref for all occurrances of the expression within the block.
 *
 *		optimizer is currently turned off...
 */
{
		int datareg = cf_freedata;
		int addreg = 16 + cf_freeaddress;
		int floatreg = 32 + cf_freefloat;
		olist = 0;
        scan(block);            /* collect expressions */
				voidall();							/* Disallow regs whose address is taken  */
				voidfloat(block);						/* disallow optimizing things which are assigned values derived from floats */
				reserveregs(&datareg, &addreg, &floatreg);		/* Allocate register vars */
        allocate(datareg, addreg,floatreg,block);             /* allocate registers  for opt*/
        repcse(block);          /* replace allocated expressions */
				loadregs();						/* Initialize allocated regs */
}