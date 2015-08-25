/*                               `
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
#include 				<ctype.h>
#include				"utype.h"
#include				"cmdline.h"
#include        "expr.h"
#include        "c.h"
#include        "errors.h"
#include				"time.h"

extern short inputline[];
extern FILE *inputFile;
extern TABLE gsyms,defsyms;
extern long ival;
extern char laststr[];
extern HASHREC **defhash;
extern char *infile;
extern int incconst;
extern char *prm_searchpath;
extern int prm_cplusplus, prm_ansi;
extern short *lptr;
extern int cantnewline;
extern char *infile;
extern int backupchar;
extern int floatregs,dataregs,addrregs,basefr,basedr,basear;
extern int prm_cmangle;
extern enum e_sym lastst;
extern char lastid[];
extern int lastch;
extern int lineno;
extern int global_flag;
typedef struct _list {
		struct _list *link;
		char *data;
} LIST;
typedef struct _startups_ {
		struct _startups_ *link;
		char *name;
		int prio;
} STARTUPS;
char *errfile;
int errlineno = 0;
IFSTRUCT			  *ifshold[10];
char 						*inclfname[10];
FILE            *inclfile[10];
int             incldepth = 0;
int             inclline[10];
short            *lptr;
LIST *incfiles = 0,*lastinc;

IFSTRUCT *ifs = 0;
int ifskip = 0;
int elsetaken = 0;

void filemac(short *string);
void datemac(short *string);
void timemac(short *string);
void linemac(short *string);

static char *unmangid;		/* In this module we have to ignore leading underscores */
static STARTUPS *startuplist, *rundownlist;
static short defkw[] = { 'd','e','f','i','n','e','d', 0 };

/* List of standard macros */
#define INGROWNMACROS 4

struct inmac {
		char *s;
		void (*func)();
} ingrownmacros[INGROWNMACROS] = { 
			{ "__FILE__",filemac }, { "__DATE__",datemac, },
			{ "__TIME__", timemac }, { "__LINE__",linemac } };

void pushif(void);

/* Moudle init */
void preprocini()
{
	floatregs = basefr;
	dataregs = basedr;
	addrregs = basear;
	incldepth = 0;
	incfiles = 0;
	ifs = 0;
	ifskip = elsetaken = 0;
	unmangid = lastid;
	if (prm_cmangle)
		unmangid++;
	startuplist = rundownlist = 0;
}
/* Preprocessor dispatch */
int preprocess(void)
{
        ++lptr;
        lastch = ' ';
        getsym();               /* get first word on line */

        if( lastst != id  && lastst != kw_else && lastst!= kw_if) {
                generror(ERR_IDEXPECT,0,0);
                return incldepth == 0;
                }
        if( strcmp(unmangid,"include") == 0 )
                return doinclude();
        else if( strcmp(unmangid,"define") == 0 )
                return dodefine();
				else if (strcmp(unmangid,"endif") == 0)
								return doendif();
				else if (lastst == kw_else)
								return doelse();
				else if (strcmp(unmangid,"ifdef") == 0)
								return doifdef(TRUE);
				else if (strcmp(unmangid,"ifndef") == 0)
								return doifdef(FALSE);
				else if (lastst == kw_if) {
								repdefines(lptr);
								defcheck(lptr);
								return doif(0);
				}
        else if( strcmp(unmangid,"elif") == 0 ) {
								repdefines(lptr);
								defcheck(lptr);
                return doelif();
				}
				else if (strcmp(unmangid,"undef") == 0)
								return(doundef());
				else if (strcmp(unmangid,"error") == 0)
								return(doerror());
				else if (strcmp(unmangid,"pragma") == 0)
								return(dopragma());
				else if (strcmp(unmangid,"line") == 0)
								return(doline());
        else    {
                gensymerror(ERR_PREPROCID,unmangid);
                return incldepth == 0;
                }
}

