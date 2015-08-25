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
 * Statement parser
 */
#include        <stdio.h> 
#include        "expr.h" 
#include        "c.h" 
#include        "errors.h"

extern int lastch;
extern enum e_sym lastst;
extern char lastid[];
extern TABLE lsyms;
extern int lineno;
extern SYM *currentfunc;
extern int skm_closepa[];
extern int incldepth;
extern short inputline[4096];
extern TABLE oldlsym;
extern int prm_cplusplus;

int asmline;
int skm_openpa[] = { openpa, begin, semicolon, 0 };
int skm_semi[] = { semicolon, end, 0 };
long nextlabel, firstlabel;
int goodcode;


static char phibuf[4096];
static int lastlineno = 0;
static int switchbreak;

SNODE *cbautoinithead, *cbautoinittail;

void stmtini(void)
{
	lastlineno = 0;
  nextlabel = firstlabel = goodcode = 0;
	asmline = FALSE;
}
/* This is the function that detects possibly incorrect assignments */
int scanassign(ENODE *node)
{  
        if( node == 0 )
                return 0;
        switch( node->nodetype ) {
                case en_rcon:
								case en_lrcon: case en_fcon:
                case en_ccon:
                case en_iucon:
                case en_lcon:
                case en_lucon:
                case en_icon:
                case en_napccon:
                case en_nacon:
								case en_absacon:
                case en_autocon:
                case en_autoreg:
												return FALSE;
								case en_bits:
								case en_floatref:
								case en_doubleref:
								case en_longdoubleref:
                case en_b_ref:
                case en_w_ref:
                case en_ul_ref:
                case en_l_ref:
                case en_ub_ref:
                case en_uw_ref:
                case en_uminus:
                case en_compl:  case en_ainc:
                case en_adec:   case en_not:
								case en_cb: case en_cub:
								case en_cw: case en_cuw:
								case en_cl: case en_cul:
								case en_cf: case en_cd: case en_cp: case en_cld:
											return scanassign(node->v.p[0]);
                case en_eq:     case en_ne:
                case en_gt:     case en_ge:
                case en_lt:     case en_le:
								case en_ugt:	case en_uge: case en_ult: case en_ule:
											return 0;		/* No error if a conditional is higher than an equate */
                case en_asadd:  case en_assub:
								case en_asalsh: case en_asarsh: case en_alsh: case en_arsh:
                case en_asmul:  case en_asdiv:
                case en_asmod:  case en_aslsh:
								case en_asumod: case en_asudiv: case en_asumul:
                case en_mul:    case en_div:
                case en_umul:    case en_udiv: case en_umod:
                case en_lsh:    case en_rsh:
                case en_mod:    case en_and:
                case en_or:     case en_xor:
                case en_lor:    case en_land:
							  case en_pmul:
                case en_asrsh:  case en_asand:  case en_pdiv:
                case en_asor:   case en_cond:  case en_asxor:
                case en_void:   
								case en_moveblock: case en_stackblock:
                case en_add:    case en_sub: case en_refassign:
                        return scanassign(node->v.p[0]) || scanassign(node->v.p[1]);
								case en_assign:
												return 1;	/* Error if assign higher than conditional */
                }
	return 0;
}
/* scan an expression and determine if there are any pias */
TYP *doassign(ENODE **exp, int canerror, int *skm)
{
	TYP *tp;
  if( (tp =expression(exp)) == 0 )  {
		if (canerror)
    	generror(ERR_EXPREXPECT,0,skm); 
		*exp = 0;
  }
	else {
		if (scanassign(*exp))
			generror(ERR_BADEQUATE,0,0);
	}
	return tp;
}
SNODE    *whilestmt(void) 
/* 
 *      whilestmt parses the c while statement. 
 */ 
{       SNODE    *snp; 
				int ogc = goodcode;
				goodcode |= GF_INLOOP;
        snp = xalloc(sizeof(SNODE)); 
				snp->next = 0;
        snp->stype = st_while; 
				snp->s1 = 0;
        getsym(); 
				needpunc(openpa,0);
				doassign(&snp->exp,TRUE,0);
				
        needpunc( closepa,skm_closepa );
				goodcode |= GF_CONTINUABLE; 
        snp->s1 = cppblockedstatement(); 
				goodcode = ogc; 
        return snp; 
} 
  
