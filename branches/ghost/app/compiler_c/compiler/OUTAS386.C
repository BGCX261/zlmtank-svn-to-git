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
#include        <stdio.h>
#include				<ctype.h>
#include        "expr.h"
#include        "c.h"
#include        "gen386.h"
#include 				"diag.h"

int skipsize = 0;
int addsize = 0;

typedef struct muldiv {
		struct muldiv * link;
		long value;
		double floatvalue;
		int size;
		int label;
} MULDIV;
/*      variable initialization         */

char segregs[] = "CSDSESFSGSSS";

extern char outfile[];
extern int prm_flat;
extern SYM *currentfunc;
extern int prm_cmangle;
extern HASHREC **globalhash;
extern int phiused;
extern long nextlabel;
extern FILE *outputFile;
extern int global_flag;

enum e_gt { nogen, bytegen, wordgen, longgen, floatgen, doublegen, longdoublegen, storagegen,srrefgen };
enum e_sg { noseg, codeseg, dataseg,bssxseg,startupxseg,rundownxseg,cppxseg };

extern int	prm_asmfile;
extern int prm_lines,prm_nasm;

static struct slit *strtab;
static int uses_float;

MULDIV *muldivlink = 0;
enum e_gt	 gentype = nogen;		/* Current DC type */
enum e_sg	 curseg = noseg;		/* Current seg */
int        outcol = 0;				/* Curront col (roughly) */
int newlabel;
int needpointer;
static int nosize = 0;
static int phiput;

/* List of opcodes
 * This list MUST be in the same order as the op_ enums 
 */
