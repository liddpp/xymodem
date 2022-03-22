#ifndef __YM_SERVER_H
#define __YM_SERVER_H

#define         YM_S_OVERTIME               10000u                      /*  握手启动有效时间            */
#define         YM_S_CONNECT_NUM            80u                         /*  尝试握手启动次数            */
#define         YM_S_ERR_COUNT              5000u                       /*  尝试等待回应次数            */

extern INT32S YMServerInit(void);
extern INT32S YMServerTransmit(INT8U *pAddr, const char *pccFileName, INT32U uiFileSize);
extern void YMServerTick(void);


#endif
