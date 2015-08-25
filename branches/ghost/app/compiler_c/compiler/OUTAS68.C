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
#include        "gen68.h"
#include 				"diag.h"

/*      variable initialization         */
extern int global_flag;
extern SYM *currentfunc;
extern HASHREC **globalhash;
extern OCODE *peep_insert, *peep_head, *peep_tail;
extern int prm_rel;
extern long nextlabel;
extern FILE *outputFile;
extern int	prm_asmfile;
extern int prm_lines;
extern int phiused;
extern int prm_cmangle;

enum e_gt  gentype = nogen;		/* Current DC type */
enum e_sg	 curseg = noseg;		/* Current seg */
int        outcol = 0;				/* Curront col (roughly) */
int 			 dataofs;						/* Offset from last label */
SYM *datasp;									/* Symbol of last named label */
static DATALINK *datahead, *datatail;	/* links for fixup gen */
static int phiput;

ASMREG reglst[] = {
	{ 0 , 0 , 0 },
};
/* List of opcodes
 * This list MUST be in the same order as the op_ enums 
 */
ASMNAME oplst[] = {
	{ "?reserved",op_reserved,0 },
	{ "?line#",op_reserved,0 },
	{ "?seq@",op_reserved,0 },
	{ "?slit",op_reserved,0 },
	{ "?label",op_reserved,0 },
	{ "?flabel",op_reserved,0 },
	{ "dc",op_reserved,0 },
	{ "dc",op_reserved,0 },
	{ "dc",op_reserved,0 },
	{ "abcd",op_abcd,0 },
	{ "add",op_add,0 },
	{ "adda",op_adda,0 },
	{ "addi",op_addi,0 },
	{ "addq",op_addq,0 },
	{ "addx",op_addx,0 },
	{ "and",op_and,0 },
	{ "andi",op_andi,0 },
	{ "asl",op_asl,0 },
	{ "asr",op_asr,0 },
	{ "bra",op_bra,0 },
	{ "beq",op_beq,0 },
	{ "bne",op_bne,0 },
	{ "blt",op_blt,0 },
	{ "ble",op_ble,0 },
	{ "bgt",op_bgt,0 },
	{ "bge",op_bge,0 },
	{ "bhi",op_bhi,0 },
	{ "bhs",op_bhs,0 },
	{ "blo",op_blo,0 },
	{ "bls",op_bls,0 },
	{ "bsr",op_bsr,0 },
	{ "bcc", op_bcc,0 },
	{ "bcs", op_bcs,0 },
	{ "bmi", op_bmi,0 },
	{ "bpl", op_bpl,0 },
	{ "bvc", op_bvc,0 },
	{ "bvs", op_bvs,0 },
	{ "bchg",op_bchg,0 },
	{ "bclr",op_bclr,0 },
	{ "bfchg",op_bfchg,0 },
	{ "bfclr",op_bfclr,0 },
	{ "bfexts",op_bfexts,0 },
	{ "bfextu",op_bfextu,0 },
	{ "bfffo",op_bfffo,0 },
	{ "bfins",op_bfins,0 },
	{ "bfset",op_bfset,0 },
	{ "bftst",op_bftst,0 },
	{ "bkpt", op_bkpt,0 },
	{ "bset",op_bset,0 },
	{ "btst",op_btst,0 },
	{ "chk",op_chk,0 },
	{ "chk2",op_chk2,0 },
	{ "clr",op_clr,0 },
	{ "cmp",op_cmp,0 },
	{ "cmpa",op_cmpa,0 },
	{ "cmpi",op_cmpi,0 },
	{ "cmpm",op_cmpm,0 },
	{ "cmp2",op_cmp2,0 },
	{ "dbeq",op_dbeq,0 },
	{ "dbne",op_dbne,0 },
	{ "dblt",op_dblt,0 },
	{ "dble",op_dble,0 },
	{ "dbgt",op_dbgt,0 },
	{ "dbge",op_dbge,0 },
	{ "dbhi",op_dbhi,0 },
	{ "dbhs",op_dbhs,0 },
	{ "dblo",op_dblo,0 },
	{ "dbls",op_dbls,0 },
	{ "dbsr",op_dbsr,0 },
	{ "dbcc", op_dbcc,0 },
	{ "dbcs", op_dbcs,0 },
	{ "dbmi", op_dbmi,0 },
	{ "dbpl", op_dbpl,0 },
	{ "dbvc", op_dbvc,0 },
	{ "dbvs", op_dbvs,0 },
	{ "dbt", op_dbt,0 },
	{ "dbf", op_dbf,0 },
	{ "dbra",op_dbra,0 },
	{ "divs",op_divs,0 },
	{ "divu",op_divu,0 },
	{ "divsl",op_divsl,0 },
	{ "divul",op_divul,0 },
	{ "eor",op_eor,0 },
	{ "eori",op_eori,0 },
	{ "exg",op_exg,0 },
	{ "ext",op_ext,0 },
	{ "extb",op_extb,0 },
	{ "illegal", op_illegal, 0 },
	{ "jmp",op_jmp,0 },
	{ "jsr",op_jsr,0 },
	{ "lea",op_lea,0 },
	{ "link",op_link,0 },
	{ "lsl",op_lsl,0 },
	{ "lsr",op_lsr,0 },
	{ "move",op_move,0 },
	{ "movea",op_movea,0 },
	{ "movec",op_movec,0 },
	{ "movem",op_movem,0 },
	{ "movep",op_movep,0 },
	{ "moveq",op_moveq,0 },
	{ "moves",op_moves,0 },
	{ "muls",op_muls,0 },
	{ "mulu",op_mulu,0 },
	{ "nbcd",op_nbcd,0 },
	{ "neg",op_neg,0 },
	{ "negx",op_negx,0 },
	{ "nop",op_nop,0 },
	{ "not",op_not,0 },
	{ "or",op_or,0 },
	{ "ori",op_ori,0 },
	{ "pack",op_pack,0 },
	{ "pea",op_pea,0 },
	{ "reset",op_reset,0 },
	{ "rol",op_rol,0 },
	{ "ror",op_ror,0 },
	{ "roxl",op_roxl,0 },
	{ "roxr",op_roxr,0 },
	{ "rtd",op_rtd,0 },
	{ "rte",op_rte,0 },
	{ "rtr",op_rtr,0 },
	{ "rts",op_rts,0 },
	{ "sbcd",op_sbcd,0 },
	{ "seteq",op_seteq,0 },
	{ "setne",op_setne,0 },
	{ "setlt",op_setlt,0 },
	{ "setle",op_setle,0 },
	{ "setgt",op_setgt,0 },
	{ "setge",op_setge,0 },
	{ "sethi",op_sethi,0 },
	{ "seths",op_seths,0 },
	{ "setlo",op_setlo,0 },
	{ "setls",op_setls,0 },
	{ "setsr",op_setsr,0 },
	{ "setcc", op_setcc,0 },
	{ "setcs", op_setcs,0 },
	{ "setmi", op_setmi,0 },
	{ "setpl", op_setpl,0 },
	{ "setvc", op_setvc,0 },
	{ "setvs", op_setvs,0 },
	{ "sett", op_sett,0 },
	{ "setf", op_setf,0 },
	{ "sub",op_sub,0 },
	{ "stop",op_stop,0 },
	{ "suba",op_suba,0 },
	{ "subi",op_subi,0 },
	{ "subq",op_subq,0 },
	{ "subx",op_subx,0 },
	{ "swap",op_swap,0 },
	{ "tas",op_tas,0 },
	{ "trap",op_trap,0 },
	{ "trapeq",op_trapeq,0 },
	{ "trapne",op_trapne,0 },
	{ "traplt",op_traplt,0 },
	{ "traple",op_traple,0 },
	{ "trapgt",op_trapgt,0 },
	{ "trapge",op_trapge,0 },
	{ "traphi",op_traphi,0 },
	{ "traphs",op_traphs,0 },
	{ "traplo",op_traplo,0 },
	{ "trapls",op_trapls,0 },
	{ "trapsr",op_trapsr,0 },
	{ "trapcc", op_trapcc,0 },
	{ "trapcs", op_trapcs,0 },
	{ "trapmi", op_trapmi,0 },
	{ "trappl", op_trappl,0 },
	{ "trapvc", op_trapvc,0 },
	{ "trapvs", op_trapvs,0 },
	{ "trapt", op_trapt,0 },
	{ "trapf", op_trapf,0 },
	{ "trapv", op_trapv,0 },
	{ "tst",op_tst,0 },
	{ "unlk",op_unlk,0 },
	{ "unpk",op_unpk,0 },
	{ "fabs",op_fabs,0 },
	{ "facos",op_facos,0 },
	{ "fadd",op_fadd,0 },
	{ "fasin",op_fasin,0 },
	{ "fatan",op_fatan,0 },
	{ "fatanh",op_fatanh,0 },
	{ "fbeq", op_fbeq, 0 },
	{ "fbne", op_fbne, 0 },
	{ "fbgt", op_fbgt, 0 },
	{ "fbngt", op_fbngt, 0 },
	{ "fbge", op_fbge, 0 },
	{ "fbnge", op_fbnge, 0 },
	{ "fblt", op_fblt, 0 },
	{ "fbnlt", op_fbnlt, 0 },
	{ "fble", op_fble, 0 },
	{ "fbnle", op_fbnle, 0 },
	{ "fbgl", op_fbgl, 0 },
	{ "fbngl", op_fbngl, 0 },
	{ "fbgle", op_fbgle, 0 },
	{ "fbngle", op_fbngle, 0 },
	{ "fbogt", op_fbogt, 0 },
	{ "fbule", op_fbule, 0 },
	{ "fboge", op_fboge, 0 },
	{ "fbult", op_fbult, 0 },
	{ "fbolt", op_fbolt, 0 },
	{ "fbuge", op_fbuge, 0 },
	{ "fbole", op_fbole, 0 },
	{ "fbugt", op_fbugt, 0 },
	{ "fbogl", op_fbogl, 0 },
	{ "fbueq", op_fbueq, 0 },
	{ "fbor", op_fbor, 0 },
	{ "fbun", op_fbun, 0 },
	{ "fbt", op_fbt, 0 },
	{ "fbf", op_fbf, 0 },
	{ "fbst", op_fbst, 0 },
	{ "fbsf", op_fbsf, 0 },
	{ "fbseq", op_fbseq, 0 },
	{ "fbsne", op_fbsne, 0 },
	{ "fcmp",op_fcmp,0 },
	{ "fcos",op_fcos,0 },
	{ "fcosh",op_fcosh,0 },
	{ "fdbeq", op_fdbeq, 0 },
	{ "fdbne", op_fdbne, 0 },
	{ "fdbgt", op_fdbgt, 0 },
	{ "fdbngt", op_fdbngt, 0 },
	{ "fdbge", op_fdbge, 0 },
	{ "fdbnge", op_fdbnge, 0 },
	{ "fdblt", op_fdblt, 0 },
	{ "fdbnlt", op_fdbnlt, 0 },
	{ "fdble", op_fdble, 0 },
	{ "fdbnle", op_fdbnle, 0 },
	{ "fdbgl", op_fdbgl, 0 },
	{ "fdbngl", op_fdbngl, 0 },
	{ "fdbgle", op_fdbgle, 0 },
	{ "fdbngle", op_fdbngle, 0 },
	{ "fdbogt", op_fdbogt, 0 },
	{ "fdbule", op_fdbule, 0 },
	{ "fdboge", op_fdboge, 0 },
	{ "fdbult", op_fdbult, 0 },
	{ "fdbolt", op_fdbolt, 0 },
	{ "fdbuge", op_fdbuge, 0 },
	{ "fdbole", op_fdbole, 0 },
	{ "fdbugt", op_fdbugt, 0 },
	{ "fdbogl", op_fdbogl, 0 },
	{ "fdbueq", op_fdbueq, 0 },
	{ "fdbor", op_fdbor, 0 },
	{ "fdbun", op_fdbun, 0 },
	{ "fdbt", op_fdbt, 0 },
	{ "fdbf", op_fdbf, 0 },
	{ "fdbst", op_fdbst, 0 },
	{ "fdbsf", op_fdbsf, 0 },
	{ "fdbseq", op_fdbseq, 0 },
	{ "fdbsne", op_fdbsne, 0 },
	{ "fdiv",op_fdiv,0 },
	{ "fetox",op_fetox,0 },
	{ "fetoxm1",op_fetoxm1,0 },
	{ "fgetexp",op_fgetexp,0 },
	{ "fgetman",op_fgetman,0 },
	{ "fint",op_fint,0 },
	{ "fintrz",op_fintrz,0 },
	{ "flog10",op_flog10,0 },
	{ "flog2",op_flog2,0 },
	{ "flogn",op_flogn,0 },
	{ "flognp1",op_flognp1,0 },
	{ "fmod",op_fmod,0 },
	{ "fmove",op_fmove,0 },
	{ "fmovecr",op_fmovecr,0 },
	{ "fmovem",op_fmovem,0 },
	{ "fmul",op_fmul,0 },
	{ "fneg",op_fneg,0 },
	{ "fnop",op_fnop,0 },
	{ "frem",op_frem,0 },
	{ "fscale",op_fscale,0 },
	{ "fseq", op_fseq, 0 },
	{ "fsne", op_fsne, 0 },
	{ "fsgt", op_fsgt, 0 },
	{ "fsngt", op_fsngt, 0 },
	{ "fsge", op_fsge, 0 },
	{ "fsnge", op_fsnge, 0 },
	{ "fslt", op_fslt, 0 },
	{ "fsnlt", op_fsnlt, 0 },
	{ "fsle", op_fsle, 0 },
	{ "fsnle", op_fsnle, 0 },
	{ "fsgl", op_fsgl, 0 },
	{ "fsngl", op_fsngl, 0 },
	{ "fsgle", op_fsgle, 0 },
	{ "fsngle", op_fsngle, 0 },
	{ "fsogt", op_fsogt, 0 },
	{ "fsule", op_fsule, 0 },
	{ "fsoge", op_fsoge, 0 },
	{ "fsult", op_fsult, 0 },
	{ "fsolt", op_fsolt, 0 },
	{ "fsuge", op_fsuge, 0 },
	{ "fsole", op_fsole, 0 },
	{ "fsugt", op_fsugt, 0 },
	{ "fsogl", op_fsogl, 0 },
	{ "fsueq", op_fsueq, 0 },
	{ "fsor", op_fsor, 0 },
	{ "fsun", op_fsun, 0 },
	{ "fst", op_fst, 0 },
	{ "fsf", op_fsf, 0 },
	{ "fsst", op_fsst, 0 },
	{ "fssf", op_fssf, 0 },
	{ "fsseq", op_fsseq, 0 },
	{ "fssne", op_fssne, 0 },
	{ "fsgldiv",op_fsgldiv,0 },
	{ "fsglmul",op_fsglmul,0 },
	{ "fsin",op_fsin,0 },
	{ "fsincos",op_fsincos,0 },
	{ "fsinh",op_fsinh,0 },
	{ "fsqrt",op_fsqrt,0 },
	{ "fsub",op_fsub,0 },
	{ "ftan",op_ftan,0 },
	{ "ftanh",op_ftanh,0 },
	{ "ftentox",op_ftentox,0 },
	{ "ftrapeq", op_ftrapeq, 0 },
	{ "ftrapne", op_ftrapne, 0 },
	{ "ftrapgt", op_ftrapgt, 0 },
	{ "ftrapngt", op_ftrapngt, 0 },
	{ "ftrapge", op_ftrapge, 0 },
	{ "ftrapnge", op_ftrapnge, 0 },
	{ "ftraplt", op_ftraplt, 0 },
	{ "ftrapnlt", op_ftrapnlt, 0 },
	{ "ftraple", op_ftraple, 0 },
	{ "ftrapnle", op_ftrapnle, 0 },
	{ "ftrapgl", op_ftrapgl, 0 },
	{ "ftrapngl", op_ftrapngl, 0 },
	{ "ftrapgle", op_ftrapgle, 0 },
	{ "ftrapngle", op_ftrapngle, 0 },
	{ "ftrapogt", op_ftrapogt, 0 },
	{ "ftrapule", op_ftrapule, 0 },
	{ "ftrapoge", op_ftrapoge, 0 },
	{ "ftrapult", op_ftrapult, 0 },
	{ "ftrapolt", op_ftrapolt, 0 },
	{ "ftrapuge", op_ftrapuge, 0 },
	{ "ftrapole", op_ftrapole, 0 },
	{ "ftrapugt", op_ftrapugt, 0 },
	{ "ftrapogl", op_ftrapogl, 0 },
	{ "ftrapueq", op_ftrapueq, 0 },
	{ "ftrapor", op_ftrapor, 0 },
	{ "ftrapun", op_ftrapun, 0 },
	{ "ftrapt", op_ftrapt, 0 },
	{ "ftrapf", op_ftrapf, 0 },
	{ "ftrapst", op_ftrapst, 0 },
	{ "ftrapsf", op_ftrapsf, 0 },
	{ "ftrapseq", op_ftrapseq, 0 },
	{ "ftrapsne", op_ftrapsne, 0 },
	{ "ftst",op_ftst,0 },
	{ "ftwotox",op_ftwotox,0 },
	{ 0,0,0 },
 };
