#include "ym_common.h"

//#define PACKET(x)           HEADR_##x##_AA
////##为连接符    

__GYMOPT      __GymServerOpt;

YMODEM_PACKET_BUF   ymServerPacketBuf;

static void YMIntToStr(INT8U *pucStr, INT32U uiIntNum)
{
    INT32U i, j;
    INT32U uiStatus, uiDiv;
    
    j = 0;
    uiDiv = 1000000000;
    uiStatus = 0;
    
    for (i = 0; i < 10; i++) {
        pucStr[j++] = (uiIntNum / uiDiv) + 48;
        
        uiIntNum %= uiDiv;
        uiDiv /= 10;
        
        if ((pucStr[j - 1] == '0') && (uiStatus == 0)) {
            j = 0;
        } else {
            uiStatus++;
        }   
    }
}

static void YMPrepareInitPacket(INT8U *pucDat, const char *pccFileName, INT32U uiFileSize) 
{
    INT16U i, j;
    INT16U usCrc;
    
    INT8U  ucFileSize[FILE_SIZE_LENGTH];
    
    pucDat[0] = YM_CMD_STX;
    pucDat[1] = 0x00;
    pucDat[2] = 0xFF;
    
    for (i = 0; (pccFileName[i] != '\0') && (i < FILE_NAME_LENGTH); i++) {
        pucDat[PACKET_HEAD_LENGTH + i] = pccFileName[i];
    }
    pucDat[PACKET_HEAD_LENGTH + i] = 0x00;
    //INT to STR 
    YMIntToStr(ucFileSize, uiFileSize);

    for (j = 0, i = PACKET_HEAD_LENGTH + i + 1; (ucFileSize[j] != '\0') && (j < FILE_SIZE_LENGTH);) {
        pucDat[i++] = ucFileSize[j++];
    }
    
    for (j = i; j < PACKET_BIG_DAT_LENGTH + PACKET_HEAD_LENGTH; j++) {
        pucDat[j] = 0x55;
    }
    
    usCrc = CalCRC16(&pucDat[PACKET_HEAD_LENGTH], PACKET_BIG_DAT_LENGTH);

    pucDat[PACKET_BIG_DAT_LENGTH + PACKET_HEAD_LENGTH]      = (INT8U)(usCrc >> 8);
    pucDat[PACKET_BIG_DAT_LENGTH + PACKET_HEAD_LENGTH + 1]  = (INT8U)(usCrc & 0xFF);
}

static void YMPreparePacket(INT8U *pucSourceBuf, INT8U *pucDat, INT8U ucPacketNo, INT32U uiBlockSize)
{
    INT32U i;
    INT32U uiSize;
    INT32U uiPacketSize;
    
    INT16U usCrc;
    
    INT8U *pucFileAddr;

    uiPacketSize = uiBlockSize < PACKET_SML_DAT_LENGTH ? PACKET_SML_DAT_LENGTH : PACKET_BIG_DAT_LENGTH;
    
    uiSize = uiBlockSize < uiPacketSize ? uiBlockSize : uiPacketSize;
    
    if (PACKET_BIG_DAT_LENGTH == uiPacketSize) {
        pucDat[0] = YM_CMD_STX;
    } else {
        pucDat[0] = YM_CMD_SOH;
    }
    pucDat[1] = ucPacketNo;
    pucDat[2] = ~ucPacketNo;
    
    pucFileAddr = pucSourceBuf;
    
    for (i = PACKET_HEAD_LENGTH; i < (PACKET_HEAD_LENGTH + uiSize); i++) {
        pucDat[i] = *pucFileAddr++;
    }
    
    if (uiSize < uiPacketSize) {
        for (i = uiSize + PACKET_HEAD_LENGTH; i < (uiPacketSize + PACKET_HEAD_LENGTH); i++) {
            pucDat[i] = 0x1A;
        }
    }
    
    usCrc = CalCRC16(&pucDat[PACKET_HEAD_LENGTH], uiPacketSize);

    pucDat[uiPacketSize + PACKET_HEAD_LENGTH]      = (INT8U)(usCrc >> 8);
    pucDat[uiPacketSize + PACKET_HEAD_LENGTH + 1]  = (INT8U)(usCrc & 0xFF);
}

static void YMPrepareLastPacket(INT8U *pucDat)
{
    INT16U i;
    
    INT16U usCrc;
    
    pucDat[0] = YM_CMD_SOH;
    pucDat[1] = 0x00;
    pucDat[2] = 0xFF;
    
    for (i = PACKET_HEAD_LENGTH; i < PACKET_SML_DAT_LENGTH + PACKET_HEAD_LENGTH; i++) {
        pucDat[i] = 0x00;
    }
    
    usCrc = CalCRC16(&pucDat[PACKET_HEAD_LENGTH], PACKET_SML_DAT_LENGTH);

    pucDat[PACKET_SML_DAT_LENGTH + PACKET_HEAD_LENGTH]      = (INT8U)(usCrc >> 8);
    pucDat[PACKET_SML_DAT_LENGTH + PACKET_HEAD_LENGTH + 1]  = (INT8U)(usCrc & 0xFF);
}

