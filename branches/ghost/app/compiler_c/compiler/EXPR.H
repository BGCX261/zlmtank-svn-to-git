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

/*      expression tree descriptions    */

enum e_node {
        en_void,        /* used for parameter lists */
	en_cb, en_cub, en_cw, en_cuw, en_cl, en_cul, en_cf, en_cd, en_cp, en_cld,
        en_icon, en_lcon,en_iucon, en_lucon, en_rcon, en_fcon, en_lrcon,
	en_ccon, en_labcon, en_nacon, en_autocon, en_absacon, en_nalabcon,
        en_napccon, en_b_ref, en_w_ref, en_l_ref, en_ub_ref, en_uw_ref,
	en_floatref, en_doubleref, en_longdoubleref, en_autoreg, en_trapcall,
        en_ul_ref, en_fcall, en_fcallb, en_intcall, en_tempref, en_regref,
	en_pfcallb, en_pfcall, en_pcallblock,
	en_add, en_sub, en_mul, en_mod,
        en_div, en_lsh, en_rsh, en_cond, en_assign, en_refassign, en_eq, en_ne,
        en_asadd, en_assub, en_asmul, en_asdiv, en_asmod, en_asrsh,
	en_asumul, en_asudiv, en_asumod, en_pmul,
        en_aslsh, en_asand, en_asor, en_asxor, en_uminus, en_not, en_compl,
        en_lt, en_le, en_gt, en_ge, en_and, en_or, en_land, en_lor,
        en_xor, en_ainc, en_adec, en_umul, en_udiv, en_umod, en_ugt,
        en_uge, en_ule, en_ult, en_moveblock, en_stackblock, en_callblock,
	en_pdiv, en_alsh, en_arsh, en_asarsh,en_asalsh, en_bits};

/*      statement node descriptions     */

enum e_stmt {
        st_line, st_expr, st_while, st_for, st_do, st_if, st_switch,
        st_case, st_goto, st_break, st_continue, st_label, st_asm,
        st_return, st_block, st__genword };

struct enode {
        enum e_node	nodetype;
	char bits;
	char startbit;
	char cflags;
	long size; /* For block moves */
        union {
                long            i;
                double          f;
                char            *sp;
                struct enode    *p[2];
                } v;
        };

struct snode {
        enum e_stmt     stype;
        struct snode    *next;          /* next statement */
        struct enode    *exp;           /* condition or expression */
        struct snode    *s1, *s2,*lst;       /* internal statements &lineno*/
	/* changed */
	struct snode          *label;         /* label number for goto */
        };

struct cse {
        struct cse      *next;
        struct enode    *exp;           /* optimizable expression */
        short           uses;           /* number of uses */
        short            duses;          /* number of dereferenced uses */
	char		size;		/* Size of the expresion */
        char             voidf;          /* cannot optimize flag */
        char             reg;            /* allocated register */
        };

#define ENODE struct enode
#define SNODE struct snode
#define CSE struct cse