SNODE    *dostmt(void) 
/* 
 *      dostmt parses the c do-while construct. 
 */ 
{       SNODE    *snp; 
				int ogc = goodcode,oswb = switchbreak;
				goodcode |= GF_INLOOP;
        snp = xalloc(sizeof(SNODE)); 
				snp->next = 0;
        snp->stype = st_do; 
        getsym(); 
				goodcode |= GF_CONTINUABLE; 
        snp->s1 = cppblockedstatement(); 
				goodcode = ogc; 
        if( lastst != kw_while ) 
								gensymerror(ERR_IDENTEXPECT,lastid);
        else   
          getsym();
				needpunc(openpa,0); 
				doassign(&snp->exp,TRUE,0);
        needpunc(closepa,skm_closepa); 
        if( lastst != eof)
          needpunc( semicolon,0 );
				switchbreak = oswb;
        return snp; 
} 
  
SNODE    *forstmt(void) 
/*
 * Generate a block for a for statement
 */
{       SNODE    *snp; 
				int ogc = goodcode,oswb = switchbreak;
				int plussemi = 0;
				goodcode |= GF_INLOOP;
        snp = xalloc(sizeof(SNODE)); 
				snp->next = 0;
        getsym(); 
				snp->label = snp->exp = snp->s2 = 0;
        snp->stype = st_for; 
        if (needpunc(openpa,skm_closepa)) { 
#ifdef CPLUSPLUS
					/* CPLUSPLUS allows you to declare a variable here */
					if (castbegin(lastst)) {
						if (prm_cplusplus) {
							ENODE *exp=0,**next = &exp;
							cbautoinithead = cbautoinittail = 0;
							dodecl(sc_auto);
							while (cbautoinithead) {
								if (*next) {
									*next = makenode(en_void,(*next),cbautoinithead->exp);
									next = &(*next)->v.p[1];
								}
								else *next = cbautoinithead->exp;
								cbautoinithead = cbautoinithead->next;
							}
							snp->label = exp;
							plussemi = 1;
						}
						else {
							generror(ERR_NODECLARE,0,0);
							while (castbegin(lastst))
								getsym();
							goto forjoin;
						}
					}
					else 
forjoin:
#endif
					{
						if( expression(&snp->label) == 0 ) 
							snp->label = 0;
						plussemi = needpunc(semicolon,0);
					}
					if (plussemi) {
						doassign(&snp->exp,FALSE,0);
						if (needpunc(semicolon,0)) {
							if( expression(&snp->s2) == 0 )  {
								snp->s2 = 0;
							}
	  				}
					}
        	needpunc(closepa,skm_closepa); 
				}
				goodcode |= GF_CONTINUABLE; 
        snp->s1 = cppblockedstatement(); 
				goodcode = ogc; 
				switchbreak = oswb;
        return snp; 
} 
  
SNODE    *ifstmt(void) 
/* 
 *      ifstmt parses the c if statement and an else clause if 
 *      one is present. 
 */ 
{       SNODE    *snp;
				int ogc = goodcode; 
				int temp=0,temp1 = 0;
        snp = xalloc(sizeof(SNODE)); 
				snp->next = 0;
        snp->stype = st_if; 
        getsym();
				needpunc(openpa,0); 
				doassign(&snp->exp,TRUE,0);
        needpunc( closepa,skm_closepa ); 
        snp->s1 = cppblockedstatement(); 
				temp1 = goodcode & (GF_RETURN | GF_BREAK | GF_CONTINUE | GF_GOTO);
        if( lastst == kw_else ) { 
					temp = goodcode & (GF_RETURN | GF_BREAK | GF_CONTINUE | GF_GOTO);
					goodcode = ogc;
          getsym(); 
          snp->s2 = cppblockedstatement(); 
					temp = temp & goodcode;
        } 
        else 
          snp->s2 = 0; 
				goodcode = ogc | (temp & temp1);
        return snp; 
} 
  
