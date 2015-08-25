#include "stdlib.h"


// misc_memset(): ���� siz �ֽڴ�С�ڴ�� p ����Ϊ ch
void memset(void *p, unsigned char ch, size_t siz)
{
	int i;

	for (i = 0; i < siz; i++)
		*(unsigned char *)p++ = ch;
}

// misc_memcpy(): ���� siz �ֽ��ڴ�� src ���ڴ�� dest
void memcpy(void *dest, void *src, size_t siz)
{
	int i;

	for (i = 0; i < siz; i++)
		*(unsigned char *)dest++ = *(unsigned char *)src++;
}

int memcmp(void *dest, void *src, size_t siz)
{
	char cmp;
	while(siz-->0)
	{
		cmp = *(char*)dest - *(char*)src;
		if (cmp != 0)
			return cmp;
		++dest;
		++src;
	}
	return 0;
		
}


// misc_strlen(): ������ '\0' ��β�ַ��� s �ĳ���
int strlen(char *s)
{
	int len;

	for (len = 0; s[len] != '\0'; len++)
		;
	return len;
}

// misc_strcmp(): �Ƚ��ַ��� s1 �� s2
// ���ʱ���� 0 ������ ASCII ����˳�򷵻� 1 ���� -1
int strcmp(char *s1, char *s2)
{
	int i;

	for (i = 0; s1[i] != '\0' || s2[i] != '\0'; i++)
		if (s1[i] != s2[i])
			return s1[i] > s2[i] ? 1 : -1;
	return 0;
}

// misc_strncmp(): �Ƚ� n ���ֽڳ����ַ��� s1 �� s2
// ����ֵͬ misc_strcmp() ���ַ������Բ��� '\0'
int strncmp(char *s1, char *s2, int n)
{
	int i;

	for (i = 0; i < n; i++)
		if (s1[i] != s2[i])
			return s1[i] > s2[i] ? 1 : -1;
	return 0;
}

// misc_strcpy(): �����ַ��� src �� dest
// siz Ϊ dest �Ŀռ��С
// ����ֵָ�� dest ������ src �ַ�����
// ��Ϊ���������������Ƽ�ʹ�ñ�����
char *strcpy(char *dest, char *src)
{
	int i;

	for (i = 0; src[i] != '\0'; i++)
		dest[i] = src[i];
	dest[i] = '\0';
	return dest + i;
}

// misc_strncpy(): ���� n ���ֽ��ַ��� src �� dest
// ����ֵָ�� dest ������ src �ַ�����
char *strncpy(char *dest, char *src, int n)
{
	int i;

	for (i = 0; src[i] != '\0' && i < n; i++)
		dest[i] = src[i];
	dest[i] = '\0';

	return dest + i;
}

// misc_strcat(): �����ַ��� src �� dest �ַ�����
// ����ֵָ�� dest ������ src �ַ�����
char *strcat(char *dest, char *src)
{
	int i, j;

	for (i = 0; dest[i] != '\0'; i++)
		;
	for (j = 0; src[j] != '\0'; i++, j++)
		dest[i] = src[j];
	dest[i] = '\0';
	return dest + i;
}

// misc_strncat(): ���� n ���ַ��� src �� dest �ַ�����
// ����ֵָ�� dest ������ src �ַ�����
char *strncat(char *dest, char *src, int n)
{
	int i, j;

	for (i = 0; dest[i] != '\0'; i++)
		;
	for (j = 0; j < n; i++, j++)
		dest[i] = src[j];
	dest[i] = '\0';
	return dest + i;
}

// misc_strchr(): ���ַ��� s �������ַ� c
// �ҵ� s �е�һ�� c �󷵻ظ��ַ��� s ��λ��ָ��
// �Ҳ������� NULL (void *)0
char *strchr(char *s, int c)
{
	int i;

	for (i = 0; s[i] != '\0'; i++)
		if (s[i] == c)
			return &s[i];
	return (void *)0;
}

// misc_strchr(): ���ַ��� s �з��������ַ� c
// �����ҵ� s �е�һ�� c �󷵻ظ��ַ��� s ��λ��ָ��
// �Ҳ������� NULL (void *)0
char *strrchr(char *s, int c)
{
	int i, len;

	len = strlen(s);
	for (i = len; s[i] != '\0'; i--)
		if (s[i] == c)
			return &s[i];
	return (void *)0;
}

// misc_strstr(): ���ַ��� s1 �������ַ��� s2
// �ҵ� s1 �е�һ��ƥ��� s2 �ַ����󷵻ظ��ַ����� s1 ����ʼλ��ָ��
// �Ҳ������� NULL (void *)0
char *strstr(char *s1, char *s2)
{
	int i, n;

	n = strlen(s2);
	for (i = 0; s1[i] != '\0'; i++)
		if (s1[i] == s2[0])
			if (strncmp(&s1[i], s2, n) == 0)
				return &s1[i];
	return (void *)0;
}