int doerror(void)
{
	char *temp;
	int i=0;
	if (ifskip)
		return incldepth == 0;
	global_flag++;
	temp = xalloc(pstrlen(lptr)*3+2);
	pstrcpy(temp,lptr);
	while (*lptr)
		i+=installphichar(*lptr++,temp,i);
	temp[i-1] = 0;
	global_flag--;
	basicerror(ERR_ERROR,temp);
	return incldepth == 0;
}
int dopragma(void)
{
	char buf[40],*p=buf;
  STARTUPS *a;
	int val = 0,sflag;
	if (ifskip)
		return incldepth == 0;
	lineToCpp();
	getsym();
	if (lastst != id)
		return incldepth == 0;
	if (!strcmp(unmangid,"startup"))
		sflag = 1;
	else if (!strcmp(unmangid,"rundown"))
		sflag = 0;
	else if (!strncmp(unmangid,"regopt",6)) {
		short *s = lptr;
		dataregs = floatregs = addrregs = 0;
		while (*s != '\n') {
			switch (*s) {
				case 'a':
				case 'A':
					addrregs = 1;
					break;
				case 'f':
				case 'F':
					floatregs = 1;
					break;
				case 'd':
				case 'D':
					dataregs = 1;
					break;
			}
			s++;
		}
		return incldepth == 0;
	}
	else return incldepth == 0;

	if (prm_cmangle)	
		*p++ = '_';
	while (isalnum(*lptr) || *lptr == '_')
		*p++=*lptr++;
	*p=0;
	while (*lptr && (*lptr == ' ' || *lptr == '\t' || *lptr == ','))
		lptr++;
	if (*lptr  && *lptr != '\n' && !isdigit(*lptr)) {
		generror(ERR_ILLCHAR,*lptr,0);
		while (*lptr)
			lptr++;
	}

	if (isdigit(*lptr))
		while (isdigit(*lptr)) {
			val *= 10;
			val += (*lptr++)-'0';
		}
	else
		val = 64;
	++global_flag;
	a = xalloc(sizeof(STARTUPS));
	a->name = litlate(buf);
	a->prio = val;
  if (sflag) {
		a->link = startuplist;
		startuplist = a;
	}
	else {
		a->link = rundownlist;
		rundownlist = a;
	}
	--global_flag;
	while (*lptr && (*lptr == ' ' || *lptr == '\t'))
		lptr++;
	if (*lptr  && *lptr != '\n')
		generror(ERR_ILLCHAR,*lptr,0);
	return incldepth == 0;
}
void dumpstartups(void)
/*
 * Dump references to startup/rundown code
 */
{
	SYM *s;
	if (startuplist) {
		startupseg();
		while (startuplist) {
			s = search(startuplist->name,&gsyms);
			if (!s || s->tp->type != bt_ifunc)
				gensymerror(ERR_UPDOWN,startuplist->name);
			else  {
				gensrref(s,startuplist->prio);
				s->tp->uflags |= UF_USED;
			}
			startuplist = startuplist->link;
		}
	}
	if (rundownlist) {
		rundownseg();
		while (rundownlist) {
			s = search(rundownlist->name,&gsyms);
			if (!s || s->tp->type != bt_ifunc)
				gensymerror(ERR_UPDOWN,rundownlist->name);
			else {
				gensrref(s,rundownlist->prio);
				s->tp->uflags |= UF_USED;
			}
			rundownlist = rundownlist->link;
		}
	}
}
int doline(void)
/*
 * Handle #line directive
 */
{
	int n;
	getsym();
	if (lastst != iconst) 
		gensymerror(ERR_PREPROCID,"#line");
	else {
		n = ival;
		getsym();
		if (lastst != sconst) 
			gensymerror(ERR_PREPROCID,"#line");
		else 
			if (!ifskip) {
				errfile = litlate(laststr);
				errlineno = n-1;
			}
	}
	return incldepth == 0;
}
int doinclude(void)
/*
 * HAndle include files
 */
{       int     rv;
				FILE *oldfile = inputFile;
				incconst = TRUE;
        getsym();               /* get file to include */
				incconst = FALSE;
				if (ifskip)
					return incldepth == 0;
        if( lastst != sconst ) {
                gensymerror(ERR_INCLFILE,"include");
                return incldepth == 0;
                }
				if (incldepth > 9) {
					generror(ERR_PREPROCID, 0,0);
					return incldepth == 0;
				}
        inputFile = SearchPath(laststr,prm_searchpath,"r");
        if( inputFile == 0 ) {
                gensymerror(ERR_CANTOPEN,laststr);
                inputFile = oldfile;
                rv = incldepth == 0;
                }
        else    {
								LIST *list;
								pushif();
								ifshold[incldepth] = ifs;
								elsetaken = 0;
								ifskip = 0;
								ifs = 0;
				        inclline[incldepth] = lineno;
        				inclfile[incldepth] = oldfile;  /* push current input file */
								inclfname[incldepth++] = infile;
								global_flag++;
								infile = litlate(laststr);
								list = xalloc(sizeof(LIST));
								list->data = infile;
								list->link = 0;
								if (incfiles)
									lastinc = lastinc->link = list;
								else
									incfiles = lastinc = list;
								errfile = infile;
								errlineno = 0;
								global_flag--;
                rv = incldepth == 1;
                lineno = 0;
                }
        return rv;
}