SNODE    *casestmt(SNODE *lst) 
/* 
 *      cases are returned as seperate statements. for normal 
 *      cases label is the case value and s2 is zero. for the 
 *      default case s2 is nonzero. 
 */ 
{       SNODE    *snp; 
        SNODE    *head=0, *tail; 
        snp = xalloc(sizeof(SNODE)); 
				snp->next = 0;
        if( lastst == kw_case ) { 
                getsym(); 
                snp->s2 = 0;
                snp->stype = st_case;
                snp->label = (SNODE *)intexpr(0);
                } 
        else if( lastst == kw_default) { 
								goodcode |= GF_DEF;
                getsym(); 
                snp->stype = st_case;
                snp->s2 = (SNODE *)1; 
                } 
        else    { 
                generror(ERR_NOCASE,0,0); 
                return 0; 
                } 
        needpunc(colon,0); 
        head = 0; 
				if (lst) {
					head = tail = lst;
					lst->next = 0;
				}
        while( lastst != end && lastst != eof &&
                lastst != kw_case && 
                lastst != kw_default ) { 
								if (goodcode & (GF_RETURN | GF_BREAK | GF_CONTINUE | GF_GOTO)) {
									if (lastst == id) {
                    while( isspace(lastch) ) 
                      getch(); 
										if (lastch != ':')
											generror(ERR_UNREACHABLE,0,0);
											
									}
									else
										generror(ERR_UNREACHABLE,0,0);
								}
								goodcode &= ~(GF_RETURN | GF_BREAK | GF_CONTINUE | GF_GOTO);
                if( head == 0 ) 
                        head = tail = statement(); 
                else    { 
                        tail->next = statement(); 
                        } 
                while( tail->next != 0 ) 
                  tail = tail->next; 
                }
				if (goodcode & GF_BREAK)
					goodcode &= ~GF_UNREACH;
        snp->s1 = head; 
        return snp; 
} 
  
int     checkcases(SNODE *head) 
/* 
 *      checkcases will check to see if any duplicate cases 
 *      exist in the case list pointed to by head. 
 */ 
{     
	SNODE	*top, *cur;
	top = head;
	while( top != 0 )
	{
		cur = top->next;
		while( cur != 0 )
		{
			if( (!(cur->s1 || cur->s2) && cur->label == top->label)
				|| (cur->s2 && top->s2) )
			{
				generror(ERR_DUPCASE,(int)cur->label,0);
				return 1;
			}
			cur = cur->next;
		}
		top = top->next;
	}
	return 0;
} 
  
SNODE    *switchstmt(void) 
/*
 * Handle the SWITCH statement
 */
{       SNODE    *snp; 
        SNODE    *head, *tail; 
				TYP *tp;
				SNODE *lst = 0;
				int ogc = goodcode,oswb=switchbreak;
	TABLE oldoldlsym;
	switchbreak = 0;
#ifdef CPLUSPLUS
	if (prm_cplusplus) {
		oldoldlsym = oldlsym;
		oldlsym = lsyms;
	}
#endif
        snp = xalloc(sizeof(SNODE)); 
				snp->next = 0;
        snp->stype = st_switch; 
        getsym(); 
        needpunc(openpa,0); 
				tp = doassign(&snp->exp,TRUE,0);
				if (tp) {
					switch  (tp->type) {
						case bt_char:
						case bt_unsignedchar:
						case bt_short:
						case bt_unsignedshort:
						case bt_long:
						case bt_unsigned:
						case bt_enum:
							break;
						default:
							generror(ERR_SWITCHINT,0,0);
					}
				}
        needpunc(closepa, skm_closepa); 
        needpunc(begin,0); 
        head = 0; 
				goodcode |= GF_UNREACH;
				goodcode &= ~GF_DEF;
				if (lastst == id)
					if (lastch == ':')
						lst = labelstmt(FALSE);
        while( lastst != end && lastst != eof) { 
								goodcode &= ~(GF_RETURN | GF_BREAK | GF_CONTINUE | GF_GOTO);
                if( head == 0 ) 
                        head = tail = casestmt(lst); 
                else    { 
                        tail->next = casestmt(lst); 
                        if( tail->next != 0 ) 
                                tail = tail->next; 
                        } 
								lst = 0;
								if (tail)
                	tail->next = 0; 
                } 
				if (!switchbreak && goodcode & GF_RETURN) {
					if ((goodcode & GF_UNREACH)  && (goodcode & GF_DEF)) 
						ogc |= GF_RETURN;
				}
				goodcode = ogc;
        snp->s1 = head; 
        getsym(); 
        checkcases(head);
				switchbreak = oswb;
#ifdef CPLUSPLUS
	if (prm_cplusplus) {
		check_funcused(&oldlsym,&lsyms);
		gather_labels(&oldlsym,&lsyms);
		cseg();
		lsyms = oldlsym;
		oldlsym.head = oldoldlsym.head;
	}
#endif
        return snp; 
} 
  
