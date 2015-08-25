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
 * Evaluate expressions very recursive descent.  MAke sure your STACK
 * is large enough to parse the expressions you want to parse.  a 4K stack
 * will handle all but the very extreme cases
 */
#include        <stdio.h>
#include        "expr.h"
#include        "c.h"
#include        "errors.h"
#include				"list.h"

extern int block_nesting;
extern int global_flag;
extern enum e_sym lastst;
extern char lastid[],laststr[];
extern TABLE gsyms,lsyms;
extern long ival;
extern long double rval;
/* Default types */
extern TYP stdint,stdlongdouble, stduns,stdstring;
TYP             stdfloat = { bt_float, 0, UF_DEFINED | UF_USED,0, 0,-1, -1, 8, {0, 0}, 0, 0,0 };
TYP             stddouble = { bt_double, 0, 0,0,0, -1, -1, 8, {0, 0}, 0, 0,0 };
TYP             stdvoid = { bt_matchall, 0, 0 ,0, 0,-1, -1, 4, {0, 0}, 0, 0,0 };
TYP             stdmatch = { bt_matchall, 0, UF_DEFINED | UF_USED,0, 0,-1, -1, 4, {0, 0}, &stdvoid, 0,0 };
TYP             stdlong = {bt_long, 0, 0,0,0,-1, -1, 4, {0, 0}, 0, 0,0 };
TYP             stdunsigned = {bt_unsigned, 0, 0,0,0,-1, -1, 4, {0, 0}, 0, 0,0 };
TYP             stdchar = {bt_char, 0, 0,0,0,-1, -1, 1, {0, 0}, 0, 0,0 };
TYP             stdfunc = {bt_func, 1, UF_DEFINED | UF_USED,0,0,-1, -1, 0, {0, 0}, &stdint, 0,0};
extern TYP      *head;          /* shared with decl */
extern TABLE    tagtable;
extern char declid[100];
extern int goodcode;
extern int prm_cplusplus;
extern int regdsize,regasize,regfsize,stackadd,stackmod;

int skm_closepa[] = { closepa, comma, semicolon, end, 0 };
int skm_closebr[] = { closebr, comma, openbr, semicolon, end, 0 };
static SYM *lastsym;
static int globaldref = 0;
static char regname[] = "processor reg" ;
static char *nm = 0;
static TYP *asntyp = 0;
static int dumpos;
static SYM undef;
void exprini(void)
{
	globaldref = 0;
	asntyp = 0;
	dumpos = 0;
	undef.value.i = 0;
	undef.name = "UNDEFINED";
}
ENODE    *makenode(enum e_node nt, char *v1, char *v2)
/*
 *      build an expression node with a node type of nt and values
 *      v1 and v2.
 */
{       ENODE    *ep;
        ep = xalloc(sizeof(ENODE));
        ep->nodetype = (char)nt;
				ep->cflags = 0;
        ep->v.p[0] = v1;
        ep->v.p[1] = v2;
        return ep;
}

TYP *deref(ENODE **node, TYP *tp)
/*
 *      build the proper dereference operation for a node using the
 *      type pointer tp.
 */
{
				ENODE *onode = *node;
				switch( tp->type ) {
								case bt_double:
                        *node = makenode(en_doubleref,*node,0);
                        break;
								case bt_longdouble:
                        *node = makenode(en_longdoubleref,*node,0);
                        break;
								case bt_float:
                        *node = makenode(en_floatref,*node,0);
                        break;
								case bt_unsignedchar:
                        *node = makenode(en_ub_ref,*node,0);
                        break;
								case bt_unsignedshort:
                        *node = makenode(en_uw_ref,*node,0);
                        break;
                case bt_char:
                        *node = makenode(en_b_ref,*node,0);
                        break;
                case bt_short:
                case bt_enum:
                        *node = makenode(en_w_ref,*node,0);
                        break;
                case bt_unsigned:
                        *node = makenode(en_ul_ref,*node,0);
												break;
                case bt_long:
                case bt_matchall:
                case bt_pointer:
								case bt_ptrfunc:
								case bt_ref:
                        *node = makenode(en_l_ref,*node,0);
                        break;
                default:
                        generror(ERR_DEREF,0,0);
                        break;
                }
				(*node)->cflags = onode->cflags;
        return tp;
}
ENODE *dummyvar(int size, TYP *type)
{
	char nm[20];
	ENODE *newnode;
	SYM *sp = xalloc(sizeof(SYM));
	sprintf(nm,"**DUMMY%d",dumpos++);
	sp->name = litlate(nm);
	sp->defalt = 0;
  sp->storage_class = sc_auto;
  sp->tp = type;
	type->uflags |= UF_USED;
	sp->extflag = FALSE;
	sp->absflag = FALSE;
	sp->intflag = FALSE;
	sp->pascaldefn = FALSE;
	sp->init = 0;
	sp->indecltable = 0;
	sp->funcparm = 0;
	sp->inreg = 0;
	sp->staticlabel = FALSE;
	sp->value.i = block_nesting;
	insert(sp,&lsyms);
	newnode = makenode(en_autocon,sp,0);
	return newnode;
}
int isintconst(int type)
{
	switch (type) {
		case en_icon:
		case en_lcon:
		case en_lucon:
		case en_iucon:
		case en_ccon:
			return 1;
	}
	return 0;
}
TYP     *nameref(ENODE **node)
/*
 * get an identifier.  If it is followed by parenthesis gather the
 * function parms.  If it is an undefined function declare it as external
 * for now.
 */
{       SYM             *sp;
        TYP             *tp,*tp1;
				ENODE *pnode=0,*qnode = 0;
				int fn = FALSE;
				char buf[100];
				strcpy(buf,lastid);
				getsym();
/* Get function args */
				if (lastst == openpa) {
					fn = TRUE;
					getsym();
					tp1 = gatherparms(&pnode);
					sp = funcovermatch(buf,tp1);
					if (sp)
						tp1->sname = nm = sp->name;
					else
#ifdef CPLUSPLUS
						if (prm_cplusplus)
							tp1->sname = nm = cppmangle(buf,tp1);
						else
#endif
							tp1->sname = nm = litlate(buf);
				}
				else {
					nm = litlate(buf);
					sp = gsearch(nm);
				}
        if( sp == 0 ) {
/* No such identifier */
                if (fn) {
/* External function, put it in the symbol table */
#ifdef CPLUSPLUS
												if (prm_cplusplus) 
													gensymerror(ERR_NOFUNCMATCH,nm);
												else
#endif
													gensymerror(ERR_NOPROTO,nm);
                        ++global_flag;
												sp = xalloc(sizeof(SYM));
                				sp->name = litlate(nm);
												sp->tp = maketype(bt_func,0);
												*(sp->tp) = stdfunc;
                        sp->storage_class = sc_externalfunc;
												sp->extflag = TRUE;
                        insert(sp,&gsyms);
                        --global_flag;
                        *node = makenode(en_napccon,sp,0);
												parmlist(&pnode,tp1,0);
												*node = makenode(en_void,*node,pnode);
                        tp = &stdint;
												qnode = makenode(en_icon,(char *)-4,0);
	                      *node = makenode(en_fcall,qnode,*node);
												goodcode |= GF_ASSIGN;
                        }
                else    {
/* External non-function.  These also get put in the symbol table so that
 * we don't keep spitting errors out but also put an error out
 */
#ifdef CPLUSPLUS
												if (prm_cplusplus && asntyp && asntyp->type == bt_ptrfunc) {
													sp = funcovermatch(lastid,asntyp);
													if (sp)
														goto foundsp;
												}
#endif
												sp = xalloc(sizeof(SYM));
                				sp->name = nm;
                        sp->tp = tp = &stdmatch;
												sp->storage_class = sc_external;
												insert(sp,&lsyms);
												*node = makenode(en_nacon,&undef,0);
												gensymerror(ERR_UNDEFINED,nm);
                        tp = deref(node,tp);
                        }
                }
        else    {
/* If we get here the symbol was already in the table
 */
foundsp:
								sp->tp->uflags |= UF_USED;
                if( (tp = sp->tp) == 0 ) {
/* This lack of type info should never happen */
                        tp = &stdmatch;
												*node = makenode(en_nacon,&undef,0);
												gensymerror(ERR_UNDEFINED,nm);
                        tp = deref(node,tp);
                        return tp;       /* guard against untyped entries */
                        }
                switch( sp->storage_class ) {
                        case sc_static:
                        case sc_global:
                        case sc_external:
                        case sc_externalfunc:
												case sc_abs:
																sp->extflag = TRUE;
																if (fn) {
/* make a function node */
																	if (sp->tp->type == bt_ptrfunc)
                                		*node = makenode(en_nacon,sp,0);
																	else
                                		*node = makenode(en_napccon,sp,0);
isfunc:
																	if (sp->tp->type != bt_ptrfunc && sp->tp->type != bt_func && sp->tp->type != bt_ifunc)
																		generror(ERR_MISMATCH,0,0);
																	if (sp->tp->type == bt_ptrfunc)
																		tp = deref(node,tp);
#ifdef CPLUSPLUS
																	if (prm_cplusplus && !strcmp(buf,"main"))
																		generror(ERR_NOMAIN,0,0);
#endif
																	parmlist(&pnode,tp1,sp->tp);
																	*node = makenode(en_void,*node,pnode);
																	qnode = makenode(en_icon,(char *)sp->tp->btp->size,0);
                                  tp = tp->btp;
																	if (sp->intflag)
	                                	*node = makenode(en_intcall,qnode,*node);
																	else
																	if (tp->type == bt_union || tp->type == bt_struct) {
																		if (sp->pascaldefn)
	                              		  *node = makenode(en_pfcallb,qnode,*node);
																		else 
	                              		  *node = makenode(en_fcallb,qnode,*node);
																		(*node)->size = tp->size;
																	}
																	else
																		if (sp->pascaldefn)
	                              			*node = makenode(en_pfcall,qnode,*node);
																		else
	                              			*node = makenode(en_fcall,qnode,*node);
																	if (tp)
																		(*node)->cflags = tp->cflags;
																	goodcode |= GF_ASSIGN;
																}
																else
/* otherwise make a node for a regular variable */
																	if (sp->absflag)
                                		*node = makenode(en_absacon,sp,0);
																	else
																		if (sp->tp->type == bt_func || sp->tp->type == bt_ifunc) {
																			fn = TRUE;
                                			*node = makenode(en_napccon,sp,0);
																		}
																		else
																			if (sp->staticlabel)
																				*node = makenode(en_nalabcon,(char *)sp->value.i,0);
																			else
                                				*node = makenode(en_nacon,sp,0);
                                break;
                        case sc_const:
/* constants and enums */
                                *node = makenode(en_icon,(char *)sp->value.i,0);
																return &stdint;
                        default:        /* auto and any errors */
                                if( sp->storage_class != sc_auto && sp->storage_class != sc_autoreg) {
                                        gensymerror(ERR_ILLCLASS2,sp->name);
																				tp = 0;
																}
																else {
/* auto variables */
															 		if (sp->storage_class == sc_auto)
                                		*node = makenode(en_autocon,sp,0);
																	else if (sp->storage_class == sc_autoreg)
                                		*node = makenode(en_autoreg,sp,0);
																	if (fn)
																		goto isfunc;
																}
                                break;
                        }
/* dereference if it isn't an array or structure address */

								(*node)->cflags = tp->cflags;
								/* deref if not an array or if is function parm */
	              if(!fn && tp && (tp->val_flag == 0 || (sp->funcparm && tp->type == bt_pointer)))
  	                     tp = deref(node,tp);
/* and dereference again if it is a refernece variable */
								if (tp->type == bt_ref) {
												tp = tp->btp;
												tp = deref(node,tp);
								}
												
                }
				lastsym = sp;
        return tp;
}

