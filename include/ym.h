#ifndef __YM_H
#define __YM_H

#define         YM_SERVER_EN                0                           /*
                                                                         * (0)  YM Client
                                                                         * (1)  YM Server
                                                                         */
#define         YM_SUCCESS                  0u                          /*  �ɹ�                        */
#define         YM_FAIL                     1u                          /*  ʧ��                        */                                                 
#define         YM_TIME_OUT                 2u                          /*  ��ʱ                        */
#define         YM_CHECK_ERR                3u                          /*  У�����                    */
#define         YM_REC_CNT_ERR              4u                          /*  ���մ����������ֵ          */
#define         YM_NCK_TRY_CNT_ERR          5u                          /*  �޻�Ӧ���Դ����������ֵ    */

#define         YM_CMD_SOH                  0x01u                       /*  128�ֽ���������             */
#define         YM_CMD_STX                  0x02u                       /*  1024�ֽ���������            */
#define         YM_CMD_EOT                  0x04u                       /*  �����������                */
#define         YM_CMD_ACK                  0x06u                       /*  Ӧ��ָ��                    */
#define         YM_CMD_NCK                  0x15u                       /*  ��Ӧ��ָ��                  */
#define         YM_CMD_CANCEL               0x18u                       /*  ȡ��ָ��                    */
#define         YM_CMD_STRC                 0x43u                       /*  �ַ���ASCII��               */

#define         PACKET_HEAD_LENGTH          3u                          /*  ����֡ͷ����                */
#define         PACKET_TAIL_LENGTH          2u                          /*  ����֡β����                */

#define         PACKET_SML_DAT_LENGTH       128u                        /*  ��֡���ݳ���                */
#define         PACKET_BIG_DAT_LENGTH       1024u                       /*  ��֡���ݳ���                */

#define         FILE_NAME_LENGTH            256u                        /*  �ļ�����󳤶�              */
#define         FILE_SIZE_LENGTH            16u                         /*  �ļ���С�ַ�����󳤶�      */


typedef struct __gymopt {                                               /*  Ymodem OPT                  */
    volatile INT8U  ucTimeEnable;                                       /*  ��ʱʹ�ܿ���                */
    volatile INT32U uiMsgTimeCnt;                                       /*  ��ʱ����ֵ                  */
} __GYMOPT;

#pragma pack(1)

typedef union {
    INT8U ucPacket[PACKET_BIG_DAT_LENGTH + PACKET_HEAD_LENGTH + PACKET_TAIL_LENGTH];
                                                                        /*  ����֡��֡����              */
    struct {
        INT8U   ucPacketHead[PACKET_HEAD_LENGTH];                       /*  ����֡ͷ��������            */
        INT32U  uiPacketDat[PACKET_BIG_DAT_LENGTH / 4];                 /*  ����֡���ݻ�����            */
        INT8U   ucPacketTail[PACKET_TAIL_LENGTH];                       /*  ����֡β��������            */
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
