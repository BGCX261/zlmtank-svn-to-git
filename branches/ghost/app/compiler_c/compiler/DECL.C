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
 * handle ALL declarative statements
 */
#include        <stdio.h>
#include				<limits.h>
#include        "expr.h"
#include        "c.h"
#include        "errors.h"

extern int stdinttype,stdunstype,stdintsize, stdldoublesize,stdaddrsize;
extern int strucadd,strucmod;

extern char laststr[];
extern int global_flag;
extern TABLE gsyms,lsyms;
extern	SNODE *cbautoinithead;
extern int prm_packing;
extern int prm_revbits;
extern int prm_68020;
extern int prm_ansi;
extern HASHREC **globalhash;
extern SYM *currentfunc;
extern int prm_cplusplus;
extern char * tn_unnamed;
extern TYP stdconst;
extern enum e_sym lastst;
extern char lastid[];
extern int block_nesting;
extern long nextlabel;

char *Cstr = "C";

TABLE *active_table;		/* This is so enums will not be put in structre tabs */

int mangleflag;
int skm_declopenpa[] = { semicolon, comma,openpa, eq,0 };
int skm_declclosepa[] = { semicolon, comma,closepa, eq,0 };
int skm_declclosebr[] = { comma,closebr, eq,0 };
int skm_declend[] = { semicolon, comma, end, eq,0 };
int skm_decl[] = {semicolon,kw_int, kw_long, kw_short, kw_char, kw_float,
		kw_double, kw_struct, kw_union, kw_enum, kw_unsigned, kw_signed, kw_auto,
		kw_extern, kw_static, kw_register, kw_typedef, id, kw_void, 0 };

TYP             *head = 0;
TYP             *tail = 0;
char            declid[100];
TABLE           tagtable = {0,0};
int ispascal;

static					SYM *lastdecl;
static 					int pcount=0;
static int bittype = -1;
static int curbit = 0;
static int curofs = 0;
static int manglelevel;
static int intflag;

void declini(void)
/*
 * init
 */
{
	bittype = -1;
	head = tail = 0;
	declid[0] = 0;
	tagtable.head = tagtable.tail = 0;
	pcount = 0;
	mangleflag = TRUE;
}
	
int     imax(int i,int j)
{       return (i > j) ? i : j;
}

char    *litlate(char *s)
{       char    *p;
        p = xalloc(strlen(s) + 1);
        strcpy(p,s);
        return p;
}

