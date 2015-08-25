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
 * error handler
 */
#include        <stdio.h>
#include 				<string.h>
#include        "expr.h"
#include        "c.h"
#include        "errors.h"
#include 				"diag.h"

extern int prm_errfile;
extern FILE *errFile;
extern int global_flag;
extern FILE *listFile;
extern int errlineno;
extern char *errfile;
extern int prm_asmfile;
extern FILE *outputFile;
extern int prm_maxerr;
extern int prm_diag;
extern int prm_listfile;
extern int lastch;
extern enum e_sym lastst;
extern char lastid[];
extern int lineno;
extern FILE            *inclfile[10];  /* shared with preproc */
extern int             incldepth;      /* shared with preproc */
extern char *infile;
extern SYM *currentfunc;
extern int prm_warning,prm_cplusplus;

int diagcount = 0;
ERRORS *errlist = 0;
static ERRORS *errtail = 0;
static ERRORS *curerr = 0; 
static char expectlist[] = { "########################=##############,:;#[]{}()" };
static int errline;

/* table of which warnings are enabled */
char nowarn[ERR_MAX];
/* table of warning keywords for the -w command line option
 */
char warnarray[ERR_MAX][4] = { 
	"all","","","","","","","",
	"","","","","","","","",
	"","","","","","","","",
	"","","","","","","","",
	"","","","","cln","","","",
	"","","","","","","","",
	"","","","","","ret","sun","sud",
	"sas","npo","urc","fun","cno","ieq","","nco",
	"lad","","","zer","dpc","nsf","lun","pro",
	"cnv","","irg","san","ssu","","","",
	"","","tua","","tui","","","",
	"","","","","","","","",
	"","","","","","suz","fsu","lli",
  "","","","","","spc","","",
	"","","","","","","","",
  "","","","","",""
};
int             total_errors = 0;
void initerr(void)
{
                                errlist = errtail = curerr = 0;
        total_errors = 0;
                                diagcount  = 0;                 
	errline = 0;
}
/*
 * handling for warnings on the command line
 */
