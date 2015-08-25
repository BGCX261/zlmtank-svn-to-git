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
/* Handles name mangling
 */
#include        <stdio.h>
#include				<malloc.h>
#include				<ctype.h>
#include        "expr.h"
#include        "c.h"
#include 				"errors.h"

extern TABLE oldlsym,lsyms,gsyms;
extern int prm_cplusplus,prm_cmangle;
#define HASHTABLESIZE 1023

#ifdef CPLUSPLUS
char destructor_name[] = "$dtr";
char constructor_name[] = "$ctr";

char *tn_void = "void";
char *tn_char = "char";
char *tn_int = "int";
char *tn_long = "long" ;
char *tn_short = "short";
char *tn_unsigned = "unsigned ";
char *tn_ellipse = "...";
char *tn_float = "float";
char *tn_double = "double";
char *tn_longdouble = "long double";
#endif /* CPLUSPLUS */

/* Insert an overloaded function ref into the function table
 */
void funcrefinsert(char *name, char *mangname, TYP *tp, TABLE *tbl)
{
#ifdef CPLUSPLUS
	char buf[100];
	SYM *sp,*sp1;
	sp = xalloc(sizeof(SYM));
	buf[0] = '$';
	buf[1] = '$';
	strcpy(&buf[2],name);
	sp->defalt = mangname;
	sp->storage_class = sc_defunc;
	sp->value.overlist.head = 0;
	sp->name = litlate(buf);
	sp->tp = tp;
	sp1 = search(buf,tbl);
	if (sp1)
		if (sp1->value.overlist.head == 0)
			sp1->value.overlist.head = sp1->value.overlist.tail = sp;
		else
			sp1->value.overlist.tail = sp1->value.overlist.tail->next = sp;
	else
		insert(sp,tbl);
#endif
}
#ifdef CPLUSPLUS
/* See if two C++ functions match */
static int fomatch(TYP *tp1, TYP *tp2)
{
	SYM *sp1 = tp1->lst.head;
	SYM *sp2 = tp2->lst.head;
	if (sp1 == (SYM *)-1 && sp2 == (SYM *)-1)
		return TRUE;
	if (sp1 && sp1->defalt)
		return TRUE;
	while (sp2 && sp1) {
		if (sp1->tp->type == bt_ellipse)
			return TRUE;
		if (!exactype(sp1->tp, sp2->tp) && !(isscalar(tp1) && isscalar(tp2)))
			return FALSE;
		if (sp1->defalt)
			return TRUE;
		sp1 = sp1->next;
		sp2 = sp2->next;
	}
	return sp1 == sp2;
}
#endif
/* Search the tables looking for a match for an argument type list */
SYM *funcovermatch(char *name,TYP *tp)
{
#ifdef CPLUSPLUS
	char buf1[100];
	TABLE *tbl1, *tbl2;
	if (prm_cplusplus) {
		SYM *sp2,*sp3,*sp4=0,*sp5=0,*sp6,*sp7;
		tbl1 = &lsyms;
		tbl2 = &gsyms;
		buf1[0] = '$';
		buf1[1] = '$';
		strcpy(&buf1[2],name);
		sp2 = search(buf1,tbl1);
		sp3 = search(buf1,tbl2);
		if (sp2)
			if (fomatch(sp2->tp,tp)) 
				sp4 = sp2;
		if (sp3)
			if (fomatch(sp3->tp,tp)) 
				sp5 = sp3;
		if (sp4 && sp5) {
			genfunc2error(ERR_AMBIGFUNC,sp4->defalt,sp5->defalt);
			return gsearch(sp4->defalt);
		}
		if (sp2) {
			sp6 = sp2->value.overlist.head;
			while (sp6) {
				if (fomatch(sp6->tp,tp)) {
					if (sp4) {
						genfunc2error(ERR_AMBIGFUNC,sp4->defalt,sp6->defalt);
						return gsearch(sp4->defalt);
					}
					else if (sp5) {
						genfunc2error(ERR_AMBIGFUNC,sp5->defalt,sp6->defalt);
						return gsearch(sp5->defalt);
					}
					else sp4 = sp6;
				}
				sp6 = sp6->next;
			}
		}
		if (sp3) {
			sp7 = sp3->value.overlist.head;
			while (sp7) {
				if (fomatch(sp7->tp,tp)) {
					if (sp4) {
						genfunc2error(ERR_AMBIGFUNC,sp4->defalt,sp7->defalt);
						return gsearch(sp4->defalt);
					}
					else if (sp5) {
						genfunc2error(ERR_AMBIGFUNC,sp5->defalt,sp7->defalt);
						return gsearch(sp5->defalt);
					}
					else sp5 = sp7;
				}
				sp7 = sp7->next;
			}
		}
		if (sp4)
			return gsearch(sp4->defalt);
		if (sp5)
			return gsearch(sp5->defalt);
	}
#endif
	return(gsearch(name));
}
#ifdef CPLUSPLUS
/* Mangle one C++ argument */
static char *cpponearg(char *buf,TYP *tp)
{
start:
	switch (tp->type) {
		case bt_ptrfunc:
			*buf++ = 'q';
			buf = cppargs(buf,tp->lst.head);
			*buf++ = '$';
			tp = tp->btp;
			goto start;
		case bt_struct:
		case bt_union:
		case bt_class:
		case bt_enum:
			sprintf(buf,"%d%s",strlen(tp->sname+1),tp->sname+1);
			buf = buf + strlen(buf);
			break;
		case bt_unsignedshort:
			*buf++ = 'u';
		case bt_short:
			*buf++ = 's';
			break;
		case bt_unsigned:
			*buf++ = 'u';
		case bt_long:
			*buf++ = 'i';
			break;
		case bt_unsignedchar:
			*buf++ = 'u';
		case bt_char:
			*buf++ = 'c';
			break;
		case bt_float:
			*buf++ = 'f';
			break;
		case bt_double:
			*buf++ = 'd';
			break;
		case bt_longdouble:
			*buf++ = 'g';
			break;
		case bt_pointer:
			*buf++ = 'p';
			tp = tp->btp;
			goto start;
		case bt_ref:
			*buf++ = 'r';
			tp = tp->btp;
			goto start;
		case bt_ellipse:
			*buf++ = 'e';
			break;
	}
	*buf = 0;
	return buf;
}
/* Mangle an entire C++ function */
static char *cppargs(char *buf,SYM *sp)
{
	*buf++ = 'q';
	if (sp == (SYM *)-1) {
		*buf++ = 'v';
	}
	else while (sp) {
		buf = cpponearg(buf,sp->tp);
		sp = sp->next;
	}
	*buf=0;
	return buf + strlen(buf);
}
/* Wrapper for function name mangling */
char * cppmangle(char *name, TYP *tp)
{
	char buf[100];
	if (*name == 0)
		return 0;
	if (prm_cmangle)
		name++;
	sprintf(buf,"@%s$",name);
	cppargs(buf+strlen(buf),tp->lst.head);
	return(litlate(buf));
}
	