ASMNAME oplst[] = { 
	{ "reserved", op_reserved,0 },
	{ "line#", op_reserved,0 },
	{ "label", op_reserved,0 },
	{ "flabel", op_reserved,0 },
	{ "seq@", op_reserved,0 },
	{ "db", op_reserved,0 },
	{ "dd", op_reserved,0 },
	{ "aaa", op_aaa,0 },
	{ "aad", op_aad,0 },
	{ "aam", op_aam,0 },
	{ "aas", op_aas,0 },
	{ "add", op_add,OPE_MATH },
	{ "adc", op_adc,OPE_MATH },
	{ "and", op_and,OPE_MATH },
	{ "arpl", op_arpl,OPE_ARPL },
	{ "bound", op_bound,OPE_BOUND },
	{ "bsf", op_bsf,OPE_BITSCAN },
	{ "bsr", op_bsr,OPE_BITSCAN },
	{ "bswap",op_bswap,OPE_REG32 },
	{ "btc", op_btc,OPE_BIT },
	{ "bt", op_bt,OPE_BIT },
	{ "bts", op_bts,OPE_BIT },
	{ "call", op_call,OPE_CALL },
	{ "cbw", op_cbw,0 },
	{ "cwde", op_cwde,0 },
	{ "cwd", op_cwd, 0 },
	{ "cdq", op_cdq,0 },
	{ "clc", op_clc,0 },
	{ "cld", op_cld,0 },
	{ "cli", op_cli,0 },
	{ "clts", op_clts, 0 },
	{ "cmc", op_cmc, 0 },
	{ "cmp", op_cmp, OPE_MATH },
	{ "cmps", op_cmps, OPE_CMPS },
	{ "cmpsb", op_cmpsb, 0 },
	{ "cmpsw", op_cmpsw, 0 },
	{ "cmpsd", op_cmpsd, 0 },
	{ "daa",op_daa,0 },
	{ "das", op_das,0 },
	{ "dec",op_dec, OPE_INCDEC},
	{ "div", op_div, OPE_RM},
	{ "enter", op_enter, OPE_ENTER},
	{ "hlt", op_hlt, 0 },
	{ "idiv", op_idiv, OPE_RM },
	{ "imul", op_imul, OPE_IMUL },
	{ "in", op_in, OPE_IN },
	{ "inc", op_inc, OPE_INCDEC },
	{ "ins", op_ins, OPE_INS },
	{ "insb", op_insb, 0 },
	{ "insw", op_insw, 0 },
	{ "insd", op_insd, 0 },
	{ "int" , op_int, OPE_IMM8 },
	{ "into", op_into, 0 },
	{ "invd", op_invd, 0 },
	{ "iret", op_iret, 0},
	{ "iretd", op_iretd, 0},
	{ "ja", op_ja, OPE_RELBRA},
	{ "jnbe", op_jnbe, OPE_RELBRA},
	{ "jae", op_jae, OPE_RELBRA},
	{ "jnb", op_jnb, OPE_RELBRA},
	{ "jnc", op_jnc, OPE_RELBRA},
	{ "jb", op_jb, OPE_RELBRA},
	{ "jc", op_jc, OPE_RELBRA},
	{ "jnae", op_jnae, OPE_RELBRA},
	{ "jbe", op_jbe, OPE_RELBRA},
	{ "jna", op_jna, OPE_RELBRA},
	{ "jecxz", op_jecxz, OPE_RELBR8},
	{ "je",op_je, OPE_RELBRA },
	{ "jg",op_jg, OPE_RELBRA },
	{ "jnle",op_jnle, OPE_RELBRA },
	{ "jl",op_jl, OPE_RELBRA },
	{ "jnge",op_jnge, OPE_RELBRA },
	{ "jge", op_jge, OPE_RELBRA },
	{ "jnl", op_jnl, OPE_RELBRA },
	{ "jle", op_jle, OPE_RELBRA },
	{ "jng", op_jng, OPE_RELBRA },
	{ "jne", op_jne, OPE_RELBRA },
	{ "jo", op_jo, OPE_RELBRA },
	{ "jno", op_jno, OPE_RELBRA },
	{ "jp", op_jp, OPE_RELBRA },
	{ "jpe", op_jpe, OPE_RELBRA },
	{ "jpo", op_jpo, OPE_RELBRA },
	{ "js", op_js, OPE_RELBRA} ,
	{ "jns", op_jns, OPE_RELBRA },
	{ "jmp", op_jmp, OPE_JMP },
	{ "lahf", op_lahf, 0 },
	{ "lar", op_lar, OPE_REGRM },
	{ "lds", op_lds, OPE_LOADSEG },
	{ "les", op_les, OPE_LOADSEG },
	{ "lfs", op_lfs, OPE_LOADSEG },
	{ "lgs", op_lgs, OPE_LOADSEG },
	{ "lss", op_lss, OPE_LOADSEG },
	{ "lea", op_lea, OPE_REGRM },
	{ "leave", op_leave, 0 },
	{ "lgdt", op_lgdt, OPE_LGDT },
	{ "lidt", op_lidt, OPE_LIDT },
	{ "lldt", op_lldt, OPE_RM16 },
	{ "lmsw", op_lmsw, OPE_RM16 },
	{ "lock", op_lock, 0 },
	{ "lods", op_lods, OPE_LODS },
	{ "lodsb", op_lodsb, 0 },
	{ "lodsw", op_lodsw, 0 },
	{ "lodsd", op_lodsd, 0 },
	{ "loop",op_loop, OPE_RELBR8 },
	{ "loope",op_loope, OPE_RELBR8 },
	{ "loopz",op_loopz, OPE_RELBR8 },
	{ "loopne",op_loopne, OPE_RELBR8 },
	{ "loopnz",op_loopnz, OPE_RELBR8 },
	{ "lsl", op_lsl, OPE_REGRM },
	{ "ltr", op_ltr, OPE_RM16 },
	{ "mov", op_mov, OPE_MOV },
	{ "movs", op_movs, OPE_MOVS },
	{ "movsb", op_movsb, 0 },
	{ "movsw", op_movsw, 0 },
	{ "movsd", op_movsd, 0 },
	{ "movsx", op_movsx, OPE_MOVSX} ,
	{ "movzx", op_movzx, OPE_MOVSX} ,
	{ "mul", op_mul, OPE_RM },
	{ "neg", op_neg, OPE_RM },
	{ "not", op_not, OPE_RM },
	{ "nop", op_nop, 0 },
	{ "or", op_or,OPE_MATH },
	{ "out", op_out,OPE_OUT },
	{ "outs", op_outs, OPE_OUTS },
	{ "outsb", op_outsb, 0 },
	{ "outsw", op_outsw, 0 },
	{ "outsd", op_outsd, 0 },
	{ "pop",op_pop, OPE_PUSHPOP },
	{ "popa", op_popa, 0 },
	{ "popad", op_popad, 0 },
	{ "popf", op_popf, 0 },
	{ "popfd", op_popfd, 0 },
	{ "push",op_push, OPE_PUSHPOP },
	{ "pusha", op_pusha, 0 },
	{ "pushad", op_pushad, 0 },
	{ "pushf", op_pushf, 0 },
	{ "pushfd", op_pushfd, 0 },
	{ "rcl", op_rcl, OPE_SHIFT },
	{ "rcr", op_rcr, OPE_SHIFT },
	{ "rol", op_rol, OPE_SHIFT },
	{ "ror", op_ror, OPE_SHIFT },
	{ "rep", op_rep, 0 },
	{ "repne", op_repne, 0 },
	{ "repe", op_repe, 0 },
	{ "repnz", op_repnz, 0 },
	{ "repz", op_repz, 0 },
	{ "ret", op_ret, OPE_RET },
	{ "sahf", op_sahf, 0 },
	{ "sal", op_sal, OPE_SHIFT },
	{ "sar", op_sar, OPE_SHIFT },
	{ "shl", op_shl, OPE_SHIFT },
	{ "shr", op_shr, OPE_SHIFT },
	{ "sbb", op_sbb,OPE_MATH },
	{ "scas", op_scas, OPE_SCAS },
	{ "scasb", op_scasb, 0 },
	{ "scasw", op_scasw, 0 },
	{ "scasd", op_scasd, 0 },
	{ "seta", op_seta, OPE_SET},
	{ "setnbe", op_setnbe, OPE_SET},
	{ "setae", op_setae, OPE_SET},
	{ "setnb", op_setnb, OPE_SET},
	{ "setnc", op_setnc, OPE_SET},
	{ "setb", op_setb, OPE_SET},
	{ "setc", op_setc, OPE_SET},
	{ "setnae", op_setnae, OPE_SET},
	{ "setbe", op_setbe, OPE_SET},
	{ "setna", op_setna, OPE_SET},
	{ "sete",op_sete, OPE_SET },
	{ "setg",op_setg, OPE_SET },
	{ "setnle",op_setnle, OPE_SET },
	{ "setl",op_setl, OPE_SET },
	{ "setnge",op_setnge, OPE_SET },
	{ "setge", op_setge, OPE_SET },
	{ "setnl", op_setnl, OPE_SET },
	{ "setle", op_setle, OPE_SET },
	{ "setng", op_setng, OPE_SET },
	{ "setne", op_setne, OPE_SET },
	{ "seto", op_seto, OPE_SET },
	{ "setno", op_setno, OPE_SET },
	{ "setp", op_setp, OPE_SET },
	{ "setpe", op_setpe, OPE_SET },
	{ "setpo", op_setpo, OPE_SET },
	{ "sets", op_sets, OPE_SET} ,
	{ "setns", op_setns, OPE_SET },
	{ "sgdt", op_sgdt, OPE_LGDT },
	{ "sidt", op_sidt, OPE_LIDT },
	{ "sldt", op_sldt, OPE_RM16 },
	{ "smsw", op_smsw, OPE_RM16 },
	{ "shld", op_shld, OPE_SHLD },
	{ "shrd", op_shrd, OPE_SHLD },
	{ "stc", op_stc, 0 },
	{ "std", op_std, 0 },
	{ "sti", op_sti, 0 },
	{ "stos", op_stos, OPE_STOS },
	{ "stosb", op_stosb, 0 },
	{ "stosw", op_stosw, 0 },
	{ "stosd", op_stosd, 0 },
	{ "str", op_str, OPE_RM16 },
	{ "sub", op_sub, OPE_MATH },
	{ "test", op_test, OPE_TEST },
	{ "verr", op_verr, OPE_RM16 },
	{ "verw", op_verw, OPE_RM16 },
	{ "wait", op_wait, 0 },
	{ "wbinvd", op_wbinvd, 0 },
	{ "xchg", op_xchg, OPE_XCHG },
	{ "xlat", op_xlat, OPE_XLAT },
	{ "xlatb", op_xlatb, 0 },
	{ "xor", op_xor,OPE_MATH },
	{ "f2xm1", op_f2xm1, 0 },
	{ "fabs", op_fabs, 0 },
	{ "fadd", op_fadd, OPE_FMATH },
	{ "faddp", op_faddp, OPE_FMATHP },
	{ "fiadd", op_fiadd, OPE_FMATHI },
	{ "fchs", op_fchs, 0 },
	{ "fclex", op_fclex, 0 },
	{ "fnclex", op_fnclex, 0 },
	{ "fcom", op_fcom, OPE_FCOM },
	{ "fcomp", op_fcomp, OPE_FCOM },
	{ "fcompp", op_fcompp, 0 },
	{ "fcos", op_fcos, 0 },
	{ "fdecstp", op_fdecstp, 0 },
	{ "fdiv", op_fdiv, OPE_FMATH },
	{ "fdivp", op_fdivp, OPE_FMATHP },
	{ "fidiv", op_fidiv, OPE_FMATHI },
	{ "fdivr", op_fdivr, OPE_FMATH },
	{ "fdivrp", op_fdivrp, OPE_FMATHP },
	{ "fidivr", op_fidivr, OPE_FMATHI },
	{ "ffree", op_ffre, OPE_FREG },
	{ "ficom", op_ficom, OPE_FICOM },
	{ "ficomp", op_ficomp, OPE_FICOM },
	{ "fild", op_fild, OPE_FILD },
	{ "fincstp", op_fincstp, 0 },
	{ "finit", op_finit, 0 },
	{ "fninit", op_fninit, 0 },
	{ "fist", op_fist, OPE_FIST },
	{ "fistp", op_fistp, OPE_FILD },
	{ "fld", op_fld, OPE_FLD },
	{ "fldz", op_fldz, 0},
	{ "fldpi", op_fldpi, 0},
	{ "fld1", op_fld1, 0},
	{ "fldl2t", op_fld2t, 0},
	{ "fldl2e", op_fld2e, 0},
	{ "fldlg2", op_fldlg2, 0},
	{ "fldln2", op_fldln2, 0},
	{ "fldcw", op_fldcw, OPE_M16 },
	{ "fldsw", op_fldsw, OPE_M16 },
	{ "fldenv", op_fldenv, OPE_MN },
	{ "fmul", op_fmul, OPE_FMATH},
	{ "fmulp", op_fmulp, OPE_FMATHP},
	{ "fimul", op_fimul, OPE_FMATHI },
	{ "fpatan", op_fpatan, 0 },
	{ "fprem", op_fprem, 0 },
	{ "fprem1", op_fprem1, 0 },
	{ "fptan", op_fptan, 0 },
	{ "frndint", op_frndint, 0 },
	{ "frstor", op_frstor, OPE_MN },
	{ "fsave", op_fsave, OPE_MN },
	{ "fnsave", op_fnsave, OPE_MN },
	{ "fscale", op_fscale, 0 },
	{ "fsin", op_fsin, 0 },
	{ "fsincos", op_fsincos, 0 },
	{ "fsqrt", op_fsqrt, 0 },
	{ "fst", op_fst, OPE_FST},
	{ "fstp", op_fstp, OPE_FSTP},
	{ "fstcw", op_fstcw, OPE_M16},
	{ "fstsw", op_fstsw, OPE_M16},
	{ "fnstcw", op_fnstcw, OPE_M16},
	{ "fnstsw", op_fnstsw, OPE_M16},
	{ "fstenv", op_fstenv, OPE_MN },
	{ "fnstenv", op_fsntenv, OPE_MN },
	{ "fsub", op_fsub, OPE_FMATH},
	{ "fsubp", op_fsubp, OPE_FMATHP},
	{ "fisub", op_fisub, OPE_FMATHI},
	{ "fsubr", op_fsubr, OPE_FMATH},
	{ "fsubrp", op_fsubrp, OPE_FMATHP},
	{ "fisubr", op_fisubr, OPE_FMATHI},
	{ "ftst", op_ftst, 0},
	{ "fucom", op_fucom, OPE_FUCOM },
	{ "fucomp", op_fucomp, OPE_FUCOM },
	{ "fucompp", op_fucompp, 0 },
	{ "fwait", op_fwait, 0 },
	{ "fxam", op_fxam, 0 },
	{ "fxch", op_fxch, OPE_FXCH },
	{ "fxtract", op_fxtract, 0 },
	{ "fyl2x", op_fyl2x, 0 },
	{ "fyl2xp1", op_fyl2xp1, 0 },
	{0,0,0 },
	};
