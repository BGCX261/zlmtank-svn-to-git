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
#include        "expr.h"
#include        "c.h"
#include        "gen386.h"
#include 				"diag.h"

extern TABLE gsyms;
extern int stdaddrsize;
extern int global_flag;
extern int fsave_mask, save_mask;
extern TYP              stdfunc;
extern struct amode     push[], pop[];
extern OCODE *peep_tail, *peep_head, *peep_insert;
extern long stackdepth,framedepth;
extern SYM *currentfunc;
extern int regs[3];
extern int prm_cplusplus,prm_linkreg;
extern int firstlabel, nextlabel;
extern long lc_maxauto;

int funcfloat;

static int     breaklab;
static int     contlab;
static int     retlab;
static int diddef;
static long gswitchbottom, gswitchcount;
static long gswitchtop;

AMODE    *makedreg(int r)
/*
 *      make an address reference to a data register.
 */
{       AMODE    *ap;
        ap = xalloc(sizeof(AMODE));
        ap->mode = am_dreg;
        ap->preg = r;
        return ap;
}

AMODE    *makefreg(int r)
/*
 *      make an address reference to a data register.
 */
{       AMODE    *ap;
        ap = xalloc(sizeof(AMODE));
        ap->mode = am_freg;
        ap->preg = r;
        return ap;
}

AMODE    *make_direct(int i)
/*
 *      make a direct reference to an immediate value.
 */
{       return make_offset(makenode(en_icon,(char *)i,0));
}

AMODE    *make_strlab(char *s)
/*
 *      generate a direct reference to a string label.
 */
{       AMODE    *ap;
        ap = xalloc(sizeof(AMODE));
        ap->mode = am_direct;
        ap->offset = makenode(en_nacon,s,0);
        return ap;
}

void genwhile(SNODE *stmt)
/*
 *      generate code to evaluate a while statement.
 */
{       int     lab1, lab2, lab3;
        initstack();            /* initialize temp registers */
        lab1 = contlab;         /* save old continue label */
        contlab = nextlabel++;  /* new continue label */
        if( stmt->s1 != 0 )      /* has block */
                {
        				lab2 = breaklab;        /* save old break label */
                breaklab = nextlabel++;
								gen_code(op_jmp,0,make_label(contlab),0);
								lab3 = nextlabel++;
								gen_label(lab3);
                genstmt(stmt->s1);
								gen_label(contlab);
								if (stmt->lst)
		 							gen_line(stmt->lst);
                initstack();
                truejp(stmt->exp,lab3);
                gen_label(breaklab);
                breaklab = lab2;        /* restore old break label */
                }
        else					        /* no loop code */
                {
								if (stmt->lst)
		 							gen_line(stmt->lst);
        				gen_label(contlab);
                initstack();
                truejp(stmt->exp,contlab);
                }
        contlab = lab1;         /* restore old continue label */
}

void gen_for(SNODE *stmt)
/*
 *      generate code to evaluate a for loop
 */
{       int     old_break, old_cont, exit_label, loop_label;
        old_break = breaklab;
        old_cont = contlab;
        loop_label = nextlabel++;
        exit_label = nextlabel++;
        contlab = nextlabel++;
        initstack();
        if( stmt->label != 0 )
                gen_expr(stmt->label,F_ALL | F_NOVALUE
                        ,natural_size(stmt->label));
				
        gen_code(op_jmp,0,make_label(contlab),0);
        gen_label(loop_label);
        if( stmt->s1 != 0 ) {
								breaklab = exit_label;
                genstmt(stmt->s1);
				}
        initstack();
        if( stmt->s2 != 0 )
                gen_expr(stmt->s2,F_ALL | F_NOVALUE,natural_size(stmt->s2));
				gen_label(contlab);
				if (stmt->lst)
		 			gen_line(stmt->lst);
        initstack();
        if( stmt->exp != 0 )
                truejp(stmt->exp,loop_label);
				else
						gen_code(op_jmp,0,make_label(loop_label),0);
				gen_label(exit_label);
 				breaklab = old_break;
				contlab = old_cont;
}

