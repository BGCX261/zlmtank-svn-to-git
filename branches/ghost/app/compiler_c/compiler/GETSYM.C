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
/* scanner
 */
/* Trigraphs implemented, won't work for token pasting though */
#include        <stdio.h>
#include				<ctype.h>
#include				<limits.h>
#include				"list.h"
#include        "expr.h"
#include        "c.h"
#include        "errors.h"
#include "utype.h"
#include "interp.h"

extern int prm_errfile;
extern int version;
extern LIST *clist;
extern FILE *cppFile, *listFile, *inputFile;
extern int prm_cplusplus,prm_cmangle,prm_ansi;
extern int ifskip,elsetaken;
extern IFSTRUCT *ifs;
extern char *errfile;
extern int errlineno;
extern int phiused;

extern char *infile;
extern short            *lptr;          /* shared with preproc */
extern FILE            *inclfile[10];  /* shared with preproc */
extern char            *inclfname[10];  /* shared with preproc */
extern IFSTRUCT				 *ifshold[10];
extern int             inclline[10];   /* shared with preproc */
extern int             incldepth;      /* shared with preproc */
extern int						prm_listfile;
int lineno;
short     inputline[4096];
int             lastch;
enum e_sym      lastst;
char            lastid[100] = "";
char            laststr[MAX_STRLEN + 1] = "";
long            ival = 0;
long double          rval = 0.0;
char            *linstack[20];  /* stack for substitutions */
char            chstack[20];    /* place to save lastch */
int             lstackptr = 0;  /* substitution stack pointer */
int cantnewline = FALSE;
int incconst = FALSE;

int backupchar = -1;

static int phiputcpp,phiputlist,phiputerror;
static int commentlevel;

void initsym(void)
{       lptr = inputline;
        inputline[0] = 0;
        lineno = 0; errlineno = 0;
				lastid[0] = 0;
				laststr[0] = 0;
				ival = 0;
				rval = 0.0;
				cantnewline=FALSE;
				incconst = FALSE;
				backupchar = -1;
				phiputcpp=phiputlist = phiputerror = FALSE;
}

