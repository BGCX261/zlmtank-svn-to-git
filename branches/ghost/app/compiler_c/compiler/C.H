/*
 * 68K/386 32-bit C compiler.
 *
 * copyright (c) 1996, David Lindauer
 * 
 * This compiler is intended for educational use.  It may not be used
 * for profit without the express written consent of the author.
 *
 * It may be freely redistributed, as long as this notice remains intact
 * and sources are distributed along with any executables derived from them.
 *
 * The author is not responsible for damages, either direct or consequential,
 * that may arise from use of this software.
 *
 * v1.5 August 1996
 * David Lindauer, gclind01@starbase.spd.louisville.edu
 *
 * Credits to Mathew Brandt for original K&R C compiler
 *
 */

/*      compiler header file    */

#define TRUE 1
#define FALSE 0


/* declaration flags */
#define DF_INT 1
#define DF_ABS 2
#define DF_CONST 4
#define DF_VOL 8
#define DF_FUNCPARMS 16
#define DF_GLOBAL 32
#define DF_AUTOREG 64

#define UF_DEFINED 1
#define UF_USED	2
#define UF_ASSIGNED 4
#define UF_CANASSIGN 8

#define GF_ASSIGN 1
#define GF_CONTINUABLE 2
#define GF_CONTINUE 4
#define GF_GOTO 8
#define GF_DEF 16
#define GF_BREAK 32
#define GF_RETURN 64
#define GF_UNREACH 128
#define GF_NOPROTO 256
#define GF_AND 512
#define GF_SUPERAND 1024
#define GF_INLOOP 2048
#define GF_INFUNCPARMS 4096

#define PF_PRIVATE 0
#define PF_PROTECTED 1
#define PF_PUBLIC 2
#define PF_VIRTUAL 4
#define PF_PURE 8
#define PF_INLINE 16
#define PF_STATIC 32
#define PF_CLASSDEF 64
#define PF_CLASSFUNC 128
#define PF_CLASSHEAD 256

/* keywords and symbols */
enum e_sym {
        id, cconst, iconst, lconst, iuconst, luconst, sconst, lsconst,
	fconst, lrconst, rconst, plus, minus,
        star, divide, lshift, rshift, modop, eq, neq, lt, leq, gt,
        geq, assign, asplus, asminus, astimes, asdivide, asmodop,
        aslshift, asrshift, asand, asor, asxor, autoinc, autodec, hook, compl,
        comma, colon, semicolon, uparrow, openbr, closebr, begin, end,
        openpa, closepa, pointsto, dot, lor, land, not, or, and, ellipse, 
	classsel, pointstar, dotstar, kw_int,
        kw_void, kw_char, kw_float, kw_double, kw_struct, kw_union,
        kw_long, kw_short, kw_unsigned, kw_signed, kw_auto, kw_extern,
        kw_register, kw_typedef, kw_static, kw_goto, kw_return,
        kw_sizeof, kw_break, kw_continue, kw_if, kw_else, kw_for,
        kw_do, kw_while, kw_switch, kw_case, kw_default, kw_enum, kw_volatile,
	kw_const, kw_phitext, kw__trap, kw__interrupt, kw__abs, kw__genword,
	kw_public, kw_private, kw_protected, kw_class, kw_friend, kw_new,
	kw_delete, kw_this, kw_operator, kw_inline,
	kw_try,kw_throw,kw_catch, kw_template, kw__pascal, kw_asm,
	kw_asmreg,kw_asminst,
	/* These next are generic register names */	
	kw_D0,kw_D1,kw_D2,kw_D3,kw_D4,kw_D5,kw_D6,kw_D7,
	kw_D8,kw_D9,kw_DA,kw_DB,kw_DC,kw_DD,kw_DE,kw_DF,
	kw_A0,kw_A1,kw_A2,kw_A3,kw_A4,kw_A5,kw_A6,kw_A7,
	kw_A8,kw_A9,kw_AA,kw_AB,kw_AC,kw_AD,kw_AE,kw_AF,
	kw_F0,kw_F1,kw_F2,kw_F3,kw_F4,kw_F5,kw_F6,kw_F7,
	kw_F8,kw_F9,kw_FA,kw_FB,kw_FC,kw_FD,kw_FE,kw_FF,
	eol, eof };

/* storage classes */
enum e_sc {
        sc_static, sc_auto, sc_global, sc_external, sc_type, sc_defunc, sc_const,
        sc_member, sc_label, sc_ulabel, sc_argument, sc_memberreg, sc_autoreg,
	sc_externalfunc,sc_abs };