void warning_setup(char select, char *string)
{
	int bool = FALSE;
	while (*string) {
		int i;
		if (string[0] == '-') {
			bool = TRUE;
			string++;
		}
		else
			if (string[0] == '+')
				string++;
		for (i=0; i < ERR_MAX; i++)
			if (!strncmp(warnarray[i],string,3)) {
				if (i== 0) {
					int j;
					for (j =0; j < ERR_MAX; j++)
						nowarn[j] = (unsigned char)bool;
				}
				else
					nowarn[i] = (unsigned char)bool;
				string += 3;
				break;
			}
		if (i==ERR_MAX) {
			fatal("Invalid warning");
		}
	}
}
#ifdef DIAGNOSTICS
void diag(char *s)
/*
 * internal diags come here
 */
{ 
        diagcount++;
  if (prm_diag) {
                printf("DIAG - %s\n",s);
                if (prm_errfile && errFile)
                        fprintf(errFile,"/*DIAG - %s*/",s);
                if (prm_listfile && listFile)
                        fprintf(listFile,"/*DIAG - %s*/",s);
                if (prm_asmfile)
                        fprintf(outputFile,"DIAG - %s\n",s);
        }
}
#endif
int printerr(char *buf, ERRORS *err)
/*
 * subroutine gets the error code and returns whether it is an error or
 * warning
 */
{
        int errlvl = 0;
        switch (err->errornumber) {
								case ERR_NOCASE:
                        sprintf(buf,"Expected 'case' or 'default'");
                        break;
                case ERR_PUNCT:
                        sprintf(buf,"Expected '%c'",expectlist[(int)err->data]);
                        break;
                case ERR_INSERT:
                        sprintf(buf,"Inserted '%c'",expectlist[(int)err->data]);
                        break;
                case ERR_NEEDCHAR:
                        sprintf(buf,"Expected '%c'",(char)err->data);
                        break;
                case ERR_ILLCHAR:
                        sprintf(buf,"Illegal character '%c'",(char)err->data);
                        break;
                case ERR_NEEDCONST:
                        sprintf(buf,"Constant value expected");
                        break;
                case ERR_UNDEFINED:
                        sprintf(buf,"Undefined symbol '%s'", (char *)err->data);
                        break;
                case ERR_DUPSYM:
                        sprintf(buf,"Duplicate symbol '%s'", (char *)err->data);
                        break;
                case ERR_IDENTEXPECT:
                        sprintf(buf,"Expected '%s'",(char *)err->data);
                        break;
                case ERR_IDEXPECT:
                        sprintf(buf,"Identifier expected");
                        break;
                case ERR_INITSIZE:
                        sprintf(buf,"Too many initializers");
                        break;
                case ERR_NOINIT:
                        sprintf(buf,"Cannot initialize '%s'",(char *)err->data);
                        break;
                case ERR_PREPROCID:
                        sprintf(buf,"Invalid preprocessor directive '%s'",(char *)err->data);
                        break;
                case ERR_INCLFILE:
                        sprintf(buf,"File name expected in #include directive");
                        break;
                case ERR_CANTOPEN:
                        sprintf(buf,"Cannot open file \"%s\" for read access",(char *)err->data);
                        break;
                case ERR_ILLCLASS:
                        sprintf(buf,"Illegal storage class specifier '%s'",(char *)err->data);
                        break;
                case ERR_ILLCLASS2:
                        sprintf(buf,"Illegal storage class specifier on '%s'",(char *)err->data);
                        break;
                case ERR_DUPCASE:
                        sprintf(buf,"Duplicate case %d",(int)err->data);
                        break;
                case ERR_RETMISMATCH:
                        sprintf(buf,"Type mismatch in return");
                        break;
                case ERR_ARGMISMATCH:
                        sprintf(buf,"Type mismatch in arg '%s'",(char *)err->data);
                        break;
                case ERR_ARGLENSHORT:
                        sprintf(buf,"Argument list too short in redeclaration of function '%s'",(char *)err->data);
                        break;
                case ERR_ARGLENLONG:
                        sprintf(buf,"Argument list too long in redeclaration of function '%s'",(char*)err->data);
                        break;
                case ERR_DECLMISMATCH:
                        sprintf(buf,"Type mismatch in redeclaration of '%s'",(char *)err->data);
                        break;
                case ERR_CALLMISMATCH:
                        sprintf(buf,"Type mismatch in arg %s",(char *)err->data);
                        break;
                case ERR_CALLLENSHORT:
                        sprintf(buf,"Argument list too short %s",(char *)err->data);
                        break;
                case ERR_CALLLENLONG:
                        sprintf(buf,"Argument list too long %s",(char *)err->data);
                        errlvl = 1;
                        break;
                case ERR_LABEL:
                        sprintf(buf,"'%s' is not a label",(char *)err->data);
                        break;
                case ERR_NOPOINTER:
                        sprintf(buf,"Pointer type expected");
                        break;
                case ERR_LVALUE:
                        sprintf(buf,"Lvalue expected");
                        break;
                case ERR_NOFUNC:
                        sprintf(buf,"'%s' is not a function",(char *)err->data);
                        break;
                case ERR_MISMATCH:
                        sprintf(buf,"Type mismatch");
                        break;
                case ERR_ELSE:
                        sprintf(buf,"Misplaced else");
                        break;
                case ERR_EXPREXPECT:
                        sprintf(buf,"Expression expected");
                        break;
                case ERR_DEREF:
                        sprintf(buf,"Illegal pointer");
                        break;
                case ERR_UNEXPECT:
                        if (lastst == id)
                                sprintf(buf,"Unexpected '%s'",lastid);
                        else
                                sprintf(buf,"Unexpected '%c'",lastch);
                        break;
                case ERR_ILLTYPE:
                        sprintf(buf,"Illegal typedef of '%s'",(char *)err->data);
                        break;
                case ERR_ARRAYMISMATCH:
                        sprintf(buf,"Non-scalar array index");
                        break;
                case ERR_PREPROCMATCH:
                        sprintf(buf,"Unbalanced preprocessor directives");
                        break;
                case ERR_MACROSUBS:
                        sprintf(buf,"Macro substitution error");
                        break;
                case ERR_DECLEXPECT:
                        sprintf(buf,"Declaration expected");
                        break;
                case ERR_INVFLOAT:
                        sprintf(buf,"Invalid floating point");
                        break;
                case ERR_INVTRAP:
                        sprintf(buf,"Invalid trap id");
                        break;
                case ERR_BFILLEGAL:
                        sprintf(buf,"Cannot use bit field as a non-member");
                        break;
                case ERR_BFTOOBIG:
                        sprintf(buf,"Bit field too big");
                        break;
                case ERR_BFTYPE:
                        sprintf(buf,"Bit field only allowed on scalar types");
                        break;
                case ERR_ERROR:
                        sprintf(buf,"User error: %s",(char *)err->data);
                        break;
                case ERR_INTERP:
                        sprintf(buf,"%s",(char *)err->data);
                        break;
                case ERR_BFADDR:
                        sprintf(buf,"Cannot take address of bit field");
                        break;
                case ERR_MODCONS:
                        sprintf(buf,"Cannot modify a const val");
                        break;
                case ERR_SZTYPE:
                        sprintf(buf,"Type expected in sizeof");
                        break;
                case ERR_FUNCRETVAL:
                        sprintf(buf,"Function should return a value");
                        errlvl = 1;
                        break;
                case ERR_STATICSYMUNUSED:
                        sprintf(buf,"Static variable '%s' is declared but never used",(char *)err->data);
                        errlvl = 1;
                        break;
                case ERR_SYMUNUSED:
                        sprintf(buf,"Variable '%s' is declared but never used",(char *)err->data);
                        errlvl = 1;
                        break;
                case ERR_FUNCUNUSED:
                        sprintf(buf,"Static function '%s' is declared but never used",(char *)err->data);
                        errlvl = 1;
                        break;
                case ERR_SYMUNDEF:
                        sprintf(buf,"Possible use of '%s' before assignment",(char *)err->data);
                        errlvl = 1;
                        break;
                case ERR_SYMASSIGNED:
                        sprintf(buf,"Variable '%s' is possibly assigned a value which is never used",(char *)err->data);
                        errlvl = 1;
                        break;
                case ERR_NONPORT:
                        sprintf(buf,"Nonportable pointer conversion");
                        errlvl = 1;
                        break;
                case ERR_UNREACHABLE:
                        sprintf(buf,"Unreachable code");
                        errlvl = 1;
                        break;
								case ERR_CODENONE:
												sprintf(buf,"Code has no effect");
												errlvl = 1;
												break;
								case ERR_BADEQUATE:
												sprintf(buf,"Possible incorrect assignment");
												errlvl = 1;
												break;
								case ERR_NOANDREG:
												sprintf(buf,"Invalid '&' on register var '%s'",(char *)err->data);	
												break;
								case ERR_NOCONTINUE:
												sprintf(buf,"Continue not allowed");	
												break;
								case ERR_DUPLABEL:
												sprintf(buf,"Duplicate label '%s'",(char *)err->data);	
												break;
								case ERR_NOFUNCARRAY:
												sprintf(buf,"Function cannot return array");	
												break;
								case ERR_NOVOIDRET:
												sprintf(buf,"Return type is void");	
												errlvl = 1;
												break;
								case ERR_ZEROSTORAGE:
												sprintf(buf,"No memory allocated for '%s'",(char *)err->data);	
												errlvl = 1;
												break;
								case ERR_ZEROPTR:
												sprintf(buf,"Illegal use of void pointer");
												break;
								case ERR_SHORTPOINTER:
												sprintf(buf,"Dangerous pointer cast");	
												errlvl = 1;
												break;
								case ERR_NOSTATICFUNC:
												sprintf(buf,"Nonexistant static func '%s'",(char *)err->data);	
												break;
								case ERR_UNUSEDLABEL:												
												errlvl = 1;
												sprintf(buf,"Unused label '%s'",(char *)err->data);	
												break;
								case ERR_NOPROTO:
												sprintf(buf,"Call to function '%s' with no prototype",(char *)err->data);	
												errlvl = prm_cplusplus==0;
												break;
								case ERR_LOSTCONV:
												sprintf(buf,"Conversion may truncate significant digits");	
												errlvl = 1;
												break;
								case ERR_UNDEFLABEL:
												sprintf(buf,"Undefined label '%s'",(char *)err->data);	
												break;
								case ERR_ILLREGISTER:
												sprintf(buf,"Illegal register var '%s'",(char *)err->data);
												errlvl = 1;
												break;
								case ERR_SUPERAND:
												sprintf(buf,"Possible superfluous &");
												errlvl = 1;
												break;
								case ERR_NODECLARE:
												sprintf(buf,"Declaration not allowed here");
												break;
								case ERR_NOMAIN:
												sprintf(buf,"Illegal call to main() from within program");
												break;
								case ERR_NOREF:
												sprintf(buf,"Illegal use of reference operator");
												break;
								case ERR_CANTREF:
												sprintf(buf,"Cannot define a pointer or reference to a reference");
												break;
								case ERR_TEMPUSED:
												sprintf(buf,"Temporary used for parameter %s",(char *)err->data);
												errlvl = 1;
												break;
								case ERR_REFMUSTINIT:
												sprintf(buf,"Reference variable '%s' must be initialized",(char *)err->data);
												break;
								case ERR_TEMPINIT:
												sprintf(buf,"Temporary used to initialize %s",(char *)err->data);
												errlvl = 1;
												break;
								case ERR_REFLVALUE:
												sprintf(buf,"Reference initialization needs lvalue");
												break;
								case ERR_REFNOCONS:
												sprintf(buf,"Reference member '%s' in a class with no constructors",(char *)err->data);
												break;
								case ERR_MISSINGDEFAULT:
												sprintf(buf,"Default missing after parameter '%s'",(char *)err->data);
												break;
								case ERR_AMBIGFUNC:
												sprintf(buf,"Ambiguity between %s",(char *)err->data);
												break;
								case ERR_NOLOCALDEFAULT:
												sprintf(buf,"Local variables may not be used as parameter defaults");
												break;
								case ERR_CPPMISMATCH:
												sprintf(buf,"Cannot cast %s",(char *)err->data);
												break;
								case ERR_NOOVERMAIN:
												sprintf(buf,"Cannot overload 'main'",(char *)err->data);
												break;
								case ERR_SWITCHINT:
												sprintf(buf,"Switch argument must be of integral type");
												break;
								case ERR_NOFUNCMATCH:
												sprintf(buf,"Could not find a match for '%s'",(char *)err->data);
												break;
/*
								case ERR_PREDEFSTRUCT:
												sprintf(buf,"'%s' must be a predefined class or struct",(char *)err->data);
												break;
								case ERR_LOCALCLASS:
												sprintf(buf,"Local class functions not supported",(char *)err->data);
												break;
								case ERR_PUREDECL:
												sprintf(buf,"Illegal pure declaration syntzx of '%s'",(char *)err->data);
												break;
								case ERR_BADESTRUCT:
												sprintf(buf,"Destructor for class '%s' expected",(char *)err->data);
												break;
								case ERR_TYPECONSTRUCT:
												sprintf(buf,"Constructor/destructor must be untyped");
												break;
								case ERR_NOTYPEQUAL:
												sprintf(buf,"Variable '%s' cannot have a type qualifier",(char *)err->data);
												break;
								case ERR_NOTACLASS:
												sprintf(buf,"Variable '%s' is not a class instance",(char *)err->data);
												break;
*/                case ERR_SIZE:
												if (err->data)
												  sprintf(buf,"Size of '%s' is unknown or zero",(char *)err->data);
												else
												  sprintf(buf,"Size is unknown or zero");
												break;
								case ERR_NEVERSTRUCT:
												sprintf(buf,"Structure '%s' is undefined",(char *)err->data);
												errlvl = 1;
												break;
												
								case ERR_LONGLONG:
												sprintf(buf,"long long int type not supported, defaulting to long int");
												errlvl = 1;
												break;
								case ERR_UPDOWN:
												sprintf(buf,"Startup/rundown function '%s' is unknown or not a function",(char *)err->data);
												break;
			case ERR_INTBITFIELDS:
                        	sprintf(buf,"Bit fields must be signed or unsigned int");
													break;
			case ERR_COMMENTMATCH:
													sprintf(buf,"File ended with comment in progress");
													break;
			case ERR_PASCAL_NO_ELLIPSE:
													sprintf(buf,"Ellipse (...) not allowed in pascal declaration");
													break;
			case ERR_PASCAL_NO_INT:
													sprintf(buf,"_int keyword not allowed in pascal declaration");
													break;
	    case ERR_SUSPICIOUS:
													sprintf(buf,"Suspicious pointer conversion");
												 	errlvl = 1;
													break;
	    case ERR_NOFUNCSTRUCT:
													sprintf(buf,"Function declaration not allowed here");
													break;
	    case ERR_STRINGTOOBIG:
													sprintf(buf,"String constant too long");
													break;
	    case ERR_CONSTTOOLARGE:
													sprintf(buf,"Numeric constant is too large");
													break;
			case ERR_MULTIPLEINIT:
												  sprintf(buf,"Multiple initialization for '%s'",(char *)err->data);
													break;
			case ERR_INVALIDSTRING:
												  sprintf(buf,"Invalid string operation");
													break;
			case ERR_AMODEEXPECT:
												  sprintf(buf,"Assembler: Address mode expected");
													break;
			case ERR_ASCALE:
												  sprintf(buf,"Assembler: Valid scale factor expected");
													break;
			case ERR_AINVINDXMODE:
												  sprintf(buf,"Assembler: Invalid indexing");
													break;
			case ERR_AILLADDRESS:
												  sprintf(buf,"Assembler: Invalid address mode");
													break;
			case ERR_ATOOMANYSPECS:
												  sprintf(buf,"Assembler: Too many specifiers");
													break;
			case ERR_ATOOMANYSEGS:
												  sprintf(buf,"Assembler: Too many segments");
													break;
			case ERR_AINVOP:
												  sprintf(buf,"Assembler: Invalid opcode");
													break;
			case ERR_AINVSIZE:
												  sprintf(buf,"Assembler: Size mismatch");
													break;
			case ERR_AUSELEA:
												  sprintf(buf,"Assembler: Must use LEA to take the address of a local variable");
													break;
			case ERR_ALABEXPECT:
												  sprintf(buf,"Assembler: Label expected");
													break;
			case ERR_ANEEDFP:
												  sprintf(buf,"Assembler: Floating point register expected");
													break;
											
							  default:
                        sprintf(buf,"Error #%d",err->errornumber);
                        break;
        }
        return errlvl;
}
void     lferror(void)
/*
 * sticck an error in the list file
 */
{  
        char buf[100];
        while(curerr) {
                                                int errlvl = printerr(buf,curerr);
                                                if (!errlvl) {
																										if (prm_listfile)
                                                        fprintf(listFile,"**** ERROR: %s\n",buf);
																								}
                                                else if (prm_warning) {
																										if (prm_listfile)
                                                        fprintf(listFile,"** WARNING: %s\n",buf);
																								}
                                                curerr = curerr->link;
                                }
                                                
}
void basicskim(int *skimlist)
/*
 * simple skim for a token with no nesting
 */
{
                int i;
                for (i=0;;i++) {
                        if (lastst == skimlist[i] || lastst == eof)
                                break;
                        if (skimlist[i] == 0) {
                                getsym();
                                i = 0;
                        }
                }
}
/*
 * the following routines do token skimming and keep track of parenthesis
 * and brace nesting levels as well
 */