TYP     *maketype(int bt,int siz)
/*
 * Create a type list entry
 */
{       TYP     *tp;
				
        tp = xalloc(sizeof(TYP));
        tp->val_flag = 0;
				tp->bits = tp->startbit = -1;
        tp->size = siz;
        tp->type = bt;
        tp->sname = 0;
				tp->cflags = 0;
				tp->uflags = 0;
        tp->lst.head = tp->lst.tail = 0;
				tp->tdef = 0;
        return tp;
}
TYP *cponetype(TYP *t)
/*
 * Copy a entry
 */
{				TYP *tp;
	tp = xalloc(sizeof(TYP));
	tp->type = t->type;
	tp->val_flag = t->val_flag;
	tp->cflags = t->cflags;
	tp->bits = t->bits;
	tp->startbit = t->startbit;
	tp->size = t->size;
	tp->lst = t->lst;
	tp->btp = t->btp;
	tp->sname = t->sname;
	tp->uflags = t->uflags;
	tp->tdef = t->tdef;
	return tp;
}
TYP *copytype(TYP *itp, int flags)
/*
 * copy a type tree
 */
{
	TYP *head, *tail,*x=itp;
	while (x->type == bt_pointer)
		x = x->btp;
	if (x->type == bt_struct || x->type == bt_union)
		if (!x->lst.head) {
			itp->cflags = flags;
			return head = tail = itp;
		}
	head = tail = cponetype(itp);
	while (itp->type == bt_pointer) {
		tail = tail->btp = cponetype(itp->btp);
		itp = itp->btp;
	}
	if (itp->type == bt_func || itp->type == bt_ptrfunc || itp->type == bt_ifunc) {
			tail->btp = cponetype(itp->btp);
	}
	else {
		tail->cflags |= flags;
	}
	return head;
}
void     decl(TABLE *table)
/*
 * parse the declare keywords and create a basic type
 */
{
				int tp;
				SYM *sp;
				int flags = 0;
				while (lastst == kw_const || lastst == kw_volatile) {
					if (lastst == kw_const)
						flags |= DF_CONST;
					else
						flags |= DF_VOL;
					getsym();
				}
        switch (lastst) {
								case kw_void:
                        head = tail = maketype(bt_void,0);
												getsym();
												break;
                case kw_char:
                        head = tail = maketype(bt_char,1);
                        getsym();
                        break;
                case kw_short:
												getsym();
												tp = bt_short;
												if (lastst == kw_unsigned) {
													tp = bt_unsignedshort;
													getsym();
												}
												if (lastst == kw_int)
													getsym();
                        head = tail = maketype(tp,2);
                        break;
                case kw_int: 
                        head = tail = maketype(stdinttype,stdintsize);
                        getsym();
                        break;
                case kw_unsigned:
												getsym();
												switch (lastst) {
													case kw_char:
														getsym();
                        		head = tail = maketype(bt_unsignedchar,1);
														break;
													case kw_short:
														getsym();
														if (lastst == kw_int)
															getsym();
                        		head = tail = maketype(bt_unsignedshort,2);
														break;
													case kw_long:
														getsym();
														if (lastst == kw_long) {
															getsym();
															generror(ERR_LONGLONG,0,0);
														}
														if (lastst == kw_int)
															getsym();
                        		head = tail = maketype(bt_unsigned,4);
														break;
													case kw_int:
														getsym();
													default:
                        		head = tail = maketype(stdunstype,stdintsize);
														break;
												}
                        break;
                case kw_signed:
												getsym();
												switch (lastst) {
													case kw_char:
														getsym();
                        		head = tail = maketype(bt_char,1);
														break;
													case kw_short:
														getsym();
														if (lastst == kw_int)
															getsym();
                        		head = tail = maketype(bt_short,2);
														break;
													case kw_long:
														getsym();
														if (lastst == kw_long) {
															getsym();
															generror(ERR_LONGLONG,0,0);
														}
														if (lastst == kw_int)
															getsym();
                        		head = tail = maketype(bt_long,4);
														break;
													case kw_int:
														getsym();
													default:
                        		head = tail = maketype(stdinttype,stdintsize);
														break;
												}
                        break;
                case id:                /* no type declarator */
												if ((sp = gsearch(lastid)) != 0 && sp->storage_class == sc_type) {
													head = tail = copytype(sp->tp,flags);
													getsym();
												}
												else
                       		head = tail = maketype(bt_long,4);
                        break;
                case kw_float:
                        head = tail = maketype(bt_float,4);
                        getsym();
                        break;
								case kw_long:
												getsym();
												if (lastst==kw_double) {
													getsym();
                        	head = tail = maketype(bt_longdouble,stdldoublesize);
													break;
												}
												if (lastst==kw_float) {
													getsym();
                        	head = tail = maketype(bt_double,8);
													break;
												}
												if (lastst == kw_long) {
													getsym();
													generror(ERR_LONGLONG,0,0);
												}
												tp = bt_long;
												if (lastst == kw_unsigned) {
													tp = bt_unsigned;
													getsym();
												}
												if (lastst == kw_long) {
													getsym();
													generror(ERR_LONGLONG,0,0);
												}
												if (lastst == kw_int)
													getsym();
                        head = tail = maketype(tp,4);
                        break;
                case kw_double:
                        head = tail = maketype(bt_double,8);
                        getsym();
                        break;
                case kw_enum:
                        getsym();
                        declenum(table);
                        break;
                case kw_struct:
                        getsym();
                        declstruct(table, bt_struct,flags);
                        break;
                case kw_union:
                        getsym();
                        declstruct(table, bt_union,flags);
                        break;
                }
	head->cflags |= flags;
	head->uflags |= UF_CANASSIGN;
}

