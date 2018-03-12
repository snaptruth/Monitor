#ifndef OUTPUTLIB_H
#define OUTPUTLIB_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <syslog.h>
#include "common.h"

/*output任务的消息队列号*/
#define OUTPUT_MSGQ_KEY (1)
/*模块名最大长度*/
/*名字定义太长，实际使用最长为5字符加上一个结束符，结构体成员共6字节*/
#define MAX_MOD_NAME_LEN 6
/*错误码的返回值定义*/
#define     OTLB_RT_OK                  0
#define     OTLB_RT_ERROR            (-1)
#define     OTLB_RT_INVALID_PARAM    (-2)
#define     OTLB_RT_MEM_ERROR        (-3)
#define     OTLB_RT_EVENT_QUEUE_FULL (-4)
#define     OTLB_RT_MSGQ_ERROR       (-5)

/*系统操作宏定义*/
#define OTLB_MALLOC malloc
#define OTLB_FREE   free

/*******************************************************************************
**服务端保存信息头数组长度定义,20个字符加一个结束符
**输出格式:打印信息与信息头之间有一个空格，格式如下：
**         ###Ascpd#[07:15:59] "打印信息"
********************************************************************************/
#define INFO_HEADER_LEN (21)

/*定义每次写入终端的最大数据长度*/
#define DATA_TO_TERMIN_LEN (256)
/*输出级别的定义*/
enum em_output_level
{
    OTLB_LOG_MIN = 0,
    OTLB_LOG_DEBUG,
    OTLB_LOG_EMERG,
    OTLB_LOG_MAX
};

/*模块的定义*/
enum em_ap_module
{
    MIN_MOD_NUM = 0,
    SAMPLE_MOD_NUM,
    SYSMONITOR_MOD_NUM,
    MAX_MOD_NUM
};
/*模块号与模块名对应*/
typedef struct st_mod_num_to_name
{
    UINT32 uiModNum;
    char ucModName[MAX_MOD_NAME_LEN];
} ST_MOD_NUM_TO_NAME;
/*输出信息头部*/
typedef struct st_log_entry
{
    UINT32 uiLevel;       /*日志级别*/
    UINT32 uiModId;       /*发出日志的模块号*/
    UINT32 uiLen;         /*日志长度*/
} ST_LOG_ENTRY;

/*系统全局参数宏定义*/
#define OTLB_INFO_BUFFER_SIZE (2048)
/*除去ST_LOG_ENTRY 和结束符，纯信息的长度*/
#define OTLB_INFO_STRING_SIZE (OTLB_INFO_BUFFER_SIZE-sizeof(ST_LOG_ENTRY)-1)

/*处理1500字节的网络报文发送缓冲区*/
#define OTLB_DATA_STREAM_BUFFER_SIZE (8192)
/*除去ST_LOG_ENTRY 和结束符，纯信息的长度*/
#define OTLB_DATA_STREAM_STRING_SIZE (OTLB_DATA_STREAM_BUFFER_SIZE-sizeof(ST_LOG_ENTRY)-1)

/*处理1500字节的网络报文接收缓冲区*/
#define OUTPUT_TASK_MAX_RECV_BYTE (8192)


/********************************************************************
**函数名:  OutputLib_OutputInfo
**输入:  UINT32 level ---  输出级别
**       UINT32 uiModId --- 调用此接口的模块号
**       const UINT8 *fmt --- 输出信息格式化字符串
**输出:  OTLB_RT_OK --- 成功
**       其他--- 失败
**描述:  解释性信息输出接口
**作者:
**日期:
**********************************************************************/
extern INT32 OutputLib_OutputInfo(UINT32 level, UINT32 uiModId, const char *fmt, ...);

/********************************************************************
**函数名: OutputLib_OutputData
**输入:  UINT32 level --- 日志级别
**       UINT32 uiModId --- 调用此接口的模块号
**       const UINT8 *pName --- 指示当前打印的数据流内容
**       UINT32 len --- 数据流的长度
**输出:  OTLB_RT_OK --- 成功
**       其他--- 失败
**描述:  将数据流转换成16进制和ASCII字符打印输出
**作者:
**日期:
**********************************************************************/
extern INT32 OutputLib_OutputData(UINT32 level, UINT32 uiModId, const char *pName, const unsigned char *pData, const UINT32  len);

/********************************************************************
**函数名: OutputLib_Init
**输入: 无
**输出: OTLB_RT_OK --- 成功
**      其他--- 失败
**描述: 初始化服务端模块
**作者:
**日期:
**********************************************************************/
extern INT32 OutputLib_Init();

#ifdef  __cplusplus
}
#endif

#endif