void promote_type(TYP *typ, ENODE **node)
/*
 * Type promotion for casts and function args 
 */
{
		switch (typ->type) {
			case bt_char:
					*node = makenode(en_cb,*node,0);
							break;
			case bt_unsignedchar:
					*node = makenode(en_cub,*node,0);
					break;
			case bt_enum:
			case bt_short:
	  			*node = makenode(en_cw,*node,0);
		  		break;
			case bt_unsignedshort:
  				*node = makenode(en_cuw,*node,0);
					break;
	  	case bt_long:
    			*node = makenode(en_cl,*node,0);
					break;
			case bt_unsigned:
					*node = makenode(en_cul,*node,0);
		  		break;
			case bt_float:
  				*node = makenode(en_cf,*node,0);
					break;
			case bt_double:
					*node = makenode(en_cd,*node,0);
					break;
			case bt_longdouble:
					*node = makenode(en_cld,*node,0);
					break;
			default:
					*node = makenode(en_cp,*node,0);
					break;
		}
	(*node)->cflags = typ->cflags;
}
TYP *gatherparms( ENODE **node)
/*
 * create a type tree and primary parameter list for a function
 *
 * At this point the parameter list is backwards from what codegen
 * needs!
 */
{
	ENODE *ep1 = 0,*ep2=0,**ep3 = &ep1;
	TABLE tbl;
	SYM **t = &tbl.head,*newt;
	TYP *tp;
	int ogc = goodcode;
	tbl.tail = tbl.head = 0;
	goodcode |= DF_FUNCPARMS;
	if (lastst == closepa) {
#ifdef CPLUSPLUS
		if (prm_cplusplus)
			tbl.head=tbl.tail=(SYM *)-1;
		else
#endif
			tbl.head=tbl.tail = 0;
	}
	else if (lastst == kw_void)
		tbl.head = tbl.tail = (SYM *)-1;
	else
	  while (lastst != closepa) {
			tp = exprnc(&ep2);
			if (!tp) {
				generror(ERR_EXPREXPECT,0,0);
				break;
			}
			ep2->cflags = tp->cflags;
			newt = xalloc(sizeof(SYM));
			newt->tp = tp;
			newt->next = 0;
			newt->name = 0;
			*t = newt;
			t = &newt->next;
			tbl.tail = newt;
			*ep3 = makenode(en_void,ep2,0);
			ep3 = &(*ep3)->v.p[1];
			if (lastst == comma)
				getsym();
		}
	needpunc(closepa,skm_closepa);
	tp = maketype(bt_func,0);
	tp->btp = &stdint;
	tp->lst = tbl;
	tp->bits = -1;
	tp->startbit = -1;
	tp->uflags = UF_DEFINED | UF_USED;
	goodcode = ogc;
	*node = ep1;
	return tp;
}
void checkparmconst(TYP *tp, TYP *tpi)
/*
 * Check the CONST flags for parameters
 */
{
	if (tpi->type != bt_pointer && tpi->type != bt_ref)
		return;
	while ((tp->type == bt_pointer || tp->type == bt_ref)&& !tp->val_flag && (!tpi || tpi->type == bt_pointer || tpi->type == bt_ref)) {
		if ((tp->cflags & DF_CONST) && (!tpi || !(tpi->cflags & DF_CONST)))
			generror(ERR_MODCONS,0,0);
		tp = tp->btp;
		tpi = tpi->btp;
	}
	if ((tp->cflags & DF_CONST) && (!tpi || !(tpi->cflags & DF_CONST)))
		generror(ERR_MODCONS,0,0);
}
void parmlist(ENODE **node, TYP *tpi, TYP *tp)
/*
 * take the primary type and node trees, the function argument expectations,
 * and check for type mismatch errors
 *
 * also reverse the primary node tree so the parms will be ready for
 * code generation
 */
{       ENODE    *ep1=0,*ep2=0,*ep3=*node;
				SYM *spi=tpi->lst.head,*sp=0;
				TYP *tp2;
				int matching = FALSE;
				if (tp)
					sp=tp->lst.head;
				if (tp && !sp)
					gensymerror(ERR_NOPROTO,tpi->sname);
				
					if (!prm_cplusplus && sp && sp != (SYM *)-1 && sp->tp->type != bt_ellipse)
							matching = TRUE;
					while (TRUE) {
							if (!spi || spi == (SYM *)-1){
								if (sp == (SYM *) -1)
									break;
								if (sp && sp->tp->type != bt_ellipse) 
									if (!sp->defalt) 
										genfuncerror(ERR_CALLLENSHORT,tpi->sname,0);
									else
										while (sp && sp != (SYM *)-1) {
											if (sp->tp->val_flag && (sp->tp->type == bt_struct || sp->tp->type == bt_union)) {
												ep1 = makenode(en_stackblock,sp->defalt,ep1);
												sp->defalt->size = sp->tp->size;
											}
											else
	    			    		   	ep1 = makenode(en_void,sp->defalt,ep1);
											sp = sp->next;
										}
								break;
							}
							else {
							ep2 = ep3->v.p[0];
							ep3 = ep3->v.p[1];
							if (matching) {
								if (!sp || sp == (SYM *)-1) {
									genfuncerror(ERR_CALLLENLONG,tpi->sname,0);
									break;
								}
								else {
									checkparmconst(spi->tp,sp->tp);
									if (!checktype(spi->tp,sp->tp))
								  	if (isscalar(sp->tp) && isscalar(spi->tp))
											promote_type(sp->tp,&ep2);
										else
											if (sp->tp->type == bt_pointer) {
												if (isintconst(ep2->nodetype)) {
													if (ep2->v.i != 0)
														generror(ERR_NONPORT,0,0);
												}
												else if (spi->tp->type != bt_pointer)
														genfuncerror(ERR_CALLMISMATCH,tpi->sname,sp->name);
						 					}
											else genfuncerror(ERR_CALLMISMATCH,tpi->sname,sp->name);
								}
							}
							}
							if (sp && sp->tp->type == bt_ref) {
								if (lvalue(ep2)) {
									while (castvalue(ep2))
										ep2 = ep2->v.p[0];
									ep2 = ep2->v.p[0];
								}
								else {
									ENODE *x;
									tp2 = sp->tp->btp;
									genfuncerror(ERR_TEMPUSED,tpi->sname,sp->name);
									ep2 = makenode(en_refassign,dummyvar(tp2->size,tp2),ep2);
								}
								}
							if (spi && spi != (SYM *) -1 && spi->tp->val_flag && (spi->tp->type == bt_struct || spi->tp->type == bt_union)) {
								ep1 = makenode(en_stackblock,ep2,ep1);
								ep2->size = sp->tp->size;
							}
							else
	        	   	ep1 = makenode(en_void,ep2,ep1);
							spi = spi->next;
 							if (sp && sp != (SYM *)-1) {
								sp = sp->next;
 								if (sp && sp->tp->type == bt_ellipse)
									matching = FALSE;
							}
					}
				if (tp)
					promote_type(tp->btp,&ep1);
				else
					promote_type(tpi->btp,&ep1);
				*node = ep1;
}