void decl1(void)
/*
 * Modifiers that could occur BEFORE the name of the var
 */
{       TYP     *temp1, *temp2, *temp3, *temp4;
lp:
        switch (lastst) {
								case kw_const:
									head->cflags |= DF_CONST;
									getsym();
									goto lp;
								case kw_volatile:
									head->cflags |= DF_VOL;
									getsym();
									goto lp;
								case kw__pascal:
									ispascal = TRUE;
									getsym();
									goto lp;
                case id:
												strcpy(declid,lastid);
                        getsym();
                        decl2();
                        break;
								case and:
#ifdef CPLUSPLUS
												if (prm_cplusplus) {
													getsym();
													if (prm_cplusplus) {
														decl1();
														if (head->type == bt_ref || head->type == bt_pointer)
															generror(ERR_CANTREF,0,0);
														else {
															temp1 = maketype(bt_ref,4);
															temp1->btp = head;
															head = temp1;
															if (tail == NULL)
																tail = head;
														}
													}
													else 
														generror(ERR_NOREF,0,0);
												}
												else
#endif
												{
													decl2();
													break;
												}
												break;
                case star:
												head->uflags &= ~UF_CANASSIGN;
                        temp1 = maketype(bt_pointer,4);
                        temp1->btp = head;
                        head = temp1;
												head->uflags |= UF_CANASSIGN;
                        if(tail == NULL)
                                tail = head;
                        getsym();
                        decl1();
                        break;
                case openpa:
                        getsym();
	                      temp1 = head;
  	                    temp2 = tail;
/* Check here for function pointer */
												if (lastst == star) {
													getsym();
													{
		  	                    head = tail = maketype(bt_ptrfunc,4);
    		  	                head->btp = temp1;
											
														decl1();
														if (needpunc(closepa,skm_declclosepa)) {
															char temp[40];
															/* in case we have a pointer type in
															 * parenthesis
															 * Isn't C grand? */
															if (lastst != openpa) {
																/* change the pointerfunc to a pointer */
																if (head->type == bt_ptrfunc) 
																	head->type = bt_pointer;
																else {
																	temp3 = head;
																	while (temp3->type != bt_ptrfunc)
																		temp3 = temp3->btp;
																	temp3->type = bt_pointer;
																}
																tail = temp2;
																decl2();
																break;
															}
															strcpy(temp,declid);
															if (head->type == bt_func) {
																declid[0] = 0;
																temp1 = head;
																head = head->btp;
															}
															else
																temp1 = 0;
															getsym();
															declfuncarg(intflag);
															if (temp1) {
																strcpy(declid,temp);
																temp1->btp = head;
																head = temp1;
															}
															if (lastst == begin) {
																temp1->type = bt_ifunc;
																break;
															}
														}
														else getsym();
													}
													
												}
												else {
/* (In case a declaration is in parenthesis */
      	                  decl1();
        	                needpunc(closepa,0);
													decl2();
												}
                        break;
                default:
                        decl2();
                        break;
                }
}

void decl2(void)
/*
 * type modifiers that come AFTER the name of the variable
 */
{       TYP     *temp1;
lp:
        switch (lastst) {
								case kw_const:
									head->cflags |= DF_CONST;
									getsym();
									goto lp;
								case kw_volatile:
									head->cflags |= DF_VOL;
									getsym();
									goto lp;
                case openbr:
												decl3();
												decl2();
                        break;
                case openpa:
                        getsym();
	                      temp1 = maketype(bt_func,0);
												head->uflags |= UF_DEFINED;
    	                  temp1->val_flag = 1;
      	                temp1->btp = head;
        	              head = temp1;
												head->uflags |= UF_DEFINED;
												declfuncarg(intflag);
												if (lastst == begin) {
													temp1->type = bt_ifunc;
													break;
												}
                        break;
								case colon:
										getsym();
										head->bits = intexpr(0);
										if (head->type != bt_long && head->type != bt_unsigned
												&& head->type != bt_short && head->type != bt_unsignedshort
												&& head->type != bt_char && head->type != bt_unsignedchar) {
											if (prm_ansi)
												generror(ERR_INTBITFIELDS,0,0);
											else
												generror(ERR_BFTYPE,0,0);
											head->bits = -1;
										}
										else
											if (prm_ansi && head->size != stdintsize) {
												generror(ERR_INTBITFIELDS,0,0);
												head->bits = -1;
											}
										break;
                }
}