/* Init module */
void outcodeini(void)
{
	strtab = 0;
	gentype = nogen;
	curseg = noseg;
	outcol = 0;
	newlabel = FALSE;
	phiput = FALSE;
}
void nl(void)
/*
 * New line
 */
{    if (prm_asmfile) {
       if(outcol > 0) {
                fputc('\n',outputFile);
                outcol = 0;
                gentype = nogen;
                }
			 if (phiused && !phiput)
								fputc(0x1f,outputFile);
		 }
}
/* Put an opcode
 */
void outop(char *name)
{
	fputc('\t',outputFile);
	while (*name)
		fputc(toupper(*name++),outputFile);
}
void putop(enum e_op op)
{    
	if (prm_nasm) {   
  	skipsize =  (op == op_lea);
  	addsize = (op == op_push);
		if (op == op_fwait) {
			/* NASM uses WAIT instead of FWAIT */
			outop(oplst[op_fwait].word+1);
			return;
		}
	}
	if (op > op_fyl2xp1)
    DIAG("illegal opcode.");
	else
		outop(oplst[op].word);
	uses_float=(op >=op_f2xm1);
}

void putconst(ENODE *offset)
/*
 *      put a constant to the outputFile file.
 */
{       switch( offset->nodetype )
                {
								case en_autoreg:
                case en_autocon:
                case en_icon:
                case en_lcon:
                case en_iucon:
                case en_lucon:
                case en_ccon:
								case en_absacon:
                        fprintf(outputFile,"0%lXH",offset->v.i);
                        break;
								case en_rcon:
								case en_fcon:
								case en_lrcon:
												fprintf(outputFile,"%f",offset->v.f);
												break;
                case en_labcon:
                case en_nalabcon:
/*												if (!prm_nasm && !prm_flat)
													fprintf(outputFile,"CS:");
*/                        fprintf(outputFile,"L_%d",offset->v.i);
                        break;
								case en_napccon:
                case en_nacon:
                        fprintf(outputFile,"%s",offset->v.p[0]);
                        break;
                case en_add:
                        putconst(offset->v.p[0]);
                        fprintf(outputFile,"+");
                        putconst(offset->v.p[1]);
                        break;
                case en_sub:
                        putconst(offset->v.p[0]);
                        fprintf(outputFile,"-");
                        putconst(offset->v.p[1]);
                        break;
                case en_uminus:
                        fprintf(outputFile,"-");
                        putconst(offset->v.p[0]);
                        break;
                default:
                        DIAG("illegal constant node.");
                        break;
                }
}
void putlen(int l)
/*
 *      append the length field to an instruction.
 */
{ 
	if (l!= 10 && l != 8 && l != 6 && l != 4 && l != 1 && l != 2 && l != 0)
     DIAG("illegal length field.");
}
void putsizedreg(char *string, int reg, int size)
{
	static char *byteregs[] = { "AL","CL","DL","BL","AH","CH","DH","BH" };
	static char *wordregs[] = { "AX", "CX", "DX","BX","SP","BP","SI","DI"  };
	static char *longregs[] = { "EAX", "ECX", "EDX","EBX","ESP","EBP","ESI","EDI" };
  if (size == 4)
		fprintf(outputFile,string,longregs[reg]);
	else if (size == 1)
		fprintf(outputFile,string,byteregs[reg]);
		else
			fprintf(outputFile,string,wordregs[reg]);
}
void pointersize(int size)
{
	if (prm_nasm && skipsize)
		return;
/*	if (needpointer)
*/		switch (size) {		
			case 10:
				fprintf(outputFile,"TBYTE ");
				break;
			case 8:                      
       	fprintf(outputFile,"QWORD ");
				break;
			case 6:
				if (!uses_float) {
					fprintf(outputFile,"FWORD ");
					break;
				}
			case 4:                      
				fprintf(outputFile,"DWORD ");
				break;
			case 2:
				fprintf(outputFile,"WORD ");
				break;
			case 1:
				fprintf(outputFile,"BYTE ");
				break;
			default:
				DIAG("Bad pointer");
		}	
		if (!prm_nasm)
			fprintf(outputFile,"PTR ");
}
void putseg(int seg, int usecolon)
{
	if (!seg)
		return;
	seg-=1;
	seg<<=1;
	fputc(segregs[seg], outputFile);
	fputc(segregs[seg+1], outputFile);
  if (usecolon)
		fputc(':', outputFile);
}

