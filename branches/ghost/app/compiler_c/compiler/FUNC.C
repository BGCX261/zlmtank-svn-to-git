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
#include				"errors.h"

extern int ispascal;
extern FILE *listFile;
extern enum e_sym lastst;
extern long firstlabel,nextlabel;
extern char lastid[];
extern TABLE gsyms,lsyms;
extern int global_flag;
extern int stdaddrsize, stdretblocksize;
extern int stackadd,stackmod;
extern TYP *head, *tail;
extern char declid[100];
extern int prm_listfile;
extern int prm_cplusplus;
extern int stdintsize;
extern int goodcode;
extern int prm_linkreg;
extern TABLE lsyms;

int block_nesting;

TABLE oldlsym;
LIST *varlisthead,*varlisttail;

static int skm_func[] = { closepa,begin,semicolon,0 };

SYM *currentfunc=0;

/*      function compilation routines           */
void funcini(void)
{
	currentfunc = 0;
}
void declfuncarg(int isint)
{       char    *names[50];             /* 50 parameters maximum */
        int     nparms, poffset, i, isinline;
        SYM     *sp1, *sp2, *spsave[50], *oldargs=0;
				TYP *temp, *temp1;
				char oldeclare[100];
				char *nm;
				if (prm_linkreg && !isint)
        	poffset = stdretblocksize;            /* size of return block */
				else
        	poffset = 0;            /* size of return block */
        nparms = 0;
				temp = head;
				temp1 = tail;
				if (!prm_cplusplus) {
					sp2 = gsearch(declid);
					if (sp2) {
						oldargs = sp2->tp->lst.head;
					}
				}
				temp->lst.head = temp->lst.tail = 0;
				if (lastst == kw_void) {
					getsym();
					if (lastst != closepa) {
						backup(lastst);
						lastst = kw_void;
					}
					else {
						temp->lst.head = (SYM *)-1;
						getsym();
					  goto verify;
					}
				}
				else
					if (lastst == closepa) {
						getsym();
						goto verify;
					}
				strcpy(oldeclare,declid);
        if(!prm_cplusplus && lastst == id && 
							((sp1 = search(nm=litlate(lastid),&gsyms)) ==0 || sp1->storage_class != sc_type) 
							&& ((sp1 = search(nm,&lsyms)) ==0 || sp1->storage_class != sc_type)) {
              /* declare parameters */
					global_flag = 0;
                while(lastst == id) {
                        names[nparms++] = nm;
                        getsym();
                        if( lastst == comma)
                                getsym();
                        else
                                break;
                        }
					global_flag = 1;
                needpunc(closepa,skm_func);
                doargdecl(sc_member,0,0,&temp->lst,isinline = FALSE);      /* declare parameters */
				}
				else {
					doargdecl(sc_member,names,&nparms,&temp->lst,isinline = TRUE);
					needpunc(closepa,skm_func);
				}
				strcpy(declid,oldeclare);
				if (!ispascal && 
							(temp->btp->type == bt_struct || temp->btp->type == bt_union))
					poffset += stdaddrsize;

        for(i = 0;i < nparms;++i) {
            if ((sp1 = search(names[i],&temp->lst)) == 0 && !isinline)
              sp1 = makeint(litlate(names[i]),&temp->lst);
						if (sp1->tp)
							sp1->tp->uflags |= UF_DEFINED;
						sp1->funcparm = TRUE;
						spsave[i] = sp1;
            sp1->storage_class = sc_auto;
				}
				/*
				 * parameter allocation.  Have to do things backwards if this
				 * function has the _pascal declarator
				 */
				if (!ispascal)
        	for(i = 0;i < nparms;++i) {
						sp1 = spsave[i];
						if( sp1->tp->type != bt_pointer && sp1->tp->size < stdintsize)
						{
							sp1->value.i = poffset + funcvaluesize(sp1->tp->size);
							poffset += stdintsize;
						}
						else
						{
							sp1->value.i = poffset;
							if (sp1->tp->type == bt_pointer)
								poffset += stdaddrsize;
							else
								poffset += sp1->tp->size;
						}
					}
				else
        	for(i = nparms-1;i >=0;--i) {
						sp1 = spsave[i];
						if( sp1->tp->type != bt_pointer && sp1->tp->size < stdintsize)
						{
							sp1->value.i = poffset + funcvaluesize(sp1->tp->size);
							poffset += stdintsize;
						}
						else
						{
							sp1->value.i = poffset;
							if (sp1->tp->type == bt_pointer)
								poffset += stdintsize;
							else
								poffset += sp1->tp->size;
						}
					}
				if (!isinline) {
					temp->lst.head = temp->lst.tail = 0;
					for (i=0; i < nparms; i++)
						insert(spsave[i],&temp->lst);
				}
#ifdef CPLUSPLUS
				if (prm_cplusplus && temp->lst.head == 0)
					temp->lst.head = (SYM *) -1; /* () is equiv to (void) in cpp */
#endif
verify:
				head = temp;
				tail = temp1;
#ifdef CPLUSPLUS
				if (prm_cplusplus)
					return;
#endif
				
				if (oldargs && head->sname) {
					SYM *newargs = head->lst.head;
					while (oldargs && newargs) {
						if (!checktype(oldargs->tp,newargs->tp))
							gensymerror(ERR_ARGMISMATCH,newargs->name);
						if (oldargs == (SYM *)-1 || newargs == (SYM *)-1)
							break;
						oldargs = oldargs->next;
						newargs = newargs->next;
					}
					if (oldargs && oldargs != (SYM *)-1)
						gensymerror(ERR_ARGLENSHORT,head->sname);
					if (newargs && newargs != (SYM *)-1)
						gensymerror(ERR_ARGLENLONG,head->sname);
				}
				if (sp2 && !checktype(sp2->tp->btp,head->btp))
					gensymerror(ERR_DECLMISMATCH,sp2->name);
}
void check_funcused(TABLE *oldlsym, TABLE *lsyms)
{
	/* oldlsym Must BE 0 at the end of a function */
	SYM *sym1;
	if (oldlsym && oldlsym->head == lsyms->head) {
		return;
	}
	sym1 = lsyms->head;
	while (sym1 && (!oldlsym || sym1 != oldlsym->head)) {
		if (sym1->tp->type != bt_func && sym1->tp->type != bt_ifunc
				&& sym1->storage_class != sc_type) {
			if (!(sym1->tp->uflags & UF_USED)) {
				if (sym1->storage_class == sc_label) {
					if (!oldlsym)
						gensymerror(ERR_UNUSEDLABEL,sym1->name);
				}
				else gensymerror(ERR_SYMUNUSED,sym1->name);
			}
			if (sym1->tp->uflags & UF_ASSIGNED)
				gensymerror(ERR_SYMASSIGNED,sym1->name);
			if (!oldlsym && sym1->storage_class == sc_ulabel)
				gensymerror(ERR_UNDEFLABEL,sym1->name);
		}
		sym1 = sym1->next;
	}
}
void funcbody(SYM *sp)
/*
 */
{
				SYM *sp1 = sp->tp->lst.head, *sp2;
				varlisthead = 0;
        global_flag = 0;
				if (sp1 != (SYM *)-1) {
					while (sp1) {
						if (sp1->name[0] != '*') {
							sp2 = xalloc(sizeof(SYM));
							memcpy(sp2,sp1,sizeof(SYM));
							insert(sp2,&lsyms);
						}
						sp1 = sp1->next;
					}
				}
				currentfunc = sp;
        cseg();
				firstlabel = nextlabel;
				if (sp->storage_class == sc_global)
					globaldef(sp);
				goodcode &= ~GF_RETURN;
        block();
				check_funcused(0,&lsyms);
				if (prm_listfile && varlisthead) {
					LIST *q = varlisthead;
      		fprintf(listFile,"\n\n*** local symbol table ***\n\n");
					while (q) {
						SYM *sp = q->data;
						while (sp) {
							list_var(sp,0);
							sp = sp->next;
						}
						q = q->link;
					}
				}
				if (sp->tp->btp->type != bt_void && !(goodcode &GF_RETURN)) 
					generror(ERR_FUNCRETVAL,0,0);
        nl();
				currentfunc = 0;

        release_local();        /* release local symbols */
  			lsyms.head = 0;
				oldlsym.head = oldlsym.tail = 0;
        global_flag = 1;
}