void genif(SNODE *stmt)
/*
 *      generate code to evaluate an if statement.
 */
{       int     lab1, lab2;
        lab1 = nextlabel++;     /* else label */
        lab2 = nextlabel++;     /* exit label */
        initstack();            /* clear temps */
        falsejp(stmt->exp,lab1);
        genstmt(stmt->s1);
        if( stmt->s2 != 0 )             /* else part exists */
                {
                gen_code(op_jmp,0,make_label(lab2),0);
                gen_label(lab1);
                genstmt(stmt->s2);
                gen_label(lab2);
                }
        else                            /* no else code */
                gen_label(lab1);
}

void gendo(SNODE *stmt)
/*
 *      generate code for a do - while loop.
 */
{       int     oldcont, oldbreak;
        oldcont = contlab;
        oldbreak = breaklab;
        contlab = nextlabel++;
        gen_label(contlab);
        if( stmt->s1 != 0 && stmt->s1->next != 0 )
                {
                breaklab = nextlabel++;
                genstmt(stmt->s1);      /* generate body */
                initstack();
                truejp(stmt->exp,contlab);
                gen_label(breaklab);
                }
        else
                {
                genstmt(stmt->s1);
                initstack();
                truejp(stmt->exp,contlab);
                }
        breaklab = oldbreak;
        contlab = oldcont;
}

AMODE *set_symbol(char *name,int flag)
{
	SYM *sp;
	AMODE *ap;
        sp = gsearch(name);
        if( sp == 0 )
                {
                ++global_flag;
                sp = xalloc(sizeof(SYM));
                sp->tp = &stdfunc;
                sp->name = name;
								if (flag)
									sp->storage_class = sc_externalfunc;
								else
                	sp->storage_class = sc_external;
								sp->extflag = 1;
                insert(sp,&gsyms);
                --global_flag;
                }
				ap = make_strlab(name);
				ap->mode = am_immed;
	return ap;
}
void call_library(char *lib_name)
/*
 *      generate a call to a library routine.
 */
{
				AMODE *ap;
				ap = set_symbol(lib_name,1);
        gen_code(op_call,0,ap,0);
}