short *plitlate(short *string)
{
	short *temp = xalloc(pstrlen(string)*sizeof(short)+sizeof(short));
	pstrcpy(temp,string);
	return temp;
}
void glbdefine(char *name, char*value)
{
{       SYM     *sp;
				short *p;
				DEFSTRUCT *def;
				if (( sp = search(name,&defsyms) )!= 0)
					return;
        ++global_flag;          /* always do #define as globals */
        sp = xalloc(sizeof(SYM));
        sp->name = litlate(name);
				def = xalloc(sizeof(DEFSTRUCT));
				def->args = 0;
				def->argcount = 0;
				def->string = p = xalloc(strlen(value)*sizeof(short));
				while (*value)
					*p++=*value++;
				*p++=0;
        sp->value.s = (char *) def;
        insert(sp,&defsyms);
        --global_flag;
        return;
}
}
/* Handle #defines
 * Doesn't check for redefine with different value
 * Does handle ANSI macros
 */
int dodefine(void)
{       SYM     *sp;
				DEFSTRUCT *def;
				short *args[40],count=0;
				short *olptr;
				int p;
        getsym();               /* get past #define */
				if (ifskip)
					return incldepth == 0;
				olptr = lptr;
        if( lastst != id ) {
                generror(ERR_IDEXPECT,0,0);
                return incldepth == 0;
                }
				if (( sp = search(unmangid,&defsyms) )!= 0)
					undef2();
        ++global_flag;          /* always do #define as globals */
        sp = xalloc(sizeof(SYM));
        sp->name = litlate(unmangid);
				def = xalloc(sizeof(DEFSTRUCT));
				def->args = 0;
				def->argcount = 0;
				if (lastch == '(') {
					getdefsym();
					getdefsym();
					while (lastst == id) {
						args[count++] = plitlate(unmangid);
						getdefsym();
						if (lastst != comma)
							break;
						getdefsym();
					}
					if (lastst != closepa)
					  generror(ERR_PUNCT,closepa,0);
					olptr = lptr;
					def->args = xalloc(count*sizeof(short *));
					memcpy(def->args,args,count*sizeof(short *));
					def->argcount = count+1;
				}
				while (iswhitespacechar(*olptr))
					olptr++;
				p = pstrlen(olptr);
				if (olptr[p-1] == 0x0a)
					olptr[p-1] = 0;
				def->string = plitlate(olptr);
        sp->value.s = (char *) def;
        insert(sp,&defsyms);
        --global_flag;
        return incldepth == 0;
}
/*
 * Undefine
 */
int doundef(void)
{
	getsym();
	if (!ifskip)
		undef2();
	return(incldepth == 0);
}
int undef2(void)
{
	if (lastst != id) 
    generror(ERR_IDEXPECT,0,0);
	else {
		SYM **p = (SYM **)LookupHash(unmangid,defhash,HASHTABLESIZE);
		if (p) {
			*p = (*p)->next;
		}
	}
}
void getdefsym(void)
{
				if (backupchar != -1) {
					lastst = backupchar;
					backupchar = -1;
					return;
				}
restart:        /* we come back here after comments */
        while(iswhitespacechar(lastch))
                getch();
        if( lastch == -1)
                lastst = eof;
        else if(isdigit(lastch))
                getnum();
        else if(isstartchar(lastch)) {
								lptr--;
                defid(unmangid,&lptr,0);	
								lastch = *lptr++;
								lastst = id;
				}
        else if (getsym2())
					goto restart;
}
int defid(short *name, short **p, char *q)
/*
 * Get an identifier during macro replacement
 */
{
	int count = 0,i=0;
			while (issymchar(**p)) {
				if (count < 100) {
					name[count++] = *(*p);
					if (q)
						i+=installphichar(*(*p),q,i);
				}
				(*p)++;
			}
			if (q) {
				if ((q[i-1] & 0xf0) == 0x90)
					q[i-1] = 0x90;
    	  q[i] = '\0';
			}
	name[count] = 0;
	return(count);
}
/* 
 * Insert a replacement string
 */
