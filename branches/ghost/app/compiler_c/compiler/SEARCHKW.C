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
 * keyword module
 */
#include        <stdio.h>
#include				<malloc.h>
#include        "expr.h"
#include        "c.h"

#define KWHASHSIZE 253

extern int asmline;
extern ASMNAME oplst[];
extern ASMREG reglst[];
extern char lastid[];
extern enum e_sym lastst;
extern int prm_cplusplus,prm_cmangle;
extern KEYWORDS prockeywords[];
extern ASMNAME *keyimage;

static HASHREC **hashtable=0;
	
KEYWORDS keywords[] = {
 
        {0,"int", kw_int}, {0,"char", kw_char}, {0,"long", kw_long},
        {0,"float", kw_float}, {0,"double", kw_double}, {0,"return", kw_return},
        {0,"struct", kw_struct}, {0,"union", kw_union}, {0,"typedef", kw_typedef},
        {0,"enum", kw_enum}, {0,"static", kw_static}, {0,"auto", kw_auto},
        {0,"sizeof", kw_sizeof}, {0,"do", kw_do}, {0,"if", kw_if},
        {0,"else", kw_else}, {0,"for", kw_for},{0,"switch", kw_switch},
        {0,"while", kw_while},{0,"short", kw_short}, {0,"extern", kw_extern},
        {0,"case", kw_case}, {0,"goto", kw_goto}, {0,"default", kw_default},
        {0,"register", kw_register}, {0,"unsigned", kw_unsigned},
	{0,"signed", kw_signed },
        {0,"break", kw_break}, {0,"continue", kw_continue}, {0,"void", kw_void},
				{0,"volatile", kw_volatile}, {0,"const", kw_const},
#ifdef CPLUSPLUS
				{0,"public",kw_public,1}, {0,"private",kw_private,1},
				{0,"protected",kw_protected,1}, { 0, "class", kw_class,1 },
				{0,"friend",kw_friend,1}, {0,"this",kw_this,1}, {0,"operator",kw_operator,1},
				{0,"new",kw_new,1}, {0,"delete",kw_delete,1},{0,"inline",kw_inline,1},
				{0,"try",kw_try,1}, {0,"catch",kw_catch,1}, {0,"template",kw_template,1},
				{0,"throw",kw_throw,1},
#endif
#ifdef INLINEASM
				{0,"asm",kw_asm,0},
#endif
        {0, 0, 0} };

void kwini(void) 
/*
 * create a keyword hash table
 */
{
	struct kwblk *q = keywords;
	ASMNAME *r;
	ASMREG *s;
	if (!hashtable) {
  	hashtable = (HASHREC *)malloc(KWHASHSIZE * sizeof(HASHREC *));
  	memset(hashtable,0,KWHASHSIZE * sizeof(HASHREC *));
		while (q->word) {
			if (prm_cplusplus || !q->flags)
				AddHash(q,hashtable,KWHASHSIZE);
			q++;
		}
		q = prockeywords;
		while (q->word) {
			if (prm_cplusplus || !q->flags)
				AddHash(q,hashtable,KWHASHSIZE);
			q++;
		}
#ifdef INLINEASM
		r = oplst;
		while (r->word) {
			if (r->atype) {
				q = malloc(sizeof(struct kwblk));
				q->next=0;
				q->word=r->word;
				q->stype = kw_asminst;
				q->flags=KW_INLINEASM;
				q->data = r;
				AddHash(q,hashtable,KWHASHSIZE);
			}
			r++;
		}
		s = reglst;
		while (s->word) {
			if (s->regtype ) {
				q = malloc(sizeof(struct kwblk));
				q->next=0;
				q->word=s->word;
				q->stype = kw_asmreg;
				q->flags=KW_INLINEASM;
				q->data = s;
				AddHash(q,hashtable,KWHASHSIZE);
			}
			s++;
		}
#endif
	}
}
int searchkw(void)
/*
 * see if the current symbol is a keyword
 */
{
	char *ident = lastid;
	struct kwblk    **kwbp;
	if (lastst != id)
		return 0;
	if (prm_cmangle)
		ident++;
	kwbp = LookupHash(ident,hashtable,KWHASHSIZE);
	
	if (kwbp) {
#ifdef INLINEASM
		if (((*kwbp)->flags==KW_INLINEASM) && !asmline)
			return 0;
		keyimage = (*kwbp)->data;
#endif 
		return lastst = (*kwbp)->stype;
	}
  return(0);
}