#include "ym_common.h"

static signed char YMODEM_BSP_Init         (void);

static INT32S YMODEM_BSP_Send               (INT8U      *p_dat, 
                                            INT16U      cnt);

static INT32S YMODEM_BSP_Recive             (INT8U      *p_dat,      
                                            INT16U      cnt);

static signed char YMODEM_BSP_Rsv          (void);


const YMODEM_BSP_API  YMODEM_BSP_TEMPLATE = {
    YMODEM_BSP_Init,
    YMODEM_BSP_Send,
    YMODEM_BSP_Recive,
    YMODEM_BSP_Rsv,
};


static signed char YMODEM_BSP_Init (void)
{
    return 0;
}

static INT32S YMODEM_BSP_Send (INT8U      *p_dat, 
                               INT16U      cnt)
{
    return sysUartWrite(UART0, p_dat, cnt);
}

static INT32S YMODEM_BSP_Recive (INT8U      *p_dat,
                                 INT16U      cnt)
{
    return sysUartRead(UART0, p_dat, cnt);
}

static signed char YMODEM_BSP_Rsv (void)
{
    return 0;
}























































































