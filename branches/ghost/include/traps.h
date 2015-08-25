

#ifndef _TRAPS_H
#define _TRAPS_H

// traps_init(): 初始化异常处理程序设置
extern void traps_init(void);

// traps_dodivide(): 产生除零错误代码
extern void traps_dodivide(void);

#endif
 