void lineToCpp(void)
/*
 * line has been preprocessed, dump it to a file
 */
{
		if (cppFile) {
			char buf[100],*q=buf;
			short *p = inputline;
			*q = 0;
			if (!phiputcpp) {
				phiputcpp = TRUE;
				if (phiused)
					fputc('\x1f',cppFile);
				fprintf(cppFile,"/* LADsoft C compiler Version %d.%02d */\n\n",version / 100, version %100);
			}
			while (*p) {
				int i;
				if (*q) {
					buf[0] = *q;
					i = 1;
				}
				else
					i = 0;
				i+=installphichar(*p++,buf,i);
				buf[i] = 0;
				q = buf;
				while (*q && (*(q+1) || ((*q &0xf0) != 0x90)))
					fputc(*q++,cppFile);
			}
		}				
}
/* Strips comments and also the newline char at the end of the line */
static void stripcomment (short *line)
{
	short *s = line, *e = s;
	while (*e) {
		if (*e == '/' && (!commentlevel || !prm_ansi || prm_cplusplus)) {
				if (*(e+1) == '*') {
					e++;
					commentlevel++;
				}
				else if (*(e+1) == '/' && !commentlevel && (!prm_ansi || prm_cplusplus)) {
					*s = 0;
					return;
				}
				else if (!commentlevel)
					*s++ = *e;
		}
		else if (commentlevel && *e == '*' && *(e+1) == '/') {
			commentlevel--;
			e++;
			if (commentlevel == 0) {
				*s++ = ' ';		/* Comments are whitespace */
			}
		}
		else if (!commentlevel)
			*s++ = *e;
    e++;
	}
	*s = 0;
}
/* strip trigraphs */
void striptrigraph(short *buf)
{
	short *cp = buf;
	while (*cp) {
		if (*cp == '?' && *(cp+1) == '?') {
			cp +=2;
			switch (*cp++) {
				case '=':
					*buf++='#';
					break;
				case '(':
					*buf++='[';
					break;
				case '/':
					*buf++='\\';
					break;
				case ')':
					*buf++=']';
					break;
				case '\'':
					*buf++='^';
					break;
				case '<':
					*buf++='{';
					break;
				case '!':
					*buf++='|';
					break;
				case '>':
					*buf++='}';
					break;
				case '-':
					*buf++='~';
					break;
				default:
					cp-=2;
					break;
			}
		}
		else *buf++ = *cp++;
	}
	*buf = 0;
}
int     getline(int listflag)
/*
 * Read in a line, preprocess it, and dump it to the list and preproc files
 * Also strip comments and alter trigraphs
 */
{       int     rv,rvc,i,prepping,temp;

		char ibuf[4096];
		int *ptr = ibuf;
		if (cantnewline) {
			return(0);
		}
		do {
				rv = FALSE;
				prepping = FALSE;
				rvc = 0;
        if( lineno > 0 && listflag && prm_listfile) {
					if (!phiputlist) {
						if (phiused) {
							fputc('\x1f',listFile);
						}
						phiputlist = TRUE;
					}
        }
        if( lineno > 0 && prm_errfile) {
					if (!phiputerror) {
						if (phiused) {
							fputc('\x1f',listFile);
						}
						phiputerror = TRUE;
					}
        }
				lferror();
				while(rvc +131 < 4096 && !rv) {
	        ++lineno; ++errlineno;
        	rv = (philine(ibuf+rvc,200,inputFile) == NULL);
					if (rv)
						break;
					rvc = strlen(ibuf);
					if (ibuf[rvc-1] != '\n') {
						ibuf[rvc++] = '\n';
						ibuf[rvc] = 0;
					}
					rvc-=2;
					while (ibuf[rvc]==' ')
						rvc--;
					if (ibuf[rvc] != '\\')
						break;
				}
				if (rvc)
					rv = FALSE;
        if(prm_listfile) {
					if (!phiputlist) {
						if (phiused) {
							fputc('\x1f',listFile);
						}
						fprintf(listFile,"LADsoft C compiler Version %d.%02d - %s\n\n",version / 100, version % 100,clist->data);
						phiputlist = TRUE;
					}
					fprintf(listFile,"%5d: %s",lineno, ibuf);
        }
        if( rv) {
								if (ifs)
									generror(ERR_PREPROCMATCH,0,0);
								if (commentlevel)
									generror(ERR_COMMENTMATCH,0,0);
								if (incldepth > 0) {
	                fclose(inputFile);
  	              inputFile = inclfile[--incldepth];
    	            lineno = inclline[incldepth];
									infile = inclfname[incldepth];
									errlineno = lineno;
									errfile = infile;
									ifs = ifshold[incldepth];
									commentlevel = 0;
									popif();
                	return getline(0);
								}
        }
        if( rv )
                return 1;
				lptr = inputline;
				ptr = ibuf;
				while((temp = parsechar(&ptr)) != 0)
					*lptr++ = temp;
				*lptr = 0;
				stripcomment(inputline);
				striptrigraph(inputline);
        lptr = inputline;
				while (iswhitespacechar(*lptr))
					lptr++;			
        if(lptr[0] == '#') {
          listflag = preprocess();
					prepping = TRUE;
				}
		} while (ifskip || prepping);
		defcheck(inputline);
		lineToCpp();
    return 0;
}

/*
 *      getch - basic get character routine.
 */
int     getch(void)
{       while( (lastch = *lptr++) == '\0') {
                if( lstackptr > 0 ) {
                        lptr = linstack[--lstackptr];
                        lastch = chstack[lstackptr];
                        return lastch;
                        }
								if (cantnewline) {
									lptr--;
									return lastch = ' ';
								}
                if(getline(incldepth == 0))
                        return lastch = -1;
                }
        return lastch;
}
 
/*
 *      getid - get an identifier.
 *
 *      identifiers are any isidch conglomerate
 *      that doesn't start with a numeric character.
 *      this set INCLUDES keywords.
 */
void     getid()
{       register int    i;
        i = 0;
				if (prm_cmangle)
					lastid[i++] = '_';		/* Mangling */
				if (lastch == 'L') {
					lastid[i++] = 'L';
					getch();
					if (lastch == '\"') {
												getch();
												i=0;
												while (lastch != '\"' && lastch) {
													*(((short*)(laststr))+i++) = lastch;
													getch();
												}
												if ((lastch & 0x7f) != '\"')
        	                        generror(ERR_NEEDCHAR,'\"',0);
          	              else
            	                    getch();
													*(((short*)(laststr))+i) = 0;
												lastst = lsconst;
												return;
					}
				}
				
				while(issymchar(lastch)) {
					if (i < 100)
						i+=installphichar(lastch,lastid,i);
					getch();
        }
				if ((lastid[i-1] & 0xf0) == 0x90)
					lastid[i-1] = 0x90;
        lastid[i] = '\0';
        lastst = id;
}
 
/*
 *      getsch - get a character in a quoted string.
 *
 *      this routine handles all of the escape mechanisms
 *      for characters in strings and character constants.
 */