void putamode(AMODE *ap)
/*
 *      output a general addressing mode.
 */

{	int oldnasm, l;
       switch( ap->mode )
                {
								case am_seg:
												putseg(ap->seg,0);
												break;
								case am_screg:
												fprintf(outputFile,"CR%d",ap->preg);
												break;
								case am_sdreg:
												fprintf(outputFile,"DR%d",ap->preg);
												break;
								case am_streg:
												fprintf(outputFile,"TR%d",ap->preg);
												break;
                case am_immed:
												if (ap->length && (ap->offset->nodetype == en_labcon 
														|| ap->offset->nodetype == en_nacon || ap->offset->nodetype == en_nalabcon
														|| ap->offset->nodetype == en_napccon)) {
													if (!prm_nasm)
														fprintf(outputFile,"OFFSET ");
												  else
														if (!nosize) 
															fprintf(outputFile,"DWORD ");
												}
												else
													if (prm_nasm && addsize)
														pointersize(ap->length);
                        putconst(ap->offset);
												break;
                case am_direct:
												pointersize(ap->length);
												putseg(ap->seg,TRUE);
/*												if (!prm_flat)
													fprintf(outputFile,"DS:");
*/												fprintf(outputFile,"[");
												oldnasm = prm_nasm;
												prm_nasm = TRUE;
                        putconst(ap->offset);
												fputc(']',outputFile);
												prm_nasm = oldnasm;
                        break;
                case am_dreg:
												putsizedreg("%s",ap->preg,ap->length);
												break;
								case am_freg:
												fprintf(outputFile,"ST(%d)",ap->preg);
												break;
								case am_indisp:
												pointersize(ap->length);
												putseg(ap->seg,TRUE);
												putsizedreg("[%s",ap->preg,4);
												if (ap->offset) {
													fputc('+',outputFile);
													putconst(ap->offset);
												}
												fputc(']',outputFile);
												break;
								case am_indispscale: {
												int scale = 1,t=ap->scale;
												while (t--)
													scale <<=1;
												pointersize(ap->length);
												putseg(ap->seg,TRUE);
												if (ap->preg == -1)
													fputc('[',outputFile);
												else
													putsizedreg("[%s+",ap->preg,4);
												putsizedreg("%s",ap->sreg,4);
												if (scale != 1)
													fprintf(outputFile,"*%d",scale);
												if (ap->offset) {
													fputc('+',outputFile);
													putconst(ap->offset);
												}
												fputc(']',outputFile);
												}
												break;
                default:
                        DIAG("illegal address mode.");
                        break;
                }
}