void decl3(void)
/*
 * Array index handling
 */
{
  TYP *temp1, *list[40];
	int count = 0,i;
	int mustsize = 0;
	head->uflags |= UF_DEFINED;
	head->uflags &= ~UF_CANASSIGN;
	while(lastst == openbr) {
                        getsym();
                        temp1 = maketype(bt_pointer,0);  
                        temp1->val_flag = 1;
                        if(lastst == closebr) {
														   	if (mustsize)
																	generror(ERR_SIZE,0,0);
    	                           temp1->size = 0;
                                getsym();
                                }
                        else {
                                temp1->size = intexpr(0);
                                needpunc(closebr,skm_declclosebr);
                                }
												list[count++] = temp1;
												mustsize = 1;
	}
	if (head != NULL) {
		list[count-1]->size *= head->size;
		if (tail == NULL)
			tail = head;
	}
	for (i=count-1; i>0; i--) {
		list[i-1]->size *= list[i]->size;
		list[count-1]->uflags |= UF_DEFINED;
	}

	for (i=0; i < count-1; i++)
		list[i]->btp = list[i+1];
	list[count-1]->btp = head;

	head = list[0];
	if (tail == NULL)
		tail = list[count-1];
}
int bitsize(int type)
/*
 * Max bit field depends on the type
 */
{
	switch (type) {
		case bt_char:
		case bt_unsignedchar:
			return 8;
		case bt_short:
		case bt_unsignedshort:
			return 16;
		case bt_long:
		case bt_unsigned:
			return 32;
	}
	return 0;
}
int oksize(void)
/*
 * See if the size field is ok or if we should gen a message
 */
{
	TYP *q = head;
	while (q->type == bt_pointer) {
		if (q->val_flag)
			return 1;
		q = q->btp;
	}
	return head->size != 0;
}
int     basedeclare(TABLE *table,int al,long ilc,int ztype, int flags)
/*
 *  Once a type declarator is found we come here to get the remainder of the
 * declaration and allocate spae
 *
 */
{       SYM     *sp=0, *sp1;
        TYP     *dhead;
        int     nbytes,ufsave;
        nbytes = 0;
        dhead = head;
				ufsave = head->uflags;
        for(;;) {
								sp1 = 0;
                declid[0] = 0;
                decl1();

								/* stupid C standard uses a proper function name
								** as an alias for a pointer to a func when is a member...
								*/
								if (al == sc_member)
								  if (head->type == bt_func) {
										head->type = bt_ptrfunc;
										head->size = 4;
									}
									else if (head->type == bt_ifunc)
										generror(ERR_NOFUNCSTRUCT,0,0);
								
								/* In case they put the extern tag on an ifunc */
								if (head->type == bt_ifunc && al == sc_external)
									al = sc_global;
		
								if (declid[0] == 0) {
									if ((flags & DF_FUNCPARMS)) {
										sprintf(declid,"**ARG%d**",pcount++);
									}
								}
								head->cflags |= flags;
                if( declid[0] != 0) {      /* otherwise just struct tag... */
                    sp = xalloc(sizeof(SYM));
										if (head->type == bt_func || head->type == bt_ifunc)
#ifdef CPLUSPLUS
											if (prm_cplusplus)
												if (!strcmp(declid,"_main")) {
													SYM *sp1;
													head->sname = sp->name = litlate(declid);
													sp1 = search(sp->name,&gsyms);
													if (sp1 && sp1->tp->type == bt_ifunc)
														generror(ERR_NOOVERMAIN,0,0);
												}
												else if (mangleflag && !ispascal)
													head->sname = sp->name = cppmangle(declid,head);
												else
													head->sname = sp->name = litlate(declid);
											else 
#endif
												head->sname = sp->name = litlate(declid);
										else
											sp->name = litlate(declid);
										sp->defalt = 0;
                    sp->storage_class = al;
                    sp->tp = head;
										sp->extflag = FALSE;
										sp->absflag = (al == sc_abs);
										sp->intflag = flags &DF_INT;
										sp->pascaldefn = ispascal;
										sp->init = 0;
										sp->indecltable = 0;
										sp->funcparm = 0;
										sp->inreg = 0;
										sp->staticlabel = FALSE;
										if (al == sc_external && lastst == assign)
											sp->storage_class = sc_global;
										if (sp->intflag && ispascal)
											generror(ERR_PASCAL_NO_INT,0,0);
										if (al == sc_autoreg) {
											if (head->size > 4) {
												sp->storage_class = sc_auto;
												gensymerror(ERR_ILLREGISTER,sp->name);
											}
											head->sname = sp->name;
										}	
										if (al != sc_type && (oksize() || sp->tp->type == bt_func || sp->tp->type == bt_ifunc)) {
											if (al == sc_abs)
												sp->value.i = ilc;
											else
												if (head->type != bt_func && head->type != bt_ifunc)
													if (sp->storage_class == sc_static && table == &lsyms) {
														sp->value.i = nextlabel++;
														sp->staticlabel = TRUE;
													}
													else
														sp->value.i = block_nesting;
												else
												  sp->value.i = 0;
											{
												int align = getalign(al,head);
												int noadj = FALSE;

												if (al != sc_member || (head->bits != -1 && bittype != head->type && head->bits +curbit > bitsize(head->type))) {
													bittype = head->type;
													curbit = 0;
												}
												else
													noadj = head->bits != -1;

												if (al == sc_member) {
														int val;
														if (!noadj) {
															val = (ilc + nbytes) %align;
															if (val)  val = align - val;
															nbytes += val;
														}
														sp->value.i = nbytes + ilc;
														if (!noadj)
	                        		if(ztype == bt_union)
  	                            nbytes = imax(nbytes,sp->tp->size);
    	                    		else if(al != sc_external)
      	                        nbytes += sp->tp->size;
												}
											}
											if (head->bits != -1) {
												if (al != sc_member) {
													generror(ERR_BFILLEGAL,0,0);
													head->bits = -1;
												}
												else if (head->bits > bitsize(head->type)) {
													generror(ERR_BFTOOBIG,0,0);
													head->bits = -1;
												}
												else {
													if (prm_revbits)
														head->startbit = head->size*8-curbit-head->bits;
													else
														head->startbit = curbit;
													curbit+=head->bits;
												}
											}
											if (sp->absflag &&( sp->tp->type == bt_func || sp->tp->type == bt_ifunc))
												gensymerror(ERR_ILLCLASS,lastid);
											if (sp->intflag &&( sp->tp->type != bt_func && sp->tp->type != bt_ifunc))
												gensymerror(ERR_ILLCLASS,lastid);
                      if( sp->tp->type == bt_func  && al != sc_static)
                            sp->storage_class = sc_externalfunc;
											
  										sp1 = search(sp->name,table);
											if (sp->name[0] == '@' && !sp1)
												funcrefinsert(declid,sp->name,head,table);
                     	if (!sp1 || (sp1->tp->type != bt_func && sp1->tp->type != bt_ifunc)) {
												if (!sp1 || al == sc_auto || al == sc_autoreg)
                         	insert(sp,table);                                                                       
												else {
													if (!exactype(sp->tp,sp1->tp))
														gensymerror(ERR_DECLMISMATCH,sp->name);
													if (sp1->tp->size == 0)
														sp1->tp = sp->tp;
													if (sp1->storage_class == sc_external || sp1->storage_class == sc_externalfunc) {
														sp1->storage_class = sp->storage_class;
														sp1->tp = sp->tp;
														sp1->value.i = sp->value.i;
													}
												}
											}
											else 
join:
											if (sp1->tp->type == bt_func && sp->tp->type == bt_ifunc){
												if (sp1->storage_class == sc_external || sp1->storage_class == sc_externalfunc || sp1->storage_class == sc_static)
													if (al == sc_static || sp1->storage_class == sc_static)
														sp->storage_class = sp1->storage_class = sc_static;
													else
														sp->storage_class = sp1->storage_class = sc_global;
											}
                     	if( sp->tp->type == bt_ifunc) { /* function body follows */
														 if (sp1)
															 if (sp1->tp->type == bt_ifunc)
															   gensymerror(ERR_MULTIPLEINIT,sp->name);
															 else
														     sp1->tp->type = bt_ifunc;
                             funcbody(sp);
														 lastdecl = sp1;
                             return nbytes;
                             }
                     	if( (sp->storage_class == sc_global || sp->storage_class == sc_static) &&
                           	sp->tp->type != bt_func && sp->tp->type != bt_ifunc
														&& !(flags & DF_FUNCPARMS)) {
													if (sp->tp->type == bt_ref && lastst != assign)
														gensymerror(ERR_REFMUSTINIT,lastid);
													if (!sp1)
														sp1 = sp;
                          doinit(sp1);
											}
											else 
												if (al == sc_auto || al == sc_autoreg) {
#ifdef CPLUSPLUS
													if (prm_cplusplus &&(flags & DF_FUNCPARMS)) {
														dodefaultinit(sp);
													}
													else{
														if (sp->tp->type == bt_ref && lastst != assign)
															gensymerror(ERR_REFMUSTINIT,lastid);
													}
#endif
												 	doautoinit(sp);
												}
										}
										else if (al != sc_type) {
											gensymerror(ERR_SIZE,declid);
											expskim(skm_declclosepa);
										}
										else if (sp->tp->type == bt_func || sp->tp->type == bt_ifunc) {
														gensymerror(ERR_ILLTYPE,declid);
														expskim(skm_declclosepa);
													}
													else {
														if (sp->tp->size) {
															sp->tp = copytype(sp->tp,0);
															sp->tp->sname = sp->name;
														}
                            insert(sp,table);
													}
                }
								if (!(flags & DF_FUNCPARMS)) {
	                if(lastst == semicolon)
                        break;
									dhead = copytype(dhead,0);
  	              needpunc(comma,0);
								}
								else {
	                if(lastst == comma)
										break;
									if (sp1)
										lastdecl = sp1;
									else
										lastdecl = sp;
                  return(nbytes);
								}
                if(declbegin(lastst) == 0)
                        break;
                head = dhead;
								head->uflags = ufsave;
                }
        getsym();
				if (sp1)
					lastdecl = sp1;
				else
					lastdecl = sp;
        return nbytes;
}
int     declare(TABLE *table,int al,int ztype, int flags)
/*
 * In this wrapper we do an ENTIRE declaration
 */
{
				int old_gf = global_flag,rv;
				if (al != sc_member)
					active_table = table;
				if (al == sc_static)
					global_flag ++;
        decl(table);
				rv = basedeclare(table,al,0,ztype,flags);
				global_flag = old_gf;
				return rv;
}
int     declare2(TABLE *table,int al,int ztype, int flags, long val)
/*
 * In this wrapper we do an ENTIRE declaration
 */
{
				int old_gf = global_flag,rv;
				if (al != sc_member)
					active_table = table;
				if (al == sc_static)
					global_flag ++;
        decl(table);
				rv = basedeclare(table,al,val,ztype,flags);
				global_flag = old_gf;
				return rv;
}

