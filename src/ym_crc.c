#include "ym_common.h"

static INT16U UpdateCRC16(INT16U usCrc, INT8U ucByte)
{
    INT32U uiCrcTemp = usCrc;    
    INT32U uiInData  = ucByte | 0x100;
 
    do {
        uiCrcTemp <<= 1;
        uiInData  <<= 1; 
        if (uiInData & 0x100) {
            ++uiCrcTemp;
        }            
        if (uiCrcTemp & 0x10000) {
            uiCrcTemp ^= 0x1021;
        }          
    } while (!(uiInData & 0x10000));
 
    return uiCrcTemp & 0xFFFFu;
}

INT16U CalCRC16(const INT8U* pucData, INT32U uiSize)
{
    INT16U usCrc = 0;
    
    const INT8U* pucDataEnd = pucData + uiSize;
 
    while (pucData < pucDataEnd) {
        usCrc = UpdateCRC16(usCrc, *pucData++);
    }
    
    usCrc = UpdateCRC16(usCrc, 0);
    usCrc = UpdateCRC16(usCrc, 0);
 
    return usCrc & 0xFFFFu;
}





































































