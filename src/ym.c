#include "ym_common.h"

INT32S YMInit(void)
{
#if YM_SERVER_EN
    YMServerInit();
#else
    YMClientInit();
#endif
}

#if YM_SERVER_EN
INT32S YMTransmit(INT8U *pAddr, const char *pccFileName, INT32U uiFileSize)
{
    return YMServerTransmit(pAddr, pccFileName, uiFileSize);
}

#else 

INT32S YMReceive(void)
{
    return YMClientReceive();
}
#endif

void YMTick(void)
{
#if YM_SERVER_EN
    YMServerTick();
#else
    YMClientTick();
#endif
}