int analyzeswitch(SNODE *stmt)
{
	gswitchbottom = 0x7fffffff;
	gswitchtop = 0x80000000;
	gswitchcount = 0;
	stmt = stmt->s1;
	while (stmt) {
		if (!stmt->s2) {
			gswitchcount++;
			if ((long)stmt->label < gswitchbottom)
				gswitchbottom = (int)stmt->label;
			if ((long)stmt->label > gswitchtop)
				gswitchtop = (int)stmt->label;
		}
		stmt = stmt->next;
	}
	gswitchtop++;
	if (gswitchcount == 0)
		return(0);
	if (gswitchcount > 3)
		if (gswitchcount*10/(gswitchtop-gswitchbottom) >= 8)
			return(1);
	return(2);
}
void bingen(int lower, int avg, int higher,AMODE *ap1, int deflab, int size,long * switchids,int * switchlabels,int *switchbinlabels)
{
	AMODE *ap2 = make_immed(switchids[avg]);
	AMODE *ap3 = make_label(switchlabels[avg]);
	if (switchbinlabels[avg] != -1)
		gen_label(switchbinlabels[avg]);
	gen_code(op_cmp,4,ap1,ap2);
	gen_code(op_je,0,ap3,0);
	if (avg == lower) {
		ap3 = make_label(deflab);
		gen_code(op_jmp,0,ap3,0);
	}
	else {
		int avg1 = (lower + avg)/2;
		int avg2 = (higher + avg+1)/2;
		if (avg+1 < higher) 
			ap3 = make_label(switchbinlabels[avg2]=nextlabel++);
		else
			ap3 = make_label(deflab);
		if (size < 0)
			gen_code(op_jg,0,ap3,0);
		else
			gen_code(op_ja,0,ap3,0);
		bingen(lower,avg1,avg,ap1,deflab,size,switchids,switchlabels,switchbinlabels);
		if (avg+1 < higher)
			bingen(avg+1,avg2,higher,ap1,deflab,size,switchids,switchlabels,switchbinlabels);
	}
}
void genbinaryswitch(SNODE *stmt, int deflab)
{
	int curlab, i=0,j,size = natural_size(stmt->exp);
	AMODE *ap1;
  long switchbottom=gswitchbottom, switchcount=gswitchcount;
  long switchtop=gswitchtop;
  int *switchlabels=0;
  long *switchids=0;
  int *switchbinlabels=0;
	curlab = nextlabel++;
  initstack();
  ap1 = gen_expr(stmt->exp,F_DREG,4);
	global_flag++;
		switchlabels = xalloc((switchcount) * sizeof(int));
		switchbinlabels = xalloc((switchcount) * sizeof(int));
		switchids = xalloc((switchcount) * sizeof(long));
	global_flag--;
	stmt = stmt->s1;
	while (stmt) {
                if( stmt->s2 )          /* default case ? */
                        {
                        stmt->label = (SNODE *) deflab;
												diddef = TRUE;
                        }
                else
                        {
												switchlabels[i] = curlab;
												switchbinlabels[i] = -1;
												switchids[i++] = (int)stmt->label;
												stmt->label = (SNODE *)curlab;
                        }
                if( stmt->next != 0 )
                        curlab = nextlabel++;
                stmt = stmt->next;
                }
	for (i=0; i < switchcount; i++)
		for (j=i+1; j < switchcount; j++)
			if (switchids[j] < switchids[i]) {
				int temp = switchids[i];
				switchids[i] = switchids[j];
				switchids[j] = temp;
				temp = switchlabels[i];
				switchlabels[i] = switchlabels[j];
				switchlabels[j] = temp;
			}
	bingen(0,(switchcount)/2,switchcount,ap1,deflab,size,switchids,switchlabels,switchbinlabels);
	freeop(ap1);
}
void gencompactswitch(SNODE *stmt, int deflab)
{       int             tablab,curlab,i, size = natural_size(stmt->exp);
        AMODE    *ap,*ap2;
  long switchbottom=gswitchbottom, switchcount=gswitchcount;
  long switchtop=gswitchtop;
  int *switchlabels=0;
				tablab = nextlabel++;
        curlab = nextlabel++;
        initstack();
        ap = gen_expr(stmt->exp,F_DREG | F_VOL,4);
				initstack();
				if (switchbottom)
					gen_code(op_sub,4,ap,make_immed(switchbottom));
				if (size < 0)
					gen_code(op_jl,0,make_label(deflab),0);
				else
					gen_code(op_jb,0,make_label(deflab),0);
				gen_code(op_cmp,4,ap,make_immed(switchtop-switchbottom));
				if (size < 0)
					gen_code(op_jge,0,make_label(deflab),0);
				else
					gen_code(op_jnc,0,make_label(deflab),0);
				gen_code(op_shl,4,ap,make_immed(2));
				ap2 = xalloc(sizeof(AMODE));
				ap->mode = am_indisp;
				ap2->preg = ap->preg;
				ap->offset = makenode(en_labcon,(char *)tablab,0);
				gen_code(op_jmp,4,ap,0);

				initstack();
				align(4);
				gen_label(tablab);
		switchlabels = xalloc((switchtop-switchbottom) * sizeof(int));
	for (i=switchbottom; i < switchtop; i++) {
		switchlabels[i-switchbottom] = deflab;
	}
	stmt = stmt->s1;
	while (stmt) {
                if( stmt->s2 )          /* default case ? */
                        {
                        stmt->label = (SNODE *)deflab;
												diddef = TRUE;
                        }
                else
                        {
												switchlabels[(int)stmt->label-switchbottom] = curlab;
												stmt->label = (SNODE *)curlab;
                        }
                if(stmt->next != 0 )
                        curlab = nextlabel++;
                stmt = stmt->next;
                }
	for (i=0; i < switchtop-switchbottom; i++)
		gen_code(op_dd,4,make_label(switchlabels[i]),0);
}

void gencase(SNODE *stmt)
/*
 *      generate all cases for a switch statement.
 */
{       while( stmt != 0 )
                {
                gen_label((int)stmt->label);
                if( stmt->s1 != 0 )
                        {
                        genstmt(stmt->s1);
                        }

                stmt = stmt->next;
                }
}

