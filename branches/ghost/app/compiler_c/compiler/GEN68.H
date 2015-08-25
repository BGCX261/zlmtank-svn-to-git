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
/*
 *      code generation structures and constants
 */

/* address mode specifications */
#define F_DREG  1       /* data register direct mode allowed */
#define F_AREG  2       /* address register direct mode allowed */
#define F_MEM   4       /* memory alterable modes allowed */
#define F_IMMED 8       /* immediate mode allowed */
#define F_ALT   7       /* alterable modes */
#define F_DALT  5       /* data alterable modes */
#define F_FREG  16  	/* FP register */
#define F_INDX  64	/* indexed memory alterable mode allowed */
#define F_ALL   63      /* all modes allowed */
#define F_VOL   128      /* need volitile operand */
#define F_NOVALUE 256    /* dont need result value */
#define F_NOBIT 512	/* Don't get the bit val, get the address */

enum e_op {
        op_reserved, op_line, op_seq, op_slit, op_label, op_funclabel, op_genword, 
	op_dcl,op_dcr,
	op_abcd,op_add,op_adda,op_addi,
	op_addq,op_addx,op_and,op_andi,op_asl,op_asr,
	op_bra,op_beq,op_bne,op_blt,op_ble,op_bgt,op_bge,
	op_bhi,op_bhs,op_blo,op_bls,op_bsr,op_bcc,
	op_bcs,op_bmi,op_bpl,op_bvc,op_bvs,op_bchg,
	op_bclr,op_bfchg,op_bfclr,op_bfexts,op_bfextu,op_bfffo,
	op_bfins,op_bfset,op_bftst,op_bkpt,op_bset,op_btst,
	op_chk,op_chk2,op_clr,op_cmp,op_cmpa,op_cmpi,
	op_cmpm,op_cmp2,op_dbeq,op_dbne,op_dblt,op_dble,op_dbgt,
	op_dbge,op_dbhi,op_dbhs,op_dblo,op_dbls,op_dbsr,
	op_dbcc,op_dbcs,op_dbmi,op_dbpl,op_dbvc,op_dbvs,
	op_dbt,op_dbf,op_dbra,op_divs,op_divu,op_divsl,
	op_divul,op_eor,op_eori,op_exg,op_ext,op_extb,
	op_illegal,op_jmp,op_jsr,op_lea,op_link,op_lsl,
	op_lsr,op_move,op_movea,op_movec,op_movem,op_movep,
	op_moveq,op_moves,op_muls,op_mulu,op_nbcd,op_neg,
	op_negx,op_nop,op_not,op_or,op_ori,op_pack,
	op_pea,op_reset,op_rol,op_ror,op_roxl,op_roxr,
	op_rtd,op_rte,op_rtr,op_rts,op_sbcd,op_seteq,
	op_setne,op_setlt,op_setle,op_setgt,op_setge,op_sethi,op_seths,
	op_setlo,op_setls,op_setsr,op_setcc,op_setcs,op_setmi,
	op_setpl,op_setvc,op_setvs,op_sett,op_setf,op_sub,
	op_stop,op_suba,op_subi,op_subq,op_subx,op_swap,
	op_tas,op_trap,op_trapeq,op_trapne,op_traplt,op_traple,op_trapgt,
	op_trapge,op_traphi,op_traphs,op_traplo,op_trapls,op_trapsr,
	op_trapcc,op_trapcs,op_trapmi,op_trappl,op_trapvc,op_trapvs,
	op_trapt,op_trapf,op_trapv,op_tst,op_unlk,op_unpk,
	op_fabs,op_facos,op_fadd,op_fasin,op_fatan,op_fatanh,
	op_fbeq,op_fbne,op_fbgt,op_fbngt,op_fbge,op_fbnge,
	op_fblt,op_fbnlt,op_fble,op_fbnle,op_fbgl,op_fbngl,
	op_fbgle,op_fbngle,op_fbogt,op_fbule,op_fboge,op_fbult,
	op_fbolt,op_fbuge,op_fbole,op_fbugt,op_fbogl,op_fbueq,
	op_fbor,op_fbun,op_fbt,op_fbf,op_fbst,op_fbsf,
	op_fbseq,op_fbsne,op_fcmp,op_fcos,op_fcosh,op_fdbeq,
	op_fdbne,op_fdbgt,op_fdbngt,op_fdbge,op_fdbnge,op_fdblt,
	op_fdbnlt,op_fdble,op_fdbnle,op_fdbgl,op_fdbngl,op_fdbgle,
	op_fdbngle,op_fdbogt,op_fdbule,op_fdboge,op_fdbult,op_fdbolt,
	op_fdbuge,op_fdbole,op_fdbugt,op_fdbogl,op_fdbueq,op_fdbor,
	op_fdbun,op_fdbt,op_fdbf,op_fdbst,op_fdbsf,op_fdbseq,
	op_fdbsne,op_fdiv,op_fetox,op_fetoxm1,op_fgetexp,op_fgetman,
	op_fint,op_fintrz,op_flog10,op_flog2,op_flogn,op_flognp1,
	op_fmod,op_fmove,op_fmovecr,op_fmovem,op_fmul,op_fneg,
	op_fnop,op_frem,op_fscale,op_fseq,op_fsne,op_fsgt,
	op_fsngt,op_fsge,op_fsnge,op_fslt,op_fsnlt,op_fsle,
	op_fsnle,op_fsgl,op_fsngl,op_fsgle,op_fsngle,op_fsogt,
	op_fsule,op_fsoge,op_fsult,op_fsolt,op_fsuge,op_fsole,
	op_fsugt,op_fsogl,op_fsueq,op_fsor,op_fsun,op_fst,
	op_fsf,op_fsst,op_fssf,op_fsseq,op_fssne,op_fsgldiv,
	op_fsglmul,op_fsin,op_fsincos,op_fsinh,op_fsqrt,op_fsub,
	op_ftan,op_ftanh,op_ftentox,op_ftrapeq,op_ftrapne,op_ftrapgt,
	op_ftrapngt,op_ftrapge,op_ftrapnge,op_ftraplt,op_ftrapnlt,op_ftraple,
	op_ftrapnle,op_ftrapgl,op_ftrapngl,op_ftrapgle,op_ftrapngle,op_ftrapogt,
	op_ftrapule,op_ftrapoge,op_ftrapult,op_ftrapolt,op_ftrapuge,op_ftrapole,
	op_ftrapugt,op_ftrapogl,op_ftrapueq,op_ftrapor,op_ftrapun,op_ftrapt,
	op_ftrapf,op_ftrapst,op_ftrapsf,op_ftrapseq,op_ftrapsne,op_ftst,
	op_ftwotox
};

