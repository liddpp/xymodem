#ifndef __YM_CLIENT_H
#define __YM_CLIENT_H

#define         YM_C_HAND_NUM               2u                          /*  Client启动前握手次数        */
#define         YM_C_OVERTIME               10000u                      /*  握手启动有效时间            */
#define         YM_C_ERR_COUNT              5000u                       /*  握手启动有效时间            */
#define         YM_C_NCK_TRY_CNT            10u                         /*  连续回应NCK最大次数         */

extern INT32S YMClientInit(void);
extern INT32S YMClientReceive();
extern void YMClientTick(void);

#endif
