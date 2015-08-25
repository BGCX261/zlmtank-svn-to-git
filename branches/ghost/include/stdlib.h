#ifndef STDLIB_H
#define STDLIB_H



typedef int (*stdputs)(char *s);
void SetStd(stdputs std);


// 可变参数宏
///? C 语言中古怪的内容，一些列的宏代换
///? 此处内容建议多看看 C 参考手册
// va_list 是可变参数的默认类型
///? 为什么 va_list 是 char * 而不是 void *？
typedef char *va_list;

// 系统默认出入栈的数据大小，一般就是 ANSI C 的 int 大小
#define STACKITEMSIZE sizeof(int)
// 将可变参数对齐到栈项目数据大小
#define VA_SIZE(n) ((sizeof(n) + STACKITEMSIZE - 1) & ~(STACKITEMSIZE - 1))
// ap 是可变参数栈底，第一个参数，v 是最后一个参数
// 那么下面计算的 va_start 就是可变参数栈顶
// 由此可见 C 函数的入栈顺序，呵呵
#define va_start(ap,v) (ap = (va_list)&v + VA_SIZE(v))
// 取出其中一个参数，并转换类型。另外自增
// 因为自增了操作子 ap 所以取数返回的时候要减 VA_SIZE(t)
// 不然取的数是下一个参数，而不是当前参数
#define va_arg(ap,t) (*(t *)((ap += VA_SIZE(t)) - VA_SIZE(t)))
// 可变参数结束
// 很多 C 编译器自带的 stdarg.h 中将 va_end 定义为空，不操作
// 这里建议将操作子清零
#define va_end(ap) (ap = (va_list)0)


typedef int size_t;

// misc_memset(): 设置 siz 字节大小内存块 p 数据为 ch
void memset(void *p, unsigned char ch, size_t siz);

// misc_memcpy(): 复制 siz 字节内存块 src 到内存块 dest
void memcpy(void *dest, void *src, size_t siz);

int memcmp(void *dest, void *src, size_t siz);
// misc_strlen(): 返回以 '\0' 结尾字符串 s 的长度
int strlen(char *s);
// misc_strcmp(): 比较字符串 s1 和 s2
// 相等时返回 0 否则以 ASCII 排列顺序返回 1 或者 -1
int strcmp(char *s1, char *s2);
// misc_strncmp(): 比较 n 个字节长度字符串 s1 和 s2
// 返回值同 misc_strcmp() 但字符串可以不以 '\0'
int strncmp(char *s1, char *s2, int n);
// misc_strcpy(): 复制字符串 src 到 dest
// siz 为 dest 的空间大小
// 返回值指向 dest 复制完 src 字符串后
// 因为可能造成溢出，不推荐使用本函数
char *strcpy(char *dest, char *src);
// misc_strncpy(): 复制 n 个字节字符串 src 到 dest
// 返回值指向 dest 复制完 src 字符串后
char *strncpy(char *dest, char *src, int n);
// misc_strcat(): 复制字符串 src 到 dest 字符串后
// 返回值指向 dest 复制完 src 字符串后
char *strcat(char *dest, char *src);
// misc_strncat(): 复制 n 个字符串 src 到 dest 字符串后
// 返回值指向 dest 复制完 src 字符串后
char *strncat(char *dest, char *src, int n);
// misc_strchr(): 在字符串 s 中搜索字符 c
// 找到 s 中第一个 c 后返回该字符在 s 的位置指针
// 找不到返回 NULL (void *)0
char *strchr(char *s, int c);
// misc_strchr(): 在字符串 s 中反相搜索字符 c
// 反相找到 s 中第一个 c 后返回该字符在 s 的位置指针
// 找不到返回 NULL (void *)0
char *strrchr(char *s, int c);
// misc_strstr(): 在字符串 s1 中搜索字符串 s2
// 找到 s1 中第一个匹配的 s2 字符串后返回该字符串在 s1 的起始位置指针
// 找不到返回 NULL (void *)0
char *strstr(char *s1, char *s2);

int vsnprintf(char *buf, size_t siz, char *fmt, va_list args);

// misc_snprintf(): 使用 fmt 格式化输出可变参数 ... 到 siz 字节大小缓存 buf
// 返回输出到缓存的字符数
int snprintf(char *buf, size_t siz, char *fmt, ...);

// misc_printf(): 使用 fmt 格式化输出可变参数 ... 到默认输出
// 返回输出的字符数
int printf(char *fmt, ...);

void dumpmem(unsigned char *p, unsigned long len);

int atoi_s(char **s);
unsigned long atol(const unsigned char *s);

unsigned long hex2dec(const unsigned char *s);


#endif