SNODE    *retstmt(void) 
/*
 * Handle return 
 */
{       SNODE    *snp; 
				TYP *tp;
        snp = xalloc(sizeof(SNODE)); 
				snp->next = 0;
        snp->stype = st_return; 
				snp->exp = 0;
        getsym(); 
				if (lastst == end || lastst == semicolon) {
					if (currentfunc->tp->btp->type != bt_void)
						generror(ERR_RETMISMATCH,0,0);
					needpunc(semicolon,0);
				}
				else {
					int ogc = goodcode;
					goodcode |= GF_SUPERAND;
        	tp = expression(&(snp->exp));
					goodcode = ogc;
        	if( lastst != eof)
                needpunc( semicolon, 0 );
					if (tp->type == bt_void) {
						generror(ERR_NOVOIDRET,0,0);
					}
					else
/*						if (tp->type == bt_pointer && tp->val_flag)
							generror(ERR_NOFUNCARRAY,0,0);
						else
*/						if (!checktype(tp,currentfunc->tp->btp))
								if (isscalar(tp) && isscalar(currentfunc->tp->btp))
									promote_type(currentfunc->tp->btp, &(snp->exp));
								else
									if (currentfunc->tp->btp->type != bt_pointer ||
											floatrecurse(snp->exp))
										generror(ERR_RETMISMATCH,0,0);
				}
        return snp; 
} 
  
SNODE    *breakstmt(void) 
/*
 * handle break
 */
{       SNODE    *snp; 
        snp = xalloc(sizeof(SNODE)); 
				snp->next = 0;
        snp->stype = st_break; 
        getsym(); 
        if( lastst != eof)
                needpunc( semicolon,0 );
        return snp; 
} 
  
SNODE    *contstmt(void) 
/*
 * handle continue
 */
{       SNODE    *snp; 
        snp = xalloc(sizeof(SNODE)); 
				snp->next = 0;
        snp->stype = st_continue; 
				if (!(goodcode & GF_CONTINUABLE))
					generror(ERR_NOCONTINUE,0,0);
        getsym();
        if( lastst != eof)
                needpunc( semicolon,0 );
        return snp;
}
SNODE *_genwordstmt(void)
/*
 * Insert data in the code stream
 */
{
				SNODE *snp;
				snp = xalloc(sizeof(SNODE));
				snp->next = 0;
				snp->stype = st__genword;
				snp->exp = 0;
				getsym();
				if (lastst != openpa) {
					generror(ERR_PUNCT,openpa,skm_semi);
					getsym();
					snp = 0;
				}
				else {
					getsym();
					snp->exp = (ENODE *) intexpr(0);
					if (lastst != closepa) {
						generror(ERR_PUNCT,closepa,skm_semi);
						snp = 0;
					}
					getsym();
				}
				if (lastst != eof)
					needpunc(semicolon,0);
				return(snp);
}
SNODE    *exprstmt(void) 
/* 
 *      exprstmt is called whenever a statement does not begin 
 *      with a keyword. the statement should be an expression. 
 */ 
{       SNODE    *snp; 
        snp = xalloc(sizeof(SNODE)); 
				snp->next = 0;
        snp->stype = st_expr; 
				goodcode &= ~(GF_ASSIGN);
  			if( expression(&snp->exp) == 0 )  {
    			generror(ERR_EXPREXPECT,0,skm_semi); 
					snp->exp = 0;
  			}
				if (!(goodcode & GF_ASSIGN))
					generror(ERR_CODENONE,0,0);
        if( lastst != eof)
                needpunc( semicolon,0 );
        return snp; 
} 
  
