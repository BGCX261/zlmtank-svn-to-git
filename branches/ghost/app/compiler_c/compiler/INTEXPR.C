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
 * Evaluate an expression which should be known at compile time.
 * This uses recursive descent.  It is roughly analogous to the
 * primary expression handler except it returns a value rather than
 * an enode list
 */
#include        <stdio.h>
#include        "expr.h"
#include        "c.h"
#include				"errors.h"

extern enum e_sym lastst;
extern char lastid[];
extern long ival;
extern TABLE defsyms;
extern TYP stdint, stdchar,stduns, stdunsigned, stdlong, *head;
extern int prm_cmangle;
static long ieprimary(TYP **tp)   
/*
 * PRimary integer
 *    defined(MACRO)
 *    id
 *    iconst
 *    (cast )intexpr
 *    (intexpr)
 */
{       long     temp=0;
        SYM     *sp;
				if (tp)
					*tp = &stdint;
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
								if (tp)
									*tp = &stdlong;
                temp = ival;
                getsym();
                return temp;
                }
        else if(lastst == iuconst) {
								if (tp)
									*tp = &stduns;
                temp = ival;
                getsym();
                return temp;
                }
        else if(lastst == luconst) {
								if (tp)
									*tp = &stdunsigned;
                temp = ival;
                getsym();
                return temp;
                }
        else if(lastst == cconst) {
								if (tp)
									*tp = &stdchar;
                temp = ival;
                getsym();
                return temp;
                }
				else if (lastst == openpa) {
					getsym();
					if (castbegin(lastst)) {
						decl(0);
						decl1();
						needpunc(closepa,0);
						if (tp)
						  *tp = head;
						return intexpr(0);
					}
					else {
				  	temp = intexpr(tp);
						return(temp);
					}
				}
        getsym();
        generror(ERR_NEEDCONST,0,0);
        return 0;
}
/*
 * Integer unary
 *   - unary
 *   ! unary
 *   ~unary
 *   primary
 */
static long ieunary(TYP **tp)
{
	long temp;
	switch (lastst) {
		case minus:
				getsym();
				temp = -ieunary(tp);
				break;
		case not:
				getsym();
				temp = !ieunary(tp);
				break;
		case compl:
				getsym();
				temp = ~ieunary(tp);
				break;
		default:
				temp = ieprimary(tp);
				break;
	}
	return(temp);
}
static long iemultops(TYP **tp)
/* Multiply ops */
{
	long val1 = ieunary(tp),val2;
	while (lastst == star || lastst == divide || lastst == modop) {
		TYP *tp1;
		long oper = lastst;
		getsym();
		val2 = ieunary(&tp1);
		switch(oper) {
			case star:
					val1 = val1 * val2;
					break;
			case divide:
					val1 = val1 / val2;
					break;
			case modop:
					val1 = val1 % val2;
					break;
		}
		if (tp)
			*tp = maxsize(*tp,tp1);
	}
	return(val1);
}
static long ieaddops(TYP **tp)
/* Add ops */
{
	long val1 = iemultops(tp),val2;
	while (lastst == plus || lastst == minus)	{
		long oper = lastst;
		TYP *tp1;
		getsym();
		val2 = iemultops(&tp1);
		if (oper == plus) 
			val1 = val1 + val2;
		else
			val1 = val1 - val2;
		if (tp)
			*tp = maxsize(*tp,tp1);
	}
	return(val1);
}
static long ieshiftops(TYP **tp)
/* Shift ops */
{
	long val1 = ieaddops(tp), val2;
	while (lastst == lshift || lastst == rshift) {
		long oper = lastst;
		TYP *tp1;
		getsym();
		val2 = ieaddops(&tp1);
		if (oper == lshift)
			val1 <<= val2;
		else
			val1 >>= val2;
		if (tp)
			*tp = maxsize(*tp,tp1);
	}
	return(val1);
}
static long ierelation(TYP **tp)
/* non-eq relations */
{
	long val1 = ieshiftops(tp), val2;
	while (lastst == lt || lastst == gt || lastst == leq || lastst == geq) {
		long oper = lastst;
		TYP *tp1;
		getsym();
		val2 = ieshiftops(&tp1);
		switch(oper) {
			case lt:
					val1 = val1 < val2;
					break;
			case gt:
					val1 = val1 > val2;
					break;
			case leq:
					val1 = val1 <= val2;
					break;
			case geq:
					val1 = val1 >= val2;
					break;
		}
		if (tp)
			*tp = maxsize(*tp,tp1);
	}
	return(val1);
}
static long ieequalops(TYP **tp)
/* eq relations */
{
	long val1 = ierelation(tp),val2;
	while (lastst == eq || lastst == neq) {
		long oper = lastst;
		TYP *tp1;
		getsym();
		val2 = ierelation(&tp1);
		if (oper == neq)
			val1 = val1 != val2;
		else
			val1 = val1 == val2;
		if (tp)
			*tp = maxsize(*tp,tp1);
	}
	return(val1);
}
static long ieandop(TYP **tp)
/* and op */
{
	long val1 = ieequalops(tp),val2;
	while (lastst == and) {
		TYP *tp1;
		getsym();
		val2 = ieequalops(&tp1);
		val1 = val1 & val2;
		if (tp)
			*tp = maxsize(*tp,tp1);
	}
	return(val1);
}
static long iexorop(TYP **tp)
/* xor op */
{
	long val1 = ieandop(tp),val2;
	while (lastst == uparrow) {
		TYP *tp1;
		getsym();
		val2 = ieandop(&tp1);
		val1 = val1 ^ val2;
		if (tp)
			*tp = maxsize(*tp,tp1);
	}         
	return(val1);
}
static long ieorop(TYP **tp)
/* or op */
{
	long val1 = iexorop(tp),val2;
	while (lastst == or) {
		TYP *tp1;
		getsym();
		val2 = iexorop(&tp1);
		val1 = val1 | val2;
		if (tp)
			*tp = maxsize(*tp,tp1);
	}
	return(val1);
}
static long ielandop(TYP **tp)
/* logical and op */
{
	long val1 = ieorop(tp),val2;
	while (lastst == land) {
		TYP *tp1;
		getsym();
		val2 = ieorop(&tp1);
		val1 = val1 && val2;
		if (tp)
			*tp = maxsize(*tp,tp1);
	}
	return(val1);
}
static long ielorop(TYP **tp)
/* logical or op */
{
	long val1 = ielandop(tp),val2;
	while (lastst == lor) {
		TYP *tp1;
		getsym();
		val2 = ielandop(&tp1);
		val1 = val1 || val2;
		if (tp)
			*tp = maxsize(*tp,tp1);
	}
	return(val1);
}
static long iecondop(TYP **tp)
/* Hook op */
{
	long val1 = ielorop(tp),val2, val3;
		if (lastst == hook) {
			TYP *tp1, *tp2;
			getsym();
			val2 = iecondop(&tp1);
			needpunc(colon,0);
			val3 = iecondop(&tp2);
			if (val1)
				val1 = val2;
			else
				val1 = val3;
		if (tp)
			*tp = maxsize(tp2,tp1);
		}
	return(val1);
}
long intexpr(TYP **tp)
/* Integer expressions */
{
	return(iecondop(tp));
}