int     declbegin(int st)
/*
 * This determines if another variable is being declared of the same type
 */
{       return st == star || st == id || st == openpa ||
                st == openbr;
}

void declenum(TABLE *table)
/*
 * declare enums
 */
{       SYM     *sp;
        TYP     *tp;
				char *nm;
        if( lastst == id) {
/* tagged */
                if((sp = search(nm = litlate(lastid),&tagtable)) == 0) {
                        sp = xalloc(sizeof(SYM));
                        sp->tp = xalloc(sizeof(TYP));
                        sp->tp->type = bt_enum;
                        sp->tp->size = 2;
                        sp->tp->lst.head = sp->tp->btp = 0;
                        sp->storage_class = sc_type;
                        sp->name = nm;
                        sp->tp->sname = sp->name;
												sp->tp->bits = sp->tp->startbit = -1;
#ifdef CPLUSPLUS
												/* tags are also typedefs in C++ */
												if (prm_cplusplus)
													insert(sp,active_table);
#endif
                        getsym();
                        if( lastst != begin)
                                generror(ERR_PUNCT,begin,skm_declend);
                        else    {
                                insert(sp,&tagtable);
                                getsym();
                                enumbody(table);
                                }
                        }
                else
                        getsym();
                head = sp->tp;
                }
        else    {
/* untagged */
                tp = xalloc(sizeof(TYP));
                tp->type = bt_enum;
                tp->lst.head = tp->btp = 0;
								tp->size = 2;
								tp->bits = tp->startbit = -1;
								tp->sname = 0;
                if( lastst != begin)
                        generror(ERR_PUNCT,begin,skm_declend);
                else    {
                        getsym();
                        enumbody(table);
                        }
                head = tp;
                }
}