int     getsch(void)        /* return an in-quote character */
{       register int    i, j;
        if(lastch == '\n')
                return -1;
        if(lastch != '\\') {
                i = lastch;
                getch();
                return i;
                }
        getch();        /* get an escaped character */
        if(isdigit(lastch)) {
                for(i = 0,j=0;j < 3;++j) {
                        if(lastch <= '7' && lastch >= '0')
                                i = (i << 3) + lastch - '0';
                        else
                                break;
                        getch();
                        }
                return i;
                }
        i = lastch;
        getch();
        switch(i) {
                case '\n':
                        getch();
                        return getsch();
								case 'a':
												return '\a';
                case 'b':
                        return '\b';
                case 'f':
                        return '\f';
                case 'n':
                        return '\n';
                case 'r':
                        return '\r';
								case 't':
												return '\t';
								case '\'':
												return '\'';
								case '\"':
												return '\"';
								case '\\':
												return '\\';
								case 'x':
												{
													int n=0,count=0;
												  while (isxdigit(lastch)) {
														count++;
													  lastch-=0x30;
														if (lastch > 10) lastch -=7;
														if (lastch > 10) lastch -=32;
														n*=16;
														n+=lastch;
														getch();
													}
													if (count > 2)
														generror(ERR_CONSTTOOLARGE,0,0);
													return n;
												}
                default:
												if (isdigit(i) && i < '9') {
													int n = 0;
													while (isdigit(i) && i < '9') {
														n = n * 8 + (lastch - '0');
														getch();
													}
													return n;
												}
                        return i;
                }
}

int     radix36(char c)
{       if(isdigit(c))
                return c - '0';
        if(c >= 'a' && c <= 'z')
                return c - 'a' + 10;
        if(c >= 'A' && c <= 'Z')
                return c - 'A' + 10;
        return -1;
}
 
/*
 *      getbase - get an integer in any base.
 */
void getbase(int b,char **ptr)
{       register long    i, j;
				int errd = 0;
        i = 0;
        while(isalnum(**ptr)) {
                if((j = radix36(*(*ptr)++)) < b) {
												if (i > (ULONG_MAX-j)/b)
													if (!errd) {
														generror(ERR_CONSTTOOLARGE,0,0);
														errd++;
													}
                        i = i * b + j;
                        }
                else break;
                }
        ival = i;
        lastst = iconst;
}
 
/*
 *      getfrac - get fraction part of a floating number.
 */
void getfrac(char **ptr)
{       double  frmul;
        frmul = 0.1;
        while(isdigit(**ptr)) {
                rval += frmul * (*(*ptr)++ - '0');
                frmul *= 0.1;
                }
}
 
/*
 *      getexp - get exponent part of floating number.
 *
 *      this algorithm is primative but usefull.  Floating
 *      exponents are limited to +/-255 but most hardware
 *      won't support more anyway.
 */
void getexp(char **ptr)
{       double  expo, exmul;
        expo = 1.0;
        if(lastst != rconst)
                rval = ival;
        if(**ptr == '-') {
                exmul = 0.1;
                (*ptr)++;
                }
        else {
                exmul = 10.0;
								if (**ptr == '+')
									(*ptr)++;
				}
        getbase(10,ptr);
        if(ival > 255)
                generror(ERR_FPCON,0,0);
        else
                while(ival--)
                        expo *= exmul;
        rval *= expo;
				lastst = rconst;
}
 
/*
 *      getnum - get a number from input.
 *
 *      getnum handles all of the numeric input. it accepts
 *      decimal, octal, hexidecimal, and floating point numbers.
 */
