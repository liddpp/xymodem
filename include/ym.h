#ifndef __YM_H
#define __YM_H

#define         YM_SERVER_EN                0                           /*
                                                                         * (0)  YM Client
                                                                         * (1)  YM Server
                                                                         */
#define         YM_SUCCESS                  0u                          /*  成功                        */
#define         YM_FAIL                     1u                          /*  失败                        */                                                 
#define         YM_TIME_OUT                 2u                          /*  超时                        */
#define         YM_CHECK_ERR                3u                          /*  校验错误                    */
#define         YM_REC_CNT_ERR              4u                          /*  接收次数超出最大值          */
#define         YM_NCK_TRY_CNT_ERR          5u                          /*  无回应重试次数超出最大值    */

#define         YM_CMD_SOH                  0x01u                       /*  128字节启动命令             */
#define         YM_CMD_STX                  0x02u                       /*  1024字节启动命令            */
#define         YM_CMD_EOT                  0x04u                       /*  传输结束命令                */
#define         YM_CMD_ACK                  0x06u                       /*  应答指令                    */
#define         YM_CMD_NCK                  0x15u                       /*  无应答指令                  */
#define         YM_CMD_CANCEL               0x18u                       /*  取消指令                    */
#define         YM_CMD_STRC                 0x43u                       /*  字符串ASCII码               */

#define         PACKET_HEAD_LENGTH          3u                          /*  数据帧头长度                */
#define         PACKET_TAIL_LENGTH          2u                          /*  数据帧尾长度                */

#define         PACKET_SML_DAT_LENGTH       128u                        /*  短帧数据长度                */
#define         PACKET_BIG_DAT_LENGTH       1024u                       /*  长帧数据长度                */

#define         FILE_NAME_LENGTH            256u                        /*  文件名最大长度              */
#define         FILE_SIZE_LENGTH            16u                         /*  文件大小字符串最大长度      */


typedef struct __gymopt {                                               /*  Ymodem OPT                  */
    volatile INT8U  ucTimeEnable;                                       /*  计时使能开关                */
    volatile INT32U uiMsgTimeCnt;                                       /*  计时计数值                  */
} __GYMOPT;

#pragma pack(1)

typedef union {
    INT8U ucPacket[PACKET_BIG_DAT_LENGTH + PACKET_HEAD_LENGTH + PACKET_TAIL_LENGTH];
                                                                        /*  数据帧整帧长度              */
    struct {
        INT8U   ucPacketHead[PACKET_HEAD_LENGTH];                       /*  数据帧头部缓冲区            */
        INT32U  uiPacketDat[PACKET_BIG_DAT_LENGTH / 4];                 /*  数据帧数据缓冲区            */
        INT8U   ucPacketTail[PACKET_TAIL_LENGTH];                       /*  数据帧尾部缓冲区            */
    } segment;
} YMODEM_PACKET_BUF;

#pragma pack()

extern INT32S YMInit(void);
                                                                                                                                                                                                                                                                                                     
#if YM_SERVER_EN

extern INT32S YMTransmit(INT8U *pAddr, const char *pccFileName, INT32U uiFileSize);

#else

extern INT32S YMReceive(void);

#endif    

extern void YMTick(void);
                                                                         




































#endif