BALANCE *newbalance(BALANCE *bal)
{
        BALANCE *rv = xalloc(sizeof(BALANCE));
        rv->back = bal;
        rv->count = 0;
        if (lastst == openpa)
                rv->type = BAL_PAREN;
        else
                rv->type = BAL_BRACKET;
        return(rv);
}
void setbalance(BALANCE **bal)
{
        if (*bal == 0)
                if (lastst = openpa || lastst == closepa)
                        *bal = newbalance(*bal);
                else
                        return;
        switch (lastst) {
                case closepa:
                                        while (*bal && (*bal)->type != BAL_PAREN) {
                                                (*bal) = (*bal)->back;
                                        }
                                        if (!((*bal)->type)--)
                                                (*bal) = (*bal)->back;
                                        else return;    
                case closebr:
                                        while (*bal && (*bal)->type != BAL_BRACKET) {
                                                (*bal) = (*bal)->back;
                                        }
                                        if (!((*bal)->type)--)
                                                (*bal) = (*bal)->back;
                case openpa:
                                        if ((*bal)->type != BAL_PAREN)
                                                *bal = newbalance(*bal);
                                        (*bal)->count++;
                                        break;
                                        
                case openbr:
                                        if ((*bal)->type != BAL_BRACKET)
                                                *bal = newbalance(*bal);
                                        (*bal)->count++;
                                        break;
        }
        return;
}
void expskim(int *skimlist)
{
        BALANCE *bal = 0;
        int i = 0;
        for (i = 0; ; i++) {
                if (lastst == openpa || lastst == openbr) {
                        setbalance(&bal);
                        getsym();
                }
                else
                        if (lastst == eof)
                                break;
                        else
                                if (lastst == skimlist[i])
                                        if (lastst == closepa || lastst == openpa) {
                                                if (!bal)
                                                        break;
                                                setbalance(&bal);
                                                getsym();
                                        }
                                        else
                                                break;
                                else
                                        if (skimlist[i] == 0) {
                                                i = 0;
                                                getsym();
                                        }
        }
        
}
void basicerror(int n, void *data)
/*
 * standard routine for putting out an error
 */
{
        char buf[100];
        ERRORS *nexterr;
        int errlvl,errored = 0;;
        global_flag++;
	        nexterr = xalloc(sizeof(ERRORS));
  	      global_flag--;
    	    nexterr->errornumber = n;
      	  nexterr->link = 0;
        	nexterr->data = data;
	        if (errlist == 0)
  	              errlist = errtail = nexterr;
    	    else {
      	          errtail->link = nexterr;
        	        errtail = nexterr;
	        }
        errlvl = printerr(buf, nexterr);
        if (curerr == 0)
                curerr = nexterr;
        if (!errlvl) {
								errline = lineno;
                fprintf(stdout,"Error   %s(%d):  %s",errfile,errlineno,buf);
								if (prm_errfile)
                	fprintf(errFile,"Error   %s(%d):  %s",errfile,errlineno,buf);
								errored++;
                total_errors++;
        }
        else if (prm_warning && !nowarn[n] && (errline != lineno)) {
								errored++;
                fprintf(stdout,"Warning %s(%d):  %s",errfile,errlineno,buf);
								if (prm_errfile)
                	fprintf(errFile,"Warning %s(%d):  %s",errfile,errlineno,buf);
				}
				if (errored) {
        	if (currentfunc) {
								unmangle(buf,currentfunc->name);
                fprintf(stdout," in function '%s'",buf);
								if (prm_errfile)
                	fprintf(errFile," in function '%s'",buf);
					}
        	fputc('\n',stdout);
					if (prm_errfile)
        		fputc('\n',errFile);
				}
  if (total_errors > prm_maxerr) {
                fatal("Too many errors");
        }
}
void Error(char *string)
/*
 * some of the library functions required a generic error function
 *
 * we are remapping it to the C/C++ error routines
 */
{
        basicerror(ERR_INTERP,(void *)string);
}
void generror(int n, int data, int *skimlist)
/*
 * most errors come here
 */
{                

        basicerror(n,(void *)data);
        if (skimlist) 
                basicskim(skimlist);
}
void gensymerror(int n, char *data)
/*
 * errors come here if the error has a symbol name
 */
{
				char buf[100];
				if (data)
					unmangle(buf,data);
				else
					buf[0] = 0;
        global_flag++;
        basicerror(n,(void *)litlate(buf));
        global_flag--;
}
/*
 * the next two functions are for reporting full C++ functions with
 * the argument list types
 */