SNODE *snp_line(void)
/*
 * construct a statement for the beginning of a new line
 */
{
	SNODE *snp3 = 0;
				if (!incldepth && lineno != lastlineno && lastst != semicolon && lastst != begin) {
					int i = 0,j,l=pstrlen(inputline);
					snp3 = xalloc(sizeof(SNODE));
					snp3->next = 0;
					snp3->stype = st_line;
					snp3->exp = (ENODE *)lineno;
					snp3->next = 0;
					for (j=0; j<l; j++)
						i+= installphichar(inputline[j],phibuf,i);
					if (phibuf[i-1] == '\n')
						i--;
					phibuf[i] = 0;
					if ((phibuf[i-1] & 0xf0) == 0x90)
						phibuf[i-1] = 0x90;
					snp3->label = (SNODE *)xalloc(i+1);
					strcpy(snp3->label, phibuf);
					lastlineno = lineno;
				}
	return snp3;
}
#ifdef CPLUSPLUS
void dodefaultinit(SYM *sp)
/*
 * Evalueate a C++ default clause
 */
{
	TYP *tp;
	if (lastst == assign) {
		getsym();
		if ((tp =autoasnop(&(sp->defalt), sp)) == 0) {
	 		generror(ERR_EXPREXPECT,0,0);
			getsym();
		}
		else sp->defalt = sp->defalt->v.p[1];
	}
}
#endif
void doautoinit(SYM *sym)
/*
 * This is here rather than in init because autoinit is a type of 
 * statement
 */
{
		if (lastst == assign) {
			SNODE *snp = snp_line();
			if (snp) 
				if (cbautoinithead == 0)
					cbautoinithead = cbautoinittail = snp;
				else {
					cbautoinittail->next = snp;
					cbautoinittail= snp;
				}
			getsym();
			snp = xalloc(sizeof(SNODE));
			snp->next = 0;
			snp->stype = st_expr;
			if (autoasnop(&(snp->exp), sym) == 0) {
				generror(ERR_EXPREXPECT,0,0);
				getsym();
			}
			else {
				if (cbautoinithead == 0)
					cbautoinithead = cbautoinittail = snp;
				else {
					cbautoinittail->next = snp;
					cbautoinittail= snp;
				}
			}
		}
}
		
SNODE    *compound(void) 
/* 
 * Process the body of a compound block.  Declarations are already
 * handled by now.
 * 
 */ 
{       SNODE    *head, *tail; 
        head = cbautoinithead;
				tail = cbautoinittail;
				goodcode &= ~(GF_RETURN | GF_BREAK | GF_CONTINUE | GF_GOTO);
        while( lastst != end  && lastst != eof) { 
								if (goodcode & (GF_RETURN | GF_BREAK | GF_CONTINUE | GF_GOTO)) 
									if (lastst == id) {
                    while( isspace(lastch) ) 
                      getch(); 
										if (lastch != ':')
											generror(ERR_UNREACHABLE,0,0);
											
									}
									else
										generror(ERR_UNREACHABLE,0,0);
								goodcode &= ~(GF_RETURN | GF_BREAK| GF_CONTINUE | GF_GOTO);
                if( head == 0 ) 
                        head = tail = statement(); 
                else    { 
                        tail->next = statement(); 
                        } 
                while( tail->next != 0 ) 
                  tail = tail->next; 
                } 
				if (head)
					tail->next = snp_line();
				if (lastst == eof)
					generror(ERR_PUNCT,end,0);
				else
        	getsym(); 

        return head; 
} 
  
SNODE    *labelstmt(int fetchnext) 
/* 
 *      labelstmt processes a label that appears before a 
 *      statement as a seperate statement. 
 */ 
{       SNODE    *snp; 
        SYM             *sp; 
        snp = xalloc(sizeof(SNODE)); 
				snp->next = 0;
        snp->stype = st_label; 
					goodcode &= ~GF_UNREACH;
        if( (sp = search(lastid,&lsyms)) == 0 ) { 
                sp = xalloc(sizeof(SYM)); 
                sp->name = litlate(lastid); 
                sp->storage_class = sc_label; 
								sp->tp = xalloc(sizeof(TYP));
								sp->tp->type = bt_unsigned;
								sp->tp->uflags = 0;
                sp->value.i = nextlabel++; 
                insert(sp,&lsyms); 
                } 
        else    { 
                if( sp->storage_class != sc_ulabel ) 
                        gensymerror(ERR_DUPLABEL,sp->name); 
                else 
                        sp->storage_class = sc_label; 
                } 
        getsym();       /* get past id */ 
        needpunc(colon,0); 
        if( sp->storage_class == sc_label ) { 
                snp->label = (SNODE *)sp->value.i; 
                snp->next = 0; 
								if (lastst != end && fetchnext)
									snp->next = statement();
                return snp; 
                } 
        return 0; 
} 
  
SNODE    *gotostmt(void) 
/* 
 *      gotostmt processes the goto statement and puts undefined 
 *      labels into the symbol table. 
 */ 
{       SNODE    *snp; 
        SYM             *sp; 
        getsym(); 
        if( lastst != id ) { 
                generror(ERR_IDEXPECT,0,0); 
                return 0; 
                } 
        snp = xalloc(sizeof(SNODE)); 
				snp->next = 0;
        if( (sp = search(lastid,&lsyms)) == 0 ) { 
                sp = xalloc(sizeof(SYM)); 
                sp->name = litlate(lastid); 
                sp->value.i = nextlabel++; 
                sp->storage_class = sc_ulabel; 
								sp->tp = xalloc(sizeof(TYP));
								sp->tp->type = bt_unsigned;
                insert(sp,&lsyms); 
                } 
				sp->tp->uflags = UF_USED;
        getsym();       /* get past label name */ 
        if( lastst != eof)
                needpunc( semicolon,0 );
        if( sp->storage_class != sc_label && sp->storage_class != sc_ulabel) 
                gensymerror( ERR_LABEL,sp->name); 
        else    { 
                snp->stype = st_goto; 
                snp->label = (SNODE *)sp->value.i; 
                snp->next = 0; 
                return snp; 
                } 
        return 0; 
} 
  
