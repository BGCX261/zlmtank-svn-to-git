#ifndef STDLIB_H
#define STDLIB_H



typedef int (*stdputs)(char *s);
void SetStd(stdputs std);


// �ɱ������
///? C �����йŹֵ����ݣ�һЩ�еĺ����
///? �˴����ݽ���࿴�� C �ο��ֲ�
// va_list �ǿɱ������Ĭ������
///? Ϊʲô va_list �� char * ������ void *��
typedef char *va_list;

// ϵͳĬ�ϳ���ջ�����ݴ�С��һ����� ANSI C �� int ��С
#define STACKITEMSIZE sizeof(int)
// ���ɱ�������뵽ջ��Ŀ���ݴ�С
#define VA_SIZE(n) ((sizeof(n) + STACKITEMSIZE - 1) & ~(STACKITEMSIZE - 1))
// ap �ǿɱ����ջ�ף���һ��������v �����һ������
// ��ô�������� va_start ���ǿɱ����ջ��
// �ɴ˿ɼ� C ��������ջ˳�򣬺Ǻ�
#define va_start(ap,v) (ap = (va_list)&v + VA_SIZE(v))
// ȡ������һ����������ת�����͡���������
// ��Ϊ�����˲����� ap ����ȡ�����ص�ʱ��Ҫ�� VA_SIZE(t)
// ��Ȼȡ��������һ�������������ǵ�ǰ����
#define va_arg(ap,t) (*(t *)((ap += VA_SIZE(t)) - VA_SIZE(t)))
// �ɱ��������
// �ܶ� C �������Դ��� stdarg.h �н� va_end ����Ϊ�գ�������
// ���ｨ�齫����������
#define va_end(ap) (ap = (va_list)0)


typedef int size_t;

// misc_memset(): ���� siz �ֽڴ�С�ڴ�� p ����Ϊ ch
void memset(void *p, unsigned char ch, size_t siz);

// misc_memcpy(): ���� siz �ֽ��ڴ�� src ���ڴ�� dest
void memcpy(void *dest, void *src, size_t siz);

int memcmp(void *dest, void *src, size_t siz);
// misc_strlen(): ������ '\0' ��β�ַ��� s �ĳ���
int strlen(char *s);
// misc_strcmp(): �Ƚ��ַ��� s1 �� s2
// ���ʱ���� 0 ������ ASCII ����˳�򷵻� 1 ���� -1
int strcmp(char *s1, char *s2);
// misc_strncmp(): �Ƚ� n ���ֽڳ����ַ��� s1 �� s2
// ����ֵͬ misc_strcmp() ���ַ������Բ��� '\0'
int strncmp(char *s1, char *s2, int n);
// misc_strcpy(): �����ַ��� src �� dest
// siz Ϊ dest �Ŀռ��С
// ����ֵָ�� dest ������ src �ַ�����
// ��Ϊ���������������Ƽ�ʹ�ñ�����
char *strcpy(char *dest, char *src);
// misc_strncpy(): ���� n ���ֽ��ַ��� src �� dest
// ����ֵָ�� dest ������ src �ַ�����
char *strncpy(char *dest, char *src, int n);
// misc_strcat(): �����ַ��� src �� dest �ַ�����
// ����ֵָ�� dest ������ src �ַ�����
char *strcat(char *dest, char *src);
// misc_strncat(): ���� n ���ַ��� src �� dest �ַ�����
// ����ֵָ�� dest ������ src �ַ�����
char *strncat(char *dest, char *src, int n);
// misc_strchr(): ���ַ��� s �������ַ� c
// �ҵ� s �е�һ�� c �󷵻ظ��ַ��� s ��λ��ָ��
// �Ҳ������� NULL (void *)0
char *strchr(char *s, int c);
// misc_strchr(): ���ַ��� s �з��������ַ� c
// �����ҵ� s �е�һ�� c �󷵻ظ��ַ��� s ��λ��ָ��
// �Ҳ������� NULL (void *)0
char *strrchr(char *s, int c);
// misc_strstr(): ���ַ��� s1 �������ַ��� s2
// �ҵ� s1 �е�һ��ƥ��� s2 �ַ����󷵻ظ��ַ����� s1 ����ʼλ��ָ��
// �Ҳ������� NULL (void *)0
char *strstr(char *s1, char *s2);

int vsnprintf(char *buf, size_t siz, char *fmt, va_list args);

// misc_snprintf(): ʹ�� fmt ��ʽ������ɱ���� ... �� siz �ֽڴ�С���� buf
// ���������������ַ���
int snprintf(char *buf, size_t siz, char *fmt, ...);

// misc_printf(): ʹ�� fmt ��ʽ������ɱ���� ... ��Ĭ�����
// ����������ַ���
int printf(char *fmt, ...);

void dumpmem(unsigned char *p, unsigned long len);

int atoi_s(char **s);
unsigned long atol(const unsigned char *s);

unsigned long hex2dec(const unsigned char *s);


#endif
