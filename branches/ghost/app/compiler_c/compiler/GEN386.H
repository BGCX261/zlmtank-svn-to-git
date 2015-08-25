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
#define F_INDX  32	/* indexed memory alterable mode allowed */
#define F_ALL   31      /* all modes allowed */
#define F_VOL   64      /* need volitile operand */
#define F_NOVALUE 128    /* dont need result value */
#define F_NOBIT 256	/* Don't get the bit val, get the address */
#define F_DEST 512	/* Is going to be used as a destination */

#define OPE_MATH 1
#define OPE_ARPL 2
#define OPE_BOUND 3
#define OPE_BITSCAN 4
#define OPE_BIT 5
#define OPE_CALL 6
#define OPE_INCDEC 7
#define OPE_RM 8
#define OPE_ENTER 9
#define OPE_IMUL 10
#define OPE_IN 11
#define OPE_IMM8 12
#define OPE_RELBRA 13
#define OPE_RELBR8 14
#define OPE_JMP 15
#define OPE_REGRM 16
#define OPE_LOADSEG 17
#define OPE_LGDT 18
#define OPE_LIDT 19
#define OPE_RM16 20
#define OPE_MOV 21
#define OPE_MOVSX 22
#define OPE_OUT 23
#define OPE_PUSHPOP 24
#define OPE_SHIFT 25
#define OPE_RET 26
#define OPE_SET 27
#define OPE_SHLD 28
#define OPE_TEST 29
#define OPE_XCHG 30
#define OPE_FMATH 31
#define OPE_FMATHP 32
#define OPE_FMATHI 33
#define OPE_FCOM 34
#define OPE_FREG 35
#define OPE_FICOM 36
#define OPE_FILD 37
#define OPE_FIST 38
#define OPE_FLD 39
#define OPE_FST 40
#define OPE_FSTP 41
#define OPE_FUCOM 42
#define OPE_FXCH 43
#define OPE_MN 44
#define OPE_M16 45
#define OPE_CMPS 46
#define OPE_INS 47
#define OPE_LODS 48
#define OPE_MOVS 49
#define OPE_OUTS 50
#define OPE_SCAS 51
#define OPE_STOS 52
#define OPE_XLAT 53
#define OPE_REG32 54

