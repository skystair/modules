#ifndef __modefunc_h__
#define __modefunc_h__

typedef enum{
    modech_reset = 0,
    modech_idle,
    modech_func1,
    modech_func2,
    modech_MAX,
}modefuncCh;

typedef struct{
    modefuncCh state;
    modefuncCh statelast;

    unsigned int  tick;
}modestruct;

extern modestruct modestr;

void modefunc_init(void);
void modefunc_tick1ms(void);
void modefunc_func(void);

void modefunc_setState(unsigned char newState);

#endif