void enumbody(TABLE *table)
/*
 * read the enumeration constants in
 */
{       long     evalue;
        SYM     *sp;
        evalue = 0;
        while(lastst == id) {
                sp = xalloc(sizeof(SYM));
                sp->value.i = evalue++;
                sp->name = litlate(lastid);
                sp->storage_class = sc_const;
                sp->tp = &stdconst;
                insert(sp,active_table);
                getsym();
								if (lastst == assign) {
										getsym();
										evalue = sp->value.i = intexpr(0);
										sp->value.i = evalue++;
										if (sp->value.i < SHRT_MIN || sp->value.i >SHRT_MAX)
											generror(ERR_CONSTTOOLARGE,0,0);
								}
                if( lastst == comma)
                        getsym();
                else if(lastst != end)
                        break;
                }
        needpunc(end,skm_declend);
}

void declstruct(TABLE *table, int ztype, int flags)
/*
 * declare a structure or union type
 */
{       SYM     *sp;
        TYP     *tp;
				char *nm = litlate(lastid);
        if(lastst == id) {
/* tagged */
                if((sp = search(nm,&tagtable)) == 0) {
/* if tag was never defined */
                        sp = xalloc(sizeof(SYM));
                        sp->name = nm;
                        sp->tp = xalloc(sizeof(TYP));
                        sp->tp->type = ztype;
                        sp->tp->lst.head = 0;
                        sp->storage_class = sc_type;
                        sp->tp->sname = sp->name;
												sp->tp->cflags = flags;
												sp->tp->uflags = UF_DEFINED;
												sp->tp->size = 0;
#ifdef CPLUSPLUS
												/* tags are also typedefs in C++ */
												if (prm_cplusplus)
													insert(sp,table);
#endif
                        getsym();
                        if(lastst != begin) {
                                insert(sp,&tagtable);
/*																if (lastst != semicolon)
																	generror(ERR_PUNCT,semicolon,skm_declend);
*/												}
                        else    {
                                insert(sp,&tagtable);
                                getsym();
                                structbody(sp->tp,sp->name,ztype);
                                }
                        }
                else {
/* Allow redefinition if it was forward declared */
                        getsym();
												if (lastst == begin) {
													getsym();
													if (sp->tp->size == 0) 
														structbody(sp->tp,sp->name,ztype);
													else {
														gensymerror(ERR_DUPSYM,sp->name);
														expskim(skm_declclosebr);
													}
												}
								}
								if (flags & (DF_CONST | DF_VOL))
									head = copytype(sp->tp,flags);
								else
                	head = sp->tp;
								head->bits = head->startbit = -1;
                }
        else    {
/* untagged */
                tp = xalloc(sizeof(TYP));
                tp->type = ztype;
								tp->cflags = flags;
								tp->uflags = UF_DEFINED;
                tp->sname = 0;
                tp->lst.head = 0;
								tp->bits = tp->startbit =-1;
                if( lastst != begin)
                       	generror(ERR_PUNCT,begin,skm_declend);
                else    {
                        getsym();
                        structbody(tp,tn_unnamed,ztype);
                        }
                head = tp;
                }
}

