#include "ym_common.h"

#define         IS_AF(c)                    ((c >= 'A') && (c <= 'F'))
#define         IS_af(c)                    ((c >= 'a') && (c <= 'f'))
#define         IS_09(c)                    ((c >= '0') && (c <= '9'))
#define         ISVALIDHEX(c)               IS_AF(c) || IS_af(c) || IS_09(c)
#define         ISVALIDDEC(c)               IS_09(c)
#define         CONVERTDEC(c)               (c - '0')
 
#define         CONVERTHEX_alpha(c)         (IS_AF(c) ? (c - 'A'+ 10) : (c - 'a'+ 10))
#define         CONVERTHEX(c)               (IS_09(c) ? (c - '0') : CONVERTHEX_alpha(c))

__GYMOPT            __GymClientOpt;
YMODEM_PACKET_BUF   ymClientPacketBuf;

static INT32U YMStrToInt(INT8U *pucStr, INT32U *puiValue)
{
    INT32U i;
    INT32U uiRes;
    INT32U uiVal;
    
    i = 0;
    uiRes = 0;
    uiVal = 0;
    
    if ((pucStr[0] == '0') && (pucStr[1] == 'x' || pucStr[1] == 'X')) {
        if (pucStr[2] == '\0') {
            return 0;
        }
        for (i = 2; i < 11; i++) {
            if (pucStr[i] == '\0') {
                *puiValue = uiVal;
                uiRes = 1;                                              /*  return 1;                   */
                break;
            }
            if (ISVALIDHEX(pucStr[i])) {
                uiVal = (uiVal << 4) + CONVERTHEX(pucStr[i]);
            } else {
                uiRes = 0;                                              /*  Return 0, Invalid input     */
                break;
            }
        }
        if (i >= 11) {                                                  /*  Over 8 digit hex --invalid  */
            uiRes = 0;
        }
    } else {                                                            /*  max 10-digit decimal input  */
        for (i = 0; i < 11; i++) {
            if (pucStr[i] == '\0') {
                *puiValue = uiVal;
                uiRes = 1;                                              /*  return 1                    */
                break;
            } else if ((pucStr[i] == 'k' || pucStr[i] == 'K') && (i > 0)) {
                uiVal = uiVal << 10;
                *puiValue = uiVal;
                uiRes = 1;
                break;
            } else if ((pucStr[i] == 'm' || pucStr[i] == 'M') && (i > 0)) {
                uiVal = uiVal << 20;
                *puiValue = uiVal;
                uiRes = 1;
                break;
            } else if (ISVALIDDEC(pucStr[i])) {
                uiVal = uiVal * 10 + CONVERTDEC(pucStr[i]);
            } else {
                uiRes = 0;                                              /*  return 0, Invalid input     */
                break;
            }
        }
        
        if (i >= 11) {                                                  /*  Over 10 digit decimal --invalid */
            uiRes = 0;
        }
    }
 
    return uiRes;
}

static INT32S YMParseInitPacket(INT8U *ucData)                          /*  解析第一包数据              */
{
    INT32U i, j;
    
    INT16U usCrc;
    INT8U  ucFileName[FILE_NAME_LENGTH];
    INT8U  ucFileSize[FILE_SIZE_LENGTH];
    
    if ((ucData[1] == 0x00) && (ucData[2] == 0xFF)) {
        usCrc = CalCRC16(&ucData[PACKET_HEAD_LENGTH], PACKET_SML_DAT_LENGTH);
        if ((INT16U)((ucData[PACKET_HEAD_LENGTH + PACKET_SML_DAT_LENGTH] << 8) | ucData[PACKET_HEAD_LENGTH + PACKET_SML_DAT_LENGTH + 1]) == usCrc) {
            for (i = 0; (ucData[i + PACKET_HEAD_LENGTH] != '\0') && (i < FILE_NAME_LENGTH); i++) {
                ucFileName[i] = ucData[ i + PACKET_HEAD_LENGTH];
            }
            
            for (j = 0, i = i + PACKET_HEAD_LENGTH + 1; (ucData[i] != '\0') && (j < FILE_NAME_LENGTH); j++, i++) {
                ucFileSize[j] = ucData[i];
            }
            
            /* 
             *  字符串转数字
             */
            
            return YM_SUCCESS;
        }
    }
    
    return -YM_CHECK_ERR;
}

