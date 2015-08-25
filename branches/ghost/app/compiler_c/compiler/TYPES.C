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

extern int prm_cplusplus;
extern char *tn_void;
extern char *tn_char;
extern char *tn_int;
extern char *tn_long;
extern char *tn_short;
extern char *tn_unsigned;
extern char *tn_ellipse;
extern char *tn_float;
extern char *tn_double;
extern char *tn_longdouble;

int exactype(TYP *typ1, TYP *typ2)
{
	SYM *s1,*s2;
	while (typ1 && typ2) {
		if (typ1->type != typ2->type) {
		  if ((typ1->type != bt_func && typ1->type != bt_ifunc) ||
					(typ2->type != bt_ifunc && typ2->type != bt_func))
				return 0;
		}
		else switch(typ1->type) {
			case bt_ptrfunc:
			case bt_ifunc:
			case bt_func:
				if (!exactype(typ1->btp,typ2->btp))
					return 0;
				s1 = typ1->lst.head;
				s2 = typ2->lst.head;
				while (s1 && s2 && (s1 != (SYM *)-1) && (s2 != (SYM *)-1)) {
					if (!exactype(s1->tp,s2->tp))
						return 0;
					s1 = s1->next;
					s2 = s1->next;
				}
				return s1 == s2;
			case bt_struct:
			case bt_union:
			case bt_class:
				if (!typ1->sname || !typ2->sname)
					return 0;
				return(!strcmp(typ1->sname,typ2->sname));
			case bt_pointer:
				if (typ1->val_flag != typ2->val_flag || (typ1->size && typ2->size && typ1->size != typ2->size))
					return 0;
		}
		typ1=typ1->btp;
		typ2=typ2->btp;
	}
	return !(typ1 || typ2);
}
int checktype(TYP *typ1, TYP *typ2)
{
#ifdef CPLUSPLUS
	if (prm_cplusplus)
		return exactype(typ1,typ2);
#endif
	if (typ1->type == typ2->type)
		return TRUE;
	else                                                 
		if ((typ2->type == bt_ptrfunc && (typ1->type == bt_ifunc || typ1->type == bt_func))
					 || (typ1->type == bt_ptrfunc && (typ2->type == bt_ifunc || typ2->type == bt_func)))
			return(TRUE);
		else
			if (typ1->type == bt_enum) {
				switch (typ2->type) {
					case bt_long: case bt_unsigned:
					case bt_short: case bt_unsignedshort:
					case bt_char: case bt_unsignedchar:
						return(TRUE);
				}
			}
			else
				if (typ2->type == bt_enum) {
					switch (typ1->type) {
						case bt_long: case bt_unsigned:
						case bt_short: case bt_unsignedshort:
						case bt_char: case bt_unsignedchar:
							return(TRUE);
					}
				}
				else
					return FALSE;
}
int checktypeassign(TYP *typ1, TYP *typ2)
{
#ifdef CPLUSPLUS
	if (prm_cplusplus)
		return exactype(typ1,typ2);
#endif
	while (typ1 && typ2 && typ1->type == typ2->type && typ1->size == typ2->size) {
		if (isscalar(typ1))
			return TRUE;
		typ1 = typ1->btp;
		typ2 = typ2->btp;
	}
	if (!typ1 && !typ2)
		return(TRUE);
	return(FALSE);
}
#ifdef CPLUSPLUS
static TYP *unmang2(char *buf, TYP *tp, int *ptr, int **ptrs)
{
	SYM *sp;
		switch (tp->type) {
			case bt_ptrfunc: {
					unmang2(buf,tp->btp,ptr,ptrs);
					buf = putptr(buf+strlen(buf),ptr,ptrs);
					sp = tp->lst.head;
					strcat(buf,"(*) (");
					if (sp != (SYM *)-1) {
						while (sp) {
							unmang2(buf+strlen(buf),sp->tp,ptr,ptrs);
							sp = sp->next;
						}
					}
					strcat(buf,")");
					buf = buf + strlen(buf);
					return 0;
				}
			case bt_float:
				strcpy(buf,tn_float);
				buf = putptr(buf,ptr,ptrs);
				return 0;
			case bt_double:
				strcpy(buf,tn_double);
				buf = putptr(buf,ptr,ptrs);
				return 0;
			case bt_longdouble:
				strcpy(buf,tn_longdouble);
				buf = putptr(buf,ptr,ptrs);
				return 0;
			case bt_unsigned:
				strcpy(buf,tn_unsigned);
				buf = buf + strlen(buf);
			case bt_long:
				strcpy(buf,tn_int);
				buf = putptr(buf,ptr,ptrs);
				return 0;
			case bt_unsignedshort:
				strcpy(buf,tn_unsigned);
				buf = buf + strlen(buf);
			case bt_short:
				strcpy(buf,tn_short);
				buf = putptr(buf,ptr,ptrs);
				return 0;
			case bt_unsignedchar:
				strcpy(buf,tn_unsigned);
				buf = buf + strlen(buf);
			case bt_char:
				strcpy(buf,tn_char);
				buf = putptr(buf,ptr,ptrs);
				return 0;
			case bt_void:
				strcpy(buf,tn_void);
				buf = putptr(buf,ptr,ptrs);
				return 0;
			case bt_pointer:
				(*ptrs)[(*ptr)++] = 0;
				return tp->btp;
			case bt_ref:
				(*ptrs)[(*ptr)++] = 1;
				return tp->btp;
			case bt_ellipse:
				strcpy(buf,tn_ellipse);
				return 0;
		}
	return 0;
}
char * typenum(char *buf, TYP *typ)
{
	int ptr = 0;
	char ptrs[50];
	*buf++ = '(';
	while (typ) {
		typ = unmang2(buf,typ,&ptr,ptrs);
		buf = buf+strlen(buf);
		if (typ) 
			*buf++ = ',';
		else
			*buf++ = ')';
	}
	*buf=0;
	return buf;
}
#endif