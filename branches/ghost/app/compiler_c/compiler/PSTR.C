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
 * String functions on shorts.  This is a translation to C... some functions
 * may not work but those that are actually used in the compiler do
 */
int pstrncmp(short *str1, short *str2,int n)
{
	while (n &&*str1++ == *str2++) n--;
	if (!n)
		return 0;
	return(--str1 > --str2) ? 1 : -1;
		
}
int pstrcmp(short *str1, short *str2)
{
	while (*str1 && *str1 == *str2) {
		str1++;
		str2++;
	}
	if (*(str1) == 0)
		if ( *(str2) == 0)
			return 0;
		else
			return -1;
	return str1 > str2 ? 1 : -1;
}
void pstrcpy(short *str1, short *str2)
{
	while (*str2)
		*str1++ = *str2++;
	*str1 = 0;
}
void pstrncpy(short *str1, short *str2, int len)
{
	memcpy(str1,str2,len*sizeof(short));
}
void pstrcat(short *str1,short *str2)
{
	while (*str1++) ;
	while (*str2)
		*str1++ = *str2++;
	*str1++ = 0;
}
short *pstrchr(short *str, short ch)
{
	while (*str && *str != ch)
		str++;
	if (*str)
		return str;
	return 0;
}
short *pstrrchr(short *str, short ch)
{
	short *start = str;
	while (*str++) ;
	str--;
	while (str != start-1 && *str != ch)
		str--;
	if (str != start-1)
		return str;
	return 0;
}
int pstrlen(short *s)
{
  int len = 0;
	while (*s++) len++;
	return len;
}
short *pstrstr(short *str1, short *str2)
{
	while (1) {
		short *pt = pstrchr(str1,str2[0]);
		if (!pt)
			return 0;
		if (!pstrcmp(pt,str2))
			return pt;
		str1 = pt + 1;
	}
}