void genfuncerror(int n, char*func, char *data)
{
				char buf[100],buf1[100],buf2[100];
				unmangle(buf1,func);
				if (data) {
					unmangle(buf2,data);
					buf[0] = '\'';
					buf[1] = 0;
					strcat(buf,buf2);
					strcat(buf,"' ");
				}
				else
					buf[0] = 0;
				strcat(buf,"in call to function ");
				strcat(buf,"'");
				strcat(buf,buf1);
				strcat(buf,"'");
        global_flag++;
        basicerror(n,(void *)litlate(buf));
        global_flag--;
}
void genfunc2error(int n, char*func, char *func2)
{
				char buf[100],buf1[100],buf2[100];
				unmangle(buf1,func);
				unmangle(buf2,func2);
				buf[0] = '\'';
				buf[1] = 0;
				strcpy(buf,buf2);
				strcat(buf,"'");
				strcat(buf," and ");
				strcat(buf,"'");
				strcat(buf,buf1);
				strcat(buf,"'");
        global_flag++;
        basicerror(n,(void *)litlate(buf));
        global_flag--;
}
/*
 * C++ errors for class names and type checking
 */
void genclasserror(int n, char *struc, char *elem)
{
				char buf[100],buf1[100],buf2[100];
				unmangle(buf1,elem);
				unmangle(buf2,struc);
				buf[0] = '\'';
				buf[1] = 0;
				strcpy(buf,buf2);
				strcat(buf,"::");
				strcat(buf,buf1);
				strcat(buf,"'");
        global_flag++;
        basicerror(n,(void *)litlate(buf));
        global_flag--;
}
void genmismatcherror(TYP *tp1, TYP *tp2)
{
#ifdef CPLUSPLUS
				char buf[100],buf1[100],buf2[100];
				typenum(buf1,tp1);
				typenum(buf2,tp2);
				buf[0] = '\'';
				buf[1] = 0;
				strcat(buf,buf1);
				strcat(buf,"'");
				strcat(buf," to ");
				strcat(buf,"'");
				strcat(buf,buf2);
				strcat(buf,"'");
        global_flag++;
        basicerror(ERR_CPPMISMATCH,(void *)litlate(buf));
        global_flag--;
#endif
}
/*
 * various utilities for special case errors
 */
void expecttoken(int n, int *skimlist)
{
        if (skimlist)
                generror(ERR_PUNCT, n, skimlist);
        else
                generror(ERR_INSERT, n, 0);
}
void generrorexp(int n, int data, int *skimlist)
{                
        basicerror(n,(void *)data);
        if (skimlist) 
                expskim(skimlist);
}
void gensymerrorexp(int n, char *data)
{
        global_flag++;
        basicerror(n,(void *)litlate(data));
        global_flag--;
}
void expecttokenexp(int n, int *skimlist)
{
        if (skimlist)
                generrorexp(ERR_PUNCT, n, skimlist);
        else
                generrorexp(ERR_INSERT, n, 0);
}