void structbody(TYP *tp,char  *name,int ztype)
/*
 * read in the structure/union elements and calculate the total size
 */
{       int     slc;
        slc = 0;
        tp->val_flag = 1;
				tp->uflags &= ~UF_CANASSIGN;
        while( lastst != end) {
								int flags=0;
                if(ztype == bt_struct)
                        slc += declare2(&tp->lst,sc_member,ztype,flags,slc);
                else
                        slc = imax(slc,declare2(&tp->lst,sc_member,ztype,flags,0));
								if (tp->lst.tail->tp->type == bt_ref)
									genclasserror(ERR_REFNOCONS,name,tp->lst.tail->name);
								tp->lst.tail->tp->uflags |= UF_DEFINED;
                }
				if (!prm_packing)
        	tp->size = (slc+strucadd)&strucmod;
				else
					tp->size = slc;
        getsym();
}
void check_used(void)
/*
 * At the end of compilition we check for some common cases where 
 * module-scoped variables are either missing or unused
 */
{
				int i;
				SYM *sp;
					for (i=0; i < HASHTABLESIZE; i++) {
						if ((sp=(SYM *) globalhash[i]) != 0) {
							while (sp) {
								if (sp->storage_class == sc_static)
									if (sp->tp->uflags & UF_USED) {
										if (sp->tp->type == bt_func)
											gensymerror(ERR_NOSTATICFUNC,sp->name);
									}
									else
										if (sp->tp->type == bt_ifunc || sp->tp->type == bt_func)
											gensymerror(ERR_FUNCUNUSED,sp->name);
										else
											gensymerror(ERR_STATICSYMUNUSED,sp->name);
								sp = sp->next;
							}
						}
					}
				sp = tagtable.head;
				while (sp) {
					if (sp->tp->size == 0) 
						gensymerror(ERR_NEVERSTRUCT,sp->name);
					sp = sp->next;
				}
}
void compile(void)
/*
 * Main compiler routine
 */
{       while(lastst != eof) {
                dodecl(sc_global);
								if (lastst != eof) {
									generror(ERR_DECLEXPECT,0,0);
									getsym();
								}
        }
        flush_peep();
				dumplits();
				initrundown();
				dumpstartups();
				check_used();
				putexterns();
}