void genxswitch(SNODE *stmt)
/*
 *      analyze and generate best switch statement.
 */
{       int     oldbreak;
	int olddiddef = diddef;
	int deflab = nextlabel++;
        oldbreak = breaklab;
        breaklab = nextlabel++;
				diddef = FALSE;
				switch (analyzeswitch(stmt)) {
  			case 2:
						genbinaryswitch(stmt,deflab);
						break;
					case 1:
						gencompactswitch(stmt,deflab);
						break;
					case 0:
						if (stmt->s1)
							stmt->s1->label = (SNODE *) nextlabel++;
						break;
				}
        gencase(stmt->s1);
				if (!diddef)
					gen_label(deflab);
        gen_label(breaklab);
        breaklab = oldbreak;
	diddef = olddiddef;
}

void genreturn(SNODE *stmt, int flag)
/*
 *      generate a return statement.
 */
{       AMODE    *ap,*ap1;
				int size;
        if( stmt != 0 && stmt->exp != 0 ) {
                initstack();
								if (currentfunc->tp->btp && currentfunc->tp->btp->type != bt_void && (currentfunc->tp->btp->type == bt_struct || currentfunc->tp->btp->type == bt_union)) {
									size = currentfunc->tp->btp->size;
									ap = gen_expr(stmt->exp,F_ALL,4);
									if (!(save_mask & 0x40))
										gen_push(ESI,am_dreg,0);                                  
									if (!(save_mask & 0x80))
										gen_push(EDI,am_dreg,0);                                  
									if (prm_linkreg) {
										ap1 = xalloc(sizeof(AMODE));
										ap1->preg = EBP;
										ap1->mode = am_indisp;
										if (currentfunc->pascaldefn && currentfunc->tp->lst.head && currentfunc->tp->lst.head != (SYM *) -1) 
											ap1->offset = makenode(en_icon,(char *)(currentfunc->tp->lst.head->value.i+((currentfunc->tp->lst.head->tp->size+3) &0xFFFFFFFCL)),0);
										else
											ap1->offset = makenode(en_icon,(char *)8,0);
									}
									else
										if (currentfunc->pascaldefn && currentfunc->tp->lst.head && currentfunc->tp->lst.head != (SYM *) -1) 
											ap1 = make_stack(-stackdepth-framedepth-currentfunc->tp->lst.head->value.i-((currentfunc->tp->lst.head->tp->size+3) & 0xfffffffcL));
										else
											ap1 = make_stack(-stackdepth-framedepth);
									gen_code(op_mov,4,makedreg(ESI),ap);		                  
									gen_code(op_mov,4,makedreg(EDI),ap1);
									gen_code(op_mov,4,makedreg(EAX),makedreg(EDI));
									gen_code(op_mov,4,makedreg(ECX),make_immed(size));  
									gen_code(op_cld,0,0,0);
									gen_code(op_rep,1,0,0);                                   
									gen_code(op_movsb,1,0,0);		                              
									if (!(save_mask & 0x80))
										gen_pop(EDI,am_dreg,0);
									if (!(save_mask & 0x40))
										gen_pop(ESI,am_dreg,0);
								}
								else {
									size = currentfunc->tp->btp->size;
  	              ap = gen_expr(stmt->exp,F_DREG | F_FREG,size);
									if (size > 4) {
										if (ap->mode != am_freg)
											gen_code(op_fld,size,ap,0);
									}
            	    else
              	  		if( ap->mode != am_dreg || ap->preg != 0 )
                	        gen_code(op_mov,size,makedreg(0),ap);
                }
								freeop(ap);
				}
				if (flag) {
	        if( retlab != -1 )
                gen_label(retlab);
          if( fsave_mask != 0 )
						diag("Float restore in return");
					if (!prm_linkreg && lc_maxauto)
						gen_code(op_add,4,makedreg(ESP),make_immed(lc_maxauto));
					if (currentfunc->intflag) {
						gen_code(op_popad,0,0,0);
						if (prm_linkreg && (lc_maxauto || currentfunc->tp->lst.head && currentfunc->tp->lst.head != (SYM *)-1)) {
							gen_code(op_leave,0,0,0);
						}
           	gen_code(op_iretd,0,0,0);
					}
					else {
           	if( save_mask != 0 )
							popregs(save_mask);
						if (prm_linkreg && (lc_maxauto || currentfunc->tp->lst.head && currentfunc->tp->lst.head != (SYM *)-1)) {
							gen_code(op_leave,0,0,0);
						}
						if (currentfunc->pascaldefn) {
							long retsize = 0;
							if (currentfunc->tp->lst.head && currentfunc->tp->lst.head != (SYM *)-1) {
								retsize = currentfunc->tp->lst.head->value.i+((currentfunc->tp->lst.head->tp->size + 3) & 0xfffffffcL);
								if (prm_linkreg)
									retsize -= 8;
							}		
							if (currentfunc->tp->btp && currentfunc->tp->btp->type != bt_void && (currentfunc->tp->btp->type == bt_struct || currentfunc->tp->btp->type == bt_union)) 
								retsize += 4;
							if (retsize) {
            		gen_code(op_ret,0,make_immed(retsize),0);
								return;
							}
						}		
            gen_code(op_ret,0,0,0);
					}
				}
        else {
					if (retlab == -1)
            retlab = nextlabel++;
          gen_code(op_jmp,0,make_label(retlab),0);
				}
}