INT32S YMServerInit(void)
{
    if (NULL != YMODEM_BSP_TEMPLATE.Init) {
        return YMODEM_BSP_TEMPLATE.Init();
    }
    
    return -YM_FAIL;
}

INT32S YMServerTransmit(INT8U *pAddr, const char *pccFileName, INT32U uiFileSize)
{
    INT32S  siRet;

    INT8U   ucFrameNo;
    
    INT32U  uiLen;
    INT32U  uiSendLen;
    
    INT32U  uiPacketSize;

    INT32U  uiRecErr;
    INT8U   ucRecData[2];
    
    YMODEM_BSP_API *pymbaBspApi;
    
    pymbaBspApi = (YMODEM_BSP_API *)&YMODEM_BSP_TEMPLATE;

    sysIrqDisable();
    __GymServerOpt.ucTimeEnable = 1;                                    /*  使能计时                    */
    __GymServerOpt.uiMsgTimeCnt = YM_S_OVERTIME;                        /*  计时                        */
    sysIrqEnable();
    
    uiRecErr = 0;
    do {
        siRet = pymbaBspApi->Receive(ucRecData, 1);
        if (siRet <= 0) {
            uiRecErr++;
        } else if (ucRecData[0] == 'C') {
            break;
        } else {
            uiRecErr++;
        }
        OSTimeDly(500);
    } while ((uiRecErr < YM_S_CONNECT_NUM) && (__GymServerOpt.uiMsgTimeCnt > 0));

    if ((uiRecErr >= YM_S_CONNECT_NUM) || (__GymServerOpt.uiMsgTimeCnt <= 0)) {
        return -YM_TIME_OUT;
    }
    
    sysIrqDisable();
    __GymServerOpt.ucTimeEnable = 0;                                    /*  禁止计时                    */ 
    sysIrqEnable();
    
    uiSendLen = 0;
    YMPrepareInitPacket(ymServerPacketBuf.ucPacket, pccFileName, uiFileSize);
    uiLen  = PACKET_BIG_DAT_LENGTH + PACKET_HEAD_LENGTH + PACKET_TAIL_LENGTH;
    do {
        uiSendLen += pymbaBspApi->Send(ymServerPacketBuf.ucPacket, uiLen - uiSendLen);
        if (uiSendLen == uiLen) {            
            break; 
        }
        OSTimeDly(1);
    } while(1);

    sysIrqDisable();
    __GymServerOpt.ucTimeEnable = 1;                                    /*  使能计时                    */
    __GymServerOpt.uiMsgTimeCnt = YM_S_OVERTIME;                        /*  计时                        */
    sysIrqEnable();
    
    uiRecErr = 0;
    ucFrameNo = 0;
    do {
        siRet = pymbaBspApi->Receive(&ucRecData[ucFrameNo], 1);
        if (siRet <= 0) {
            uiRecErr++;
        } else if (ucRecData[0] == YM_CMD_ACK) {
            if (ucFrameNo > 0) {
                if (ucRecData[1] == 'C') {
                    break;
                } else {
                    uiRecErr++;
                }
            } else {
                ucFrameNo++;
            }
        } else {
            uiRecErr++;
        }
        OSTimeDly(1);
    } while((uiRecErr < YM_S_ERR_COUNT) && (__GymServerOpt.uiMsgTimeCnt > 0));
    
    if ((uiRecErr >= YM_S_ERR_COUNT) || (__GymServerOpt.uiMsgTimeCnt <= 0)) {
        return -YM_TIME_OUT;
    }   
        
    sysIrqDisable();
    __GymServerOpt.ucTimeEnable = 0;                                    /*  禁止计时                    */
    sysIrqEnable();
    
    ucFrameNo = 1;
    
    while (uiFileSize) {
        YMPreparePacket(pAddr, ymServerPacketBuf.ucPacket, ucFrameNo, uiFileSize);
        uiPacketSize = ymServerPacketBuf.ucPacket[0] == YM_CMD_SOH ? PACKET_SML_DAT_LENGTH : PACKET_BIG_DAT_LENGTH;
        
        uiLen = uiPacketSize + PACKET_HEAD_LENGTH + PACKET_TAIL_LENGTH;
        uiSendLen = 0;
        do {
            uiSendLen = pymbaBspApi->Send(ymServerPacketBuf.ucPacket, uiLen - uiSendLen);
            if (uiSendLen == uiLen) {            
                break; 
            }
            OSTimeDly(1);
        } while(1);
        
        uiRecErr = 0;
        
        sysIrqDisable();
        __GymServerOpt.ucTimeEnable = 1;                                /*  使能计时                    */
        __GymServerOpt.uiMsgTimeCnt = YM_S_OVERTIME;                    /*  计时                        */
        sysIrqEnable();
        
        do {
            siRet = pymbaBspApi->Receive(ucRecData, 1);
            if (siRet <= 0) {
                uiRecErr++;
            } else if ((ucRecData[0] == YM_CMD_ACK) || (ucRecData[0] == YM_CMD_NCK)) {
                break;
            } else {
                uiRecErr++;
            }
            OSTimeDly(1);
        } while((uiRecErr < YM_S_ERR_COUNT) && (__GymServerOpt.uiMsgTimeCnt > 0));
        
        if ((uiRecErr >= YM_S_ERR_COUNT) || (__GymServerOpt.uiMsgTimeCnt <= 0)) {
            return -YM_TIME_OUT;
        }     
            
        sysIrqDisable();
        __GymServerOpt.ucTimeEnable = 0;                                /*  禁止计时                    */
        sysIrqEnable();
        
        if (ucRecData[0] == YM_CMD_ACK) {
            pAddr += uiPacketSize;
            ucFrameNo ++;
            if (uiFileSize > uiPacketSize) {
                uiFileSize -= uiPacketSize;
            } else {
                uiFileSize = 0;
            }
        }
    }

    sysIrqDisable();
    __GymServerOpt.ucTimeEnable = 1;                                    /*  使能计时                    */
    __GymServerOpt.uiMsgTimeCnt = YM_S_OVERTIME;                        /*  计时                        */
    sysIrqEnable();
    
    OSTimeDly(1);
    ucFrameNo = YM_CMD_EOT;
    pymbaBspApi->Send(&ucFrameNo, 1);
    
    uiRecErr = 0;
    do {
        siRet = pymbaBspApi->Receive(ucRecData, 1);
        if (siRet <= 0) {
            uiRecErr++;
        } else if (ucRecData[0] == YM_CMD_NCK) {
            uiRecErr = 0;
            ucFrameNo = YM_CMD_EOT;
            pymbaBspApi->Send(&ucFrameNo, 1);
        } if (ucRecData[0] == YM_CMD_ACK) {
            break;
        } else {
            uiRecErr++;
        }
        OSTimeDly(1);
    } while((uiRecErr < YM_S_ERR_COUNT) && (__GymServerOpt.uiMsgTimeCnt > 0));
    
    if ((uiRecErr >= YM_S_ERR_COUNT) || (__GymServerOpt.uiMsgTimeCnt <= 0)) {
        return -YM_TIME_OUT;
    }
        
    sysIrqDisable();
    __GymServerOpt.ucTimeEnable = 1;                                    /*  使能计时                    */
    __GymServerOpt.uiMsgTimeCnt = YM_S_OVERTIME;                        /*  计时                        */
    sysIrqEnable();
    
    uiRecErr = 0;
    do {
        siRet = pymbaBspApi->Receive(ucRecData, 1);
        if (siRet <= 0) {
            uiRecErr++;
        } else if (ucRecData[0] == 'C') {
            break;
        } else {
            uiRecErr++;
        }
        OSTimeDly(1);
    } while((uiRecErr < YM_S_ERR_COUNT) && (__GymServerOpt.uiMsgTimeCnt > 0));
    
    if ((uiRecErr >= YM_S_ERR_COUNT) || (__GymServerOpt.uiMsgTimeCnt <= 0)) {
        return -YM_TIME_OUT;
    }
        
    sysIrqDisable();
    __GymServerOpt.ucTimeEnable = 0;                                    /*  禁止计时                    */ 
    sysIrqEnable();
    
    uiSendLen = 0;
    YMPrepareLastPacket(ymServerPacketBuf.ucPacket);
    uiLen = PACKET_SML_DAT_LENGTH + PACKET_HEAD_LENGTH + PACKET_TAIL_LENGTH;
    do {
        uiSendLen = pymbaBspApi->Send(ymServerPacketBuf.ucPacket, uiLen - uiSendLen);
        if (uiSendLen == uiLen) {            
            break; 
        }
        OSTimeDly(1);
    } while(1);

    sysIrqDisable();
    __GymServerOpt.ucTimeEnable = 1;                                    /*  使能计时                    */
    __GymServerOpt.uiMsgTimeCnt = YM_S_OVERTIME;                        /*  计时                        */
    sysIrqEnable();
    
    uiRecErr = 0;
    do {
        siRet = pymbaBspApi->Receive(ucRecData, 1);
        if (siRet <= 0) {
            uiRecErr++;
        } else if (ucRecData[0] == YM_CMD_ACK) {
            break;
        } else {
            uiRecErr++;
        }
        OSTimeDly(1);
    } while((uiRecErr < YM_S_ERR_COUNT) && (__GymServerOpt.uiMsgTimeCnt > 0));

    if ((uiRecErr >= YM_S_ERR_COUNT) || (__GymServerOpt.uiMsgTimeCnt <= 0)) {
        return -YM_TIME_OUT;
    }
        
    sysIrqDisable();
    __GymServerOpt.ucTimeEnable = 0;                                    /*  禁止计时                    */
    sysIrqEnable();
    
    return YM_SUCCESS;
}

void YMServerTick(void)
{
    if ((__GymServerOpt.ucTimeEnable > 0) && (__GymServerOpt.uiMsgTimeCnt > 0)) {
        if (1 == __GymServerOpt.uiMsgTimeCnt) {
            __GymServerOpt.ucTimeEnable = 0;
        }
        
        __GymServerOpt.uiMsgTimeCnt--;
    }                                   
}







