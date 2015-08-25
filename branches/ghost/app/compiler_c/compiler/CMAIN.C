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
 * The following symbols must be defined on the compile command line:
 *    PROGNAME
 *    ENVNAME
 *    GLBDEFINE
 *	  SOURCEXT
 */
#include        <stdio.h>
#include				<malloc.h>
#include 				<string.h>
#include 				<setjmp.h>
#include				<signal.h>
#include				<stdlib.h>
#include				"utype.h"	
#include				"cmdline.h"	
#include        "expr.h"
#include				"errors.h"
#include        "c.h"
#include 				"diag.h"

#define VERSION 151

typedef struct list {
		struct list *link;
		void *data;
} LIST;

extern char GLBDEFINE[],SOURCEXT[],ENVNAME[],PROGNAME[];

extern ERRORS *errlist;
extern TABLE    tagtable;
extern int      total_errors;
extern int diagcount;
extern char *errfile;
extern TABLE gsyms;

FILE            *inputFile = 0, *listFile = 0, *outputFile = 0, *cppFile = 0, *errFile = 0;
int stoponerr = 0;
LIST *clist = 0;
LIST *deflist = 0;
int prm_linkreg = TRUE;
int prm_stackcheck = FALSE;
int prm_warning = TRUE;
#ifdef OBJEXT
int prm_asmfile = FALSE;
#else
int prm_asmfile = TRUE;
#endif
int prm_ansi = FALSE;
int prm_listfile = FALSE;
int prm_maxerr = 25;
int prm_diag = FALSE;
int prm_bss = TRUE;
int prm_cppfile = FALSE;
int prm_packing = FALSE;
int prm_revbits = FALSE;
int prm_lines = TRUE;
int prm_cplusplus = FALSE;
int prm_cmangle = TRUE;
int basear = 1, basedr = 1, basefr = 1;
int prm_debug = FALSE;
int version = VERSION;
int prm_errfile = FALSE;

char *prm_searchpath = 0;
jmp_buf ctrlcreturn;
int file_count = 0;

char     *infile, listfile[40],outfile[40],cppfile[40],errorfile[40];
void bool_setup(char select, char *string);
void err_setup(char select, char *string);
void incl_setup(char select, char *string);
void def_setup(char select, char *string);
void codegen_setup(char select, char *string);
void optimize_setup(char select, char *string);
void warning_setup(char select, char *string);
void parsefile(char select, char *string);

/* setup for ARGS.C */
ARGLIST ArgList[] = {
  { 'i', ARG_BOOL, bool_setup },
  { 'e', ARG_BOOL, bool_setup },
  { 'f', ARG_CONCATSTRING, parsefile },
  { 'l', ARG_BOOL, bool_setup },
#ifdef OBJEXT
  { 'v', ARG_BOOL, bool_setup },
#endif
  { 'w', ARG_CONCATSTRING, warning_setup },
  { 'A', ARG_BOOL, bool_setup },
  { 'C', ARG_CONCATSTRING, codegen_setup },
  { 'O', ARG_CONCATSTRING, optimize_setup },
  { 'E', ARG_CONCATSTRING, err_setup },
  { 'I', ARG_CONCATSTRING, incl_setup },
	{ 'D', ARG_CONCATSTRING, def_setup },
#ifdef OBJEXT
  { 'S', ARG_BOOL, bool_setup },
#endif
  { 0, 0, 0 }
};