void genstmt(SNODE *stmt)
/*
 *      genstmt will generate a statement and follow the next pointer
 *      until the block is generated.
 */
{
		   while( stmt != 0 )
                {
                switch( stmt->stype )
                        {
												case st_block:
																genstmt(stmt->exp);
																break;
                        case st_label:
                                gen_label((int)stmt->label);
                                break;
                        case st_goto:
                                gen_code(op_jmp,0,make_label((int)stmt->label),0);
                                break;
                        case st_expr:
                                initstack();
                                gen_expr(stmt->exp,F_ALL | F_NOVALUE,
                                        natural_size(stmt->exp));
                                break;
                        case st_return:
                                genreturn(stmt,0);
                                break;
                        case st_if:
                                genif(stmt);
                                break;
                        case st_while:
                                genwhile(stmt);
                                break;
                        case st_do:
                                gendo(stmt);
                                break;
                        case st_for:
                                gen_for(stmt);
                                break;
                        case st_continue:
                                gen_code(op_jmp,0,make_label(contlab),0);
                                break;
                        case st_break:
                                gen_code(op_jmp,0,make_label(breaklab),0);
                                break;
                        case st_switch:
                                genxswitch(stmt);
                                break;
												case st_line:
		 														gen_line(stmt);
																break;
												case st_asm:
																if (stmt->exp)
																	add_peep(stmt->exp);
																break;
												case st__genword:
																gen_code(op_genword,0,make_immed((int)stmt->exp),0);
																break;
                        default:

                                diag("unknown statement.");
                                break;
                        }
                stmt = stmt->next;
                }
}
#ifdef CPLUSPLUS
void scppinit(void)
{
	if (!strcmp(currentfunc->name,"_main")) {
		AMODE *ap1,*ap2,*ap3,*ap4;
		int lbl = nextlabel++;
		initstack();
		ap1 = temp_data();
		ap4 = xalloc(sizeof(AMODE));
		ap4->preg = ap1->preg;
		ap4->mode = am_indisp;
		ap4->offset = makenode(en_icon,0,0);
		ap2 = set_symbol("cppistart",0);
		ap3 = set_symbol("cppiend",0);
		gen_code(op_mov,4,ap1,ap2);
		gen_label(lbl);
		gen_code(op_push,4,ap1,0);
		gen_code(op_call,4,ap4,0);
		gen_code(op_pop,4,ap1,0);
		gen_code(op_add,4,ap1,make_immed(4));
		gen_code(op_cmp,4,ap1,ap3);
		gen_code(op_jb,0,make_label(lbl),0);
		freeop(ap1);
	}
}
#endif
void genfunc(SNODE *stmt)
/*
 *      generate a function body.
 */
{       retlab = contlab = breaklab = -1;
		funcfloat = 0;
				init_muldivval();
				stackdepth = 0;
				if (stmt->stype == st_line) {
					gen_line(stmt);
					stmt = stmt->next;
				}
					
				gen_codelab(currentfunc);  /* name of function */
        opt1(stmt);			/* push args & also subtracts SP */
#ifdef CPLUSPLUS
				if (prm_cplusplus)
					scppinit();
#endif
        genstmt(stmt);
        genreturn(0,1);
}