enum e_am {
        am_none, am_dreg, am_areg, am_freg, am_ind, am_ainc, am_adec, am_indx, am_baseindxaddr,
        am_direct, am_adirect, am_immed, am_mask, am_fmask, am_baseindxdata, am_pcindx,
	am_divsl, am_bf, am_sr };
/*      addressing mode structure       */

struct amode {
        enum e_am       mode;
        char            preg;
        char            sreg;
	char		scale;
        char            tempflag;
        struct enode    *offset;
        };

/*      output code structure   */

struct ocode {
        struct ocode    *fwd, *back;
        enum e_op       opcode;
        short           length;
        struct amode    *oper1, *oper2,*oper3;
	long addr;
	char noopt;
	char diag;
        };

/* Used for fixup gen */
typedef struct dl {
		struct dl *next;
		SYM *sp;
		int offset;
		short type;
} DATALINK;

#define AMODE struct amode
#define OCODE struct ocode


enum e_gt { nogen, bytegen, wordgen, longgen, floatgen, doublegen, longdoublegen, srrefgen };
enum e_sg { noseg, codeseg, dataseg, bssxseg,startupxseg,rundownxseg,cppxseg,fixcseg,fixdseg };


/*  structures for binary code gen: mem arenas and fixups
 */
typedef struct _arena {
	struct _mem {
		struct _arena *link;
		unsigned char *mem;
	} *m,*active;
	long size;
	long pos;
	short id;
	struct _fixup {
		struct _fixup *link;
		SYM *sp;
		long address;
		long offset;
		long label;
		char size;
		char pcrel;
		char pureofs;
	} *head,*tail;
} ARENA;

typedef struct _mem MEM;
typedef struct _fixup FIXUP;

#define MEMSIZE 1024	/* size of the smallest hunk in an arena */
#include "cc68.p"