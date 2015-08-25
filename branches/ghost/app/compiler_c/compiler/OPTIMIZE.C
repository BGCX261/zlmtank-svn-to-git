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
 * this module combines constants at compile time.  Integer constants
 * will get combined slightly better than floating point ones
 */
#include        <stdio.h>
#include        "expr.h"
#include        "c.h"

int maxinttype(ENODE *ep1, ENODE *ep2)
{
	int type1=ep1->nodetype;
	int type2 = ep2->nodetype;
	if (type1 == en_lucon || type2 == en_lucon)
		return en_lucon;
	if (type1 == en_lcon || type2 == en_lcon)
		return en_lcon;
	if (type1 == en_iucon || type2 == en_iucon)
		return en_iucon;
	if (type1 == en_icon || type2 == en_icon)
		return en_icon;
	return en_ccon;
}
int maxfloattype(ENODE *ep1, ENODE *ep2)
{
	int type1=ep1->nodetype;
	int type2 = ep2->nodetype;
	if (type1 == en_lrcon || type2 == en_lrcon)
		return en_lrcon;
	if (type1 == en_rcon || type2 == en_rcon)
		return en_rcon;
	return en_fcon;
}
int getmode(ENODE *ep1,ENODE *ep2)
/*
 * get the constant mode of a pair of nodes
 * 0 = Neither node is a constant
 * 1 = icon,icon
 * 2 = icon,rcon
 * 3 = rcon,icon
 * 4 = rcon,rcon
 * 5 = icon,nothing
 * 6 = rcon,nothing
 * 7 = nothing,icon
 * 8 = nothing,rcon
 */
{
				int mode = 0;
				if (isintconst(ep1->nodetype) )
					if (ep2)  {
						if (isintconst(ep2->nodetype))
							mode = 1;
						else  if (ep2->nodetype == en_rcon || ep2->nodetype == en_lrcon || ep2->nodetype == en_fcon)
							mode = 2;
						else mode = 5;
					}
					else
						mode = 5;
				else if (ep1->nodetype == en_rcon || ep1->nodetype == en_lrcon || ep1->nodetype == en_fcon)
					if (ep2)  {
						if (isintconst(ep2->nodetype))
							mode = 3;
						else  if (ep2->nodetype == en_rcon || ep2->nodetype == en_lrcon || ep2->nodetype == en_fcon)
							mode = 4;
						else mode = 6;
					}
					else
						mode = 6;
				else if (ep2)
					if (isintconst(ep2->nodetype))
						mode = 7;
						else  if (ep2->nodetype == en_rcon || ep2->nodetype == en_lrcon || ep2->nodetype == en_fcon)
						mode = 8;
	return(mode);
}
	