int floatrecurse(ENODE *node)
/*
 * Go through a node and see if it will be promoted to type FLOAT
 */
{
	if (!node)
		return 0;
	switch (node->nodetype) {
								case en_rcon:
								case en_lrcon:
								case en_fcon:
								case en_doubleref:
								case en_longdoubleref:
								case en_floatref:
								case en_cld:
								case en_cd:
								case en_cf:
												return 1;
                case en_labcon: case en_trapcall: 
                case en_nacon:  case en_autocon:  case en_autoreg: case en_nalabcon:
                case en_l_ref:  case en_tempref: case en_napccon: case en_absacon:
								case en_cl: case en_regref:
                case en_ul_ref:
								case en_cul:
								case en_cp:
                case en_icon:
								case en_lcon: case en_iucon: case en_lucon: case en_ccon:
								case en_bits:
                case en_ub_ref:
								case en_cub:
                case en_b_ref:
								case en_cb:
                case en_uw_ref:
								case en_cuw:
                case en_cw:
                case en_w_ref:
                case en_eq:     case en_ne:
                case en_lt:     case en_le:
                case en_gt:     case en_ge:
								case en_ugt: case en_uge: case en_ult: case en_ule:
                        return 0;
								case en_fcall: case en_pfcall:
								case en_fcallb: case en_pfcallb:
								case en_callblock:
												return(floatrecurse(node->v.p[1]));
                case en_not:    case en_compl:
                case en_uminus: 
                case en_ainc:   case en_adec:
								case en_moveblock: case en_stackblock:
                        return floatrecurse(node->v.p[0]);
								case en_refassign: case en_assign:
                case en_add:    case en_sub:
								case en_umul:		case en_udiv:	case en_umod: case en_pmul:
                case en_mul:    case en_div:
                case en_mod:    case en_and:
                case en_or:     case en_xor:
								case en_asalsh: case en_asarsh: case en_alsh: case en_arsh:
                case en_lsh:    case en_rsh:
                case en_land:   case en_lor:
                case en_asadd:  case en_assub:
                case en_asmul:  case en_asdiv:
                case en_asmod:  case en_asand:
								case en_asumod: case en_asudiv: case en_asumul:
                case en_asor:   case en_aslsh: case en_asxor:
                case en_asrsh:
                        return(floatrecurse(node->v.p[0]) || floatrecurse(node->v.p[1]));
                case en_void:   case en_cond:
                        return floatrecurse(node->v.p[1]);
	}
	return(0);
}
void floatcheck(ENODE *node)
/*
 * Error if node will be promoted to type float
 */
{
	if (floatrecurse(node))
		generror(ERR_INVFLOAT,0,0);
}
int     castbegin(int st)
/*
 *      return 1 if st in set of [ kw_char, kw_short, kw_long, kw_int,
 *      kw_float, kw_double, kw_struct, kw_union, kw_float, or is typedef ]
 */
{      
	SYM *sp;
	switch(st) {
		case kw_void:
		case kw_char: case kw_short: case kw_int: case kw_long:
		case kw_float: case kw_double:	
		case kw_struct:	case kw_union: case kw_signed:
		case kw_unsigned:	case kw_volatile:	case kw_const:
			return 1;
		default:
			if (st != id)
				return 0;
	} 
	nm = lastid;
	sp = gsearch(lastid);
	if (!sp)
		sp = search(lastid,&lsyms);
	if (sp && sp->storage_class == sc_type)
		return 1 ;
	return 0;
}