/* Init module */
void outcodeini(void)
{
	gentype = nogen;
	curseg = noseg;
	outcol = 0;
	datahead = datatail = 0;   
	phiput = FALSE;
}
/*
 * Register a fixup 
 */
void datalink(int flag)
{
	DATALINK *p;
	if (!prm_rel)
		return;
	global_flag++;							/* Global tab */
	p = xalloc(sizeof(DATALINK));
	p->sp = datasp;
	p->type = flag;
	p->offset = dataofs;
	p->next = 0;
	if (datahead) {
		datatail->next = p;
		datatail=datatail->next;
	}
	else
		datahead = datatail = p;
	global_flag--;
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
void putop(int op)
{       
	if (op > op_ftwotox)
    DIAG("illegal opcode.");
	else
		outop(oplst[op].word);
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
                        fprintf(outputFile,"$%lX",offset->v.i);
												break;
								case en_rcon:
								case en_fcon:
								case en_lrcon:
												fprintf(outputFile,"%f",offset->v.f);
												break;
                case en_labcon:
                case en_nalabcon:
                        fprintf(outputFile,"L_%ld",offset->v.i);
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
{       switch( l )
                {
                case 0:
                        break;  /* no length field */
                case 1:
                        fprintf(outputFile,".B");
                        break;
                case 2:
                        fprintf(outputFile,".W");
                        break;
                case 4:
                        fprintf(outputFile,".L");
                        break;
								case 6:
												fprintf(outputFile,".S");
												break;
								case 8:
												fprintf(outputFile,".D");
												break;
								case 10:
												fprintf(outputFile,".X");
												break;
                default:
                        DIAG("illegal length field.");
                        break;
                }
}

void putamode(AMODE *ap)
/*
 *      outputFile a general addressing mode.
 */
{       int scale,t;
				switch( ap->mode )
                {
								case am_sr:
												fprintf(outputFile, "SR");
												break;
								case am_bf:
												fprintf(outputFile," {%d:%d}",ap->preg,ap->sreg);
												break;
								case am_divsl:
												fprintf(outputFile,"D%d:D%d",ap->preg, ap->sreg);
												break;
                case am_immed:
                        fprintf(outputFile,"#");
                case am_direct:
                        putconst(ap->offset);
                        break;
                case am_adirect:
												fputc('(',outputFile);
                        putconst(ap->offset);
												fputc(')',outputFile);
												putlen(ap->preg);
                        break;
                case am_areg:
                        fprintf(outputFile,"A%d",ap->preg);
                        break;
                case am_dreg:
                        fprintf(outputFile,"D%d",ap->preg);
                        break;
								case am_freg:
												fprintf(outputFile,"FP%d",ap->preg);
												break;
                case am_ind:
                        fprintf(outputFile,"(A%d)",ap->preg);
                        break;
                case am_ainc:
                        fprintf(outputFile,"(A%d)+",ap->preg);
                        break;
                case am_adec:
                        fprintf(outputFile,"-(A%d)",ap->preg);
                        break;
                case am_indx:
                        fprintf(outputFile,"(");
                        putconst(ap->offset);
                        fprintf(outputFile,",A%d)",ap->preg);
                        break;
								case am_pcindx:
                        fprintf(outputFile,"(");
                        putconst(ap->offset);
                        fprintf(outputFile,",PC)");
                        break;
                case am_baseindxdata:
												scale = 1;
												t = ap->scale;
												while (t--)
													scale <<=1;
                        fprintf(outputFile,"(");
                        putconst(ap->offset);
												if (ap->preg != -1)
                        	fprintf(outputFile,",A%d",ap->preg);
                        fprintf(outputFile,",D%d.L",ap->sreg);
												if (scale != 1)
													fprintf(outputFile,"*%d",scale);
												fputc(')', outputFile);
                        break;
                case am_baseindxaddr:
												scale = 1;
												t = ap->scale;
												while (t--)
													scale <<=1;
                        fprintf(outputFile,"(");
                        putconst(ap->offset);
												if (ap->preg != -1)
                        	fprintf(outputFile,",A%d",ap->preg);
                        fprintf(outputFile,",A%d.L",ap->sreg);
												if (scale != 1)
													fprintf(outputFile,"*%d",scale);
												fputc(')', outputFile);
                        break;
                case am_mask:
                        put_mask((int)ap->offset, ap->preg);
                        break;
								case am_fmask:
                        put_fmask((int)ap->offset, ap->preg);
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
		int op = cd->opcode,len = cd->length;
		AMODE *aps = cd->oper1,*apd = cd->oper2, *ape = cd->oper3;   
		nl();
		if (!prm_asmfile)	{
			;/* oc_putop(cd); */
			return;
		}
		if (op == op_line) {
			if (!prm_lines)
				return;
			fprintf(outputFile,";\n; Line %d:\t%s\n;\n",len,(char *)aps);
			return;
		}
		if (op == op_slit) {
								int l =genstring((char *)cd->oper1,(int)cd->oper2);
								if ((int)cd->oper2)
									genword(0);
								else {
									if (!(l & 1))
										genbyte(0);
									genbyte(0);
								}
								return;
		}
		if( op == op_dcl)
		{
			putop(op);
			putlen(len);
      fprintf(outputFile,"\t");
			putamode(aps);
			if (prm_rel)
				fprintf(outputFile,"-*");
      fprintf(outputFile,"\n");
			return;
		}
	else
		{	
			putop(op);
     	putlen(len);
		}
    if( aps != 0 )
    {
      fprintf(outputFile,"\t");
			putamode(aps);
      if( apd != 0 )
      {
				if (apd->mode != am_bf)
          fprintf(outputFile,",");
        putamode(apd);
				if (ape) {
					if (ape->mode != am_bf)
						fprintf(outputFile,",");
					putamode(ape);
				}
      }
    }
  fprintf(outputFile,"\n");
}

void put_fmask(int mask, int reverse)
/*
 *      generate a register mask for floating restore and save.
 */
{
				unsigned put = FALSE,i,bit;
				if (!reverse) {
					bit = 0x80;
					for (i=0; i < 8; i++) {
						if (bit & (unsigned) mask) {
							if (put)
								fputc('/', outputFile);
							put = TRUE;
							putreg(i+16);
						}
		 				bit >>= 1;
					}
			
				}
				else{
 					bit = 1;
					for (i=0; i < 8; i++) {
						if (bit & (unsigned)mask) {
							if (put)
								fputc('/', outputFile);
							put = TRUE;
							putreg(i+16);
						}
						bit <<= 1;
					}
				}
}
void put_mask(int mask, int reverse)
/*
 *      generate a register mask for integer restore and save.
 */
{
				unsigned put = FALSE,i,bit;
				if (!reverse) {
					bit = 0x8000;
					for (i=0; i < 16; i++) {
						if (bit & (unsigned) mask) {
							if (put)
								fputc('/', outputFile);
							put = TRUE;
							putreg(i);
						}
		 				bit >>= 1;
					}
			
				}
				else{
 					bit = 1;
					for (i=0; i < 16; i++) {
						if (bit & (unsigned)mask) {
							if (put)
								fputc('/', outputFile);
							put = TRUE;
							putreg(i);
						}
						bit <<= 1;
					}
				}
}

void putreg(int r)
/*
 *      generate a register name from a tempref number.
 */
{       if( r < 8 )
                fprintf(outputFile,"D%d",r);
        else if (r <16)
                fprintf(outputFile,"A%d",r - 8);
				else fprintf(outputFile,"FP%d",r-16);
}

void gen_strlab(SYM *sp)
/*
 *      generate a named label.
 */
{
		datasp = sp;
		if (prm_asmfile) {
				nl();
				if (curseg == codeseg && currentfunc->pascaldefn) {
					char buf[100],*q=buf,*p=sp->name;
					if (prm_cmangle)
						p++;
					while(*p)
						*q++=toupper(*p++);
					*q++ = 0;
        	fprintf(outputFile,"%s:\n",buf);
				}
				else
        	fprintf(outputFile,"%s:\n",sp->name);
		}
		else
			;/* oc_namedlab(sp); */
		dataofs = 0;
}

void put_label(OCODE *cd)
/*
 *      outputFile a compiler generated label.
 */
{
       	if (prm_asmfile) {
					nl();
					fprintf(outputFile,"L_%ld:\n",(long)(cd->oper1));
				}
				else
					;/* oc_unnamedlab(cd); */
}
void put_staticlabel(long label)
{
				if (prm_asmfile) {
					nl();
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
                nl();
                fprintf(outputFile,"\tDC.S\t%f",val);
                gentype = floatgen;
                outcol = 19;
                }
			else
				;/* oc_genfloat(val); */
	dataofs+=4;
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
                nl();
                fprintf(outputFile,"\tDC.D\t%f",val);
                gentype = doublegen;
                outcol = 19;
                }
			else
				;/* oc_gendouble(val); */
	dataofs+=8;
}
void genlongdouble(long double val)
/*
 * Output a double value
 */
{ 		if (prm_asmfile)
        if( gentype == longdoublegen && outcol < 60) {
                fprintf(outputFile,",%f",val);
                outcol += 8;
                }
        else    {
                nl();
                fprintf(outputFile,"\tDT\t%f",val);
                gentype = longdoublegen;
                outcol = 19;
                }
			else
				;/* oc_genlongdouble(val); */
	dataofs+=8;
}

int genstring(char *str, int uselong)
/*
 * Generate a string literal
 */
{
	if (uselong) {
		while  (*(short *)str) {
			genword(*((short *)str));
			str += sizeof(short);
		}
		genword(0);
		return pstrlen(str)*2;
	}
	else {
		int size = 0;
		while (*str) {
			genbyte(*str++);
			size++;
		}
		return size;
	}
}
void genbyte(long val)
/*
 * Output a byte value
 */
{ 		if (prm_asmfile)
        if( gentype == bytegen && outcol < 60) {
                fprintf(outputFile,",$%X",val & 0x00ff);
                outcol += 4;
                }
        else    {
                nl();
                fprintf(outputFile,"\tDC.B\t$%X",val & 0x00ff);
                gentype = bytegen;
                outcol = 19;
                }
			else
				;/* oc_genbyte(val); */
	dataofs+=1;
}

void genword(long val)
/*
 * Output a word value
 */
{     if (prm_asmfile)
        if( gentype == wordgen && outcol < 58) {
                fprintf(outputFile,",$%X",val & 0x0ffff);
                outcol += 6;
                }
        else    {
                nl();
                fprintf(outputFile,"\tDC.W\t$%X",val & 0x0ffff);
                gentype = wordgen;
                outcol = 21;
                }
			else
				;/* oc_genword(val); */
	dataofs+=2;
}

void genlong(long val)
/*
 * Output a long value
 */
{     if (prm_asmfile)
        if( gentype == longgen && outcol < 56) {
                fprintf(outputFile,",$%lX",val);
                outcol += 10;
                }
        else    {
                nl();
                fprintf(outputFile,"\tDC.L\t$%lX",val);
                gentype = longgen;
                outcol = 25;
                }
			else
				;/* oc_genlong(val); */
	dataofs+=4;
}
/*
 * Generate a startup or rundown reference
 */
void gensrref(SYM *sp,int val)
{
			if (prm_asmfile)
        if( gentype == srrefgen && outcol < 56) {
                fprintf(outputFile,",%s,%d",sp->name,val);
                outcol += strlen(sp->name)+1;
                }
        else    {
                nl();
                fprintf(outputFile,"\tDC.L\t%s,%d",sp->name,val);
                gentype = srrefgen;
                outcol = 25;
                }
			else 
				;/* oc_gensrref(sp,val); */
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
			datalink(FALSE);
			if (prm_asmfile) {
        if( gentype == longgen && outcol < 55 - strlen(sp->name)) {
                fprintf(outputFile,",%s",buf);
                outcol += (11 + strlen(sp->name));
                }
        else    {
                nl();
                fprintf(outputFile,"\tDC.L\t%s",buf);
                outcol = 26 + strlen(sp->name);
                gentype = longgen;
                }
			}
			else
				;/* oc_genref(sp,val); */
	dataofs+=4;
}
void genpcref(SYM *sp,int offset)
/*
 * Output a reference to the code area (also gens fixups )
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
			datalink(TRUE);
			if (prm_asmfile) {
        if( gentype == longgen && outcol < 55 - strlen(sp->name)) {
                fprintf(outputFile,",%s",buf);
                outcol += (11 + strlen(sp->name));
                }
        else    {
                nl();
                fprintf(outputFile,"\tDC.L\t%s",buf);
                outcol = 26 + strlen(sp->name);
                gentype = longgen;
                }
			}
			else
				;/* oc_genpcref(sp,val); */
	dataofs+=4;
}

void genstorage(int nbytes)
/*
 * Output bytes of storage
 */
{			if (prm_asmfile) {
        nl();
        fprintf(outputFile,"\tDS.B\t$%X\n",nbytes);
			}
			else
				;/* oc_genstorage(nbytes); */
	dataofs+=nbytes;
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
                nl();
                fprintf(outputFile,"\tDC.L\tL_%d",n);
                outcol = 22;
                gentype = longgen;
                }
			else
				;/* oc_genlabref(n); */
	datalink(TRUE);
	dataofs+=4;
}