static INT32S YMParseLastPacket(INT8U *ucData)                          /*  解析最后一包数据            */
{
    INT16U usCrc;
    
    if ((ucData[1] == 0x00) && (ucData[2] == 0xFF)) {
        usCrc = CalCRC16(&ucData[PACKET_HEAD_LENGTH], PACKET_SML_DAT_LENGTH);
        if ((INT16U)((ucData[PACKET_HEAD_LENGTH + PACKET_SML_DAT_LENGTH] << 8) | ucData[PACKET_HEAD_LENGTH + PACKET_SML_DAT_LENGTH + 1]) == usCrc) {
            
            return YM_SUCCESS;
        }
    }
    
    return -YM_CHECK_ERR;
}
                                                                        /*  解析中间包数据              */
static INT32S YMParsePacket(INT8U *ucData, INT8U ucFrameNo, INT32U uiPacketLen)   
{
    INT16U usCrc;
    
    if ((ucData[1] == ucFrameNo) && (((ucData[2] ^ 0xFF) & 0xFF) == ucFrameNo)) {
        usCrc = CalCRC16(&ucData[PACKET_HEAD_LENGTH], uiPacketLen - PACKET_HEAD_LENGTH - PACKET_TAIL_LENGTH);
        if ((INT16U)((ucData[uiPacketLen - 2] << 8) | ucData[uiPacketLen - 1]) == usCrc) {
            
            return YM_SUCCESS;
        }
    }
    
    return -YM_CHECK_ERR;
}

INT32S YMClientInit(void)
{
    if (NULL != YMODEM_BSP_TEMPLATE.Init) {
        return YMODEM_BSP_TEMPLATE.Init();
    }
    
    return -YM_FAIL;
}