void put_code(OCODE *cd)
/*
 *      outputFile a generic instruction.
 */
{       
		int op = cd->opcode,len=0,len2=0;
		AMODE *aps = cd->oper1,*apd = cd->oper2, *ap3 = cd->oper3;
		if (!prm_asmfile)	
			return;
		if (op == op_line) {
			if (!prm_lines)
				return;
			fprintf(outputFile,";\n; Line %d:\t%s\n;\n",(int)apd,(char *)aps);
			return;
		}
		if (aps)
			len = aps->length;
		if (apd)
			len2 = apd->length;
		needpointer = (len != len2) || ((!aps || aps->mode !=am_dreg) && (!apd || apd->mode !=am_dreg));
		putop(op);
		if (prm_nasm && op >=op_ja && op <= op_jns)
			fprintf(outputFile,"\tNEAR");
		switch (op) {
			case op_rep:
			case op_repz:
			case op_repe:
			case op_repnz:
			case op_repne:
			case op_lock:
				return;
		}
    putlen(len);
        if( aps != 0 ) {
                fprintf(outputFile,"\t");
								if (op == op_dd)
									nosize = TRUE;
								putamode(aps);
								nosize = FALSE;
                if( apd != 0 ) 
                        {
                        fprintf(outputFile,",");
                        putamode(apd);
                        }
                if( ap3 != 0 ) 
                        {
                        fprintf(outputFile,",");
                        putamode(ap3);
                        }
				}
  fprintf(outputFile,"\n");
}

