#ifndef __YM_CLIENT_H
#define __YM_CLIENT_H

#define         YM_C_HAND_NUM               2u                          /*  Client����ǰ���ִ���        */
#define         YM_C_OVERTIME               10000u                      /*  ����������Чʱ��            */
#define         YM_C_ERR_COUNT              5000u                       /*  ����������Чʱ��            */
#define         YM_C_NCK_TRY_CNT            10u                         /*  ������ӦNCK������         */

extern INT32S YMClientInit(void);
extern INT32S YMClientReceive();
extern void YMClientTick(void);

#endif