void dooper(ENODE ** node, int mode)
/*
 *      dooper will execute a constant operation in a node and
 *      modify the node to be the result of the operation.
 *			It will also cast integers to floating point values when
 *			necessary
 */                             
{     ENODE    *ep,*ep1,*ep2;
      ep = *node;
			ep1 = ep->v.p[0];
			ep2 = ep->v.p[1];
			if (mode ==5) {
					if (floatrecurse(ep2)) {
					ep1->v.f = ep1->v.i;
					ep1->nodetype = en_rcon;
				}
				return;
			}
			else if (mode == 7) {
				if (floatrecurse(ep1)) {
					ep2->v.f = ep2->v.i;
					ep2->nodetype = en_rcon;
				}
				return;
			}
			else if (mode == 6 || mode == 8)
				return;
			else
        switch( ep->nodetype ) {
                case en_add:
										 		switch (mode) {
													case 1:	
                        		ep->nodetype = maxinttype(ep1,ep2);
                        		ep->v.i = ep1->v.i + ep2->v.i;
														break;
													case 2:	
                        		ep->nodetype = maxfloattype(ep1,ep2);
                        		ep->v.f = ep1->v.i + ep2->v.f;
														break;
													case 3:	
                        		ep->nodetype = maxfloattype(ep1,ep2);
                        		ep->v.f = ep1->v.f + ep2->v.i;
														break;
													case 4:	
                        		ep->nodetype = maxfloattype(ep1,ep2);
                        		ep->v.f = ep1->v.f + ep2->v.f;
														break;
                        }
												break;
                case en_sub:
										 		switch (mode) {
													case 1:	
                        		ep->nodetype = maxinttype(ep1,ep2);
                        		ep->v.i = ep1->v.i - ep2->v.i;
														break;
													case 2:	
                        		ep->nodetype = maxfloattype(ep1,ep2);
                        		ep->v.f = ep1->v.i - ep2->v.f;
														break;
													case 3:	
                        		ep->nodetype = maxfloattype(ep1,ep2);
                        		ep->v.f = ep1->v.f - ep2->v.i;
														break;
													case 4:	
                        		ep->nodetype = maxfloattype(ep1,ep2);
                        		ep->v.f = ep1->v.f - ep2->v.f;
														break;
												}
                        break;
								case en_pmul:
								case en_umul:
                case en_mul:
										 		switch (mode) {
													case 1:	
                        		ep->nodetype = maxinttype(ep1,ep2);
                        		ep->v.i = ep1->v.i * ep2->v.i;
														break;
													case 2:	
                        		ep->nodetype = maxfloattype(ep1,ep2);
                        		ep->v.f = ep1->v.i * ep2->v.f;
														break;
													case 3:	
                        		ep->nodetype = maxfloattype(ep1,ep2);
                        		ep->v.f = ep1->v.f * ep2->v.i;
														break;
													case 4:	
                        		ep->nodetype = maxfloattype(ep1,ep2);
                        		ep->v.f = ep1->v.f * ep2->v.f;
														break;
												}
                        break;
								case en_pdiv:
                case en_div:
                case en_udiv:
										 		switch (mode) {
													case 1:	
                        		ep->nodetype = maxinttype(ep1,ep2);
                        		ep->v.i = ep1->v.i / ep2->v.i;
														break;
													case 2:	
                        		ep->nodetype = maxfloattype(ep1,ep2);
                        		ep->v.f = ep1->v.i / ep2->v.f;
														break;
													case 3:	
                        		ep->nodetype = maxfloattype(ep1,ep2);
                        		ep->v.f = ep1->v.f / ep2->v.i;
														break;
													case 4:	
                        		ep->nodetype = maxfloattype(ep1,ep2);
                        		ep->v.f = ep1->v.f / ep2->v.f;
														break;
												}
                        break;
								case en_mod:
                        		ep->nodetype = maxinttype(ep1,ep2);
												ep->v.i = ep1->v.i % ep2->v.i;
												break;
                case en_lsh:
								case en_alsh:
                        		ep->nodetype = maxinttype(ep1,ep2);
                        ep->v.i = ep1->v.i << ep2->v.i;
                        break;
                case en_rsh:
								case en_arsh:
                        		ep->nodetype = maxinttype(ep1,ep2);
                        ep->v.i = ep1->v.i >> ep2->v.i;
                        break;
                case en_and:
                        		ep->nodetype = maxinttype(ep1,ep2);
                        ep->v.i = ep1->v.i & ep2->v.i;
                        break;
                case en_or:
                        		ep->nodetype = maxinttype(ep1,ep2);
                        ep->v.i = ep1->v.i | ep2->v.i;
                        break;
                case en_xor:
                        		ep->nodetype = maxinttype(ep1,ep2);
                        ep->v.i = ep1->v.i ^ ep2->v.i;
                        break;
                }
}
/*
 * this stuff has been superceded by a lookup table
 * I'll fix it eventually
 */
long     pwrof2(int i)
/*
 *      return which power of two i is or -1.
 */
{       long     p, q;
        q = 2;
        p = 1;
        while( q > 0 )
                {
                if( q == i )
                        return p;
                q <<= 1;
                ++p;
                }
        return -1;
}