void gen_strlab(SYM *s)
/*
 *      generate a named label.
 */
{
		if (prm_asmfile)
			if (curseg == dataseg || curseg == bssxseg) {
				newlabel = TRUE;
			 	fprintf(outputFile,"\n%s",s->name);
				outcol = strlen(s->name)+1;
			}
			else
				if (currentfunc->pascaldefn) {
					char buf[100],*q=buf,*p=s->name;
					if (prm_cmangle)
						p++;
					while(*p)
						*q++=toupper(*p++);
					*q++ = 0;
        	fprintf(outputFile,"%s:\n",buf);
				}
				else
        	fprintf(outputFile,"%s:\n",s->name);
}

void put_label(int lab)
/*
 *      outputFile a compiler generated label.
 */
{
       if (prm_asmfile)
					fprintf(outputFile,"L_%d:\n",lab);
}
void put_staticlabel(long label)
{
				if (prm_asmfile) {
					nl();
					if (curseg == dataseg || curseg == bssxseg) {
						newlabel = TRUE;
					 	fprintf(outputFile,"\nL_%ld",label);
						outcol = 8;
					}
					else
						fprintf(outputFile,"L_%ld:\n",label);
				}
}

void genfloat(float val)
/*
 * Output a float value
 */
{ 		if (prm_asmfile)
        if( gentype == floatgen && outcol < 60) {
                fprintf(outputFile,",%f",val);
                outcol += 8;
                }
        else    {
								if (!newlabel)
									nl();
								else newlabel = FALSE;
                fprintf(outputFile,"\tDD\t%f",val);
                gentype = floatgen;
                outcol = 19;
                }
}

void gendouble(double val)
/*
 * Output a double value
 */
{ 		if (prm_asmfile)
        if( gentype == doublegen && outcol < 60) {
                fprintf(outputFile,",%f",val);
                outcol += 8;
                }
        else    {
								if (!newlabel)
									nl();
								else newlabel = FALSE;
                fprintf(outputFile,"\tDQ\t%f",val);
                gentype = doublegen;
                outcol = 19;
                }
}
void genlongdouble(double val)
/*
 * Output a double value
 */
{ 		if (prm_asmfile)
        if( gentype == longdoublegen && outcol < 60) {
                fprintf(outputFile,",%f",val);
                outcol += 8;
                }
        else    {
								if (!newlabel)
									nl();
								else newlabel = FALSE;
                fprintf(outputFile,"\tDT\t%f",val);
                gentype = longdoublegen;
                outcol = 19;
                }
}
int genstring(char *str, int uselong)
/*
 * Generate a string literal
 */
{
	if (uselong) {
		while  (*(short *)str) {
			genword(*((short *)str));
			str+=2;
		}
		return pstrlen(str)*2;
	}
	else {
		while (*str)
			genbyte(*str++);
		return strlen(str);
	}
}
void genbyte(long val)
/*
 * Output a byte value
 */
{ 		if (prm_asmfile)
        if( gentype == bytegen && outcol < 60) {
                fprintf(outputFile,",0%XH",val & 0x00ff);
                outcol += 4;
                }
        else    {
								if (!newlabel)
									nl();
								else newlabel = FALSE;
                fprintf(outputFile,"\tDB\t0%XH",val & 0x00ff);
                gentype = bytegen;
                outcol = 19;
                }
}

void genword(long val)
/*
 * Output a word value
 */
{     if (prm_asmfile)
        if( gentype == wordgen && outcol < 58) {
                fprintf(outputFile,",0%XH",val & 0x0ffff);
                outcol += 6;
                }
        else    {
								if (!newlabel)
									nl();
								else newlabel = FALSE;
                fprintf(outputFile,"\tDW\t0%XH",val & 0x0ffff);
                gentype = wordgen;
                outcol = 21;
                }
}

void genlong(long val)
/*
 * Output a long value
 */
{     if (prm_asmfile)
        if( gentype == longgen && outcol < 56) {
                fprintf(outputFile,",0%lXH",val);
                outcol += 10;
                }
        else    {
								if (!newlabel)
									nl();
								else newlabel = FALSE;
                fprintf(outputFile,"\tDD\t0%lXH",val);
                gentype = longgen;
                outcol = 25;
                }
}

void gensrref(SYM *sp, int val)
{
			if (prm_asmfile)
        if( gentype == srrefgen && outcol < 56) {
                fprintf(outputFile,",%s,%d",sp->name,val);
                outcol += strlen(sp->name)+1;
                }
        else    {
                nl();
                fprintf(outputFile,"\tDD\t%s,%d",sp->name,val);
                gentype = srrefgen;
                outcol = 25;
                }
}
void genref(SYM *sp,int offset)
/*
 * Output a reference to the data area (also gens fixups )
 */
{       char    sign;
			char buf[40];
        if( offset < 0) {
                sign = '-';
                offset = -offset;
                }
        else
                sign = '+';
			sprintf(buf,"%s%c%d",sp->name,sign,offset);
			if (prm_asmfile) {
        if( gentype == longgen && outcol < 55 - strlen(sp->name)) {
                fprintf(outputFile,",%s",buf);
                outcol += (11 + strlen(sp->name));
                }
        else    {
								if (!newlabel)
									nl();
								else newlabel = FALSE;
                fprintf(outputFile,"\tDD\t%s",buf);
                outcol = 26 + strlen(sp->name);
                gentype = longgen;
                }
			}
}
void genpcref(SYM *sp,int offset)
/*
 * Output a reference to the code area (also gens fixups )
 */
{
	genref(sp,offset);
}
void genstorage(int nbytes)
/*
 * Output bytes of storage
 */
{			if (prm_asmfile) {
								if (!newlabel)
									nl();
								else newlabel = FALSE;
				if (prm_nasm)
        	fprintf(outputFile,"\tRESB\t0%XH",nbytes);
				else
        	fprintf(outputFile,"\tDB\t0%XH DUP (?)",nbytes);
				outcol = 28;
				gentype = storagegen;
			}
}