int tostring()
{
	short string[2048];
	int st = lastst;
	string[0] = 0;
	while (lastst == st) {
		if (st == lsconst) {
			if (pstrlen(string) +pstrlen(laststr) > 2040)
				generror(ERR_STRINGTOOBIG,0,0);
			else
				pstrcat(string,laststr);
		}
		else {
			if (strlen(string) +strlen(laststr) > 4090)
				generror(ERR_STRINGTOOBIG,0,0);
			else
				strcat(string,laststr);
		}
		getsym();
	}
	return(stringlit(string,st == lsconst));
}
TYP     *primary(ENODE **node)
/*
 *      primary will parse a primary expression and set the node pointer
 *      returning the type of the expression parsed. primary expressions
 *      are any of:
 *                      id
 *                      constant
 *                      string
 *                      ( expression )
 *                      primary++
 *                      primary--
 *                      primary[ expression ]
 *                      primary.id
 *                      primary->id
 *                      primary( parameter list )
 *											(* expression)( parameter list )
 *                      (typecast)primary
 *                      (typecast)(unary)
 */
{       ENODE    *pnode, *qnode, *rnode;
        SYM             *sp=0;
        TYP             *tptr,*tp1,*tp2;
				int flag = 0;
				int gcode,gdf;
				int isstring = FALSE;
        switch( lastst ) {
/* This trap thing should be in stmt.c */
						case kw__trap:    
									getsym();
									if (needpunc(openpa,0)) {
										long num = intexpr(0);
										if (num > 15 || num < 0)
											generror(ERR_INVTRAP,0,0);
										if (lastst == comma) 
											getsym();
										tptr = gatherparms(&pnode);
										parmlist(&pnode,tptr,0);
										qnode = makenode(en_icon,0,0);
										pnode = makenode(en_void,makenode(en_icon,(char *)num,0),pnode);
										pnode = makenode(en_trapcall,qnode,pnode);
									}
									goodcode |= GF_ASSIGN;
									*node = pnode;
									return &stdint;

						case kw_D0:
						case kw_D1:
						case kw_D2:
						case kw_D3:
						case kw_D4:
						case kw_D5:
						case kw_D6:
						case kw_D7:
						case kw_D8:
						case kw_D9:
						case kw_DA:
						case kw_DB:
						case kw_DC:
						case kw_DD:
						case kw_DE:
						case kw_DF:
												tptr = xalloc(sizeof(TYP));
												*tptr = stduns;
												tptr->sname = regname;
												pnode = makenode(en_regref,(char *)(regdsize*256+lastst-kw_D0),0);
												pnode = makenode(en_ul_ref,pnode,0);
												*node = pnode;
												getsym();
												return tptr;
						case kw_A0:
						case kw_A1:
						case kw_A2:
						case kw_A3:
						case kw_A4:
						case kw_A5:
						case kw_A6:
						case kw_A7:
						case kw_A8:
						case kw_A9:
						case kw_AA:
						case kw_AB:
						case kw_AC:
						case kw_AD:
						case kw_AE:
						case kw_AF:
												tptr = xalloc(sizeof(TYP));
												*tptr = stduns;
												tptr->sname = regname;
												pnode = makenode(en_regref,(char *)(regasize*256+lastst-kw_D0),0);
												pnode = makenode(en_ul_ref,pnode,0);
												*node = pnode;
												getsym();
												return tptr;
						case kw_F0:
						case kw_F1:
						case kw_F2:
						case kw_F3:
						case kw_F4:
						case kw_F5:
						case kw_F6:
						case kw_F7:
						case kw_F8:
						case kw_F9:
						case kw_FA:
						case kw_FB:
						case kw_FC:
						case kw_FD:
						case kw_FE:
						case kw_FF:
												tptr = xalloc(sizeof(TYP));
												*tptr = stdlongdouble;
												tptr->sname = regname;
												pnode = makenode(en_regref,(char *)(regfsize *256+lastst-kw_D0),0);
												pnode = makenode(en_longdoubleref,pnode,0);
												*node = pnode;
												getsym();
												return tptr;
            case id:  
                        tptr = nameref(&pnode);
                        break;
                case iconst:
                        tptr = &stdint;
                        pnode = makenode(en_icon,(char *)ival,0);
                        getsym();
												*node = pnode;
												return tptr;
                case iuconst:
                        tptr = &stduns;
                        pnode = makenode(en_iucon,(char *)ival,0);
                        getsym();
												*node = pnode;
												return tptr;
                case lconst:
                        tptr = &stdlong;
                        pnode = makenode(en_lcon,(char *)ival,0);
                        getsym();
												*node = pnode;
												return tptr;
                case luconst:
                        tptr = &stdunsigned;
                        pnode = makenode(en_lucon,(char *)ival,0);
                        getsym();
												*node = pnode;
												return tptr;
                case cconst:
                        tptr = &stdchar;
                        pnode = makenode(en_ccon,(char *)ival,0);
                        getsym();
												*node = pnode;
												return tptr;
								case rconst:
												tptr = &stddouble;
        								pnode = xalloc(sizeof(ENODE));
								        pnode->nodetype = en_rcon;
												pnode->cflags = 0;
												pnode->v.f = rval;
												getsym();
												*node = pnode;
												return tptr;
								case lrconst:
												tptr = &stdlongdouble;
        								pnode = xalloc(sizeof(ENODE));
								        pnode->nodetype = en_lrcon;
												pnode->cflags = 0;
												pnode->v.f = rval;
												getsym();
												*node = pnode;
												return tptr;
								case fconst:
												tptr = &stdfloat;
        								pnode = xalloc(sizeof(ENODE));
								        pnode->nodetype = en_fcon;
												pnode->cflags = 0;
												pnode->v.f = rval;
												getsym();
												*node = pnode;
												return tptr;
                case sconst:
												isstring = TRUE;
                        tptr = &stdstring;
                        pnode = makenode(en_labcon,(char *)tostring(),0);
												*node = pnode;
												break;
                case lsconst:
												isstring = TRUE;
                        tptr = &stdstring;
                        pnode = makenode(en_labcon,(char *)tostring(),0);
												*node = pnode;
												break;
                case openpa:
                        getsym();
												if (lastst == star) {
/* function pointers */
																gcode = goodcode;
																goodcode &= ~(GF_AND | GF_SUPERAND);
                                getsym();
																gdf = globaldref;
																globaldref = 1;
																tptr = expression(&pnode);
																globaldref = gdf;
																if (needpunc(closepa, skm_closepa)) {
																	if (tptr->type == bt_ptrfunc) {
																		tptr = deref(&pnode,tptr);
																		goodcode = gcode;
																		if (needpunc(openpa,skm_closepa)) {
																			goodcode |= GF_ASSIGN;
																			tp1 = gatherparms(&qnode);
																			tp1->sname = tptr->sname;
																			parmlist(&qnode,tp1,tptr);
																			rnode = makenode(en_icon,(char *)tptr->btp->size,0);
																			pnode = makenode(en_void,pnode,qnode);
																			if (tptr->cflags & DF_INT)
	                              				pnode = makenode(en_intcall,rnode,pnode);
																			else
																				if (tptr->type == bt_union || tptr->type == bt_struct) {
																					if (lastsym && lastsym->pascaldefn)
	                              						pnode = makenode(en_pfcallb,rnode,pnode);
																					else
	                              						pnode = makenode(en_fcallb,rnode,pnode);
																					pnode->size = tptr->size;
																				}
																				else
																					if (lastsym && lastsym->pascaldefn)
	                              						pnode = makenode(en_pfcall,rnode,pnode);
																					else
	                              						pnode = makenode(en_fcall,rnode,pnode);
																			pnode->cflags = tptr->btp->cflags;
																			tptr = tptr->btp;
																			break;
																		}
																	}
																	else {
																		goodcode = gcode | (goodcode & GF_ASSIGN);
						                      	 break;
 																	}
																}
																else
																	goodcode = gcode | GF_ASSIGN;
																*node = pnode;
																return tptr;
												}
												else
castcont:
                          if( !castbegin(lastst) ) {
/* an expression in parenthesis */
																gcode = goodcode;
																goodcode &= ~(GF_AND | GF_SUPERAND);
																gdf = globaldref;
																globaldref = 0;
																tptr = expression(&pnode);
																globaldref = gdf;
																goodcode = gcode | (goodcode & GF_ASSIGN);
                                needpuncexp(closepa,skm_closepa);
																goto contfor;
                                }
                          else    {       /* cast operator */
/* a cast */
																declid[0] = 0;
                                decl(0); /* do cast declaration */
                                decl1();
                                tptr = head;
                                if (needpunc(closepa, 0)) {
																	gcode = goodcode;
																	goodcode &= ~(GF_AND | GF_SUPERAND);
																	gdf = globaldref;
																	globaldref = 0;
                                	if( (unary(&pnode)) == 0 ) {
                                        generror(ERR_IDEXPECT,0,0);
                                        tptr = 0;
                                	}
																	globaldref = gdf;
																	goodcode = gcode | (goodcode & GF_ASSIGN);
																	pnode->cflags = tptr->cflags;
																	if (tptr) {
																		promote_type(tptr, &pnode);
																	}
																}
																else
																	return(0);
													}
												*node = pnode;
												return tptr;
                default:
                        return 0;
                }
contfor:
/* modifiers that can appear after an expression */
        for(;;) {
								int i;
                switch( lastst ) {
                        case autoinc:
												case autodec:
																if (isstring)
																	generror(ERR_INVALIDSTRING,0,0);
                                if( tptr->type == bt_pointer )
                                        i = tptr->btp->size;
                                else
                                        i = 1;
																if (i == 0)
																	generror(ERR_ZEROPTR,0,0);
                                if(! lvalue(pnode) ) 
                                  generror(ERR_LVALUE,0,0);
																if (pnode->cflags & DF_CONST)
																	generror(ERR_MODCONS,0,0);
							
                                pnode = makenode(lastst==autoinc ? en_ainc : en_adec,pnode,(char *)i);
																goodcode |= GF_ASSIGN;
                                getsym();
																break;
                        case openbr:    /* build a subscript reference */
																flag = 1;
                                if( tptr->type != bt_pointer )
                                        generrorexp(ERR_NOPOINTER,0,skm_closebr);
                                else
                                        tptr = tptr->btp;
                                getsym();
                                qnode = makenode(en_icon,(char *)tptr->size,0);
																gcode = goodcode;
																goodcode &= ~(GF_AND | GF_SUPERAND);
																gdf = globaldref;
																globaldref = 0;
																tp2 = expression(&rnode);
																globaldref = gdf;
																goodcode = gcode & ~GF_ASSIGN;
																if (!isscalar(tp2) || tp2->type == bt_float || tp2->type == bt_double || tp2->type == bt_longdouble)
                                  generror(ERR_ARRAYMISMATCH,0,0);
                                qnode = makenode(en_pmul,qnode,rnode);
                                pnode = makenode(en_add,pnode,qnode);
																pnode->cflags = tptr->cflags;
                                if( tptr->val_flag == 0 )
                                        tptr = deref(&pnode,tptr);
                                needpuncexp(closebr,skm_closebr);
                            		break;
                        case pointsto: /* pointer reference */
                                if( tptr->type != bt_pointer ) {
                                        generror(ERR_NOPOINTER,0,0);
																				while (lastst == pointsto || lastst == dot) {
																					getsym();
																					getsym();
																				}
																				break;
																}
                                else
                                        tptr = tptr->btp;
																pnode->cflags = tptr->cflags;
                                if( tptr->val_flag == 0 ) {
                                        pnode = makenode(en_l_ref,pnode,0);
																				pnode->cflags = tptr->cflags;
																}
																
/*
 *      fall through to dot operation
 */
                        case dot:
																if (isstring)
																	generror(ERR_INVALIDSTRING,0,0);
                                getsym();       /* past -> or . */
                                if( lastst != id )
                                        generror(ERR_IDEXPECT,0,0);
                                else    {
                                        sp = search(nm=litlate(lastid),&tptr->lst);
                                        if( sp == 0 ) {
                        												tptr = &stdmatch;
																								pnode = makenode(en_nacon,&undef,0);
                                                gensymerror(ERR_UNDEFINED,nm);
																								getsym();
																								while (lastst == pointsto || lastst == dot) {
																									getsym();
																									getsym();
																								}
																				}
                                        else    {
																									
                                                tp2 = sp->tp;
																								if (pnode->nodetype == en_fcallb || pnode->nodetype == en_pfcallb) {
																									if (pnode->nodetype == en_pfcallb) 
																										pnode = makenode(en_pcallblock,dummyvar(pnode->size,tp2),pnode);
																									else
																										pnode = makenode(en_callblock,dummyvar(pnode->size,tp2),pnode);
																											
																								}
                                                qnode = makenode(en_icon,(char *)sp->value.i,0);
                                                pnode = makenode(en_add,pnode,qnode);
																								pnode->cflags = tptr->cflags | pnode->v.p[0]->cflags;
																								tp2->uflags |= tptr->uflags;
																								tptr = tp2; 
                                                if( tptr->val_flag == 0 )
                                                    tptr = deref(&pnode,tptr);
																								if (tp2->bits != -1) {
																									qnode = pnode;
																									pnode = makenode(en_bits,qnode,0);
																									pnode->bits = tp2->bits;
																									pnode->startbit = tp2->startbit;
																									pnode->cflags = tptr->cflags | pnode->v.p[0]->cflags;
																								}
                                        }
                                        getsym();       /* past id */
                                }
                                break;
                        case openpa:    /* function reference */
/* this SHOULD have been handled in nameref rather than here.  However
 * there are some weird cases like putting function names in parenthesis
 * or in hook expressions, so this has to be here too.
 */
																flag = 1;
																if (isstring)
																	generror(ERR_INVALIDSTRING,0,0);
                                if( tptr->type != bt_func &&
                                        tptr->type != bt_ifunc && tptr->type != bt_ptrfunc) {
                                        gensymerrorexp(ERR_NOFUNC,nm);
																				expskim(skm_closepa);
																}
                                else {
#ifdef CPLUSPLUS
																				if (prm_cplusplus && !strcmp(nm,"main"))
																					generror(ERR_NOMAIN,0,0);
#endif
																				if (!sp && tptr->sname)
																					sp=gsearch(tptr->sname);
																				if (tptr->type == bt_ptrfunc)
																					tptr = deref(&pnode,tptr);
                                				getsym();
																				tp1 =gatherparms(&qnode);
																				parmlist(&qnode,tp1,tptr);
																				rnode = makenode(en_icon,(char *)tptr->btp->size,0);
																				pnode = makenode(en_void,pnode,qnode);
                                        tptr = tptr->btp;
																				if (sp && sp->intflag)
	                                				pnode = makenode(en_intcall,rnode,pnode);
																				else
																				if (tptr->type == bt_union || tptr->type == bt_struct) {
																					if (lastsym && lastsym->pascaldefn)
	                              						pnode = makenode(en_pfcallb,rnode,pnode);
																					else
	                              						pnode = makenode(en_fcallb,rnode,pnode);
																					pnode->size = tptr->size;
																				}
																				else
																					if (lastsym && lastsym->pascaldefn)
	                              						pnode = makenode(en_pfcall,rnode,pnode);
																					else
	                              						pnode = makenode(en_fcall,rnode,pnode);
																				if (tptr)
																					pnode->cflags = tptr->cflags;
																}
																goodcode |= GF_ASSIGN;
                                break;
                        default:
                                goto fini;
                        }
                }
fini:   *node = pnode;
/* symbol level error checking */
				if (!flag && !isstring && !(goodcode & GF_AND) && lastsym && tptr->type != bt_func && tptr->type != bt_ifunc
							&&!tptr->val_flag && lastsym->storage_class != sc_type 
							&& lastsym->storage_class != sc_static && lastsym->storage_class != sc_global
							&& lastsym->storage_class != sc_external && lastsym->storage_class != sc_externalfunc) {
					if (!(lastsym->tp->uflags & UF_DEFINED) && lastst != assign
						&&lastst != asplus && lastst != asminus && lastst != astimes
						&& lastst!= asdivide && lastst != asmodop && lastst != asrshift
						&& lastst != aslshift && lastst != asor && lastst != asxor && lastst != asand) {
							gensymerror(ERR_SYMUNDEF,lastsym->name);
							lastsym->tp->uflags |= UF_DEFINED;
					}
					lastsym->tp->uflags &=  ~UF_ASSIGNED;
				}
        return tptr;
}

