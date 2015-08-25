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
 * handles initialization of module-scoped declarations
 */
#include        <stdio.h>
#include				<limits.h>
#include        "expr.h"
#include        "c.h"
#include        "errors.h"

extern int stdldoublesize;
extern enum e_sym lastst;
extern char lastid[], laststr[];
extern TABLE gsyms;
extern int skm_declend[];
extern long bittab[];
extern int nextlabel;
extern TYP stdmatch;
extern int skm_closebr[];
extern int prm_cplusplus;
extern SYM *currentfunc;
extern int prm_bss;
static SYM *locsp;
static short cursize;
static short startbit,bits;
static long totbits;
static int allocated;
static ENODE *cpprefhead;
static ENODE **cppreftail;
static int baseoffs = 0;

static struct decldata **sptr;

static struct declares {
		struct declares *link;
		SYM *sp;
} *declarations, *decltail;
	
void initini(void)
{
	cpprefhead = 0;
	declarations = decltail = 0;
}
void initrundown(void)
/*
 * Dump out C++ pointer and reference initializations functions 
 */
{
	struct declares *q;
	long nbytes,obytes,val,onbytes;

#ifdef CPLUSPLUS
	if (cpprefhead) {
		int lbl = nextlabel++;
		SNODE stmt;
		currentfunc = xalloc(sizeof(SYM));
		currentfunc->tp = maketype(bt_func,0);
		currentfunc->tp->btp = maketype(bt_void,0);
		currentfunc->tp->lst.head = 0;
		currentfunc->intflag = 0;
		stmt.stype = st_expr;
		stmt.next = 0;
		stmt.exp = cpprefhead;
		cseg();
	
		gen_label(lbl);
		genfunc(&stmt);
		flush_peep();
		cppseg();
		gen_labref(lbl);
		currentfunc = 0;
		cpprefhead = 0;
	}
#endif
	nl();
	dseg();
	q = declarations;
	nbytes = 0;
	while (q) {
		SYM *sp = q->sp;
		if ((sp->init || !prm_bss) && !sp->absflag) {
			int align = getalign(sc_global,sp->tp);
			val = align - (nbytes % align);
			if (val != align) {
				genstorage(val);
				nbytes += val;
				nl();
			}
			onbytes = nbytes;
			nbytes += gen(sp);
			sp->value.i = onbytes;
		}
		q = q->link;
	}
	if (prm_bss) {
		obytes  = nbytes;
		nbytes = 0;
		bssseg();
		q = declarations;
		while (q) {
			SYM *sp = q->sp;
			if (!sp->init && !sp->absflag) {
				int align = getalign(sc_global,sp->tp);
				val = align - (nbytes % align);
				if (val != align) {
					genstorage(val);
					nbytes += val;
					nl();
				}
			onbytes = nbytes;
			nbytes += gen(sp);
			sp->value.i = onbytes + obytes;
			}
			q = q->link;
		}
	}
}
static long gen(SYM *sp)
{
		struct decldata *decldata = sp->init;
		long nbytes=0;
		if (sp->storage_class == sc_global) {
			globaldef(sp);
		}
		if (sp->staticlabel)
			put_staticlabel(sp->value.i);
		else
			gen_strlab(sp);
		if (decldata) {
			while (decldata) {
				switch (decldata->mode) {
					case dd_byte: 
						genbyte(decldata->val.i);
						nbytes++;;
						break;
					case dd_word:
						genword(decldata->val.i);
						nbytes += 2;
						break;
					case dd_long:
						genlong(decldata->val.i);
						nbytes += 4;
						break;
					case dd_float:
						genfloat(decldata->val.f);
						nbytes += 4;
						break;
					case dd_double:
						gendouble(decldata->val.f);
						nbytes += 8;
						break;
					case dd_ldouble:
						genlongdouble(decldata->val.f);
						nbytes += stdldoublesize;
						break;
					case dd_pcname:
						genpcref(decldata->val.sp,decldata->offset);
						nbytes += 4;
						break;
					case dd_dataname:
						genref(decldata->val.sp,decldata->offset);
						nbytes += 4;
						break;
					case dd_storage:
						genstorage(decldata->val.i);
						nbytes += decldata->val.i;
						break;
					case dd_label:
						gen_labref(decldata->val.i);
						nbytes += 4;
						break;
				}
				decldata = decldata->link;
			}
		}
		else {
			if (sp->tp->size == 0)
				gensymerror(ERR_ZEROSTORAGE,sp->name);
			else
				genstorage(nbytes += sp->tp->size);
		}
	nl();
	return nbytes;
}
#ifdef CPLUSPLUS
void cppinitinsert(ENODE *node)
/*
 * insert an initialization function on the list
 */
{
	if (!cpprefhead) {
		cpprefhead = node;
		cppreftail = &cpprefhead;
	}
	else {
		*cppreftail = makenode(en_void,*cppreftail,node);
		cppreftail = & (*cppreftail)->v.p[1];
	}
}
#endif
void doinit(SYM *sp)
/*
 * Handle static variable initialize
 */
{
				sp->tp->uflags |= UF_DEFINED;
				allocated = FALSE;
				baseoffs = 0;
				totbits = 0;
				cursize = -1;
				bits = -1;
				locsp = sp;
				sptr = &sp->init;
				if (!sp->indecltable) {
				 	struct declares * p = xalloc(sizeof(struct declares));
					p -> sp = sp;
					p->link = 0;
					if (declarations)
						decltail = decltail->link = p;
					else
						decltail = declarations = p;
					sp->indecltable = TRUE;
				}
        if( lastst == assign) {
								if (sp->init)
									gensymerror(ERR_MULTIPLEINIT,sp->name);
								if (sp->absflag) {
									generror(ERR_NOINIT,0,skm_declend);
				          return;
								}
                getsym();
                inittype(sp->tp);
								*sptr = 0;
        }
        endinit();
}