void dodecl(int defclass)
/*
 * Declarations come here, ALWAYS
 */
{
				SYM *sp;
				int flags = 0;
				long val;
				char *nm;
				cbautoinithead = 0;
        for(;;) {
						ispascal = FALSE;
            switch(lastst) {
								case semicolon:
												getsym();
												break;
								case kw_typedef:
												getsym();
												if (defclass == sc_global)
													declare(&gsyms,sc_type,bt_struct, 0);
												else
													declare(&lsyms,sc_type,bt_struct, 0);
												break;
                case kw_register:
                        if( defclass != sc_auto || flags & DF_VOL) {
                          gensymerror(ERR_ILLCLASS,lastid);
                        	getsym();
												}
												else  {
                        	getsym();
          	                  declare(&lsyms,sc_autoreg,bt_struct,flags | DF_AUTOREG);
												};
										break;
                case id:
										if (defclass == sc_auto)
												if (!(((sp = search(nm = litlate(lastid),&gsyms)) != 0 && sp->storage_class == sc_type)
												     || ((sp = search(nm,&lsyms)) != 0 && sp->storage_class == sc_type)))
													return;
								case kw_volatile:
								case kw_const:
                case kw_char: case kw_int: case kw_short: case kw_unsigned:
                case kw_long: case kw_struct: case kw_union: case kw_signed:
                case kw_enum: case kw_void:
                case kw_float: case kw_double:
                    if( defclass == sc_global)
                            declare(&gsyms,sc_global,bt_struct,flags | DF_GLOBAL);
                    else if( defclass == sc_auto)
                            declare(&lsyms,sc_auto,bt_struct,flags);
                    else
                        declare(&lsyms,sc_auto,bt_struct,flags);
                    break;
                case kw_static:
                        if( defclass == sc_member) {
                           gensymerror(ERR_ILLCLASS,lastid);
														getsym();
														break;
												}
												getsym();
												if( defclass == sc_auto)
														declare(&lsyms,sc_static,bt_struct,flags | DF_GLOBAL);
												else
														declare(&gsyms,sc_static,bt_struct,flags | DF_GLOBAL);
                        break;
                case kw_extern: {
												int thismangle = FALSE;
                        getsym();
#ifdef CPLUSPLUS
												if (prm_cplusplus && lastst == sconst) {
													if (!strcmp(laststr,Cstr)) {
														mangleflag = FALSE;
														manglelevel++;
														getsym();
														if (lastst == begin) {
															getsym();
														  thismangle = FALSE;
														}
														else thismangle = TRUE;
													}
												}
#endif
                        if( defclass == sc_member) {
                            gensymerror(ERR_ILLCLASS,lastid);
														break;
												}
                        ++global_flag;
                        declare(&gsyms,sc_external,bt_struct,flags);
                        --global_flag;
												if (thismangle && !--manglelevel)
													mangleflag = TRUE;
												}
                        break;
								case end:
#ifdef CPLUSPLUS
												if (prm_cplusplus && manglelevel) {
													mangleflag = (!--manglelevel);
													getsym();
													continue;
												}
#endif
												return;
								case kw__interrupt:
												intflag = 1;
												flags |= DF_INT;
												getsym();
												continue;
								case kw__abs:
												++global_flag;
												getsym();
												if (lastst != openpa) {
													generror(ERR_PUNCT,openpa,0);
												}
												else {
													getsym();
													val = intexpr(0);
													if (lastst != closepa)
														generror(ERR_PUNCT,closepa,skm_declclosepa);
												}
												getsym();
												declare2(&gsyms,sc_abs,bt_struct,flags,val);
												--global_flag;
												break;
                default:
                        return;
                }
						flags = 0;
						intflag = 0;
            }
}
void doargdecl(int defclass, char *names[], int *nparms, TABLE *table, int isinline)
/*
 * Function arguments are declared here
 */
{
				SYM *sp;
				int flags = isinline ? DF_FUNCPARMS : 0;
        for(;;) {
            switch(lastst) {
								case ellipse: {
									sprintf(declid,"**ELLIPSE%d**",pcount++);
									sp = xalloc(sizeof(SYM));
									sp->name = litlate(declid);
                  sp->storage_class = sc_auto;
									sp->tp = maketype(bt_ellipse,0);
									sp->tp->uflags |= UF_DEFINED | UF_USED;
									insert(sp,table);
									if (ispascal)
										generror(ERR_PASCAL_NO_ELLIPSE,0,0);
									getsym();
									goto exit;
								}
								case kw_const:
								case id:
                case kw_char: case kw_int: case kw_short: case kw_unsigned:
                case kw_long: case kw_struct: case kw_union: case kw_signed:
                case kw_enum: case kw_void:
                case kw_float: case kw_double:
                    declare(table,sc_auto,bt_struct,flags);
                    break;
                case kw_static:
								case kw_auto:
								case kw_register:
                        gensymerror(ERR_ILLCLASS,lastid);
                        getsym();
												continue;
                default:
												goto exit;
                }
							if (isinline) {
								names[(*nparms)++] = litlate(declid);
							}
						
				flags &= ~DF_CONST;
				}
exit:
#ifdef CPLUSPLUS
		if (prm_cplusplus) {
			SYM *sp = table->head;
			int found = FALSE;
			while (sp) {
				if (sp->defalt)
					found = TRUE;
				else 
					if (found)
						gensymerror(ERR_MISSINGDEFAULT,sp->name);
				sp = sp->next;
			}
		}
#endif
}