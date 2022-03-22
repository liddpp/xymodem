#ifndef __YM_BSP_H
#define __YM_BSP_H

typedef struct {
    signed char (*Init)         (void);
    
    INT32S      (*Send)          (INT8U      *p_dat, 
                                 INT16U      cnt);
    
    INT32S      (*Receive)       (INT8U      *p_dat,
                                 INT16U      cnt);
    
    signed char (*Rsv)          (void);
    
} YMODEM_BSP_API;

extern const YMODEM_BSP_API  YMODEM_BSP_TEMPLATE;



#endif