SYM     *makeint(char *name, TABLE *table)
{       SYM     *sp;
        TYP     *tp;
				global_flag++;
        sp = xalloc(sizeof(SYM));
        tp = xalloc(sizeof(TYP));
				global_flag--;
        tp->type = bt_long;
        tp->size = 4;
        tp->btp = tp->lst.head = 0;
        tp->sname = 0;
        sp->name = name;
        sp->storage_class = sc_auto;
        sp->tp = tp;
        insert(sp,table);
        return sp;
}
void addblocklist(SYM *sp)
{
	LIST *l;
	if (!sp) return;
	l = xalloc(sizeof(LIST));
	l->link = 0;
	l->data = sp;
	if (varlisthead)
		varlisttail = varlisttail->link = l;
	else
		varlisthead = varlisttail = l;
}
void block(void)
{       
				SNODE *snp3;
				block_nesting = 1;
				lastst = 0;
				snp3 = snp_line();
				lastst = begin;
				needpunc(begin,0);
        dodecl(sc_auto);
				cseg();
				if (snp3)
					snp3->next = compound();
				else
					snp3 = compound();
				addblocklist(lsyms.head);
        genfunc(snp3);
        flush_peep();
}
void gather_labels(TABLE *oldlsym, TABLE *lsyms)
{
	TABLE sym;
	sym.head = 0;
	if (oldlsym->head == lsyms->head) {
		return;
	}
	else {
		SYM *sp = lsyms->head,*r = oldlsym->head;
		lsyms->head = 0;
		while (sp && sp != r) {
			SYM *q = sp->next;
			sp->next = 0;
			if (sp->storage_class == sc_label || sp->storage_class == sc_ulabel) {
				if (!oldlsym->head) {
					oldlsym->head = oldlsym->tail = sp;
				}
				else {
					oldlsym->tail->next = sp;
					oldlsym->tail = sp;
				}
			}
			else
				if (!lsyms->head)
					lsyms->head = lsyms->tail = sp;
				else
					lsyms->tail = lsyms->tail->next = sp;
			sp = q;
		}
		if (oldlsym->tail)
			oldlsym->tail->next = 0;
	}
}
SNODE *compoundblock(void)
{
	SNODE *snp;
	TABLE oldoldlsym;
	SYM *q;
	block_nesting++;
	oldoldlsym = oldlsym;
	oldlsym = lsyms;
	dodecl(sc_auto);
	cseg();
	snp = compound();
	check_funcused(&oldlsym,&lsyms);
	q = lsyms.head;
	gather_labels(&oldlsym,&lsyms);
	while (q && q->next != oldlsym.head)
		q = q->next;
	if (q) 
		q->next = 0;
	addblocklist(lsyms.head);
		
	lsyms = oldlsym;
	oldlsym.head = oldoldlsym.head;
	block_nesting--;
	return(snp);
}