SNODE *asm_statement(int shortfin);
SNODE    *statement(void) 
/* 
 *      statement figures out which of the statement processors 
 *      should be called and transfers control to the proper 
 *      routine. 
 */ 
{       SNODE    *snp, *snp2, **psnp; 
				SNODE		 *snp3=snp_line();

        switch( lastst ) { 
								case kw_asm:
												asmline = TRUE;
												getsym();
												if (lastst == begin) {
													getsym();
													snp = snp3;
													if (snp)
														psnp = &snp->next;
													else
														psnp = &snp;
													*psnp = 0;
													while (lastst != end && lastst != eof) {
														*psnp = asm_statement(TRUE);
														while (*psnp)
															psnp=&(*psnp)->next;
														*psnp = snp_line();
														while (*psnp)
															psnp=&(*psnp)->next;
													}
													asmline = FALSE;
													if (lastst == end)
														getsym();
													return snp;
												}
												snp = asm_statement(FALSE);
												asmline = FALSE;
												break;
                case semicolon: 
                        getsym(); 
                        snp = 0; 
                        break; 
                case begin: 
                        getsym(); 
                        snp2 = compoundblock(); 
												snp = xalloc(sizeof(SNODE));
												snp->next = 0;
												snp->exp = snp2;
												snp->stype = st_block;
												break;
                case kw_if: 
                        snp = ifstmt(); 
                        break; 
                case kw_while: 
                        snp = whilestmt(); 
												snp->lst = snp3;
												snp3 = 0;
                        break; 
                case kw_for: 
                        snp = forstmt(); 
												snp->lst = snp3;
												snp3 = 0;
                        break; 
                case kw_return: 
                        snp = retstmt(); 
												goodcode |= GF_RETURN;
                        break; 
                case kw_break: 
												goodcode |= GF_BREAK;
												switchbreak = 1;
                        snp = breakstmt(); 
                        break; 
                case kw_goto: 
												goodcode |= GF_GOTO;
                        snp = gotostmt(); 
                        break; 
                case kw_continue: 
												goodcode |= GF_CONTINUE;
                        snp = contstmt(); 
                        break; 
                case kw_do: 
                        snp = dostmt(); 
                        break; 
                case kw_switch: 
                        snp = switchstmt(); 
                        break; 
								case kw_else:
												generror(ERR_ELSE,0,0);
												getsym();
												break;
								case kw__genword:
												snp = _genwordstmt();
												break;
                case id: 
                        while( isspace(lastch) ) 
                                getch(); 
                        if( lastch == ':') 
                                return labelstmt(TRUE); 
                        /* else fall through to process expression */ 
                default:
#ifdef CPLUSPLUS
												if (castbegin(lastst)) {
													if (prm_cplusplus) {
														cbautoinithead = cbautoinittail = 0;
														dodecl(sc_auto);
														snp = cbautoinithead;
													}
													else {
													  generror(ERR_NODECLARE,0,skm_semi);
													  snp = 0;
													}
												}
												else 
#endif
												{
	                       	snp = exprstmt(); 
												}
                        break; 
                } 
        if( snp != 0 ) {
								if (snp3) {
									snp3->next = snp;
									snp = snp3;
								}
				} 
        return snp; 
}
/* Handling for special C++ situations */
SNODE *cppblockedstatement(void)
{
	SNODE *snp;
	TABLE oldoldlsym;
	long oldlcauto;
#ifdef CPLUSPLUS
	if (prm_cplusplus) {
		oldoldlsym = oldlsym;
		oldlsym = lsyms;
	}
#endif
	snp = statement();
#ifdef CPLUSPLUS
	if (prm_cplusplus) {
		check_funcused(&oldlsym,&lsyms);
		gather_labels(&oldlsym,&lsyms);
		cseg();
		lsyms = oldlsym;
		oldlsym.head = oldoldlsym.head;
	}
#endif
	return(snp);
}
 