typedef enum {
	ZEROPAD = 1,	/* pad with zero */
	SIGN = 2,		/* unsigned/signed long */
	PLUS = 4,		/* show plus */
	SPACE = 8,		/* space if plus */
	LEFT = 16,		/* left justified */
	SPECIAL = 32,	/* 0x */
	SMALL = 64,		/* use 'abcdef' instead of 'ABCDEF' */
} FLAGS;

// ����ʮ���������ַ�
#define isdigit(c) ((c) >= '0' && (c) <= '9')

#define ishex(c) ((isdigit(c)) || ((c)>='A' && (c)<='F') || ((c)>='a' && (c)<='f'))

#define dodiv(n, base) ({	\
	int __res;	\
	__asm__(	\
		"divl %4"	\
		:"=a" (n),"=d" (__res)	\
		:"0" (n),"1" (0),"r" (base)	\
	);	\
	__res;	\
})

// atoi_s(): ����ת��ʮ�����޷��������ַ��� *s
// s -> skip ���Զ����� *s
int atoi_s(char **s)
{
	int i = 0;

	while (isdigit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}

unsigned long atol(const unsigned char *s)
{
        unsigned char *c = (unsigned char *)s;              /* current char */
        unsigned long total = 0;;         /* current total */

        while (isdigit(*c)) {
            total = 10 * total + (*c - '0');     /* accumulate digit */
            c = c++;    /* get next char */
        }

            return total;   /* return result, negated if necessary */
}



unsigned long hex2dec(const unsigned char *s)
{
	unsigned char *c = (unsigned char *)s;
	unsigned long total=0;
	unsigned char d = 0;
	int digitwidth = 0;

	while(ishex(*c) && (digitwidth++)<2*sizeof(long))
	{
		if( *c >='0' && *c <= '9')
		{
			d = '0';
		}

		if( *c >='a' && *c <= 'f')
		{
			d = 'a' - 10;
		}

		if( *c >='A' && *c <= 'F')
		{
			d = 'A' - 10;
		}

		total = 16 * total + (*c - d);
		c = c ++;
		
	}
	return total;
}




// number(): ָ�� base �������� num ������ buf ռλ siz �ֽ�
int number(char *buf, size_t siz, int num, int base, int precision, FLAGS flags)
{
	int i, n;
	char c, sign, tmp[36];
	char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	n = 0;
	if (flags & SMALL)
		digits = "0123456789abcdefghijklmnopqrstuvwxyz";
	if (flags & LEFT)
		flags &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return 0;
	c = (flags & ZEROPAD) ? '0' : ' ';
	if (flags & SIGN && num < 0) {
		sign = '-';
		num = -num;
	} else
		sign = (flags & PLUS) ? '+' : ((flags & SPACE) ? ' ' : 0);
	if (sign)
		siz--;
	if (flags & SPECIAL) {
		if (base == 16)
			siz -= 2;
		else if (base == 8)
			siz--;
	}
	i = 0;
	if (num == 0)
		tmp[i++]='0';
	else while (num != 0)
		tmp[i++] = digits[dodiv(num, base)];
	if (i > precision)
		precision = i;
	siz -= precision;
	if (!(flags & (ZEROPAD + LEFT)))
		while (siz-- > 0)
			buf[n++] = ' ';
	if (sign)
		buf[n++] = sign;
	if (flags & SPECIAL) {
		if (base == 8)
			buf[n++] = '0';
		else if (base == 16) {
			buf[n++] = '0';
			buf[n++] = digits[33];
		}
	}
	if (!(flags & LEFT))
		while (siz-- > 0)
			buf[n++] = c;
	while (i < precision--)
		buf[n++] = '0';
	while (i-- > 0)
		buf[n++] = tmp[i];
	while (siz-- > 0)
		buf[n++] = ' ';
	return n;
}

// vsnprintf(): ʹ�� fmt ��ʽ������ɱ�������� args �� siz �ֽڴ�С���� buf
// ���������������ַ���
///! �������� Linux �д����޸Ķ�����GPL not BSD
///? û���ϸ��� siz ���޲�
int vsnprintf(char *buf, size_t siz, char *fmt, va_list args)
{
	int i, n, len;
	char *p;
	char *s;
	int *ip;
	FLAGS flags;
	int width;	/* width of output field */
	int precision;		/* min. # of digits for integers; max number of chars for from string */
	int qualifier;		/* 'h', 'l', or 'L' for integer fields */

	n = 0;
	if (siz <= 0 || !buf)
		return 0;

	for (p = fmt; *p; p++) {
		if (n > siz - 1)
			break;

		if (*p != '%') {
			buf[n++] = *p;
			continue;
		}

		flags = 0;
		while (1)
			switch (*++p) {
			case '-': flags |= LEFT; break;
			case '+': flags |= PLUS; break;
			case ' ': flags |= SPACE; break;
			case '#': flags |= SPECIAL; break;
			case '0': flags |= ZEROPAD; break;
			default: goto prevend;
			}
prevend:

		width = -1;
		if (isdigit(*p))
			width = atoi_s(&p);
		else if (*p == '*') {
			width = va_arg(args, int);
			if (width < 0) {
				width = -width;
				flags |= LEFT;
			}
		}

		precision = -1;
		if (*p == '.') {
			p++;
			if (isdigit(*p))
				precision = atoi_s(&p);
			else if (*p == '*')
				precision = va_arg(args, int);
			if (precision < 0)
				precision = 0;
		}

		qualifier = -1;
		if (*p == 'h' || *p == 'l' || *p == 'L')
			qualifier = *p++;

		switch (*p) {
		case 'b':
			break;
		case 'c':
			if (!(flags & LEFT))
				while (--width > 0)
					buf[n++] = ' ';
			buf[n++] = (unsigned char)va_arg(args, int);
			while (--width > 0)
				buf[n++] = ' ';
			break;
		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			n += number(&buf[n], width, va_arg(args, unsigned long), 10, precision, flags);
			break;
		case 'n':
			ip = va_arg(args, int *);
			*ip = n;
			break;
		case 'o':
			n += number(&buf[n], width, va_arg(args, unsigned long), 8, precision, flags);
			break;
		case 'p':
			if (width == -1) {
				width = 8;
				flags |= ZEROPAD;
			}
			n += number(&buf[n], width, (unsigned long)va_arg(args, void *), 16, precision, flags);
			break;
		case 's':
			s = va_arg(args, char *);
			len = strlen(s);
			if (precision < 0)
				precision = len;
			else if (len > precision)
				len = precision;

			if (!(flags & LEFT))
				while (len < width--)
					buf[n++] = ' ';
			for (i = 0; i < len; ++i)
				buf[n++] = *s++;
			while (len < width--)
				buf[n++] = ' ';
			break;
		case 'x':
			flags |= SMALL;
		case 'X':
			n += number(&buf[n], width, va_arg(args, unsigned long), 16, precision, flags);
			break;
		default:
			if (*p != '%')
				buf[n++] = '%';
			if (*p)
				buf[n++] = *p;
			else
				--p;
			break;
		}
	}
//end:
	buf[n] = '\0';
	return n;
}

// misc_snprintf(): ʹ�� fmt ��ʽ������ɱ���� ... �� siz �ֽڴ�С���� buf
// ���������������ַ���
int snprintf(char *buf, size_t siz, char *fmt, ...)
{
	int i;
	va_list args;

	va_start(args, fmt);
	i = vsnprintf(buf, siz, fmt, args);
	va_end(args);
	return i;
}

// misc_buf: 1K ��С�ַ�����
char misc_buf[1024] = "\0";


stdputs pstdputs = 0;

void SetStd(stdputs std)
{
  pstdputs = std;
}


// misc_printf(): ʹ�� fmt ��ʽ������ɱ���� ... ��Ĭ�����
// ����������ַ���
int printf(char *fmt, ...)
{
	int i;
	va_list args;

	va_start(args, fmt);
	i = vsnprintf(misc_buf, sizeof(misc_buf), fmt, args);
	va_end(args);



	pstdputs(misc_buf);


	return i;
}

void dumpmem(unsigned char *p, unsigned long len)
{
	int byte_count = 0;

	char buf[8] = {0};
	char *pbuf = (char *)buf;
	long buf_c = 0;

	unsigned char *tmp_line_add;
	tmp_line_add = p;

	//misc_printf("cmd=%s, arg1=%s, arg2 = %s", argv[0],argv[1],argv[2]);
	
	printf("dump memory from 0x%x, %d bytes len:\n",p,len);


	while(len !=0)
	{

		//if newline
	  switch (byte_count)
	  {


		case 9:
		{
			//prrint ascii sections
			printf("   ");
			pbuf =(char *) buf;
			while(buf_c != 0)
			{
				if((*pbuf < 0x20) || (*pbuf  >0x7f))
				{
					printf(".");
				
				}else{

					if( *pbuf == '\\')
					{
						printf("\\");
						
					}else if( *pbuf == '\"')
					{
						printf("\"");
						
					}else if( *pbuf == '\'')
					{
						printf("\'");
						
					}else{
						
					printf("%c",*pbuf);
					}
				}
				buf_c--;
				pbuf++;
			}

			//end print ascii

			printf("\n");
			byte_count = 0;
			break;
		}


		case 0:
		{
			
			tmp_line_add = p;
			printf("0x%08x  ", tmp_line_add);
			byte_count = byte_count +1;
			break;
		}

		case 5:
	  	{
			printf("    ");
		}
		default:
		{

			

		printf("0x%02x ",(unsigned char)(*p));
		buf[buf_c] = *p;
		buf_c ++;
		byte_count++;
		p++;
		len--;
		break;
		}
	  }

	}



	
}