enum e_op { op_reserved, op_line, op_label, op_funclabel, op_seq,
	op_genword,op_dd,
	op_aaa,op_aad,op_aam,op_aas,op_add,op_adc,
	op_and,op_arpl,op_bound,op_bsf,op_bsr,op_bswap,op_btc,
	op_bt,op_bts,op_call,op_cbw,op_cwde,op_cwd,
	op_cdq,op_clc,op_cld,op_cli,op_clts,op_cmc,
	op_cmp,op_cmps,op_cmpsb,op_cmpsw,op_cmpsd,op_daa,
	op_das,op_dec,op_div,op_enter,op_hlt,op_idiv,
	op_imul,op_in,op_inc,op_ins,op_insb,op_insw,
	op_insd,op_int,op_into,op_invd,op_iret,op_iretd,op_ja,
	op_jnbe,op_jae,op_jnb,op_jnc,op_jb,op_jc,
	op_jnae,op_jbe,op_jna,op_jecxz,op_je,op_jg,
	op_jnle,op_jl,op_jnge,op_jge,op_jnl,op_jle,
	op_jng,op_jne,op_jo,op_jno,op_jp,op_jpe,
	op_jpo,op_js,op_jns,op_jmp,op_lahf,op_lar,
	op_lds,op_les,op_lfs,op_lgs,op_lss,op_lea,
	op_leave,op_lgdt,op_lidt,op_lldt,op_lmsw,op_lock,
	op_lods,op_lodsb,op_lodsw,op_lodsd,op_loop,op_loope,
	op_loopz,op_loopne,op_loopnz,op_lsl,op_ltr,op_mov,
	op_movs,op_movsb,op_movsw,op_movsd,op_movsx,op_movzx,
	op_mul,op_neg,op_not,op_nop,op_or,op_out,
	op_outs,op_outsb,op_outsw,op_outsd,op_pop,op_popa,op_popad,
	op_popf,op_popfd,op_push,op_pusha,op_pushad,op_pushf,op_pushfd,op_rcl,op_rcr,
	op_rol,op_ror,op_rep,op_repne, op_repe, op_repnz, 
	op_repz,op_ret,op_sahf,op_sal,op_sar,op_shl,
	op_shr,op_sbb,op_scas,op_scasb,op_scasw,op_scasd,
	op_seta,op_setnbe,op_setae,op_setnb,op_setnc,op_setb,
	op_setc,op_setnae,op_setbe,op_setna,op_sete,op_setg,
	op_setnle,op_setl,op_setnge,op_setge,op_setnl,op_setle,
	op_setng,op_setne,op_seto,op_setno,op_setp,op_setpe,
	op_setpo,op_sets,op_setns,op_sgdt,op_sidt,op_sldt,
	op_smsw,op_shld,op_shrd,op_stc,op_std,op_sti,
	op_stos,op_stosb,op_stosw,op_stosd,op_str,op_sub,op_test,
	op_verr,op_verw,op_wait,op_wbinvd,op_xchg,
	op_xlat,op_xlatb,op_xor,op_f2xm1,op_fabs,op_fadd,op_faddp,
	op_fiadd,op_fchs,op_fclex,op_fnclex,op_fcom,op_fcomp,
	op_fcompp,op_fcos,op_fdecstp,op_fdiv,op_fdivp,op_fidiv,
	op_fdivr,op_fdivrp,op_fidivr,op_ffre,op_ficom,op_ficomp,
	op_fild,op_fincstp,op_finit,op_fninit,op_fist,op_fistp,
	op_fld,op_fldz,op_fldpi,op_fld1,op_fld2t,op_fld2e,op_fldlg2,
	op_fldln2,op_fldcw,op_fldsw,op_fldenv,op_fmul,op_fmulp,op_fimul,
	op_fpatan,op_fprem,op_fprem1,op_fptan,op_frndint, op_frstor,op_fsave,
	op_fnsave,op_fscale,op_fsin,op_fsincos,op_fsqrt,op_fst,
	op_fstp,op_fstcw,op_fstsw,op_fnstcw, op_fnstsw,
	op_fstenv,op_fsntenv,op_fsub,
	op_fsubp,op_fisub,op_fsubr,op_fsubrp,op_fisubr,op_ftst,
	op_fucom,op_fucomp,op_fucompp,op_fwait,op_fxam,op_fxch,op_fxtract,
	op_fyl2x,op_fyl2xp1
	};
enum e_asmw {
	akw_byte, akw_word, akw_dword, akw_fword, akw_qword, akw_tbyte,
	akw_offset, akw_ptr };

enum e_am {
        am_none, am_dreg, am_freg, am_screg, am_sdreg, am_streg, am_seg,
	am_indisp, am_indispscale,
        am_direct, am_immed, am_ext };

/*      addressing mode structure       */

struct amode {
        enum e_am       mode;
        char            preg;
        char            sreg;
        char            tempflag;
	char		scale;
	char 		length;
	enum { e_default, e_cs,e_ds,e_es,e_fs,e_gs, e_ss } seg;
        struct enode    *offset;
        };

/*      output code structure   */

struct ocode {
        struct ocode    *fwd, *back;
        enum e_op       opcode;
        struct amode    *oper1, *oper2,*oper3;
	int addr;
	int size;
	char diag;
	char noopt;
        };

/* Used for fixup gen */
typedef struct dl {
		struct dl *next;
		char *string;
		int offset;
		short type;
} DATALINK;

#define AMODE struct amode
#define OCODE struct ocode

#define FLOAT printf("codegen-Floating point not implemented in push/pop\n");
/* 386 register set */
#define EAX 0
#define ECX 1
#define EDX 2
#define EBX 3
#define ESP 4
#define EBP 5
#define ESI 6
#define EDI 7
#define AL 0
#define CL 1
#define DL 2
#define BL 3
#define ES 0
#define CS 1
#define SS 2
#define DS 3


#include "cc386.p"