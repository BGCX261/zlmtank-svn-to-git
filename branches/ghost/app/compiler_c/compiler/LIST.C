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
 * Listing file
 */
#include        <stdio.h>
#include        "expr.h"
#include        "c.h"

extern int prm_listfile;
extern HASHREC **globalhash;
extern FILE *listFile;
extern TABLE gsyms;
extern char *registers[];

/* Unnamed structure tags */
char *tn_unnamed = "<no name> ";


static char * unmangledname(char *str)
{
	static char name[40];
	unmangle(name,str);
	return name;
}
/* Put the storage class */
void put_sc(int scl)
{
  if (!prm_listfile)
    return;
       switch(scl) {
                case sc_static:
                        fprintf(listFile,"Static      ");
                        break;
                case sc_auto:
                        fprintf(listFile,"Auto        ");
                        break;
                case sc_autoreg:
                case sc_memberreg:
                        fprintf(listFile,"Register    ");
                        break;
                case sc_global:
                        fprintf(listFile,"Global      ");
                        break;
                case sc_abs:
                        fprintf(listFile,"Absolute    ");
                        break;
                case sc_external:
                case sc_externalfunc:
                        fprintf(listFile,"External    ");
                        break;
                case sc_type:
                        fprintf(listFile,"Type        ");
                        break;
                case sc_const:
                        fprintf(listFile,"Constant    ");
                        break;
                case sc_member:
                        fprintf(listFile,"Member      ");
                        break;
                case sc_label:
                        fprintf(listFile,"Label");
                        break;
                case sc_ulabel:
                        fprintf(listFile,"Undefined label");
                        break;
                }
}

/* Put the type */
void put_ty(TYP *tp)
{       if((tp == 0) || (!prm_listfile))
                return;
        switch(tp->type) {
								case bt_matchall:
												fprintf(listFile,"Undefined");
												break;
                case bt_char:
                        fprintf(listFile,"Char");
                        break;
                case bt_short:
                        fprintf(listFile,"Short");
                        break;
                case bt_enum:
                        fprintf(listFile,"enum ");
                        goto ucont;
                case bt_long:
                        fprintf(listFile,"Long");
                        break;
                case bt_unsigned:
                        fprintf(listFile,"Unsigned Long");
                        break;
                case bt_float:
                        fprintf(listFile,"Float");
                        break;
                case bt_double:
                        fprintf(listFile,"Double");
                        break;
                case bt_longdouble:
                        fprintf(listFile,"Long Double");
                        break;
                case bt_pointer:
                        if( tp->val_flag == 0)
                                fprintf(listFile,"Pointer to ");
                        else
                                fprintf(listFile,"Array of ");
                        put_ty(tp->btp);
                        break;
                case bt_union:
                        fprintf(listFile,"union ");
                        goto ucont;
                case bt_struct:
                        fprintf(listFile,"struct ");
ucont:                  if(tp->sname == 0)
                                fprintf(listFile,tn_unnamed);
                        else
                                fprintf(listFile,"%s ",unmangledname(tp->sname));
                        break;
								case bt_void:
												fprintf(listFile,"Void");
												break;
								case bt_ptrfunc:
                        fprintf(listFile,"Pointer to ");
                case bt_ifunc:
                case bt_func:
                        fprintf(listFile,"Function returning ");
                        put_ty(tp->btp);
                        break;
                }
	if (tp->startbit != -1)
		fprintf(listFile,"  Bits %d to %d",tp->startbit,tp->startbit+tp->bits-1);
}
/* List a variable */
void list_var(SYM *sp, int i)
{       int     j;
				long val;
				if (!prm_listfile)
					return;
        for(j = i; j; --j)
                fprintf(listFile,"    ");
				if ((sp->storage_class == sc_auto || sp->storage_class == sc_autoreg) && !sp->inreg)
					val = (long)getautoval(sp->value.i);
				else
					val = sp->value.u;
				if (sp->inreg)
        	fprintf(listFile,"%-10s = Register %-3s ",unmangledname(sp->name),registers[-val]);
				else
        	fprintf(listFile,"%-10s = %06x       ",unmangledname(sp->name),val);
        put_sc(sp->storage_class);
        put_ty(sp->tp);
        fprintf(listFile,"\n");
        if(sp->tp == 0)
                return;
        if((sp->tp->type == bt_struct || sp->tp->type == bt_union) &&
                sp->storage_class == sc_type)
                list_table(&(sp->tp->lst),i+1);
}

/* List an entire table */
void list_table(TABLE *t,int j)
{       SYM     *sp;
				int i;
				if (!prm_listfile)
					return;
				if (t == &gsyms) {
					for (i=0; i < HASHTABLESIZE; i++) {
						if ((sp=(SYM *) globalhash[i]) != 0) {
							while (sp) {
								list_var(sp,j);
        	 			sp = sp->next;
							}
						}
					}
				}
				else {
        	sp = t->head;
        	while(sp != NULL) {
                list_var(sp,j);
                sp = sp->next;
                }
				}
			
}