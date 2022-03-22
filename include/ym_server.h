#ifndef __YM_SERVER_H
#define __YM_SERVER_H

#define         YM_S_OVERTIME               10000u                      /*  ����������Чʱ��            */
#define         YM_S_CONNECT_NUM            80u                         /*  ����������������            */
#define         YM_S_ERR_COUNT              5000u                       /*  ���Եȴ���Ӧ����            */

extern INT32S YMServerInit(void);
extern INT32S YMServerTransmit(INT8U *pAddr, const char *pccFileName, INT32U uiFileSize);
extern void YMServerTick(void);


#endif