int     stringlit(char *s, int uselong)
/*
 *      make s a string literal and return it's label number.
 */
{       OCODE     *ip;
				ip = xalloc(sizeof(OCODE));
				ip->opcode = op_label;
				ip->oper1 = (AMODE *)nextlabel;
        ip->back = 0;
				if (!peep_head) 
					peep_head = peep_tail = peep_insert = ip;
				else {
					if (peep_insert->fwd) {
						peep_insert->fwd->back = ip;
					}
					ip->back = peep_insert;
					ip->fwd = peep_insert->fwd;
					if (peep_tail == peep_insert)
						peep_tail = ip;
					peep_insert = peep_insert->fwd = ip;
				}
        ip = xalloc(sizeof(OCODE));
				ip->opcode = op_slit;
				if (uselong) {
        	ip->oper1 = plitlate(s);
					ip->oper2 = (AMODE *)1;
				}
				else {
        	ip->oper1 = litlate(s);
					ip->oper2 = 0;
				}
				if (peep_insert->fwd)
					peep_insert->fwd->back = ip;
				ip->back = peep_insert;
				ip->fwd = peep_insert->fwd;
				if (peep_tail == peep_insert)
					peep_tail = ip;
				peep_insert = peep_insert->fwd = ip;
				
        return nextlabel++;
}
void dumplits(void)
{
}