long     mod_mask(int i)
/*
 *      make a mod mask for a power of two.
 */
{       long     m;
        m = 0;
        while( i-- )
                m = (m << 1) | 1;
        return m;
}

void opt0(ENODE ** node)
/*
 *      opt0 - delete useless expressions and combine constants.
 *
 *      opt0 will delete expressions such as x + 0, x - 0, x * 0,
 *      x * 1, 0 / x, x / 1, x mod 0, etc from the tree pointed to
 *      by node and combine obvious constant operations. It cannot
 *      combine name and label constants but will combine icon type
 *      nodes.
 */
{       ENODE    *ep;
        int             sc,mode,val;
				double dval;
        ep = *node;
        if( ep == 0 )
                return;
        switch( (*node)->nodetype ) {
                case en_b_ref:
                case en_w_ref:          /* optimize unary node */
                case en_ub_ref:
                case en_uw_ref:          /* optimize unary node */
                case en_l_ref:
								case en_ul_ref:
								case en_floatref: case en_doubleref: case en_longdoubleref:
								case en_cb: case en_cub:
								case en_cw: case en_cuw:
								case en_cl: case en_cul:
								case en_cf: case en_cd: case en_cp: case en_bits: case en_cld:
				case en_ainc:
				case en_adec:
				case en_not:
				case en_compl:
                        opt0( &((*node)->v.p[0]));
                        return;
                case en_uminus:
                        opt0( &(ep->v.p[0]));
                        if( isintconst(ep->v.p[0]->nodetype))
                                {
                                ep->nodetype = ep->v.p[0]->nodetype;
                                ep->v.i = -ep->v.p[0]->v.i;
                                }
                        else if( ep->v.p[0]->nodetype == en_rcon || ep->v.p[0]->nodetype == en_fcon || ep->v.p[0]->nodetype == en_lrcon)
                                {
                                ep->nodetype = ep->v.p[0]->nodetype;
                                ep->v.i = -ep->v.p[0]->v.f;
                                }
                        return;
                case en_add:
                case en_sub:
                        opt0(&(ep->v.p[0]));
                        opt0(&(ep->v.p[1]));
												mode = getmode(ep->v.p[0],ep->v.p[1]);
												switch (mode) {
													case 1: case 2: case 3: case 4:
														dooper(node,mode);
														break;
													case 5:
														if (ep->v.p[0]->v.i == 0) {
															if (ep->nodetype == en_sub) {
																*node = makenode(en_uminus,ep->v.p[1],0);
															}
															else *node = ep->v.p[1];
														}
														else dooper(node,mode);
														break;
													case 6:
														if (ep->v.p[0]->v.f == 0) {
															if (ep->nodetype == en_sub) {
																*node = ep->v.p[1];
																ep->v.p[1]->v.f = - ep->v.p[1]->v.f;

															}
															else *node = ep->v.p[1];
														}
														else dooper(node,mode);
														break;
												  case 7:
														if (ep->v.p[1]->v.i == 0) {
															*node = ep->v.p[0];
														}
														else dooper(node,mode);
														break;
													case 8:
														if (ep->v.p[1]->v.f == 0) {
															*node = ep->v.p[0];
														}
														else dooper(node,mode);
														break;
												}
                        return;
                case en_mul:
								case en_umul:
								case en_pmul:
								case en_asmul: case en_asumul:
                        opt0(&(ep->v.p[0]));
                        opt0(&(ep->v.p[1]));
												mode = getmode(ep->v.p[0],ep->v.p[1]);
												switch(mode) {
													case 1: case 2: case 3: case 4:
														dooper(node,mode);
														break;
													case 5:
														if (!floatrecurse(ep->v.p[1])) {
                                sc = pwrof2(ep->v.p[0]->v.i);
                                if( sc != -1 )
                                        {
																				ENODE *temp = ep->v.p[0];
																				ep->v.p[0 ] = ep->v.p[1];
																				ep->v.p[1] = temp;
                                        ep->v.p[1]->v.i = sc;
																				switch(ep->nodetype) {
																					case en_mul:
																						ep->nodetype = en_alsh;
																						break;
																					case en_asmul:
																						ep->nodetype = en_asalsh;
																						break;
																					case en_umul:
																					case en_pmul:
																						ep->nodetype = en_lsh;
																						break;
																					case en_asumul:
																						ep->nodetype = en_aslsh;
																						break;
																				}
																				break;
                                        }
                            }
														val = ep->v.p[0]->v.i;
														if (val == 0)
															*node = ep->v.p[0];
														else if (val == 1)
															*node = ep->v.p[1];
														else if (val == -1)
															*node = makenode(en_uminus,(char *)ep->v.p[1],0);
														else dooper(node,mode);
														break;
													case 6:
														dval = ep->v.p[0]->v.f;
														if (dval == 0)
															*node = ep->v.p[0];
														else if (dval == 1)
															*node = ep->v.p[1];
														else if (dval == -1)
															*node = makenode(en_uminus,(char *)ep->v.p[1],0);
														else dooper(node,mode);
														break;
													case 7:
														if (!floatrecurse(ep->v.p[0])) {
                                sc = pwrof2(ep->v.p[1]->v.i);
                                if( sc != -1 )
                                        {
                                        ep->v.p[1]->v.i = sc;
																				switch(ep->nodetype) {
																					case en_mul:
																						ep->nodetype = en_alsh;
																						break;
																					case en_asmul:
																						ep->nodetype = en_asalsh;
																						break;
																					case en_umul:
																					case en_pmul:
																						ep->nodetype = en_lsh;
																						break;
																					case en_asumul:
																						ep->nodetype = en_aslsh;
																						break;
																				}
																				break;
                                        }
                            }
														val = ep->v.p[1]->v.i;
														if (val == 0)
															*node = ep->v.p[1];
														else if (val == 1)
															*node = ep->v.p[0];
														else if (val == -1)
															*node = makenode(en_uminus,(char *)ep->v.p[0],0);
														else dooper(node,mode);
														break;
													case 8:
														dval = ep->v.p[1]->v.f;
														if (dval == 0)
															*node = ep->v.p[1];
														else if (dval == 1)
															*node = ep->v.p[0];
														else if (dval == -1)
															*node = makenode(en_uminus,(char *)ep->v.p[0],0);
														else dooper(node,mode);
														break;
												}
                        break;
								case en_pdiv:
                case en_div:
								case en_udiv:
								case en_asdiv:
								case en_asudiv:
                        opt0(&(ep->v.p[0]));
                        opt0(&(ep->v.p[1]));
												mode = getmode(ep->v.p[0],ep->v.p[1]);
												switch(mode) {
													case 1:
													case 2:
													case 3:
													case 4: 
														dooper(node,mode);
														break;
													case 5:
														if (ep->v.p[0]->v.i == 0)
															*node = ep->v.p[0];
														else
															dooper(node,mode);
														break;
													case 6:
														if (ep->v.p[0]->v.f == 0)
															*node = ep->v.p[0];
														else
															dooper(node,mode);
														break;
													case 7:
														if (!floatrecurse(ep->v.p[0])) {
                                sc = pwrof2(ep->v.p[1]->v.i);
                                if( sc != -1 )
                                        {
                                        ep->v.p[1]->v.i = sc;
																				switch(ep->nodetype) {
																					case en_div:
																						ep->nodetype = en_arsh;
																						break;
																					case en_asdiv:
																						ep->nodetype = en_asarsh;
																						break;
																					case en_udiv:
																					case en_pdiv:
																						ep->nodetype = en_rsh;
																						break;
																					case en_asudiv:
																						ep->nodetype = en_asrsh;
																						break;
																				}
																				break;
                                        }
                            }
														val = ep->v.p[1]->v.i;
														if (val == 1)
															*node = ep->v.p[0];
														else if (val == -1)
															*node = makenode(en_uminus,(char *)ep->v.p[0],0);
														else dooper(node,mode);
														break;
													case 8:
														dval = ep->v.p[1]->v.f;
														if (dval == 1)
															*node = ep->v.p[0];
														else if (dval == -1)
															*node = makenode(en_uminus,(char *)ep->v.p[0],0);
														else dooper(node,mode);
														break;
												}
                        break;
                case en_mod:
								case en_umod:
								case en_asmod: case en_asumod:
                        opt0(&(ep->v.p[0]));
                        opt0(&(ep->v.p[1]));
												mode = getmode(ep->v.p[0],ep->v.p[1]);
												switch(mode) {
													case 7:
														if (!floatrecurse(ep->v.p[0])) {
                                sc = pwrof2(ep->v.p[1]->v.i);
                                if( sc != -1 )
                                        {
                                        ep->v.p[1]->v.i = mod_mask(sc);
																				if (ep->nodetype == en_asmod || ep->nodetype == en_asumod)
                                        	ep->nodetype = en_asand;
																				else
                                        	ep->nodetype = en_and;
																				break;
                                        }
                                }
													case 1:
													case 2:
													case 3:
													case 4: 
													case 5: case 6: case 8:
														dooper(node,mode);
												}
                        break;
                case en_and:    case en_or:
                case en_xor:    case en_rsh:
                case en_lsh:    case en_arsh: case en_alsh:
                        opt0(&(ep->v.p[0]));
                        opt0(&(ep->v.p[1]));
                        if( isintconst(ep->v.p[0]->nodetype) &&
                                isintconst(ep->v.p[1]->nodetype) )
                                dooper(node,getmode(ep->v.p[0],ep->v.p[1]));
                        break;
                case en_land:   case en_lor:
				case en_lt:		case en_le:
				case en_ugt:	case en_uge: case en_ult: case en_ule:
				case en_gt:		case en_ge:
				case en_eq:		case en_ne:
								case en_asalsh: case en_asarsh:
                case en_asand:  case en_asor:  case en_asxor:
                case en_asadd:  case en_assub:
                case en_asrsh:
                case en_aslsh:  case en_cond:
                case en_fcall:  case en_void: case en_trapcall: case en_intcall:
								case en_pfcall: case en_pfcallb:
                case en_assign: case en_moveblock: case en_stackblock: case en_callblock:
								case en_fcallb: case en_refassign: case en_pcallblock:
                        opt0(&(ep->v.p[0]));
                        opt0(&(ep->v.p[1]));
                        break;
                }
}
long     xfold2(ENODE * node)
/*
 *      xfold2 will remove constant nodes and return the values to
 *      the calling routines.
 */
{       long     i;
        if( node == 0 )
                return 0;
        switch( node->nodetype )
                {
                case en_icon:
								case en_lcon:
								case en_iucon:
								case en_lucon:
								case en_ccon:
                        i = node->v.i;
                        node->v.i = 0;
                        return i;
                case en_add:
                        return xfold2(node->v.p[0]) + xfold2(node->v.p[1]);
                case en_sub:
                        return xfold2(node->v.p[0]) - xfold2(node->v.p[1]);
								case en_or:
                        return xfold2(node->v.p[0]) | xfold2(node->v.p[1]);
								case en_xor:
                        return xfold2(node->v.p[0]) ^ xfold2(node->v.p[1]);
                case en_uminus:
                        return - xfold2(node->v.p[0]);
                case en_mul:
								case en_pmul:
								case en_umul:
                case en_lsh:
								case en_alsh:
								case en_asalsh: case en_asarsh: case en_arsh:
                case en_rsh:    case en_div:  case en_pdiv:
                case en_mod:    case en_asadd:
                case en_assub:  case en_asmul:
                case en_asdiv:  case en_asmod:
                case en_land: case en_lor:
                case en_asand:	case en_and:
                case en_asor:   case en_void:  case en_asxor:
								case en_pfcall: case en_pfcallb:
                case en_fcall:  case en_assign: case en_moveblock: case en_trapcall:
								case en_stackblock: case en_intcall: case en_callblock: case en_fcallb:
								case en_asumul: case en_asudiv: case en_asumod: case en_refassign:
								case en_udiv: case en_pcallblock:
                        fold_const2(&node->v.p[0]);
                        fold_const2(&node->v.p[1]);
                        return 0;
                case en_b_ref:  case en_w_ref:
                case en_ub_ref:  case en_uw_ref:
                case en_l_ref:  case en_compl: case en_ul_ref:
								case en_floatref: case en_doubleref: case en_longdoubleref:
                case en_not: case en_bits:
                        fold_const2(&node->v.p[0]);
                        return 0;
                }
        return 0;
}