int     inittype(TYP *tp)
/*
 * Init for basic types
 */
{       int     nbytes;
        switch(tp->type) {

								case bt_float:
												nbytes = initfloat();
												break;
								case bt_longdouble:
												nbytes = initlongdouble();
												break;
								case bt_double:
												nbytes = initdouble();
												break;
                case bt_char:
                        nbytes = initchar();
                        break;
                case bt_unsignedchar:
                        nbytes = inituchar();
                        break;
                case bt_unsignedshort:
                        nbytes = initushort();
                        break;
                case bt_short:
                case bt_enum:
                        nbytes = initshort();
                        break;
                case bt_ptrfunc:
                        nbytes = initpointerfunc();
                        break;
                case bt_pointer:
                        if( tp->val_flag)
                                nbytes = initarray(tp);
                        else
                                nbytes = initpointer();
                        break;
#ifdef CPLUSPLUS
								case bt_ref:
												nbytes = initref(tp->btp);
												break;
#endif
                case bt_unsigned:
                        nbytes = initulong();
                        break;
                case bt_long:
                case bt_matchall:
                        nbytes = initlong();
                        break;
                case bt_struct:
                        nbytes = initstruct(tp);
                        break;
                default:
  											gensymerror(ERR_NOINIT,locsp->name);
                        nbytes = 0;
                }
				baseoffs+=nbytes;
        return nbytes;
}

int initarray(TYP *tp)
/*
 * Init for arrays
 */
{       int     nbytes;
				int canpad = FALSE;
        char    *p;
				int needend = FALSE;
        nbytes = 0;
        if( lastst == begin) {
                getsym();               /* skip past the brace */
								needend = TRUE;
								if ((tp->btp->type == bt_char  || tp->btp->type ==bt_unsignedchar)&& lastst == sconst)
									goto grabchar;
								if ((tp->btp->type == bt_short  || tp->btp->type ==bt_unsignedshort)&& lastst == lsconst)
									goto grabchar;
                while(lastst != end) {
                        nbytes += inittype(tp->btp);
                        if( lastst == comma)
                                getsym();
                        else if( lastst != end) {
                                expecttoken(end,0);
																break;
												}
                        }
                getsym();               /* skip closing brace */
                }
        else { 
grabchar:		
						allocated = TRUE;
						nbytes = 0;
						if ((tp->btp->type == bt_char  || tp->btp->type ==bt_unsignedchar)&& lastst == sconst) {
							canpad = TRUE;
							while (lastst == sconst) {
                p = laststr;
								while (*p) {
									nbytes++;
									agflush(1,*p++);
								}
                getsym();
							}
							if (needend)
								needpunc(end,skm_declend);
							agflush(1,0);
							nbytes++;
					  } else if ((tp->btp->type == bt_short  || tp->btp->type ==bt_unsignedshort)&& lastst == lsconst) {
							while (lastst == lsconst) {
                p = laststr;
								while (*p) {
									nbytes+=2;
									agflush(2,*p++);
								}
                getsym();
							}
							if (needend)
								needpunc(end,skm_declend);
							agflush(2,0);
							nbytes +=2;
            }
            else generror(ERR_PUNCT,semicolon,0);
				}
        if( nbytes < tp->size) {
								nl();
                makestorage( tp->size - nbytes);
                nbytes = tp->size;
                }
        else if (tp->size == 0)
					tp->size = nbytes;
				else if( nbytes > tp->size && (!canpad || nbytes > tp->size+1))
                generror(ERR_INITSIZE,0,0);    /* too many initializers */
        return nbytes;
}