void gen_labref(int n)
/*
 * Generate a reference to a label
 */
{			if (prm_asmfile)
        if( gentype == longgen && outcol < 58) {
                fprintf(outputFile,",L_%d",n);
                outcol += 6;
                }
        else    {
								if (!newlabel)
									nl();
								else newlabel = FALSE;
                fprintf(outputFile,"\tDD\tL_%d",n);
                outcol = 22;
                gentype = longgen;
                }
}

int     stringlit(char *s, int uselong)
/*
 *      make s a string literal and return it's label number.
 */
{       struct slit     *lp;
        ++global_flag;          /* always allocate from global space. */
        lp = xalloc(sizeof(struct slit));
        lp->label = nextlabel++;
				if (uselong) 
					lp->str = plitlate(s);
				else
        	lp->str = litlate(s);
        lp->next = strtab;
				lp->type = uselong;
        strtab = lp;
        --global_flag;
        return lp->label;
}

void dumplits(void)
/*
 *      dump the string literal pool.
 */
{
        while( strtab != 0) {
                cseg();
                nl();
                put_label(strtab->label);
								genstring(strtab->str,strtab->type);
								if (strtab->type)
									genword(0);
								else
									genbyte(0);
                strtab = strtab->next;
                }
        nl();
}

/*
 * Exit if from a special segment
 */
void exitseg(void)
{
	if (!prm_nasm) {
		if (curseg == startupxseg) {
			curseg = noseg;
			fprintf(outputFile,"cstartup\tENDS\n");
		}
		else if (curseg == rundownxseg) {
			curseg = noseg;
			fprintf(outputFile,"crundown\tENDS\n");
		}
		else if (curseg == cppxseg) {
			curseg = noseg;
			fprintf(outputFile,"cppinit\tENDS\n");
		}
	}
}
/*
 * Switch to cseg 
 */
void cseg(void)
{			if (prm_asmfile)
       	if( curseg != codeseg) {
                nl();
								exitseg();
								if (prm_nasm)
                	fprintf(outputFile,"[SECTION .text]\n");
								else
                	fprintf(outputFile,"\t.CODE\n");
                curseg = codeseg;
                }
}
/*
 * Switch to deseg
 */
void dseg(void)
{     if (prm_asmfile)  
				if( curseg != dataseg) {
                nl();
								exitseg();
								if (prm_nasm)
                	fprintf(outputFile,"[SECTION .data]\n");
								else
                	fprintf(outputFile,"\t.DATA\n");
                curseg = dataseg;
                }
}
/*
 * Switch to bssseg
 */
void bssseg(void)
{     if (prm_asmfile)  
				if( curseg != bssxseg) {
                nl();
								exitseg();
								if (prm_nasm)
                	fprintf(outputFile,"[SECTION .bss]\n");
								else
                	fprintf(outputFile,"\t.DATA?\n");
                curseg = bssxseg;
                }
}
/*
 * Switch to startupseg
 */
void startupseg(void)
{     if (prm_asmfile)  
				if( curseg != startupxseg) {
                nl();
								exitseg();	
								if (prm_nasm)
                	fprintf(outputFile,"[SECTION cstartup]\n");
								else
                	fprintf(outputFile,"cstartup\tSEGMENT USE32 PUBLIC DWORD \042INITDATA\042\n");
                curseg = startupxseg;
                }
}
/*
 * Switch to rundownseg
 */
void rundownseg(void)
{     if (prm_asmfile)  
				if( curseg != rundownxseg) {
                nl();
								exitseg();
								if (prm_nasm)
	                fprintf(outputFile,"[SECTION crundown]\n");
								else
  	              fprintf(outputFile,"crundown\tSEGMENT USE32 PUBLIC DWORD \042EXITDATA\042\n");
                curseg = rundownxseg;
                }
}
void cppseg(void)
{     if (prm_asmfile)  
				if( curseg != cppxseg) {
                nl();
								exitseg();
								if (prm_nasm)
                	fprintf(outputFile,"[SECTION cppinit]\n");
								else
                	fprintf(outputFile,"cppinit\tSEGMENT USE32 PUBLIC DWORD \042CPPDATA\042\n");
                curseg = cppxseg;
                }
}
void gen_virtual(char *name)
{
	if (prm_asmfile) {
		nl();
		fprintf(outputFile,"@%s\tSEGMENT VIRTUAL",name);
	}
}
void gen_endvirtual(char *name)
{
	if (prm_asmfile) {
		nl();
		fprintf(outputFile,"@%s\tENDS",name);
	}
}
/*
 * Align
 */