void fold_const2(ENODE ** node)
/*
 *      reorganize an expression for optimal constant grouping.
 */
{       ENODE    *ep;
        long   i;
        ep = *node;
        if( ep == 0 )
                return;
				switch (ep->nodetype) {
						case en_add:
								if (ep->v.p[0]->nodetype == en_cp && isintconst(ep->v.p[0]->v.p[0]->nodetype)) {
												ep->v.p[0]->v.p[0]->v.i += xfold2(ep->v.p[1]);
												return;
												}
								else if (ep->v.p[1]->nodetype == en_cp && isintconst(ep->v.p[1]->v.p[0]->nodetype)) {
												ep->v.p[1]->v.p[0]->v.i += xfold2(ep->v.p[1]);
												return;
												}
                else if( isintconst(ep->v.p[0]->nodetype))
                        {
                        ep->v.p[0]->v.i += xfold2(ep->v.p[1]);
                        return;
                        }

                else if( isintconst(ep->v.p[1]->nodetype))
                        {
                        ep->v.p[1]->v.i += xfold2(ep->v.p[0]);
                        return;
                        }

								break;
						case en_sub:
								if (ep->v.p[0]->nodetype == en_cp && isintconst(ep->v.p[0]->v.p[0]->nodetype == en_icon)) {
												ep->v.p[0]->v.p[0]->v.i -= xfold2(ep->v.p[1]);
												return;
												}
								else if (ep->v.p[1]->nodetype == en_cp && isintconst(ep->v.p[1]->v.p[0]->nodetype)) {
												ep->v.p[1]->v.p[0]->v.i -= xfold2(ep->v.p[1]);
												return;
												}
                else if( isintconst(ep->v.p[0]->nodetype))
                        {
                        ep->v.p[0]->v.i -= xfold2(ep->v.p[1]);
                        return;
                        }
                else if( isintconst(ep->v.p[1]->nodetype))
                        {
                        ep->v.p[1]->v.i -= xfold2(ep->v.p[0]);
                        return;
                        }

								break;
						case en_or:
                if(isintconst( ep->v.p[0]->nodetype) )
                        {
                        ep->v.p[0]->v.i |= xfold2(ep->v.p[1]);
                        return;
                        }
                else if(isintconst( ep->v.p[1]->nodetype) )
                        {
                        ep->v.p[1]->v.i |= xfold2(ep->v.p[0]);
                        return;
                        }
								break;
						case en_xor:
                if( isintconst(ep->v.p[0]->nodetype))
                        {
                        ep->v.p[0]->v.i ^= xfold2(ep->v.p[1]);
                        return;
                        }
                else if( isintconst(ep->v.p[1]->nodetype))
                        {
                        ep->v.p[1]->v.i ^= xfold2(ep->v.p[0]);
                        return;
                        }
								break;
				}
        i = xfold2(ep);
        if( i != 0 ) {
									ep = xalloc(sizeof(ENODE));
									ep->nodetype = en_icon;
									ep->v.i = i;
                	ep = makenode(en_add,ep,*node);
                *node = ep;
        }
}
void opt4(ENODE ** node)
/*
 *      apply all constant optimizations.
 */
{
		opt0(node);
        fold_const2(node);
        opt0(node);
}