int initstruct(TYP *tp)
/*
 * Init for structures
 */
{       SYM     *sp;
        int     nbytes;
        needpunc(begin,0);
        nbytes = 0;
        sp = tp->lst.head;      /* start at top of symbol table */
        while(sp != 0 && lastst != end) {
								startbit = sp->tp->startbit;
								bits = sp->tp->bits;
								if (nbytes < sp->value.i)
									makestorage(sp->value.i - nbytes);
								nbytes = sp->value.i;
                nbytes += inittype(sp->tp);
                if( lastst == comma)
                        getsym();
                else if(lastst == end)
                        break;
                else
                        expecttoken(end,0);
                sp = sp->next;
                }
				nbytes += agbits(-1,0);
        if( nbytes < tp->size) {
					makestorage(tp->size - nbytes);
				}
        needpunc(end,skm_declend);
        return tp->size;
}

int makelabel(void)
{
	struct decldata *q = *sptr = xalloc(sizeof(struct decldata));
	q->val.i = tostring();
	sptr = &(*sptr)->link;
	q->mode = dd_label;
}
int makeref(int ispc, SYM *sp, long val)
{
	struct decldata *q = *sptr = xalloc(sizeof(struct decldata));
	sp->extflag = TRUE;
	q->val.sp = sp;
	q->offset = val;
	sptr = &(*sptr)->link;
	q->mode = ispc ? dd_pcname : dd_dataname;
}
int makestorage(long val)
{
	struct decldata *q = *sptr = xalloc(sizeof(struct decldata));
	q->val.i = val;
	sptr = &(*sptr)->link;
	q->mode = dd_storage;
}
int agflush(int size, long val)
/*
 * flush a bit field when it is full
 */
{
	struct decldata *q = *sptr = xalloc(sizeof(struct decldata));
	q->val.i = val;
	sptr = &(*sptr)->link;
	switch (size) {
		case 1:
			q->mode = dd_byte;
			return 1;
		case 2:
			q->mode = dd_word;
			return 2;
		case 4:
			q->mode = dd_long;
			return 4;
		default:
			q->mode = dd_byte;
			return 0;
	}
}
/* Aggregate bits */
int agbits(int size, long value)
/*
 * ombine bit declarations into a bit field
 */
{
	long rv = 0;
	if (cursize != -1 && (size != cursize || bits == -1 || (bits != -1 &&startbit==0))) {
		rv = agflush(cursize,totbits);
		cursize = -1;
		totbits = 0;
	}
	if (bits != -1) {
		totbits |= (value & bittab[bits-1]) << startbit;
		cursize = size;
	}
	else {
		if (size != -1)
			rv+= agflush(size,value);
		cursize = -1;
		totbits = 0;
	}
	startbit = -1;
	return(rv);
}
/* Basic type subroutines */
int initfloat(void)
{
	struct decldata *q = *sptr = xalloc(sizeof(struct decldata));
	q->val.f = floatexpr();
	sptr = &(*sptr)->link;
	q->mode = dd_float;
			allocated = TRUE;
			return(4);
}
int initlongdouble(void)
{
	struct decldata *q = *sptr = xalloc(sizeof(struct decldata));
	q->val.f = floatexpr();
	sptr = &(*sptr)->link;
	q->mode = dd_ldouble;
			allocated = TRUE;
			return(stdldoublesize);
}
int initdouble(void)
{
	struct decldata *q = *sptr = xalloc(sizeof(struct decldata));
			allocated = TRUE;
	q->val.f = floatexpr();
	sptr = &(*sptr)->link;
	q->mode = dd_double;
			return(8);
}
int initchar(void)
{       
			long t;
			int rv;
			allocated = TRUE;
			rv = agbits(1,t = intexpr(0));
			if (t < SCHAR_MIN || t > SCHAR_MAX)
				generror(ERR_CONSTTOOLARGE,0,0);
			return rv;
}
int initshort(void)
{   
			long t;
			int rv;
			allocated = TRUE;
			rv = agbits(2,t = intexpr(0));
			if (t < SHRT_MIN || t > SHRT_MAX)
				generror(ERR_CONSTTOOLARGE,0,0);
			return rv;
}
int inituchar(void)
{       
			long t;
			int rv;
			allocated = TRUE;
			rv = agbits(1,t = intexpr(0));
			if (t < 0 || t > UCHAR_MAX)
				generror(ERR_CONSTTOOLARGE,0,0);
			return rv;
}
int initushort(void)
{   
			long t;
			int rv;
			allocated = TRUE;
			rv = agbits(2,t = intexpr(0));
			if (t < 0 || t > USHRT_MAX)
				generror(ERR_CONSTTOOLARGE,0,0);
			return rv;
}
int initlong(void)
{       
			long t;
			int rv;
			allocated = TRUE;
			rv = agbits(4,t = intexpr(0));
			if (t < LONG_MIN || t > LONG_MAX)
				generror(ERR_CONSTTOOLARGE,0,0);
			return rv;
}
int initulong(void)
{   
			long t;
			int rv;
			allocated = TRUE;
			rv = agbits(4,t = intexpr(0));
			if (t < 0 || t > ULONG_MAX)
				generror(ERR_CONSTTOOLARGE,0,0);
			return rv;
}
void getreflvalue(ENODE **node, TYP **tp,int pointer)
/*
 * create a node tree for pointer and references that are going in as data
 */
{
	SYM *sp;
	ENODE *pnode=0;
	sp = 0;
	while (lastst == star) {
		getsym();
		getreflvalue(&pnode,tp,pointer);
		if ((*tp)->type == bt_pointer) {
			*tp = (*tp)->btp;
			pnode = makenode(en_l_ref,pnode,0);
		}
		else {
			generror(ERR_DEREF,0,0);
		}
		*node = pnode;
		return;
	}
	if (lastst == and && pointer) {
		getsym();
		getreflvalue(&pnode,tp,pointer);
		if (!((*tp)->type == bt_ifunc) && !((*tp)->type == bt_func) && !(*tp)->val_flag) {
			TYP *tp1 = maketype(bt_pointer,4);
			tp1->btp = *tp;
			*tp = tp1;
			if (pnode)
				pnode = pnode->v.p[0];
		}
		*node = pnode;
		return;
	}
	if (lastst != id) {
		long temp;
		getsym();
		temp = intexpr(tp);
		(*node) = makenode(en_icon,(char *)temp,0);
		return;
	}
	if ((sp = search(lastid,&gsyms)) == 0) {
		gensymerror(ERR_UNDEFINED,lastid);
		return;
	}
	pnode = makenode(en_nacon,(char *)sp->name,0);
	if (pointer && !sp->tp->val_flag)
		pnode = makenode(en_l_ref,pnode,0);
	*tp = sp->tp;
	getsym();
	while (TRUE) {
		long temp;
		switch(lastst) {
	    case openbr:    /* build a subscript reference */
				getsym();
				temp = intexpr(0);
      	if( (*tp)->type != bt_pointer )
        	generrorexp(ERR_NOPOINTER,0,skm_closebr);
	      else {
  	      (*tp) = (*tp)->btp;
					pnode = makenode(en_add,pnode,makenode(en_icon,(char *)(temp*(*tp)->size),0));
				}
      	needpuncexp(closebr,skm_closebr);
      	break;
      case pointsto:
        if( (*tp)->type != bt_pointer ) {
        	generror(ERR_NOPOINTER,0,0);
					while (lastst == pointsto || lastst == dot) {
						getsym();
						getsym();
					}
					break;
				}
        else {
  	      (*tp) = (*tp)->btp;
					pnode = makenode(en_l_ref,pnode,0);
				}
/*
 *      fall through to dot operation
 */
      case dot:
        getsym();       /* past -> or . */
	      if( lastst != id )
  	      generror(ERR_IDEXPECT,0,0);
    	  else    {
      	  sp = search(lastid,&(*tp)->lst);
        	if( sp == 0 ) {
						(*tp) = &stdmatch;
	          gensymerror(ERR_UNDEFINED,lastid);
						getsym();
						while (lastst == pointsto || lastst == dot) {
							getsym();
							getsym();
						}
					}
  	      else    {
						pnode = makenode(en_add,pnode,makenode(en_icon,(char *)sp->value.i,0));
    	      (*tp) = sp->tp;
      	    getsym();       /* past id */
        	}
        	break;

				}
			case plus:
				getsym();
				pnode = makenode(en_add,pnode,makenode(en_icon,(char *)intexpr(0),0));
				break;
			case minus:
				getsym();
				pnode = makenode(en_add,pnode,makenode(en_icon,(char *)(-intexpr(0)),0));
				break;
			default:
				*node =pnode;
				return;
		}
	}
}
int checkrefeval(ENODE *node)
/*
 * see if a reference node can be evaluated at compile time
 */
{
	switch(node->nodetype) {
		case en_add:
			return(checkrefeval(node->v.p[0]) || checkrefeval(node->v.p[1]));
		case en_l_ref:
			return 1;
		case en_nacon:
		case en_icon:
		case en_lcon:
		case en_lucon:
		case en_iucon:
			return 0;
		default:
			return 1;
	}
}
int refeval(ENODE *one, ENODE **two)
/*
 * compile time evaluation of references and pointers */
{
	while (1) {
		switch (one->nodetype) {
			case en_add:
				return(refeval(one->v.p[0],two) + refeval(one->v.p[1],two));
			case en_icon:
			case en_lcon:
			case en_lucon:
			case en_iucon:
				return(one->v.i);
			case en_nacon:
				*two = one;
				return 0;
			default:
				return 0;
		}
	}
}
#ifdef CPLUSPLUS
int initref(TYP *tp)
/*
 * init for reference variables
 */
{
	SYM *sp;
	TYP *tp1;
	ENODE *node=0,*node1;
	int ofs;
	allocated = TRUE;
	getreflvalue(&node,&tp1,0);
	if (!node || !checktype(tp,tp1)) {
		genstorage(4);
		generror(ERR_REFLVALUE,0,0);
	}
	else {
		if (!checkrefeval(node)) {
			ofs = refeval(node,&node1);
			sp = search(node1->v.sp,&gsyms);
			makeref(FALSE,sp,ofs);
			sp->tp->uflags |= UF_USED;
		}
		else  {
			node1 = makenode(en_l_ref,makenode(en_nacon,locsp->name,0),0);
			node1 = makenode(en_assign,node1,node);
			cppinitinsert(node1);
			genstorage(4);
		}
	}
	endinit();
	return 4;
}
#endif
int initpointer(void)
{       SYM     *sp;
				ENODE *node=0,*node1;
				TYP *tp1;
				allocated = TRUE;
       	if(lastst == sconst) {
                makelabel();
                }
       	else if(lastst == lsconst) {
                makelabel();
                }
				else if (lastst == iconst || lastst == lconst || lastst == iuconst || lastst == luconst) {
								long temp;
								TYP *tp;
                agflush(4,temp = intexpr(&tp));
								if (temp && (tp->type != bt_pointer && tp->type != bt_ptrfunc))
									generror(ERR_NONPORT,0,0);
				}
				else {
					getreflvalue(&node,&tp1,1);
					if (!node || (tp1->type != bt_pointer && tp1->type != bt_func && tp1->type != bt_ifunc && !tp1->val_flag))
						gensymerror(ERR_NOINIT,locsp->name);
					else {
						if (!checkrefeval(node)) {
							int ofs = refeval(node,&node1);
							sp = search(node1->v.sp,&gsyms);
							if (sp->tp->type == bt_func || sp->tp->type == bt_ifunc) {
  	            makeref(TRUE,sp,ofs);
							}
							else
      	        makeref(FALSE,sp,ofs);
							sp->tp->uflags |= UF_USED;
						}
						else  {
							if (!prm_cplusplus) 
								gensymerror(ERR_NOINIT,locsp->name);
#ifdef CPLUSPLUS
							else {
								node1 = makenode(en_l_ref,makenode(en_nacon,locsp->name,0),0);
								node1 = makenode(en_assign,node1,node);
								cppinitinsert(node1);
							}
#endif
							makestorage(4);
						}
					 }
				}
        endinit();
        return 4;       /* pointers are 4 bytes long */
}
int initpointerfunc(void)
{       SYM     *sp;
				char *nm;
				allocated = TRUE;
        if(lastst == and)     /* address of a variable */
                getsym();
        if(lastst != id) {
								long temp;
								TYP *tp;
                agflush(4,temp = intexpr(&tp));
								if (temp && (tp->type != bt_pointer && tp->type != bt_ptrfunc))
									generror(ERR_NONPORT,0,0);
							endinit();
							return(4);
					 }
        else  {
#ifdef CPLUSPLUS
					if (prm_cplusplus)
						nm = cppmangle(lastid,locsp->tp);
					else
#endif
						nm = lastid;
					if( (sp = gsearch(nm)) == 0)
				 		gensymerror(ERR_UNDEFINED,nm);
        	else
          	getsym();
				}
        makeref(TRUE,sp,0);
				if (sp)
					sp->tp->uflags |= UF_USED;
        endinit();
        return 4;       /* pointers are 4 bytes long */
}
/* Finish init */
void endinit(void)
{       if( lastst != comma && lastst != semicolon && lastst != end) {
                expecttoken(end,0);
                while( lastst != comma && lastst != semicolon && lastst != end && lastst != eof)
                        getsym();
                }
}