void getnum(void)
{
	int isfloat=FALSE;
	char buf[50],*ptr = buf;
	while (isxdigit(lastch) || lastch == 'x' || lastch == 'X') {
		*ptr++ = lastch;
		getch();
	}
	if (lastch == '.') {
		isfloat = TRUE;
		*ptr++=lastch;
		getch();
		while (isdigit(lastch)) {
			*ptr++ = lastch;
			getch();
		}
	}
	if (lastch == 'e' || lastch == 'E') {
		isfloat = TRUE;
		*ptr++ = lastch;
		getch();
		if (lastch == '+' || lastch == '-') {
			*ptr++=lastch;
			getch();
		}
		while (isdigit(lastch)) {
			*ptr++ = lastch;
			getch();
		}
	}
	if (lastch == 'F') {
		isfloat = TRUE;
	}
	*ptr = 0;
	ptr = buf;
	if (!isfloat) {
		if (*ptr == '0') {
			ptr++;
      if(*ptr == 'x' || *ptr == 'X') {
      	ptr++;
      	getbase(16,&ptr);
    	}
    	else getbase(8,&ptr);
   	}
		else
			getbase(10,&ptr);
		if (lastch == 'U') {
			lastst = iuconst;
			getch();
			if (lastch == 'L') {
				lastst = luconst;
				getch();
			}
		}
		else if (lastch == 'L') {
			lastst = iconst;
			getch();
			if (lastch == 'U') {
				lastst = luconst;
				getch();
			}
		}
	}
  else    {
                getbase(10,&ptr);
	                if(*ptr == '.') {
											ptr++;
                        rval = ival;    /* float the integer part */
                        getfrac(&ptr);      /* add the fractional part */
                        lastst = rconst;
  		                  }
      	          if(*ptr == 'e' || *ptr == 'E') {
                        ptr++;
                        getexp(&ptr);       /* get the exponent */
                        }
									if (lastch == 'F') {
											if (lastst != rconst) {
												rval = ival;
											}
											lastst = fconst;
											getch();
									}
									else if (lastch == 'L') {
											if (lastst != rconst) {
												rval = ival;
											}
											lastst = lrconst;
											getch();
									}
								}
}
 