/* Unmangle a pointer reference */
char * putptr(char *buf, int *ptr, int **ptrs)
{
	int *p = *ptrs;
	buf = buf+strlen(buf);
	while ((*ptr)--) {
		*buf++ = ' ';
		if (*p++) 
			*buf++= '&';
		else
			*buf++ = '*';
	}
	*ptr = 0;
	*buf = 0;
	return buf;
}			
			
/* Argument unmangling for C++ */
static char *unmang1(char *buf, char *name, int *ptr, int **ptrs)
{
start:
		if (isdigit(*name)) {
			int v = *name++ - '0';
			while (isdigit(*name)) 
				v = v*10+ *name++ - '0';
			while (v--) 
				*buf++ = *name++;
			*buf = 0;
		}
		else switch (*name++) {
			case 'q': {
					char buf1[100],buf2[100];
					strcpy(buf1," (*) ");
					buf+=5;
				  name = unmangcppfunc(buf1,name);
					name++;
					name = unmang1(buf2,name,ptr,ptrs);
					buf = putptr(buf,ptr,ptrs);
					strcpy(buf,buf2);
					strcat(buf,buf1);
					buf = buf +strlen(buf);
					
				}
				break;	
			case 'u':
				strcpy(buf,"unsigned ");
				buf = buf+9;
				switch(*name++) {
					case 'i':
						strcpy(buf,tn_int);
						buf = putptr(buf,ptr,ptrs);
						break;
					case 'l':
						strcpy(buf,tn_long);
						buf = putptr(buf,ptr,ptrs);
						break;
					case 's':
						strcpy(buf,tn_short);
						buf = putptr(buf,ptr,ptrs);
						break;
					case 'c':
						strcpy(buf,tn_char);
						buf = putptr(buf,ptr,ptrs);
						break;
				}
			case 'f':
				strcpy(buf,tn_float);
				buf = putptr(buf,ptr,ptrs);
				break;
			case 'd':
				strcpy(buf,tn_double);
				buf = putptr(buf,ptr,ptrs);
				break;
			case 'g':
				strcpy(buf,tn_longdouble);
				buf = putptr(buf,ptr,ptrs);
				break;
			case 'i':
				strcpy(buf,tn_int);
				buf = putptr(buf,ptr,ptrs);
				break;
			case 'l':
				strcpy(buf,tn_long);
				buf = putptr(buf,ptr,ptrs);
				break;
			case 's':
				strcpy(buf,tn_short);
				buf = putptr(buf,ptr,ptrs);
				break;
			case 'c':
				strcpy(buf,tn_char);
				buf = putptr(buf,ptr,ptrs);
				break;
			case 'v':
				strcpy(buf,tn_void);
				buf = putptr(buf,ptr,ptrs);
				break;
			case 'p':
				(*ptrs)[(*ptr)++] = 0;
				goto start;
			case 'r':
				(*ptrs)[(*ptr)++] = 1;
				goto start;
			case 'e':
				strcpy(buf,tn_ellipse);
				break;
			case '$':
				name--;
				return name;
		}
	return name;
}
/* Unmangle an entire C++ function */
static char * unmangcppfunc(char *buf, char *name)
{
	int ptr = 0;
	char ptrs[50];
	*buf++ = '(';
	while (*name && *name != '$') {
		name = unmang1(buf,name,&ptr,&ptrs);
		buf = buf+strlen(buf);
		if (*name && *name != '$')
			*buf++ = ',';
		else
			*buf++ = ')';
	}
	*buf=0;
	return buf;
}
#endif
/* Name unmangling in general */
void unmangle(char *buf, char *name)
{
	if (name[0] == '_' && prm_cmangle) {
		strcpy(buf,&name[1]);
	}
#ifdef CPLUSPLUS
	else 
		if (name[0] != '@')
			strcpy(buf,name);
		else {
			name++;
			while (*name != '$' && *name)
				*buf++ = *name++;
			if (*name) {
				name+=2;
				unmangcppfunc(buf,name);
			}
			else *buf++ = 0;
		}
#endif
}