/*
 * Switch to cseg 
 */
void cseg(void)
{			if (prm_asmfile)
       	if( curseg != codeseg) {
                nl();
                fprintf(outputFile,"\tSECTION\tcode\n");
                curseg = codeseg;
                }
}
/*
 * Switch to dseg
 */
void dseg(void)
{     if (prm_asmfile)  
				if( curseg != dataseg) {
                nl();
                fprintf(outputFile,"\tSECTION\tdata\n");
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
                fprintf(outputFile,"\tSECTION\tbss\n");
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
                fprintf(outputFile,"\tSECTION\tcstartup\n");
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
                fprintf(outputFile,"\tSECTION\tcrundown\n");
                curseg = rundownxseg;
                }
}
/*
 * Switch to cppseg
 */
void cppseg(void)
{     if (prm_asmfile)  
				if( curseg != cppxseg) {
                nl();
                fprintf(outputFile,"\tSECTION\tcppinit\n");
                curseg = cppxseg;
                }
}
void gen_virtual(char *name)
/*
 * Generate a virtual segment
 */
{
	if (prm_asmfile) {
		nl();
		fprintf(outputFile,"@%s\tVIRTUAL",name);
	}
}
void gen_endvirtual(char *name)
/*
 * Generate the end of a virtual segment
 */
{
	if (prm_asmfile) {
		nl();
		fprintf(outputFile,"@%s\tENDVIRTUAL",name);
	}
}
void genlongref(DATALINK *p)
/*
 * Generate a reference reference for fixup tables
 */
{
	if (prm_asmfile) 
        if( gentype == longgen && outcol < 56) {
                fprintf(outputFile,",%s+$%X",p->sp->name,p->offset);
                outcol += 10;
                }
        else    {
                nl();
                fprintf(outputFile,"\tDC.L\t%s+$%X",p->sp->name,p->offset);
                gentype = longgen;
                outcol = 25;
                }
	else
		;/* oc_longen(p->sp,p->offset); */
}
/*
 * Assembly file header
 */
