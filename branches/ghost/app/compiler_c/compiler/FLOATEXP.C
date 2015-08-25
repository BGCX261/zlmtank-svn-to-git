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
 * this evaluates an expression that may have floating point values in it
 * e.g. the initializer for a module-scoped float or double
 */
#include        <stdio.h>
#include        "expr.h"
#include        "c.h"
#include				"errors.h"

extern long ival;
extern long double rval;
extern enum e_sym lastst;
extern char lastid[];
extern TABLE defsyms;
extern int prm_cmangle;

long double floatexpr(void);

/* Primary for constant floats
 *   id
 *   iconst
 *   rconst
 *   defined(MACRO)
 *   (cast) floatexpr
 */
static long double feprimary(void)   
{       long double     temp=0;
        SYM     *sp;
        if(lastst == id) {
								char *lid = lastid;
								if (prm_cmangle)
									lid++;
                sp = gsearch(lastid);
                if(sp == NULL) {
											gensymerror(ERR_UNDEFINED,lastid);
                       getsym();
                       return 0;
                       }
                if(sp->storage_class != sc_const && sp->tp->type != bt_enum) {
                       generror(ERR_NEEDCONST,0,0);
                       getsym();
                       return 0;
                       }
                getsym();
                return sp->value.i;
        }
        else if(lastst == iconst) {
                temp = ival;
                getsym();
                return temp;
                }
        else if(lastst == lconst) {
                temp = ival;
                getsym();
                return temp;
                }
        else if(lastst == iuconst) {
                temp = (unsigned)ival;
                getsym();
                return temp;
                }
        else if(lastst == luconst) {
                temp = (unsigned)ival;
                getsym();
                return temp;
                }
				else if (lastst = rconst || lastst == lrconst || lastst == fconst) {
								temp = rval;
								getsym();
								return temp;
				}
				else if (lastst == openpa) {
					getsym();
					if (castbegin(lastst)) {
						decl(0);
						decl1();
						needpunc(closepa,0);
						return floatexpr();
					}
					else {
				  	temp = floatexpr();
						needpunc(closepa,0);
						return(temp);
					}
				}
        getsym();
        generror(ERR_NEEDCONST,0,0);
        return 0;
}
/* Unary for floating const
 *    -unary
 *    !unary
 *    primary
 */
static long double feunary(void)
{
	long double temp;
	switch (lastst) {
		case minus:
				getsym();
				temp = -feunary();
				break;
		case  en_not:
				getsym();
				temp = !feunary();
				break;
		default:
				temp = feprimary();
				break;
	}
	return(temp);
}
/* Multiply ops */
static long double femultops(void)
{
	long double val1 = feunary(),val2;
	while (lastst == star || lastst == divide) {
		int oper = lastst;
		getsym();
		val2 = feunary();
		switch(oper) {
			case star:
					val1 = val1 * val2;
					break;
			case divide:
					val1 = val1 / val2;
					break;
		}
	}
	return(val1);
}
/* Add ops */
static long double feaddops(void)
{
	long double val1 = femultops(),val2;
	while (lastst == plus || lastst == minus)	{
		int oper = lastst;
		getsym();
		val2 = femultops();
		if (oper == plus) 
			val1 = val1 + val2;
		else
			val1 = val1 - val2;
	}
	return(val1);
}
/* Floating point constant expression */
long double floatexpr(void)
{
	return(feaddops());
}