int			castvalue(ENODE *node)
/*
 * See if this is a cast operator */
{
				switch(node->nodetype) {
								case en_cb: case en_cub: case en_bits:
								case en_cw: case en_cuw:
								case en_cl: case en_cul:
								case en_cf: case en_cd: case en_cp: case en_cld:
                        return 1;
                }
        return 0;
}
int     lvalue(ENODE *node)
/* See if this is an lvalue; that is has it been dereferenced?
 */
{       
				if (!prm_cplusplus) {
					while (castvalue(node))
						node = node->v.p[0];
				}
				switch( node->nodetype ) {
                case en_b_ref:
                case en_w_ref:
                case en_l_ref:
                case en_ub_ref:
                case en_uw_ref:
                case en_ul_ref:
								case en_floatref:
								case en_doubleref:
								case en_longdoubleref:
                        return 1;
								case en_bits:
                        return lvalue(node->v.p[0]);
                }
        return 0;
}

TYP     *unary(ENODE **node)
/*
 *      unary evaluates unary expressions and returns the type of the
 *      expression evaluated. unary expressions are any of:
 *
 *                      primary
 *                      ++unary
 *                      --unary
 *                      !unary
 *                      ~unary
 *                      &unary
 *                      -unary
 *                      *unary
 *                      sizeof(typecast)
 *
 */
{       TYP             *tp,*tp1;
        ENODE    *ep1, *ep2;
        int             flag, i,gdf;
        flag = 0;
        switch( lastst ) {
                case autodec:
                        flag = 1;
                /* fall through to common increment */
                case autoinc:
                        getsym();
												gdf = globaldref;
												globaldref = 0;
                        tp = unary(&ep1);
												globaldref = gdf;
                        if( tp == 0 ) {
                                generror(ERR_IDEXPECT,0,0);
                                return 0;
                                }
												goodcode |= GF_ASSIGN;
                        if( lvalue(ep1)) {
                                if( tp->type == bt_pointer ) {
																				if (tp->btp->size == 0)
																					generror(ERR_ZEROPTR,0,0);
                                        ep2 = makenode(en_icon,(char *)tp->btp->size,0);
																}
                                else
                                        ep2 = makenode(en_icon,(char *)1,0);
																if (ep1->cflags & DF_CONST)
																	generror(ERR_MODCONS,0,0);
                                ep1 = makenode(flag ? en_assub : en_asadd,ep1,ep2);
                                }
                        else {
                                generror(ERR_LVALUE,0,0);
																return(0);
												}
                        break;
                case minus:
								case plus: {
												int stt = lastst;
                        getsym();
												gdf = globaldref;
												globaldref = 0;
                        tp = unary(&ep1);
												globaldref = gdf;
												goodcode &= ~GF_ASSIGN;
                        if( tp == 0 ) {
                                generror(ERR_IDEXPECT,0,0);
                                return 0;
                                }
												if (stt == minus) 
                        	ep1 = makenode(en_uminus,ep1,0);
												}
                        break;
                case not:
                        getsym();
												gdf = globaldref;
												globaldref = 0;
                        tp = unary(&ep1);
												globaldref = gdf;
												goodcode &= ~GF_ASSIGN;
                        if( tp == 0 ) {
                                generror(ERR_IDEXPECT,0,0);
                                return 0;
                                }
                        ep1 = makenode(en_not,ep1,0);
                        break;
                case compl:
                        getsym();
												gdf = globaldref;
												globaldref = 0;
                        tp = unary(&ep1);
												globaldref = gdf;
												goodcode &= ~GF_ASSIGN;
                        if( tp == 0 ) {
                                generror(ERR_IDEXPECT,0,0);
                                return 0;
                                }
												floatcheck(ep1);
                        ep1 = makenode(en_compl,ep1,0);
                        break;
                case star:
                        getsym();
												gdf = globaldref;
												globaldref = 0;
                        tp = unary(&ep1);
												globaldref = gdf;
												goodcode &= ~GF_ASSIGN;
                        if( tp == 0 ) {
                                generror(ERR_IDEXPECT,0,0);
                                return 0;
                                }
                        if( tp->btp == 0 ) {
                                generror(ERR_DEREF,0,0);
												}
                        else
														if (tp->btp->type != bt_void)
                                tp = tp->btp;
												ep1->cflags = tp->cflags;
                        if( tp->val_flag == 0 )
                                tp = deref(&ep1,tp);
                        break;
                case and:
                        getsym();
												if (!(goodcode & GF_INFUNCPARMS))
													goodcode |= GF_AND;
												gdf = globaldref;
												globaldref = 0;
                        tp = unary(&ep1);
												globaldref = gdf;
												goodcode &= ~GF_AND;
												goodcode &= ~GF_ASSIGN;
                        if( tp == 0 ) {
                                generror(ERR_IDEXPECT,0,0);
                                return 0;
                                }
												else 
													if (tp->startbit != -1) 
														generror(ERR_BFADDR,0,0);
													else if (tp->cflags & DF_AUTOREG)
														gensymerror(ERR_NOANDREG,tp->sname);
													else if (tp->type == bt_pointer && tp->val_flag && !(goodcode & GF_SUPERAND))
														generror(ERR_SUPERAND,0,0);

                        if( lvalue(ep1)) {
                                ep1 = ep1->v.p[0];
															 	if (ep1->nodetype == en_regref)
																	gensymerror(ERR_NOANDREG,tp->sname);
												}
                        tp1 = xalloc(sizeof(TYP));
                        tp1->size = 4;
                        tp1->type = bt_pointer;
                        tp1->btp = tp;
                        tp1->val_flag = 0;
                        tp1->lst.head = 0;
                        tp1->sname = 0;
												tp = tp1;
												break;
                case kw_sizeof:
                        getsym();
                        needpunc(openpa,0);
												if (castbegin(lastst)) {
sizeof_cast:
                        	decl(0);
													decl1();
												}
												else if (lastst == id) {
													SYM *sp = gsearch(lastid);
													if (!sp)
														sp = search(lastid,&lsyms);
													if (sp) {
															goto sizeof_primary;
													}
													if ((sp = search(nm,&tagtable)) != 0) {
														head = sp->tp;
														getsym();
													}
													else {
														generror(ERR_SIZE,0,0);
														head = 0;
													}
												}
												else if (lastst == kw_enum) {
													getsym();
													if (lastst == id) {
														SYM *sp;
														if ((sp = search(nm,&tagtable)) != 0) {
															head = sp->tp;
															getsym();
														}
														else goto sizeof_primary;
													}
												}
												else  {
													ENODE *node = 0;
sizeof_primary:
													head = unary(&node);
												}
                        if( head != 0 ) {
																if (head->size == 0)
																	generror(ERR_SIZE,0,0);
                                ep1 = makenode(en_icon,(char *)head->size,0);
												}
                        else    {
                                generror(ERR_IDEXPECT,0,0);
                                ep1 = makenode(en_icon,(char *)0,0);
                                }
												goodcode &= ~GF_ASSIGN;
                        tp = &stdint;
                        needpunc(closepa,0);
                        break;
                default:
                        tp = primary(&ep1);
                        break;
                }
/* Dereference if necessary */
				if (globaldref) {
					globaldref = 0;
					if (tp->type != bt_ptrfunc) {
	 					if( tp == 0 ) {
               generror(ERR_IDEXPECT,0,0);
				       return 0;
  	      	}
						if( tp->btp == 0 ) {
		     			 generror(ERR_DEREF,0,0);
						}
						else
							if (tp->btp->type != bt_void)
         				tp = tp->btp;
						ep1->cflags = tp->cflags;
  	      	if( tp->val_flag == 0 )
    	        tp = deref(&ep1,tp);
					}
				}
        *node = ep1;
        return tp;
}
TYP *maxsize(TYP *tp1, TYP *tp2)
/*
 * return the type that has the maximum size
 */
{
	if (tp1->type > tp2->type)
		return tp1;
	return tp2;
}
TYP     *forcefit(ENODE **node1,TYP *tp1,ENODE **node2,TYP *tp2, int max, int allowpointers)
/*
 * compare two types and determine if they are compatible for purposes
 * of the current operation.  Return an appropriate type.  Also checks for
 * dangerous pointer conversions...
 */
{				int error = ERR_MISMATCH;
				TYP *tp3;
       	switch( tp1->type ) {
								case bt_void:
												if (tp2->type != bt_void && tp2->type != bt_matchall)
													break;
												return tp1;
                case bt_long:
												if (isintconst((*node2)->nodetype)) {
													if (isscalar(tp2) || (!prm_cplusplus && (tp2->type == bt_pointer || tp2 ->type == bt_ptrfunc)))
														return tp2;
													if (tp2->type != bt_matchall)
														break;
												}
                case bt_matchall:
                        if( tp2->type == bt_matchall)
													return(&stdint);
                        else if( tp2->type == bt_pointer)
                          return tp2;
												else if  (allowpointers && (tp2->type == bt_pointer || tp2->type == bt_ptrfunc))
													return &stdint;
												else if (isscalar(tp2))
													if (max)
														return(maxsize(tp1,tp2));
													else
														return(tp1);
                        break;
                case bt_pointer:
												if (!prm_cplusplus && !max && (tp2->type == bt_short || tp2->type == bt_unsignedshort
													|| tp2->type == bt_char || tp2->type == bt_unsignedchar)) {
														error = ERR_SHORTPOINTER;
														break;
												}
                        if( tp2->type == bt_pointer ||tp2->type == bt_ptrfunc) {
											  	if  (allowpointers) {
												  	while (tp1->type == bt_pointer && tp2->type == bt_pointer) {
															tp1 = tp1->btp;
															tp2 = tp2->btp;
														}
														if (tp1->type != tp2 ->type || tp1->size != tp2->size)
															if (tp1->type != bt_void && tp2->type != bt_void)
																generror(ERR_SUSPICIOUS,0,0);
													  return &stdint;
													}
													else
                            return tp1;
												}
												if (!prm_cplusplus && isscalar(tp2))
													if (max)
														return(maxsize(tp1,tp2));
													else
														return(tp1);
                        break;
								case bt_enum:
                case bt_unsignedshort:
                case bt_short:
												if (isintconst((*node2)->nodetype)) {
													if (!max && ((*node2)->v.i < -65536L || ((*node2)->v.i > 65535L))) {
														error = ERR_LOSTCONV;
														break;
													}
													return tp1;
												}
												else
					 								if (!max && (tp2->type == bt_long || tp2->type == bt_unsigned || (!prm_cplusplus && (tp2->type == bt_pointer) || tp2->type == bt_ptrfunc))) {
						 								error = ERR_LOSTCONV;
							 	 						break;
													}
											  if  (allowpointers && tp2->type == bt_pointer)
													return &stdint;
												else if (isscalar(tp2))
													if (max)
														return(maxsize(tp1,tp2));
													else
														return(tp1);
												break;
                case bt_char:
                case bt_unsignedchar:
												if (isintconst((*node2)->nodetype)) {
													if (!max && ((*node2)->v.i < -256 || ((*node2)->v.i > 255))) {
														error = ERR_LOSTCONV;
														break;
													}
													return tp1;
												}
												else
					 								if (!max && (tp2->type == bt_long || tp2->type == bt_unsigned || ( !prm_cplusplus && tp2->type == bt_pointer)
						 									|| tp2->type == bt_short || tp2->type == bt_unsignedshort)) {
						 								error = ERR_LOSTCONV;
							 	 						break;
													}
											  if  (allowpointers && (tp2->type == bt_pointer || tp2->type == bt_ptrfunc))
													return &stdint;
												else if (isscalar(tp2))
													if (max)
														return(maxsize(tp1,tp2));
													else
														return(tp1);
												break;
								case bt_float:
								case bt_double:
								case bt_longdouble:
												if (isscalar(tp2))
													return(tp1);
												break;
                case bt_unsigned:
												if (isintconst((*node1)->nodetype)) {
													if (isscalar(tp2) || (!prm_cplusplus && tp2->type == bt_pointer))
														return tp2;
													break;
												}
                        if( !prm_cplusplus && (tp2->type == bt_pointer  || tp2->type == bt_ptrfunc))
											  	if  (allowpointers)
														return &stdint;
													else
														return tp2;
											  if (isscalar(tp2))
                          return tp2;
                        break;
								case bt_ptrfunc:
												if (!prm_cplusplus && (tp2->type == bt_short || tp2->type == bt_unsignedshort
													|| tp2->type == bt_char || tp2->type == bt_unsignedchar)) {
														error = ERR_SHORTPOINTER;
														break;
												}
												if (tp2->type == bt_pointer)
													tp3 = tp2->btp;
												else
													tp3 = tp2;
												if (tp3->type == bt_func || tp3->type == bt_ifunc || tp3->type == bt_ptrfunc
															|| (!prm_cplusplus && tp2->type == bt_pointer))
											  	if  (allowpointers)
													  return &stdint;
													else
                            return tp1;
												break;
								case bt_func:
								case bt_ifunc:
												if (tp2->type == bt_func || tp2->type == bt_ifunc)
													return tp1;
												break;
                }
#ifdef CPLUSPLUS
				if (error == ERR_MISMATCH && prm_cplusplus)
					genmismatcherror(tp2,tp1);
				else
#endif
        	generror( error,0,0 );
        return tp1;
}