void align(int size)
{
			if (prm_asmfile) {
				nl();
				if (prm_nasm)
/* NASM 0.91 wouldn't let me use parenthesis but this should work
 * according to the documented precedence levels
 */
					fprintf(outputFile,"\tTIMES $$-$ & %d NOP\n",3);
				else
					fprintf(outputFile,"\tALIGN\t%d\n",4);
			}
}
/* muldiv val init
 */
void init_muldivval(void)
{
	muldivlink = 0;
}
/*
 * queue muldiv val
 */
void queue_muldivval(int label, long number)
{
	MULDIV *p = xalloc(sizeof(MULDIV));
	p->link = muldivlink;
	p->value = number;
	p->label = label;
	p->size = 0;
	muldivlink = p;
}
void queue_floatval(int label, double number, int size)
{
	MULDIV *p = xalloc(sizeof(MULDIV));
	p->link = muldivlink;
	p->floatvalue = number;
	p->label = label;
	p->size = size;
	muldivlink = p;
}
void dump_muldivval(void)
{
	int tag = FALSE;
	if (prm_asmfile) {
		fprintf(outputFile,"\n");
		if (muldivlink) {
			tag = TRUE;
			align(4);
		}
		while (muldivlink) {
			put_label(muldivlink->label);
			if (muldivlink->size == 0)
				fprintf(outputFile,"\tDD\t0%xH\n",muldivlink->value);
			else if (muldivlink->size == 6)
				fprintf(outputFile,"\tDD\t%f\n",muldivlink->floatvalue);
			else if (muldivlink->size == 8)
				fprintf(outputFile,"\tDQ\t%f\n",muldivlink->floatvalue);
			else
				fprintf(outputFile,"\tDT\t%f\n",muldivlink->floatvalue);
			muldivlink = muldivlink->link;
		}
		if (tag)
			fprintf(outputFile,"\n");
	}
}
void asm_header(void)
{
	nl();
	if (prm_nasm)
				fprintf(outputFile,"[BITS 32]\n\n");
	else {
		fprintf(outputFile,"\tTITLE\t'%s'\n",outfile);
		if (prm_flat)
				fprintf(outputFile,"\t.486p\n\t.MODEL FLAT\n\n");
		else
				fprintf(outputFile,"\t.486p\n\t.MODEL SMALL\n\n");
	}
}
void globaldef(SYM *sp)
{
	char buf[100],*q=buf,*p=sp->name;
	if (curseg == codeseg && currentfunc->pascaldefn) {
		if (prm_cmangle)
			p++;
		while(*p)
			*q++=toupper(*p++);
		*q++ = 0;
	}
	else
		strcpy(buf,p);
	if (prm_nasm)
      fprintf(outputFile,"[GLOBAL\t%s]\n",buf);
	else
      fprintf(outputFile,"\tPUBLIC\t%s\n",buf);
}			
void putexterns(void)
/*
 * Output the fixup tables and the global/external list
 */
{       SYM     *sp;
			int i;
			if (prm_asmfile){
						int notyet = TRUE;
				nl();
				exitseg();
				for (i=0; i < HASHTABLESIZE; i++) {
					if ((sp=(SYM *) globalhash[i]) != 0) {
						while (sp) {
    	    		if( sp->storage_class == sc_externalfunc && sp->extflag) {
								char buf[100],*q=buf,*p=sp->name;
								if (curseg == codeseg && sp->pascaldefn) {
									if (prm_cmangle)
										p++;
									while(*p)
										*q++=toupper(*p++);
									*q++ = 0;
								}
								else
									strcpy(buf,p);
								if (prm_nasm) {
							 		if (notyet) {
										fprintf(outputFile,"\n[SECTION .text]\n");
										notyet = FALSE;
									}
      	          fprintf(outputFile,"[EXTERN\t%s]\n",buf);
								}
								else {
							 		if (notyet) {
										fprintf(outputFile,"\n\t.CODE\n");
										notyet = FALSE;
									}
      	          fprintf(outputFile,"\tEXTRN\t%s:PROC\n",buf);
								}
							}
         			sp = sp->next;
						}
					}
				}
				notyet = TRUE;
				for (i=0; i < HASHTABLESIZE; i++) {
					if ((sp=(SYM *) globalhash[i]) != 0) {
						while (sp) {
	        		if( sp->storage_class == sc_external && sp->extflag) {
								char buf[100],*q=buf,*p=sp->name;
								if (curseg == codeseg && sp->pascaldefn) {
									if (prm_cmangle)
										p++;
									while(*p)
										*q++=toupper(*p++);
									*q++ = 0;
								}
								else
									strcpy(buf,p);
							  if (prm_nasm) {
							 		if (notyet) {
										fprintf(outputFile,"\n[SECTION .data]\n");
										notyet = FALSE;
									}
  	              fprintf(outputFile,"[EXTERN\t%s]\n",buf);
							  }
							  else {
									if (notyet) {
										fprintf(outputFile,"\n\t.DATA\n");
										notyet = FALSE;
									}
  	              fprintf(outputFile,"\tEXTRN\t%s\n",buf);
							  }
							}
         			sp = sp->next;
						}
					}
				}
				if (!prm_nasm)
					fprintf(outputFile,"\tEND\n");
			}
}