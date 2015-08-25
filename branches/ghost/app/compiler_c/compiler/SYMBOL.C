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
/* Handles symbol tables 
 */
#include        <stdio.h>
#include				<malloc.h>
#include        "expr.h"
#include        "c.h"
#include 				"errors.h"

#define ROTR(x,bits) (((x << (16 - bits)) | (x >> bits)) & 0xffff)
#define ROTL(x,bits) (((x << bits) | (x >> (16 - bits))) & 0xffff)
#define HASHTABLESIZE 1023

extern TABLE oldlsym;
extern int prm_cplusplus,prm_cmangle;
HASHREC **globalhash=0;
HASHREC **defhash=0;

TABLE gsyms,lsyms,defsyms;

void symini(void)
{
	gsyms.head = gsyms.tail = lsyms.head = lsyms.tail = defsyms.head = defsyms.tail = 0;
	if (!globalhash) {
  	globalhash = (HASHREC *)malloc(HASHTABLESIZE * sizeof(HASHREC *));
  	defhash = (HASHREC *)malloc(HASHTABLESIZE * sizeof(HASHREC *));
	}
  memset(globalhash,0,HASHTABLESIZE * sizeof(HASHREC *));
  memset(defhash,0,HASHTABLESIZE * sizeof(HASHREC *));
}

/* Sym tab hash function */	
static unsigned int ComputeHash(char *string,int size)
{
  unsigned int len = strlen(string), rv;
  char *pe = len + string;
  unsigned char blank = ' ';

  rv = len | blank;
  while(len--) {
    unsigned char cback = (unsigned char)(*--pe) | blank;
    rv = ROTR(rv,2) ^ cback;
  }
  return(rv % size);
}
/* Add a hash item to the table */
HASHREC *AddHash(HASHREC *item,HASHREC **table,int size)
{
  int index = ComputeHash(item->key,size);
  HASHREC **p;

  item->link = 0;

  if (*(p = &table[index])) {
    HASHREC *q = *p,*r = *p;
    while (q) {
			r = q;
      if (!strcmp(r->key,item->key))
				return(r);
			q = q->link;
		}
		r->link = item;
  }
  else
    *p = item;
  return(0);
}
/*
 * Find something in the hash table
 */
HASHREC **LookupHash(char *key, HASHREC **table, int size)
{
  int index = ComputeHash(key,size);
  HASHREC **p;

  if (*(p = &table[index])) {
    HASHREC *q= *p;
    while (q) {
      if (!strcmp(q->key, key))
				return(p);
			p = *p;
			q=q->link;
		}
	}
	return(0);
}
/*
 * Some tables use hash tables and some use linked lists
 * This is the global symbol search routine
 */
SYM     *search(char *na,TABLE *table)
{
	SYM *thead = table->head;
	SYM **p;
	if (table == &gsyms) {
		p=((SYM **)LookupHash(na,globalhash,HASHTABLESIZE));
		if (p)
			p = *p;
		return (SYM *) p;
	}
	else if (table == &defsyms) {
		p=((SYM **)LookupHash(na,defhash,HASHTABLESIZE));
		if (p)
			p = *p;
		return (SYM *) p;
	}
	else
	       while( thead != 0) {
                if(strcmp(thead->name,na) == 0)
                        return thead;
                thead = thead->next;
                }
        return 0;
}

SYM     *gsearch(char *na)
{       SYM     *sp;
        if( (sp = search(na,&lsyms)) == 0)
                sp = search(na,&gsyms);
        return sp;
}
/* The global symbol insert routine */
void insert(SYM *sp,TABLE *table)

{
	if (table == &gsyms) {
		if (AddHash(sp,globalhash,HASHTABLESIZE))
			gensymerror(ERR_DUPSYM,sp->name);
	}
  else if (table == &defsyms) {
		AddHash(sp,defhash,HASHTABLESIZE);
	}
	else if (table == &lsyms) {
		SYM *thead = table->head,*qhead = 0;
		/* Only check the current local block... */
	  while( thead != oldlsym.head) {
        if(strcmp(thead->name,sp->name) == 0) {
              qhead = thead;
					 		break;
					}	
        thead = thead->next;
        }
		if (qhead) 
       gensymerror(ERR_DUPSYM,sp->name);
		else {
		/* Putting local symbols in backwards */
      if( table->head == 0) {
        table->head = table->tail = sp;
      	sp->next = 0;
			}
      else    {
				sp->next = table->head;
				table->head = sp;
      }
		}
	}
	else if( search(sp->name,table) == 0) {
                if( table->head == 0)
                        table->head = table->tail = sp;
                else    {
                        table->tail->next = sp;
                        table->tail = sp;
                        }
                sp->next = 0;
                }
        else
                gensymerror(ERR_DUPSYM,sp->name);
}