int     isscalar(TYP *tp)
/*
 * this is misnamed... it checks for ANY basic numeric type
 */
{       return  tp->type == bt_char ||  tp->type == bt_unsignedchar ||
                tp->type == bt_short || tp->type == bt_unsignedshort ||
                tp->type == bt_long || tp->type == bt_unsigned || tp->type == bt_enum
								|| tp->type == bt_float || tp->type == bt_double || tp->type == bt_longdouble;
}
void checknp(TYP *tp1,TYP*tp2,ENODE *ep1, ENODE *ep2)
/*
 * look for non-portable pointer conversions
 */
{

#ifdef CPLUSPLUS
	if (prm_cplusplus)
		return;
#endif
	if (tp1->type == bt_pointer || tp2->type == bt_pointer)
		if (tp1->type != tp2->type) 
			if ((!isintconst(ep1->nodetype) || ep1->v.i != 0) && (!isintconst(ep2->nodetype) || ep2->v.i != 0))
				generror(ERR_NONPORT,0,0);
}
TYP     *multops(ENODE **node)
/*
 *      multops parses the multiply priority operators. the syntax of
 *      this group is:
 *
 *              unary
 *              multop * unary
 *              multop / unary
 *              multop % unary
 */
{       ENODE    *ep1, *ep2;
        TYP             *tp1, *tp2;
        int		      oper;
        tp1 = unary(&ep1);
        if( tp1 == 0 )
                return 0;
        while( lastst == star || lastst == divide || lastst == modop ) {
                oper = lastst;
                getsym();       /* move on to next unary op */
                tp2 = unary(&ep2);
								goodcode &= ~GF_ASSIGN;
                if( tp2 == 0 ) {
                        generror(ERR_IDEXPECT,0,0);
                        *node = ep1;
                        return tp1;
                        }
                tp1 = forcefit(&ep1,tp1,&ep2,tp2,TRUE,FALSE);
                switch( oper ) {
                        case star:
                                if( tp1->type == bt_unsigned ||tp1->type == bt_unsignedchar || tp1->type == bt_unsignedshort)
                                        ep1 = makenode(en_umul,ep1,ep2);
                                else
                                        ep1 = makenode(en_mul,ep1,ep2);
                                break;
                        case divide:
                                if( tp1->type == bt_unsigned ||tp1->type == bt_unsignedchar || tp1->type == bt_unsignedshort)
                                        ep1 = makenode(en_udiv,ep1,ep2);
                                else
                                        ep1 = makenode(en_div,ep1,ep2);
                                break;
                        case modop:
                                if( tp1->type == bt_unsigned ||tp1->type == bt_unsignedchar || tp1->type == bt_unsignedshort)
                                        ep1 = makenode(en_umod,ep1,ep2);
                                else
                                        ep1 = makenode(en_mod,ep1,ep2);
																floatcheck(ep1);
                                break;
                        }
                }
        *node = ep1;
        return tp1;
}

TYP     *addops(ENODE **node)
/*
 *      addops handles the addition and subtraction operators.
 */
{       ENODE    *ep1, *ep2, *ep3;
        TYP             *tp1, *tp2;
        int             oper;
        tp1 = multops(&ep1);
        if( tp1 == 0 )
                return 0;
        while( lastst == plus || lastst == minus ) {
                oper = (lastst == plus);
                getsym();
                tp2 = multops(&ep2);
								goodcode &= ~GF_ASSIGN;
                if( tp2 == 0 ) {
                        generror(ERR_IDEXPECT,0,0);
                        *node = ep1;
                        return tp1;
                        }
                if( tp1->type == bt_pointer ) {
										if (tp2->type == bt_pointer) {
                			forcefit(&ep1,tp1,&ep2,tp2,TRUE,FALSE);
                			ep1 = makenode( oper ? en_add : en_sub,ep1,ep2);
											ep1->cflags = ep1->v.p[0]->cflags | ep2->cflags;
											if (tp1->btp->size == 0)
												generror(ERR_ZEROPTR,0,0);
											else
												if (tp1->btp->size > 1)
													ep1 = makenode(en_pdiv,ep1,makenode(en_icon,(char *)tp1->btp->size,0));
											ep1->cflags = ep1->v.p[0]->cflags;
			 							  tp1 = &stdint;
											continue;
										}
										else {
                        tp2 = forcefit(0,&stdint,&ep2,tp2,TRUE,FALSE);
												if (tp1->btp->size == 0)
													generror(ERR_ZEROPTR,0,0);
                        ep3 = makenode(en_icon,(char *)tp1->btp->size,0);
                        ep2 = makenode(en_pmul,ep3,ep2);
												ep2->cflags = ep2->v.p[1]->cflags;
                        }
								}
                else if( tp2->type == bt_pointer ) {
                        tp1 = forcefit(0,&stdint,&ep1,tp1,TRUE,FALSE);
												if (tp2->btp->size == 0)
													generror(ERR_ZEROPTR,0,0);
                        ep3 = makenode(en_icon,(char *)tp2->btp->size,0);
                        ep1 = makenode(en_pmul,ep3,ep1);
												ep1->cflags = ep1->v.p[1]->cflags;
                        }
                tp1 = forcefit(&ep1,tp1,&ep2,tp2,TRUE,FALSE);
                ep1 = makenode( oper ? en_add : en_sub,ep1,ep2);
								ep1->cflags = ep1->v.p[1]->cflags | ep2->cflags;
                }
exit:
        *node = ep1;
        return tp1;
}