void asm_header(void)
{
}
void globaldef(SYM *sp)
/*
 * Stick in a global definition
 */
{
	if (prm_asmfile) {
		char buf[100],*q=buf,*p=sp->name;
		nl();
		if (curseg == codeseg && sp->pascaldefn) {
			if (prm_cmangle)
				p++;
			while(*p)
				*q++=toupper(*p++);
			*q++ = 0;
		}
		else
			strcpy(buf,p);
    fprintf(outputFile,"\tXDEF\t%s\n",buf);
	}
}
void putexterns(void)
/*
 * Output the fixup tables and the global/external list
 */
{       SYM     *sp;
			DATALINK *p;
			int i;
				int started = FALSE;
				p = datahead;
				curseg = fixcseg;
				while (p) {
					if (p->type) {
						if (!started && prm_asmfile) {
                nl();
                fprintf(outputFile,"\tSECTION\tcodefix\n");
								started = TRUE;
						}
						genlongref(p);
					}
					p = p->next;
				}
				started = FALSE;
				p = datahead;
				curseg = fixdseg;
				while (p) {
					if (!p->type) {
						if (!started && prm_asmfile) {
                nl();
                fprintf(outputFile,"\tSECTION\tdatafix\n");
								started = TRUE;
						}
						genlongref(p);
					}
					p = p->next;
				}
			curseg = noseg;
			if (prm_asmfile) {
				nl();
				for (i=0; i < HASHTABLESIZE; i++) {
					if ((sp=(SYM *) globalhash[i]) != 0) {
						while (sp) {
       				if( (sp->storage_class == sc_external  || sp->storage_class == sc_externalfunc)&& sp->extflag) {
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
            	  fprintf(outputFile,"\tXREF\t%s\n",buf);
							}
	        		sp = sp->next;
						}
					}
				}
			}
			else
			;/* oc_gencode(); */
}