/* basic types */
enum e_bt {
	/* This first two lines MUST be ordered for the 
	 * lostconv stuff to work
	 */
        bt_char, bt_unsignedchar, bt_enum,bt_short, bt_unsignedshort, 
	bt_long, bt_unsigned, bt_float, bt_double, bt_longdouble, bt_untyped,
	bt_pointer,bt_ref,bt_void,bt_struct, bt_union, bt_func, bt_class, bt_iclass,
	bt_ifunc, bt_ptrfunc,bt_matchall, bt_ellipse, bt_bitfield };

struct slit {
        struct slit     *next;
        int             label;		/* label to use */
        char            *str;		/* string to dump */
	char		type;		/* ascii/phitext or wide */
        };

/* symbols */
struct sym {
        struct sym      *next;			/* next symbol (local tabs only) */
        char            *name;			/* symbol name */
        enum e_sc	storage_class;		/* storage class */
	int		extflag:1;		/* if external, was it used */
	int		absflag:1;		/* the _abs keyword was used */
	int		intflag:1;		/* the _interrupt keyword was used */
	int		indecltable:1;		/* global already in dump table */
	int		pascaldefn:1;		/* function: pascal type */
	int		inreg:1;  		/* function var is in reg, no stack space needed */
	int		funcparm:1;		/* function var is a param and has space allocated */
	int 		staticlabel:1;	/* Var is a static declared in a function */
/*	char		pflags;       
	struct sym 	*parent;      */          /* Parent class */
	int		linkid;			/* ID to use for linkage purposes (binary output) */
	ENODE *		defalt;			/* Default for function params */
						/* Also name for CPP overload lists */
	/* these fields depend on storage_class */
        union   {
                long            i;		/* int val */
                unsigned long   u;		/* nsigned val */
                double          f;		/* float val */
		struct stab {	
			struct sym *head, *tail; /* Overload table*/
			} overlist;
                char            *s;		/* string val */
                }
                        value;
	/* Type declarations */
        struct typ {
                enum e_bt		       type;	/* the type */
                char            val_flag;       /* set if is an array */
       		char		uflags;			/* Track usage */
		char		cflags;		/* const & vol flags */
		char 		classflags;	/* Public, protected, private */
		char		bits;		/* -1 for not a bit val, else bit field len */
		char		startbit;	/* start of bit field */	
                long            size;		/* total size of type */
		/* local symbol tables */
		struct stab lst;	 	/* Symbol table for structs & functions */
                struct typ      *btp;		/* pointer to next type (pointers & arrays */
                char            *sname;		/* structure name ? */
		struct sym *tdef;		/* Pointer to a typedef entry */
                }
                        *tp;
	struct decldata {
			struct decldata *link;
			enum { dd_byte,dd_word,dd_long,dd_float,dd_double,
			       dd_ldouble, dd_pcname, dd_dataname, dd_storage,
			       dd_label
			     } mode;
			union {
				long i;
				double f;
				struct sym *sp;
			} val;		
			long offset;
	} *init;
};

#define SYM     struct sym
#define TYP     struct typ
#define TABLE   struct stab

#define MAX_STRLEN      120
#define MAX_STLP1       121

/* struct for preprocessor if tracking */
typedef struct ifstruct {
		struct ifstruct *link;		/* link */
		short iflevel;			
		short elsetaken;
} IFSTRUCT;
		
/* #define tracking */
typedef struct {
		char *string;
		short argcount;
		char **args;
} DEFSTRUCT;

/* error list */
struct errl {
		struct errl *link;
		short errornumber;
		void *data;
};

/* used for error skimming */
#define BALANCE struct balance
#define BAL_PAREN   0
#define BAL_BRACKET 0
#define ERRORS struct errl

struct balance {
		struct balance *back;
		short type;
		short count;
};

/* Global symbol table is a hash table */
#define HASHTABLESIZE 1023

typedef struct _hashrec_ {
   struct _hashrec_ *link;	/* Link to next element in list */
   char *key;	/* Full key */
} HASHREC;

typedef struct {
	char *word;
	short atype;
	short amode;
} ASMNAME;

typedef struct kwblk {
				struct kwblk *next;
        char            *word;
        short		stype;
	enum { KW_CPLUSPLUS=1,KW_INLINEASM=2 } flags;
	ASMNAME		*data;
        } KEYWORDS;

typedef struct {
	char *word;
	char regtype;
	char regnum;
	char size;
} ASMREG;

#include "cc.p"