TYP     *shiftop(ENODE **node)
/*
 *      shiftop handles the shift operators << and >>.
 */
{       ENODE    *ep1, *ep2;
        TYP             *tp1, *tp2;
        int             oper;
        tp1 = addops(&ep1);
        if( tp1 == 0)
                return 0;
        while( lastst == lshift || lastst == rshift) {
                oper = (lastst == lshift);
                getsym();
                tp2 = addops(&ep2);
								goodcode &= ~GF_ASSIGN;
                if( tp2 == 0 )
                        generror(ERR_IDEXPECT,0,0);
                else    {
                        tp1 = forcefit(&ep1,tp1,&ep2,tp2,TRUE,FALSE);
												if (tp1->type == bt_unsigned ||
														tp1->type == bt_unsignedchar ||
														tp1->type == bt_unsignedshort)
                        	ep1 = makenode(oper ? en_lsh : en_rsh,ep1,ep2);
												else
                        	ep1 = makenode(oper ? en_alsh : en_arsh,ep1,ep2);
                        }
								floatcheck(ep1);
                }
        *node = ep1;
        return tp1;
}

TYP     *relation(ENODE **node)
/*
 *      relation handles the relational operators < <= > and >=.
 */
{       ENODE    *ep1, *ep2;
        TYP             *tp1, *tp2;
        int             nt;
        tp1 = shiftop(&ep1);
        if( tp1 == 0 )
                return 0;
        for(;;) {
                switch( lastst ) {

                        case lt:
                                if( tp1->type == bt_unsigned || tp1->type == bt_unsignedchar || tp1->type == bt_unsignedshort)
                                        nt = en_ult;
                                else
                                        nt = en_lt;
                                break;
                        case gt:
                                if( tp1->type == bt_unsigned || tp1->type == bt_unsignedchar || tp1->type == bt_unsignedshort)
                                        nt = en_ugt;
                                else
                                        nt = en_gt;
                                break;
                        case leq:
                                if( tp1->type == bt_unsigned || tp1->type == bt_unsignedchar || tp1->type == bt_unsignedshort)
                                        nt = en_ule;
                                else
                                        nt = en_le;
                                break;
                        case geq:
                                if( tp1->type == bt_unsigned || tp1->type == bt_unsignedchar || tp1->type == bt_unsignedshort)
                                        nt = en_uge;
                                else
                                        nt = en_ge;
                                break;
                        default:
                                goto fini;
                        }
                getsym();
                tp2 = shiftop(&ep2);
								goodcode &= ~GF_ASSIGN;
                if( tp2 == 0 )
                        generror(ERR_IDEXPECT,0,0);
                else    {
												checknp(tp1,tp2,ep1,ep2);
                        tp1 = forcefit(&ep1,tp1,&ep2,tp2,TRUE,TRUE);
                        ep1 = makenode(nt,ep1,ep2);
                        }
                }
fini:   *node = ep1;
        return tp1;
}

TYP     *equalops(ENODE **node)
/*
 *      equalops handles the equality and inequality operators.
 */
{       ENODE    *ep1, *ep2;
        TYP             *tp1, *tp2;
        int             oper;
        tp1 = relation(&ep1);
        if( tp1 == 0 )
                return 0;
        while( lastst == eq || lastst == neq ) {
                oper = (lastst == eq);
                getsym();
                tp2 = relation(&ep2);
								goodcode &= ~GF_ASSIGN;
								checknp(tp1,tp2,ep1,ep2);
                if( tp2 == 0 )
                        generror(ERR_IDEXPECT,0,0);
                else    {
                        tp1 = forcefit(&ep1,tp1,&ep2,tp2,TRUE,TRUE);
                        ep1 = makenode( oper ? en_eq : en_ne,ep1,ep2);
                        }
                }
        *node = ep1;
        return tp1;
}

TYP     *binop(ENODE **node,TYP *(*xfunc)(),int nt,int sy)
/*
 *      binop is a common routine to handle all of the legwork and
 *      error checking for bitand, bitor, bitxor, andop, and orop.
 */
{       ENODE    *ep1, *ep2;
        TYP             *tp1, *tp2;
        tp1 = (*xfunc)(&ep1);
        if( tp1 == 0 )
                return 0;
        while( lastst == sy ) {
                getsym();
                tp2 = (*xfunc)(&ep2);
								goodcode &= ~GF_ASSIGN;
                if( tp2 == 0 )
                        generror(ERR_IDEXPECT,0,0);
                else    {
                        tp1 = forcefit(&ep1,tp1,&ep2,tp2,TRUE,sy == lor || sy == land);
                        ep1 = makenode(nt,ep1,ep2);
                        }
								floatcheck(ep1);
                }
        *node = ep1;
        return tp1;
}

TYP     *bitand(ENODE **node)
/*
 *      the bitwise and operator...
 */
{       return binop(node,equalops,en_and,and);
}

TYP     *bitxor(ENODE **node)
{       return binop(node,bitand,en_xor,uparrow);
}

TYP     *bitor(ENODE **node)
{       return binop(node,bitxor,en_or,or);
}

TYP     *andop(ENODE **node)
{       return binop(node,bitor,en_land,land);
}

TYP     *orop(ENODE **node)
{       return binop(node,andop,en_lor,lor);
}