INT32S YMClientReceive()
{
    INT32S  siRet;
    
    INT8U   ucSeqNo;
    INT8U   ucHandNum;
    INT8U   ucFrameDatBuf;

    INT32U  uiLen;
    INT32U  uiRecLen;
    
    INT32U  uiPacketSize;

    INT32U  uiRecErr;
    INT32U  uiNckTryCnt;
    INT32U  uiConnectCnt;
    
    INT8U   ucRecData[PACKET_BIG_DAT_LENGTH + PACKET_HEAD_LENGTH + PACKET_TAIL_LENGTH];
    
    YMODEM_BSP_API *pymbaBspApi;
    
    pymbaBspApi = (YMODEM_BSP_API *)&YMODEM_BSP_TEMPLATE;
    
    uiRecErr = 0;
    ucHandNum = 0;
    ucSeqNo = 0;
    do {
        ucFrameDatBuf = 'C';
        pymbaBspApi->Send(&ucFrameDatBuf, 1);
        
        sysIrqDisable();
        __GymClientOpt.ucTimeEnable = 1;                                /*  使能计时                    */
        __GymClientOpt.uiMsgTimeCnt = YM_C_OVERTIME;                    /*  超时时间                    */
        sysIrqEnable();
    
        do {
            siRet = pymbaBspApi->Receive(ucRecData, 1);
            if (siRet <= 0) {
                uiRecErr++;
            } else if ( (ucRecData[0] == YM_CMD_SOH) || 
                        (ucRecData[0] == YM_CMD_STX)) {
                break;
            } else {
                uiRecErr++;
            }
            OSTimeDly(1);
        } while((uiRecErr < YM_C_ERR_COUNT) && (__GymClientOpt.uiMsgTimeCnt > 0));
        
        if ((uiRecErr >= YM_C_ERR_COUNT) || (__GymClientOpt.uiMsgTimeCnt <= 0)) {
            uiRecErr = 0;                
            ucSeqNo++;
            if (ucSeqNo > 5) {
                break;
            }
            
            continue;
        }
        sysIrqDisable();
        __GymClientOpt.ucTimeEnable = 0;                                /*  禁止计时                    */ 
        sysIrqEnable();
        
        uiRecErr = 0; 
        uiRecLen = 0;     
        uiPacketSize = ((ucRecData[0] == YM_CMD_STX) ? PACKET_BIG_DAT_LENGTH : PACKET_SML_DAT_LENGTH) + \
                                                                                   PACKET_HEAD_LENGTH + \
                                                                                   PACKET_TAIL_LENGTH;
        
        uiLen = uiPacketSize - 1;
        
        sysIrqDisable();
        __GymClientOpt.ucTimeEnable = 1;                                /*  使能计时                    */
        __GymClientOpt.uiMsgTimeCnt = YM_C_OVERTIME;                    /*  计时                        */
        sysIrqEnable();
        
        do {
            uiRecLen += pymbaBspApi->Receive(ucRecData + uiRecLen + 1, uiLen - uiRecLen);
            if (uiRecLen == uiLen) {
                break;
            } else {
                uiRecErr++;
            }
            OSTimeDly(1);
        } while((uiRecErr < YM_C_ERR_COUNT) && (__GymClientOpt.uiMsgTimeCnt > 0));
        
        if (uiRecErr >= YM_C_ERR_COUNT) {
            return -YM_REC_CNT_ERR;
        }
        if (__GymClientOpt.uiMsgTimeCnt <= 0) {
            return -YM_TIME_OUT;
        }

        sysIrqDisable();
        __GymClientOpt.ucTimeEnable = 0;                                /*  禁止计时                    */ 
        sysIrqEnable();
        
        memcpy(&ymClientPacketBuf.ucPacket, ucRecData, uiPacketSize);
        
        siRet = YMParseInitPacket(ymClientPacketBuf.ucPacket);
        if (0 > siRet) {
            return siRet;
        }
        
        ucHandNum++;
        if (ucHandNum >= YM_C_HAND_NUM) {
            ucFrameDatBuf = YM_CMD_ACK;
            pymbaBspApi->Send(&ucFrameDatBuf, 1);
            ucFrameDatBuf = 'C';
            pymbaBspApi->Send(&ucFrameDatBuf, 1);
        }
    } while (ucHandNum < YM_C_HAND_NUM);
    
    if (ucSeqNo > 5) {
        return -YM_REC_CNT_ERR;
    }

    ucSeqNo = 1;
    uiRecErr = 0;
    uiNckTryCnt = 0;
    do {
        siRet = pymbaBspApi->Receive(ucRecData, 1);
        if (siRet <= 0) {
            uiRecErr++;
        } else if ( (ucRecData[0] == YM_CMD_SOH) || 
                    (ucRecData[0] == YM_CMD_STX)) {
                        
            uiRecErr = 0;
            uiRecLen = 0;   
            uiConnectCnt = 0;        
            uiPacketSize = ((ucRecData[0] == YM_CMD_STX) ? PACKET_BIG_DAT_LENGTH : PACKET_SML_DAT_LENGTH) + \
                                                                                       PACKET_HEAD_LENGTH + \
                                                                                       PACKET_TAIL_LENGTH;
            
            uiLen = uiPacketSize - 1;
                        
            sysIrqDisable();
            __GymClientOpt.ucTimeEnable = 1;                            /*  使能计时                    */
            __GymClientOpt.uiMsgTimeCnt = YM_C_OVERTIME;                /*  计时                        */
            sysIrqEnable();            
                        
            do {
                uiRecLen += pymbaBspApi->Receive(ucRecData + uiRecLen + 1, uiLen - uiRecLen);
                if (uiRecLen == uiLen) {
                    break;
                } else {
                    uiConnectCnt++;
                }
                OSTimeDly(1);
            } while((uiConnectCnt < YM_C_ERR_COUNT) && (__GymClientOpt.uiMsgTimeCnt > 0));
            
            if ((uiConnectCnt >= YM_C_ERR_COUNT) || (__GymClientOpt.uiMsgTimeCnt <= 0)) {
                uiNckTryCnt++;
                ucFrameDatBuf = YM_CMD_NCK;
                pymbaBspApi->Send(&ucFrameDatBuf, 1);
                    
                continue;
            }
                
            sysIrqDisable();
            __GymClientOpt.ucTimeEnable = 0;                            /*  禁止计时                    */ 
            sysIrqEnable();
            
            memcpy(&ymClientPacketBuf.ucPacket, ucRecData, uiPacketSize);
            
            siRet = YMParsePacket(ymClientPacketBuf.ucPacket, ucSeqNo, uiPacketSize);
            if (0 > siRet) {
                uiNckTryCnt++;
                ucFrameDatBuf = YM_CMD_NCK;
                pymbaBspApi->Send(&ucFrameDatBuf, 1);
            } else {
                ucSeqNo++;
                uiNckTryCnt = 0;
                ucFrameDatBuf = YM_CMD_ACK;
                pymbaBspApi->Send(&ucFrameDatBuf, 1);
            }
        } else if (ucRecData[0] == YM_CMD_EOT) {
            break;
        } else {
            uiRecErr++;
        }
        OSTimeDly(1);
    } while((uiRecErr < YM_C_ERR_COUNT) && (uiNckTryCnt < YM_C_NCK_TRY_CNT));
    
    if (uiRecErr >= YM_C_ERR_COUNT) {
            
        return -YM_REC_CNT_ERR;
    }
    if (uiNckTryCnt >= YM_C_NCK_TRY_CNT) {
            
        return -YM_NCK_TRY_CNT_ERR;
    }

    ucFrameDatBuf = YM_CMD_NCK;
    pymbaBspApi->Send(&ucFrameDatBuf, 1);

    sysIrqDisable();
    __GymClientOpt.ucTimeEnable = 1;                                    /*  使能计时                    */
    __GymClientOpt.uiMsgTimeCnt = YM_C_OVERTIME;                        /*  计时                        */
    sysIrqEnable();
    
    uiRecErr = 0;
    do {
        siRet = pymbaBspApi->Receive(ucRecData, 1);
        if (siRet <= 0) {
            uiRecErr++;
        } else if (ucRecData[0] == YM_CMD_EOT) {
            break;
        } else {
            uiRecErr++;
        }
        OSTimeDly(1);
    } while((uiRecErr < YM_C_ERR_COUNT) && (__GymClientOpt.uiMsgTimeCnt > 0));
    
    if (uiRecErr >= YM_C_ERR_COUNT) {   
        return -YM_REC_CNT_ERR;
    }
    if (0 == __GymClientOpt.uiMsgTimeCnt) {
        return -YM_TIME_OUT;
    }
    
    ucFrameDatBuf = YM_CMD_ACK;
    pymbaBspApi->Send(&ucFrameDatBuf, 1);
    ucFrameDatBuf = 'C';
    pymbaBspApi->Send(&ucFrameDatBuf, 1);

    sysIrqDisable();
    __GymClientOpt.ucTimeEnable = 1;                                    /*  使能计时                    */
    __GymClientOpt.uiMsgTimeCnt = YM_C_OVERTIME;                        /*  计时                        */
    sysIrqEnable();
    
    uiRecErr = 0;
    do {
        siRet = pymbaBspApi->Receive(ucRecData, 1);
        if (siRet <= 0) {
            uiRecErr++;
        } else if (ucRecData[0] == YM_CMD_SOH) {
            break;
        } else {
            uiRecErr++;
        }
        OSTimeDly(1);
    } while((uiRecErr < YM_C_ERR_COUNT) && (__GymClientOpt.uiMsgTimeCnt > 0));
    
    if (uiRecErr >= YM_C_ERR_COUNT) {   
        return -YM_REC_CNT_ERR;
    }
    if (0 == __GymClientOpt.uiMsgTimeCnt) {
        return -YM_TIME_OUT;
    }
    
    uiRecLen = 0;         
    uiPacketSize = PACKET_SML_DAT_LENGTH + PACKET_HEAD_LENGTH + PACKET_TAIL_LENGTH;
    
    uiLen = uiPacketSize - 1;
                
    sysIrqDisable();
    __GymClientOpt.ucTimeEnable = 1;                                    /*  使能计时                    */
    __GymClientOpt.uiMsgTimeCnt = YM_C_OVERTIME;                        /*  计时                        */
    sysIrqEnable();            
              
    uiRecErr = 0;
    do {
        uiRecLen += pymbaBspApi->Receive(ucRecData + uiRecLen + 1, uiLen - uiRecLen);
        if (uiRecLen == uiLen) {
            break;
        } else {
            uiRecErr++;
        }
        OSTimeDly(1);
    } while((uiRecErr < YM_C_ERR_COUNT) && (__GymClientOpt.uiMsgTimeCnt > 0));
    
    if (uiRecErr >= YM_C_ERR_COUNT) {   
        return -YM_REC_CNT_ERR;
    }
    if (0 == __GymClientOpt.uiMsgTimeCnt) {
        return -YM_TIME_OUT;
    }
        
    sysIrqDisable();
    __GymClientOpt.ucTimeEnable = 0;                                    /*  禁止计时                    */ 
    sysIrqEnable();
    
    memcpy(&ymClientPacketBuf.ucPacket, ucRecData, uiPacketSize);
    
    siRet = YMParseLastPacket(ymClientPacketBuf.ucPacket);
    if (0 == siRet) {
        ucFrameDatBuf = YM_CMD_ACK;
        pymbaBspApi->Send(&ucFrameDatBuf, 1);
        
        return YM_SUCCESS;
    }
    
    return -YM_FAIL;
}

void YMClientTick(void)
{
    if ((__GymClientOpt.ucTimeEnable > 0) && (__GymClientOpt.uiMsgTimeCnt > 0)) {
        if (1 == __GymClientOpt.uiMsgTimeCnt) {
            __GymClientOpt.ucTimeEnable = 0;
        }
        
        __GymClientOpt.uiMsgTimeCnt--;
    }                                   
}