void bool_setup(char select, char *string)
/*
 * activation routine (callback) for boolean command line arguments
 */
{
	int bool = (int)string;
	if (select == 'S')
		prm_asmfile = bool;
  if (select == 'l')
		prm_listfile = bool;
	if (select == 'i')
		prm_cppfile = bool;
	if (select == 'e')
		prm_errfile = bool;
	if (select == 'v')
		prm_debug = bool;
	if (select == 'A')
		prm_ansi = bool;
}
void codegen_setup(char select, char *string)
/*
 * activation for code-gen type command line arguments
 */
{
		int bool = TRUE;
		while (*string) {
				switch (*string) {
					case 'd':
						prm_diag = bool;
						break;
					case 'p':
						prm_packing = bool;
						break;
					case 'r':
						prm_revbits = bool;
						break;
					case 'b':
						prm_bss=bool;
						break;
					case 'l':
						prm_lines = bool;
						break;
					case 'm':
						prm_cmangle = bool;
						break;
					case 'S':
						prm_stackcheck = bool;
						break;
					case 'R':
						prm_linkreg = bool;
						break;
					case '-':
						bool = FALSE;
						break;
					case '+':
						bool = TRUE;
						break;
					default:
						if (!confcodegen(*string,bool)) 
							fatal("Illegal codegen parameter ");
				}
			string++;
		}
}
void optimize_setup(char select, char *string)
/*
 * activation for optimizer command line arguments
 */
{
		int bool = TRUE;
		while (*string) {
			switch (*string) {
				case 'R':
					string++;
					while (*string && *string != '+' && *string != '-') {
						switch (*string++) {
							case 'a':
								basear = bool;
								break;
							case 'd':
								basedr = bool;
								break;
							case 'f':
								basefr = bool;
								break;
							default:
								goto errorx;
						}
						if (!*string)
							return;
					}
					break;
				case '-':
					bool = FALSE;
					break;
				case '+':
					bool = TRUE;
					break;
				default:
errorx:
					fatal("Illegal optimizer parameter");
			}
			string++;
		}
}
void err_setup(char select, char *string)
/*
 * activation for the max errs argument
 */
{
	prm_maxerr = atoi(string);
}
void incl_setup(char select, char * string)
/*
 * activation for include paths
 */
{
	if (prm_searchpath)
		prm_searchpath = realloc(prm_searchpath,strlen(string)+strlen(prm_searchpath)+1);
	else {
		prm_searchpath = malloc(strlen(string)+1);
		prm_searchpath[0] = 0;
	}
	strcat(prm_searchpath,string);
}
void def_setup(char select, char *string)
/*
 * activation for command line #defines
 */
{
	char *s = malloc(strlen(string)+1);
	LIST *l = malloc(sizeof(LIST));
	strcpy(s,string);
	l->link = deflist;
	deflist = l;
	l->data = s;
}
void setglbdefs(void)
/*
 * function declares any global defines from the command line and also
 * declares a couple of other macros so we can tell what the compiler is
 * doing
 */
{
	LIST *l = deflist;
	while (l) {
		char *s = l->data;
		char *n = s;
		while (*s && *s != '=')
			s++;
		if (*s == '=')
			*s++=0;
		glbdefine(n,s);
		l = l->link;
	}
#ifdef CPLUSPLUS
	if (prm_cplusplus) {
		glbdefine("__cplusplus","");
	}
#endif
	glbdefine(GLBDEFINE,"");
}
void InsertAnyFile(FILE *inf, FILE *outf, char *filename, char *path, int drive)
/*
 * Insert a file name onto the list of files to process
 */