int definsert(short *end, short *begin, short * text, int len, int replen)
{
	short *q;
	int i,p, r;
	int val;
	if (begin != inputline) 
		if (*(begin-1) == '#') {
			if (*(begin-2) != '#') {
				begin--;
				replen++;
				r = pstrlen(text);
			
				text[r++] = '\"';
				text[r] = 0;
				for (i=r; i >= 0; i--)
					text[i+1] = text[i];
				*text = '\"';
			}
		}
	p = pstrlen(text);
	val = p - replen;
	r = pstrlen(begin);
	if (val + strlen(begin) >= len-1) {
		generror(ERR_MACROSUBS,0,0);
		return(-8000);
	}
	if (val > 0)
		for (q = begin + r+1; q >= end; q--)
			*(q+val) = *q;
	else
		if (val < 0) {
			r = pstrlen(end)+1;
			for (q = end; q < end+r; q++ )
				*(q+val) = *q;
		}
	for (i=0; i < p; i++)
		begin[i] = text[i]; 
	return(val);
}
/* replace macro args */	
int defreplace(short *macro, int count, short **oldargs, short **newargs)
{
	int i,rv;
	int instring = 0;
	short narg[1024];
	short name[100];
	short *p=macro,*q;
	while (*p) {
		if (*p == instring)
			instring = 0;
		else if (*p == '\'' || *p == '"')
			instring = *p;
		else if (!instring && isstartchar(*p)) {
			q = p;
			defid(name,&p,0);
			for (i=0; i < count; i++)
				if (!pstrcmp(name,oldargs[i])) {
					pstrcpy(narg,newargs[i]);
					if ((rv = definsert(p,q,narg,1024-(q-macro),p-q)) == -8000)
						return(FALSE);
					else {
						p += rv;
						break;
					}
				}
		}
		p++;
	}
	return(TRUE);
}
/* Handlers for default macros */
void cnvt(short *out,char *in)
{
	while (*in) 
		*out++=*in++;
	*out = 0;
}
void filemac(short *string)
{
	char str1[40];
	sprintf(str1,"\"%s\"",infile);
	cnvt(string,str1);
}
void datemac(short *string)
{
	char str1[40];
	struct tm *t1;
	time_t t2;
	time(&t2);
	t1 = localtime(&t2);
 	strftime(str1,40,"\"%b %d %Y\"",t1);
	cnvt(string,str1);
}
void timemac(short *string)
{
	char str1[40];
	struct tm *t1;
	time_t t2;
	time(&t2);
	t1 = localtime(&t2);
	str1[0] = '"';
 	strftime(str1,40,"\"%X\"",t1);
	cnvt(string,str1);
}
void linemac(short *string)
{
	char str1[40];
	sprintf(str1,"%d",lineno);
	cnvt(string,str1);
}
/* Scan for default macros and replace them */
void defmacroreplace(short *macro, short *name)
{
	int i;
	macro[0] = 0;
	for (i=0; i < INGROWNMACROS; i++)
		if (!strcmp(name,ingrownmacros[i].s)) {
			(ingrownmacros[i].func)(macro);
			break;
		}
}
/* Scan line for macros and do replacements */
void defcheck(short *line)
{
	short macro[1024];
	short name[100];
	short *args[40];
	char ascii[60];
	int tryagain = TRUE, changed = FALSE, waiting = FALSE,rv;
	short *p = line,*q;
	SYM *sp;
	while (tryagain) {
		p = line;
		tryagain = FALSE;
		while(*p) {
			q = p;
			if (*p == '"') {
				waiting = !waiting;
				p++;
			}
			else if (waiting)
				p++;
			else if (isstartchar(*p)) {
				defid(name,&p,ascii);
				if ((sp = search(ascii,&defsyms)) != 0) {
					DEFSTRUCT *def = sp->value.s;
					pstrcpy(macro,def->string);
					if (def->argcount) {
						int count = 0;
						short *q = p;
							while (iswhitespacechar(*q))
								q++;
						if (*q++ != '(')
							goto join;
						p = q;
						if (def->argcount > 1) {
							do {
								short *nm = name;
								int nestedparen = 0;
								while (((*p != ',' && *p != ')') || nestedparen) && *p != '\n') {
										if (*p == '(')
											nestedparen++;
										if (*p == ')' && nestedparen)
											nestedparen--;
										*nm++ = *p++;
								}
								while (iswhitespacechar(*(nm-1)))
									nm--;
								*nm = 0;
								nm = name;
								while (iswhitespacechar(*nm))
									nm++;
								args[count++] = plitlate(nm);
							} while (*p++ == ',');
						}
						else while (iswhitespacechar(*p++));
						if (*(p-1) != ')' || count != def->argcount-1) {
							generror(ERR_MACROSUBS,0,0);
							return;
						}
						/* Can't replace if tokenizing next */
						if (*p == '#' && *(p+1) == '#')
							continue;
						if (count == 0)
							goto insert;
						if (!defreplace(macro,count,def->args,args))
							return;
					}
insert:
					if ((rv=definsert(p,q,macro,4096-(q-line),p-q))==-8000)
						return;
					p+=rv;
					changed = tryagain = TRUE;
				}
				else {
join:
					defmacroreplace(macro,ascii);
					if (macro[0]) {
						if ((rv=definsert(p,q,macro,4096-(q-line),p-q))==-8000)
							return;
						p += rv;
						changed = TRUE;
					}
				}
			}
			else p++;
		}
	}
	/* Token pasting */
	if (changed) {
		p = q = line;
		while (*p) {
			if (*p == '#' && *(p+1) == '#')
				p+=2;
			else
				*q++ = *p++;
		}
		*q = 0;
	}
}
static void repdefines(short *lptr)
/*
 * replace 'defined' keyword in #IF and #ELIF statements
 */
{
	short *q = lptr;
	short name[40];
	char ascii[60];
	while (*lptr) {
		if (!pstrncmp(lptr,defkw,7)) {
			lptr +=7;
			if (*lptr == '(') 
				lptr++;
			else 
		 		expecttoken(openpa,0);
      while(iswhitespacechar(*lptr))
              lptr++;
			defid(name,&lptr,ascii);
      while(iswhitespacechar(*lptr))
              lptr++;
			if (*lptr == ')')
				lptr++;
			else
				expecttoken(closepa,0);
			if (search(ascii,&defsyms) != 0)
				*q++ = '1';
			else
				*q++ = '0';
			*q++ = ' ';
				
		}
		else {
			*q++ = *lptr++;
		}
	}
  *q = 0;
}
void pushif(void)
/* Push an if context */
{
	IFSTRUCT *p;
	global_flag++;
	p = xalloc(sizeof(IFSTRUCT));
	global_flag--;
	p->link = ifs;
	p->iflevel = ifskip;
	p->elsetaken = elsetaken;
	elsetaken = FALSE;
	ifs = p;
}
void popif(void)
/* Pop an if context */
{
	if (ifs) {
		ifskip = ifs->iflevel;
		elsetaken = ifs->elsetaken;
		ifs = ifs->link;
	}
	else {
		ifskip = 0;
		elsetaken = 0;
	}
}	
void ansieol(void)
{
	if (prm_ansi) {
		while (iswhitespacechar(*lptr))
			lptr++;
		if (*lptr) {
			lastch = *lptr;
			lastst = kw_if;
			generror(ERR_UNEXPECT,0,0);
		}
	}
}
int doifdef (int flag)
/* Handle IFDEF */
{
	SYM *sp;
	getch();
	while(isspace(lastch))
		getch();
	if (!isstartchar(lastch)) {
    generror(ERR_IDEXPECT,0,0);
    return incldepth == 0;
	}
	else
		getid();
 	sp = search(unmangid,&defsyms);
  pushif();
	if (sp && !flag || !sp && flag)
		ifskip = TRUE;
	ansieol();
	return(incldepth == 0);
}
int doif(int flag)
/* Handle #if */
{
	getsym();
  pushif();
	cantnewline = TRUE;
	if (!intexpr(0))
		ifskip = TRUE;
	cantnewline = FALSE;
	ansieol();
	return(incldepth == 0);
}
int doelif(void)
/* Handle #elif */
{
	int is;
	getsym();
	cantnewline = TRUE;
	is = !intexpr(0);
	cantnewline = FALSE;
	if (ifs) {
		if (!ifs->iflevel)
			ifskip = !ifskip || is || elsetaken;
			if (!ifskip)
				elsetaken = TRUE;
	}
	else
		generror(ERR_PREPROCMATCH,0,0);
	ansieol();
	return(incldepth == 0);
}
/* handle else */
int doelse(void)
{
	if (ifs) {
		if (!ifs->iflevel)
			ifskip = !ifskip || elsetaken;
	}
	else
		generror(ERR_PREPROCMATCH,0,0);
	ansieol();
	return(incldepth == 0);
}
/* HAndle endif */
int doendif(void)
{
	if (!ifs)
		generror(ERR_PREPROCMATCH,0,0);
	popif();
	ansieol();
	return(incldepth == 0);
}