TYP     *conditional(ENODE **node)
/*
 *      this routine processes the hook operator.
 */
{       TYP             *tp1, *tp2, *tp3;
        ENODE    *ep1, *ep2, *ep3;
        tp1 = orop(&ep1);       /* get condition */

        if( tp1 == 0 )
                return 0;
        if( lastst == hook ) {
								int gcode1,gcode2;
								goodcode &=~GF_ASSIGN;
                getsym();
                if( (tp2 = conditional(&ep2)) == 0) {
                        generror(ERR_IDEXPECT,0,0);
												goodcode &= ~GF_ASSIGN;
                        goto cexit;
                        }
                needpunc(colon,0);
								gcode2 = goodcode;
								goodcode &=~GF_ASSIGN;
                if( (tp3 = conditional(&ep3)) == 0) {
                        generror(ERR_IDEXPECT,0,0);
												goodcode &= ~GF_ASSIGN;
                        goto cexit;
                        }
								gcode1 = gcode2 & goodcode;
								goodcode = (goodcode &~GF_ASSIGN) | gcode1;
                tp1 = forcefit(&ep2,tp2,&ep3,tp3,TRUE,FALSE);
                ep2 = makenode(en_void,ep2,ep3);
                ep1 = makenode(en_cond,ep1,ep2);
                }
cexit:
			  *node = ep1;
        return tp1;
}
TYP			*autoasnop(ENODE **node, SYM *sp)
/*
 * 			Handle assignment operators during auto init of local vars
 */
{       ENODE    *ep1, *ep2;
        TYP             *tp1, *tp2;

        if( (tp1 = sp->tp) == 0 ) {
					tp1 = &stdmatch;
					*node = makenode(en_nacon,&undef,0);
					gensymerror(ERR_UNDEFINED,nm);
          tp1 = deref(&ep1,tp1);
          return tp1;       /* guard against untyped entries */
        }
        if( sp->storage_class != sc_auto && sp->storage_class != sc_autoreg)
          gensymerror(ERR_ILLCLASS2,sp->name);
				if (sp->storage_class == sc_auto)
        	ep1 = makenode(en_autocon,sp,0);
				else if (sp->storage_class == sc_autoreg)
        	ep1 = makenode(en_autoreg,sp,0);
				if (tp1) {
					tp1->uflags |= UF_DEFINED | UF_USED;
				}
				if ((tp1->uflags & UF_CANASSIGN) && !(goodcode & GF_INLOOP))
					tp1->uflags |=  UF_ASSIGNED;
        if( tp1->val_flag == 0)
          tp1 = deref(&ep1,tp1);
        if( tp1 == 0 )
                return 0;
        tp2 = asnop(&ep2,tp1);
        if( tp2 == 0) {
          generror(ERR_LVALUE,0,0);
					*node = makenode(en_nacon,&undef,0);
				}
        else    {
					if (tp1->type == bt_struct || tp1->type == bt_union) {
						if (!checktypeassign(tp1,tp2)) {
             	generror(ERR_LVALUE,0,0);
							*node = makenode(en_nacon,&undef,0);
						}
						else {
							checknp(tp1,tp2,ep1,ep2);
							if (ep2->nodetype == en_fcallb || ep2->nodetype == en_pfcallb)
								if (ep2->nodetype == en_pfcallb) 
									ep1 = makenode(en_pcallblock,ep1,ep2);
								else
									ep1 = makenode(en_callblock,ep1,ep2);
							else
								ep1 = makenode(en_moveblock,ep1,ep2);
							ep1->size = tp1->size;
							*node = ep1;
						}
					}
          else    {
						if (tp1->type == bt_ref) {
							if (lvalue(ep2)) {
							 	while (castvalue(ep2))
							 		ep2 = ep2->v.p[0];
								ep2 = ep2->v.p[0];
								tp1 = tp1->btp;
							}
							else {
								tp1 = tp1->btp;
								gensymerror(ERR_TEMPINIT,sp->name);
								ep2 = makenode(en_refassign,dummyvar(tp1->size,tp1),ep2);
							}
						}
						else if (tp1->val_flag) {
							generror(ERR_LVALUE,0,0);
							*node = makenode(en_nacon,&undef,0);
							return tp1;
						}
						
						checknp(tp1,tp2,ep1,ep2);
  	        tp1 = forcefit(&ep1,tp1,&ep2,tp2,FALSE,FALSE);
    	      *node = makenode(en_assign,ep1,ep2);
					}
        }
				return(tp1);
}
void asncombine(ENODE **node)
/*
 * a simple optimization which turns an equate into a functional equate
 * if possible.  Code gen proceeds a little more cleanly if this info
 * is known.
 */
{
	ENODE *var = (*node)->v.p[0];
	ENODE *exp = (*node)->v.p[1];
  ENODE *var2 = exp->v.p[0];
		int op = 0;
		switch(exp->nodetype) {
			case en_add:
				if (equalnode(var,var2)) {
					op = en_asadd;
					break;
				} else return;
			case en_sub:
				if (equalnode(var,var2)) {
					op = en_assub;
					break;
				} else return;
			case en_mul:
				if (equalnode(var,var2)) {
					op = en_asmul;
					break;
				} else return;
			case en_umul:
				if (equalnode(var,var2)) {
					op = en_asumul;
					break;
				} else return;
			case en_div:
				if (equalnode(var,var2)) {
					op = en_asdiv;
					break;
				} else return;
			case en_udiv:
				if (equalnode(var,var2)) {
					op = en_asudiv;
					break;
				} else return;
			case en_mod:
				if (equalnode(var,var2)) {
					op = en_asmod;
					break;
				} else return;
			case en_umod:
				if (equalnode(var,var2)) {
					op = en_asumod;
					break;
				} else return;
			case en_lsh:
				if (equalnode(var,var2)) {
					op = en_aslsh;
					break;
				} else return;
			case en_alsh:
				if (equalnode(var,var2)) {
					op = en_asalsh;
					break;
				} else return;
			case en_rsh:
				if (equalnode(var,var2)) {
					op = en_asrsh;
					break;
				} else return;
			case en_arsh:
				if (equalnode(var,var2)) {
					op = en_asarsh;
					break;
				} else return;
			case en_and:
				if (equalnode(var,var2)) {
					op = en_asand;
					break;
				} else return;
			case en_or:
				if (equalnode(var,var2)) {
					op = en_asor;
					break;
				} else return;
			case en_xor:
				if (equalnode(var,var2)) {
					op = en_asxor;
					break;
				} else return;
			default:
					return;
		}
		exp->nodetype = op;
		(*node) = exp;
}
TYP     *asnop(ENODE **node,TYP *tptr)
/*
 *      asnop handles the assignment operators.
 */
{       ENODE    *ep1, *ep2, *ep3;
        TYP             *tp1, *tp2,*oldastyp;
        int             op;
				oldastyp = asntyp;
				if (tptr) 
					asntyp = tptr;
				else
					asntyp = 0;
        tp1 = conditional(&ep1);
				lastsym = 0;
				if (!tptr)
					asntyp = tp1;
        if( tp1 == 0 )
                return 0;
        for(;;) {
                switch( lastst ) {
                        case assign:
                                op = en_assign;
ascomm:                         getsym();
                                tp2 = asnop(&ep2,asntyp);
ascomm2:                        ep3 = ep1;
																if (tp1) {
																	tp1->uflags |= UF_DEFINED;
																}
																goodcode |= GF_ASSIGN;
																if ((tp1->uflags & UF_CANASSIGN) && !(goodcode & GF_INLOOP))
																	tp1->uflags |=  UF_ASSIGNED;
																if( tp2 == 0 || tp1 == 0)
                                        generror(ERR_LVALUE,0,0);
																else {
																		if (tp1->type == bt_struct || tp1->type == bt_union) {
																				if (!checktypeassign(tp1,tp2)) {
                                        	generror(ERR_LVALUE,0,0);
																				}
																				else {
																					if (op != en_asadd && op != en_assub)
																						checknp(tp1,tp2,ep1,ep2);
																					if (ep2->nodetype == en_fcallb || ep2->nodetype == en_pfcallb)
																						if (ep2->nodetype == en_pfcallb) 
																							ep1 = makenode(en_pcallblock,ep1,ep2);
																						else
																							ep1 = makenode(en_callblock,ep1,ep2);
																					else
																						ep1 = makenode(en_moveblock,ep1,ep2);
																					ep1->size = tp1->size;
																				}
																		}
                                  else    {
																				if (!lvalue(ep1)) 
          																generror(ERR_LVALUE,0,0);
																				if (op != en_asadd && op != en_assub)
																					checknp(tp1,tp2,ep1,ep2);
                                        tp1 = forcefit(&ep1,tp1,&ep2,tp2,FALSE,FALSE);
                                        ep1 = makenode(op,ep1,ep2);
                                        }
																}
															  if (ep3->cflags & DF_CONST)
																  generror(ERR_MODCONS,0,0);
  															if (op == en_assign) {
	  															asncombine(&ep1);
																	op = ep1->nodetype;
																}
		  													if (op == en_asmod || op == en_asumod ||
			  																op ==en_aslsh || op== en_asrsh ||
				  															op == en_asalsh || op== en_asarsh ||
					  														op == en_asand || op== en_asor || op == en_asxor)
						  										floatcheck(ep1);
                                break;
                        case asplus:
                                op = en_asadd;
		
ascomm3:												getsym();
																tp2 = asnop(&ep2,asntyp);
                                if( tp1->type == bt_pointer ) {
																				if (tp1->btp->size == 0)
																					generror(ERR_ZEROPTR,0,0);
                                        ep3 = makenode(en_icon,(char *)tp1->btp->size,0);
                                        ep2 = makenode(en_pmul,ep2,ep3);
																				tp2 = tp1;
                                        }
                                goto ascomm2;
                        case asminus:
                                op = en_assub;
                                goto ascomm3;
                        case astimes:
                                if( tp1->type == bt_unsigned ||tp1->type == bt_unsignedchar || tp1->type == bt_unsignedshort)
                                        op = en_asumul;
                                else
                                        op =en_asmul;
                                goto ascomm;
                        case asdivide:
                                if( tp1->type == bt_unsigned ||tp1->type == bt_unsignedchar || tp1->type == bt_unsignedshort)
                                        op = en_asudiv;
                                else
                                        op =en_asdiv;
                                goto ascomm;
                        case asmodop:
                                if( tp1->type == bt_unsigned ||tp1->type == bt_unsignedchar || tp1->type == bt_unsignedshort)
                                        op = en_asumod;
                                else
                                        op =en_asmod;
                                goto ascomm;
                        case aslshift:
														if (tp1->type == bt_unsigned ||
																tp1->type == bt_unsignedchar ||
																tp1->type == bt_unsignedshort)
															op = en_aslsh;
														else
                              op = en_asalsh;
                                goto ascomm;
                        case asrshift:
														if (tp1->type == bt_unsigned ||
																tp1->type == bt_unsignedchar ||
																tp1->type == bt_unsignedshort)
															op = en_asrsh;
														else
                              op = en_asarsh;
                                goto ascomm;
                        case asand:
                                op = en_asand;
                                goto ascomm;
                        case asor:
                                op = en_asor;
                                goto ascomm;
                        case asxor:
                                op = en_asxor;
                                goto ascomm;
                        default:
                                goto asexit;
                        }
                }
asexit: *node = ep1;
				asntyp = oldastyp;
        return tp1;
}

TYP     *exprnc(ENODE **node)
/*
 *      evaluate an expression where the comma operator is not legal.
 * e.g. a function argument
 */
{       TYP     *tp;
        tp = asnop(node,0);
        if( tp == 0 )
                *node = 0;
				else if ((*node)->nodetype == en_fcallb || (*node)->nodetype == en_pfcallb) {
						if ((*node)->nodetype == en_pfcallb) 
							(*node) = makenode(en_pcallblock,dummyvar(tp->size,tp),(*node));
  					else
							(*node) = makenode(en_callblock,dummyvar(tp->size,tp),(*node));
				}
				if (tp && tp->type == bt_void)
					goodcode |= GF_ASSIGN;
        return tp;
}

TYP     *commaop(ENODE **node)
/*
 *      evaluate the comma operator. comma operators are kept as
 *      void nodes.
 */
{       TYP             *tp1;
        ENODE    *ep1, *ep2;
	int ocode;
        tp1 = asnop(&ep1,0);
        if( tp1 == 0 )
                return 0;
        if( lastst == comma ) {
								ocode = goodcode | ~GF_ASSIGN;
								goodcode &= ~GF_ASSIGN;
								getsym();
                tp1 = commaop(&ep2);
								goodcode &= ocode;
                if( tp1 == 0 ) {
                        generror(ERR_IDEXPECT,0,0);
                        goto coexit;
                        }
                ep1 = makenode(en_void,ep1,ep2);
                }
coexit: *node = ep1;
        return tp1;
}

TYP     *expression(ENODE **node)
/*
 *      evaluate an expression where all operators are legal.
 */
{       TYP     *tp;
        tp = commaop(node);
        if( tp == 0 )
                *node = 0;
				else if ((*node)->nodetype == en_fcallb || (*node)->nodetype == en_pfcallb) {
						if ((*node)->nodetype == en_pfcallb) 
							(*node) = makenode(en_pcallblock,dummyvar(tp->size,tp),(*node));
  					else
							(*node) = makenode(en_callblock,dummyvar(tp->size,tp),(*node));
				}
				if (tp && tp->type == bt_void)
					goodcode |= GF_ASSIGN;
        return tp;
}