{
  char *newbuffer, buffer[100],*p = buffer;
	LIST *r = &clist;

	file_count++;

	if (drive != -1) {
		*p++ = (char)(drive + 'A');
		*p++ = ':';
	}
	if (path) {
		strcpy(p,path);
		strcat(p,"\\");
	}
	else
		*p = 0;
  /* Allocate buffer and make .C if no extension */
	strcat(buffer,filename);
  AddExt(buffer,".C");
  newbuffer = (char *) malloc(strlen(buffer) + 1);
  strcpy(newbuffer,buffer);

  /* Insert file */
	while (r->link)
		r = r->link;
	r->link = malloc(sizeof(LIST));
	r = r->link;
	r->link = 0;
	r->data = newbuffer;
}
void dumperrs(FILE *file);
void setfile(char *buf,char *orgbuf,char *ext)
/*
 * Get rid of a file path an add an extension to the file name
 */
{
	char *p = strrchr(orgbuf,'\\');
	if (!p) p = orgbuf;
	else p++;
	strcpy(buf,p);
	StripExt(buf);
	AddExt(buf,ext);
}
int parse_arbitrary(char *string)
/*
 * take a C string and and convert it to ARGC, ARGV format and then run
 * it through the argument parser
 */
{
	char *argv[40];
	int rv,i;
	int argc = 1;
	if (!string || !*string)
		return 1;
	while (1) {
		while (*string == ' ') 
			string++;
		if (!*string)
			break;
		argv[argc++] = string;
		while (*string && *string != ' ') string++;
		if (!*string)
			break;
		*string = 0;
		string++;
	}
  rv = parse_args(&argc,argv,TRUE);
  for (i= 1; i < argc; i++)
		InsertAnyFile(0,0,argv[i],0,-1);
	return rv;
}
void parsefile(char select, char *string)
/*
 * parse arguments from an input file
 */
{
	FILE *temp;
	if (!(temp = fopen(string,"r")))
		fatal("Argument file not found");
	while (!feof(temp)) {
		char buf[256];
		buf[0] = 0;
		fgets(buf,256,temp);
		if (buf[strlen(buf)-1] == '\n')
			buf[strlen(buf)-1] = 0;
		if (!parse_arbitrary(buf))
			break;
	}
	fclose(temp);
}
void addinclude(void)
/*
 * Look up the INCLUDE environment variable and append it to the
 * search path
 */
{
	char *string = getenv("CCINCL");
	if (string && string[0]) {
		char temp[500];
		strcpy(temp,string);
		if (prm_searchpath) {
			strcat(temp,";");
			strcat(temp,prm_searchpath);
			free(prm_searchpath);
		}
		prm_searchpath = malloc(strlen(temp)+1);
		strcpy(prm_searchpath,temp);
	}
}
int parseenv(char *name)
/*
 * Parse the environment argument string
 */
{
	char *string = getenv(name);
	return parse_arbitrary(string);
}
void dumperrs(FILE *file)
{
#ifdef DIAGNOSTICS
	if (diagcount)
		fprintf(file,"%d Diagnostics detected\n",diagcount);
#endif
	if (total_errors)
		fprintf(file,"%d Total errors\n",total_errors);
	if (prm_diag)
		mem_summary();
}
void summary(void)
{       
	if (prm_listfile) {
    fprintf(listFile,"\f\n *** global scope symbol table ***\n\n");
    list_table(&gsyms,0);
		if (tagtable.head) {
    	fprintf(listFile,"\n *** structures and unions ***\n\n");
    	list_table(&tagtable,0);
		}
		dumperrs(listFile);
  }
}
void ctrlchandler(int aa)
{
	longjmp(ctrlcreturn,1);
}
int main(int argc,char *argv[])
{
	char buffer[40];
	char *p;
  banner("%s Version %d.%02d Copyright (c) 1994-1997, LADsoft",PROGNAME,VERSION/100,VERSION %100);
  outfile[0] = 0;

	/* parse the environment and command line */
	/* inpout cfg file has already been parsed */
  if (!parseenv(ENVNAME))
    usage(argv[0]);
  if (!parse_args(&argc, argv, TRUE) || (!file_count && argc == 1))
    usage(argv[0]);


	/* tack the environment includes in */
	addinclude();

	/* processor-dependent initialization */
	confsetup();

  /* Scan the command line for file names or response files */
	{ int i;
		for (i=1; i <argc; i++) 
			InsertAnyFile(0,0,argv[i],0,-1);
	}


  /* Set up a ctrl-C handler so we can exit the prog */
	signal(SIGINT,ctrlchandler);
	if (setjmp(ctrlcreturn))
		exit(1);
	/* loop through and compile all the files on the file list */
	while (clist) {
		inasmini();
		memini();
	  symini();
		stmtini();
		kwini();
		initini();
		preprocini();
		exprini();
		declini();
		funcini();
		peepini();
		outcodeini();
		regini();
		printf("file: %s\n",clist->data);
	  strcpy(buffer,clist->data);
#ifdef OBJEXT
		if (prm_asmfile)
#endif
			setfile(outfile,buffer,SOURCEXT);
#ifdef OBJEXT
		else
			setfile(outfile,buffer,OBJEXT);
#endif
		setfile(cppfile,buffer,".I");
		setfile(listfile,buffer,".LST");
		setfile(errorfile,buffer,".ERR");
		prm_cplusplus = FALSE;
	  AddExt(buffer,".C");
#ifdef CPLUSPLUS
		p = strrchr(buffer,'.');
		if (*(p-1) != '.') {
			if (p[1] == 'c' || p[1] == 'C')
				if (p[2] == 'p' || p[2] == 'P')
					if (p[3] == 'p' || p[3] == 'P')
						prm_cplusplus = TRUE;
		}
#endif
		infile = errfile = litlate(buffer);

		if (prm_cppfile) {
			if (!(cppFile = fopen(cppfile,"w")))
				fatal("Can't open cpp file %s",cppfile);
		}
  	if (!(inputFile = fopen(infile,"r")))
    	fatal("Can't open input file %s",infile);
  	if (!(outputFile = fopen(outfile,"w"))) {
    	fclose(inputFile);
    	fatal("Can't open output file %s",outfile);
	  }
		if (prm_asmfile)
			asm_header();
	  if (prm_listfile)
  		if (!(listFile = fopen(listfile,"w"))) {
    		fclose(inputFile);
		    fclose(outputFile);
    		fatal("Can't open list file %s",listfile);
  		}
	  if (prm_errfile)
  		if (!(errFile = fopen(errorfile,"w"))) {
				fclose(listFile);
    		fclose(inputFile);
		    fclose(outputFile);
    		fatal("Can't open error file %s",errorfile);
  		}
		initerr();
	  initsym();
		setglbdefs();
  	getch();
  	getsym();
	  compile();
		summary();
  	release_global();
		dumperrs(stdout);
	  fclose(inputFile);
		if (prm_asmfile) {
  		fclose(outputFile);
		}
  	if (prm_listfile)
    	fclose(listFile);
  	if (prm_errfile)
    	fclose(errFile);
  	if (prm_cppfile)
    	fclose(cppFile);
		clist = clist->link;

		/* Remove the ASM file if there are errors */
	if (total_errors)
			remove(outfile);

		/* Remove the ERR file if no warnings / errors */
		if (!errlist && prm_errfile)
			remove(errorfile);

		/* Flag to stop if there are any errors */
		stoponerr |= total_errors;
	}
  if (stoponerr)
   	return(1);
  else
    return(0);
}