//
// Created by hc on 2020/5/13.
//

#ifndef C20_JT1078HEADER_H
#define C20_JT1078HEADER_H
#include <stdint.h>

#define JT1078_MAX_LENGTH 950           //每个包的最大长度包头+包体不超过这个数字,为了留出一些空间,实际上编码包头+包体长度必须不大于这个值(_PKG_MAX_LENGTH-1000)

#define HEAD_BUFSIZE       sizeof(JT_1078_HEADER)         //包头大小

//分包处理标记
#define RCV_ATOMIC_PACKAGE  0x00
#define RCV_FIRST_PACKAGE   0x01
#define RCV_LAST_PACKAGE    0x02
#define RCV_MIDDLE_PACKAGE  0x03
#define FIRST_RECEIVE_BYTES 16

#define PKG_HD_INIT                0          //初始状态，准备接收数据包头前15字节
#define PKG_HD_RECVING             1          //接收包头中，包头不完整，继续接收中
#define PKG_HD_REMAINING_INIT      2          //后续包头字节
#define PKG_HD__REMAINING_RECVING  3          //
#define PKG_BD_INIT                4          //包头刚好收完，准备接收包体
#define PKG_BD_RECVING             5          //接收包体中，包体不完整，继续接收中，处理后直接回到_PKG_HD_INIT状态
#define PKG_RV_FINISHED            6          //完整包收完


//一些和网络通讯相关的结构
//包头结构
#pragma pack (1)

typedef struct
{
    uint32_t      DWFramHeadMark;           //帧头标识
    uint8_t       V2:2;                     //固定为2
    uint8_t       P1:1;                     //固定为0
    uint8_t       X1:1;                     //RTP头是否需要扩展位，固定为0
    uint8_t       CC4:4;                    //固定为1
    uint8_t       M1:1;                     //标志位，确定是否是完整数据帧的边界
    uint8_t       PT7:7;                    //负载类型
    uint16_t      WdPackageSequence;        //RTP数据包序号每发送一个RTP数据包序列号加1
    uint8_t       BCDSIMCardNumber[6];      //SIM卡号
    uint8_t       Bt1LogicChannelNumber;    //逻辑通道号
    uint8_t       DataType4:4;              //数据类型
    uint8_t       SubpackageHandleMark4:4;  //分包处理标记
    uint64_t      Bt8timeStamp;             //时间戳
    uint16_t      WdLastIFrameInterval;     //与上一帧的时间间隔
    uint16_t      WdLastFrameInterval;      //与上一帧的时间间隔
    uint16_t      WdBodyLen;                //数据体长度

}JT_1078_HEADER;
#pragma pack()

#endif //C20_JT1078HEADER_H