int getsym2(void) 
/*
 * translate character sequences to appropriate token names
 */
{       register int    i, j;
				int size;
swlp:
						 switch(lastch) {
                case '+':
                        getch();
                        if(lastch == '+') {
                                getch();
                                lastst = autoinc;
                                }
                        else if(lastch == '=') {
                                getch();
                                lastst = asplus;
                                }
                        else lastst = plus;
                        break;
                case '-':
                        getch();
                        if(lastch == '-') {
                                getch();
                                lastst = autodec;
                                }
                        else if(lastch == '=') {
                                getch();
                                lastst = asminus;
                                }
                        else if(lastch == '>') {
                                getch();
#ifdef CPLUSPLUS
																if (prm_cplusplus && lastch == '*') {
																	getch();
																	lastst = pointstar;
																}
																else
#endif
                                	lastst = pointsto;
                                }
                        else lastst = minus;
                        break;
                case '*':
                        getch();
                        if(lastch == '=') {
                                getch();
                                lastst = astimes;
                                }
                        else lastst = star;
                        break;
                case '/':
                        getch();
                        if(lastch == '=') {
                                getch();
                                lastst = asdivide;
                                }
                        else lastst = divide;
                        break;
                case '^':
                        getch();
                        if(lastch == '=') {
                                getch();
                                lastst = asxor;
                                }
                        else lastst = uparrow;
                        break;
                case ';':
                        getch();
                        lastst = semicolon;
                        break;
                case ':':
                        getch();
#ifdef CPLUSPLUS
												if (prm_cplusplus && lastch == ':') {
													lastst = classsel;
													getch();
												}
												else
#endif
                        	lastst = colon;
                        break;
                case '=':
                        getch();
                        if(lastch == '=') {
                                getch();
                                lastst = eq;
                                }
                        else lastst = assign;
                        break;
                case '>':
                        getch();
                        if(lastch == '=') {
                                getch();
                                lastst = geq;
                                }
                        else if(lastch == '>') {
                                getch();
                                if(lastch == '=') {
                                        getch();
                                        lastst = asrshift;
                                        }
                                else lastst = rshift;
                                }
                        else lastst = gt;
                        break;
                case '<':
                        getch();
												if (incconst) {
                      	  for(i = 0;i < MAX_STRLEN;++i) {
                                if(lastch == '>')
                                        break;
                                if((j = getsch()) == -1)
                                        break;
                                else
                                        laststr[i] = j;
	                              }
  	                      laststr[i] = 0;
    	                    lastst = sconst;
      	                  if(lastch != '>')
        	                        generror(ERR_NEEDCHAR,'>',0);
          	              else
            	                    getch();
												} else
                        	if(lastch == '=') {
                                getch();
                                lastst = leq;
                                }
                        	else if(lastch == '<') {
                                getch();
                                if(lastch == '=') {
                                        getch();
                                        lastst = aslshift;
                                        }
                                else lastst = lshift;
                                }
                        	else lastst = lt;
                        break;
                case '\'':
                        getch();
                        ival = getsch();        /* get a string char */
                        if(lastch != '\'')
                                generror(ERR_NEEDCHAR,'\'',0);
                        else
                                getch();
                        lastst = cconst;
                        break;
								case 0x2d4:
												getch();
												i=0;
												while (lastch != '\"' && lastch)
													i=installphichar(lastch,laststr,i);
												if ((lastch & 0x7f) != '\"')
        	                        generror(ERR_NEEDCHAR,'\"',0);
          	              else
            	                    getch();
												size = strlen(laststr);
												lastst = sconst;
												break;
                case '\"':
					 							size = 0;
												while (lastch == '\"') {
                        	getch();
                        	for(i = size;i < MAX_STRLEN;++i) {
                                if(lastch == '\"')
                                        break;
                                if((j = getsch()) == -1)
                                        break;
                                else
                                        laststr[i] = j;
                                }
	                        laststr[i] = 0;
													size = i;
    	                    lastst = sconst;
      	                  if(lastch != '\"')
        	                        generror(ERR_NEEDCHAR,'\"',0);
          	              else
            	                    getch();
												}
                        break;
                case '!':
                        getch();
                        if(lastch == '=') {
                                getch();
                                lastst = neq;
                                }
                        else lastst = not;
                        break;
                case '%':
                        getch();
                        if(lastch == '=') {
                                getch();
                                lastst = asmodop;
                                }
                        else lastst = modop;
                        break;
                case '~':
                        getch();
                        lastst = compl;
                        break;
                case '.':
												if (isdigit(*lptr))
													getnum();
												else {	
	                        getch();
#ifdef CPLUSPLUS
													if (prm_cplusplus && lastch == '*') {
														getch();
														lastst = dotstar;
													}
													else 
#endif
													  if (lastch == '.') {
													  	getch();
															if (lastch == '.') {
																getch();
																lastst = ellipse;
																break;
															}
															else {
          	        	      	  generror(ERR_ILLCHAR,lastch,0);
															}
														}
                        	lastst = dot;
												}
                        break;
                case ',':
                        getch();
                        lastst = comma;
                        break;
                case '&':
                        getch();
                        if( lastch == '&') {
                                lastst = land;
                                getch();
                                }
                        else if( lastch == '=') {
                                lastst = asand;
                                getch();
                                }
                        else
                                lastst = and;
                        break;
                case '|':
                        getch();
                        if(lastch == '|') {
                                lastst = lor;
                                getch();
                                }
                        else if( lastch == '=') {
                                lastst = asor;
                                getch();
                                }
                        else
                                lastst = or;
                        break;
                case '(':
                        getch();
                        lastst = openpa;
                        break;
                case ')':
                        getch();
                        lastst = closepa;
                        break;
                case '[':
                        getch();
                        lastst = openbr;
                        break;
                case ']':
                        getch();
                        lastst = closebr;
                        break;
                case '{':
                        getch();
                        lastst = begin;
                        break;
                case '}':
                        getch();
                        lastst = end;
                        break;
                case '?':
                        getch();
                        lastst = hook;
                        break;
                default:
												if (iscommentchar(lastch)) {
													do {
														getch();
													} while (!iscommentchar(lastch) && lastch != '\n');
												}
												else
                        	generror(ERR_ILLCHAR,lastch,0);
                        getch();
												return 1;
                }
	return 0;
}
/*
 *      getsym - get next symbol from input stream.
 *
 *      getsym is the basic lexical analyzer.  It builds
 *      basic tokens out of the characters on the input
 *      stream and sets the following global variables:
 *
 *      lastch:         A look behind buffer.
 *      lastst:         type of last symbol read.
 *      laststr:        last string constant read.
 *      lastid:         last identifier read.
 *      ival:           last integer constant read.
 *      rval:           last real constant read.
 *
 *      getsym should be called for all your input needs...
 */
void     getsym(void)
{

				if (backupchar != -1) {
					lastst = backupchar;
					backupchar = -1;
					return;
				}
				if (cantnewline && !*lptr) {
					lastst = eol;
					return;
				}
        while(iswhitespacechar(lastch)) {
          getch();
					if (cantnewline && !*lptr) {
						lastst = eol;
						return;
					}
				}
        if( lastch == -1)
                lastst = eof;
        else if(isdigit(lastch))
                getnum();
        else if(isstartchar(lastch)) {
                getid();
                searchkw();
				}
        else getsym2();
}
/*
 * when we need specific punctuation, call one of these routines
 */
int needpunc(enum e_sym p, int *skimlist)
{       if( lastst == p) {
                getsym();
								return(TRUE);
				}
        else
                expecttoken(p,skimlist);
	return(FALSE);
}
int needpuncexp(enum e_sym p, int *skimlist)
{       if( lastst == p) {
                getsym();
								return(TRUE);
				}
        else
                expecttokenexp(p,skimlist);
	return(FALSE);
}
/*
 * having to back up a character is rare, but sometimes...
 */
void backup(int st)
{
  backupchar = lastst;
  lastst = st;
}