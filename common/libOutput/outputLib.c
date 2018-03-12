#include <stdio.h>
#include <sys/klog.h>
#include <utmp.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h>
#include "outputLib.h"


INT32 g_OutputMsgQId = 0;
char *pServBuf = NULL;

static char *OutputLib_GetModNameByModNum(UINT32 uiModNum);
static void  OutputLib_GetSysTime(char *);
static INT32 OutputLib_TakeOutputEvent(void);

/*模块名-模块号对应实例*/
ST_MOD_NUM_TO_NAME g_apModInfo[MAX_MOD_NUM + 1] =
{
    { MIN_MOD_NUM       , "Error" },
    { SAMPLE_MOD_NUM    , "sample" },
    { SYSMONITOR_MOD_NUM, "sysMon"},
    { MAX_MOD_NUM       , "Error" }
};


/*******************************************************************
**函数名:  OutputLib_Signal_Handler
**输入: 注册信号
**输出:
**描述:
**作者:
**日期:
*********************************************************************/
static void OutputLib_Signal_Handler(int nSignal)
{
    switch(nSignal)
    {
        case SIGTERM:
            break;
        case SIGKILL:
            break;
        default:
            break;
    }

    OTLB_FREE(pServBuf);
    closelog();
    exit(-1);
}

/********************************************************************
**函数名: OutputLib_Client_Init
**输入:  无
**输出:  OTLB_RT_OK --- 成功
**       其他值 --- 失败
**描述:  初始化客户端模块，获取队列描述符
**作者:
**日期:
**********************************************************************/
static INT32 OutputLib_Client_Init()
{
    UINT16 nCount = 100;

    while(nCount > 0)
    {
        /*客户端只能GET，不能进行create*/
        /*如果GET不成功，则一直循环Get，循环GET10S后放弃*/
        if((g_OutputMsgQId = msgget((key_t)OUTPUT_MSGQ_KEY, 0666)) == -1)
        {
            usleep(100 * 1000);
            nCount --;
//            syslog(LOG_ERR , "OutputLibClient_Init get message failed" );
        }
        else
        {
            break;
        }
    }

    /*如果连续100次都失败了，则返回错误*/
    if(0 == nCount)
    {
        return OTLB_RT_MSGQ_ERROR;
    }

    return OTLB_RT_OK;
}

/********************************************************************
**函数名: OutputLib_Server_Init
**输入:  无
**输出:  OTLB_RT_OK --- 成功
**       其他值 --- 失败
**描述:  初始化客户端模块，获取队列描述符
**作者:
**日期:
**********************************************************************/
INT32 OutputLib_Server_Init()
{
    UINT16 nCount = 100;

    while(nCount > 0)
    {
        /*如果GET不成功，则一直循环Get，循环GET10S后放弃*/
        if((g_OutputMsgQId = msgget((key_t)OUTPUT_MSGQ_KEY, 0666 | IPC_CREAT)) == -1)
        {
            usleep(100 * 1000);
            nCount --;
            syslog(LOG_ERR , "OutputTask creats msgQ failed");
        }
        else
        {
            break;
        }
    }

    /*如果连续100次都失败了，则返回错误*/
    if(0 == nCount)
    {
        return OTLB_RT_MSGQ_ERROR;
    }

    return OTLB_RT_OK;
}


/********************************************************************
**函数名: OutputLib_AddOutputEvent
**输入:  ST_LOG_ENTRY *pLogEntry  --- 需要添加的输出项
**输出:  OTLB_RT_OK --- 成功
**       其他--- 失败
**描述:  输出项的添加接口
**作者:
**日期:
**********************************************************************/
static INT32 OutputLib_AddOutputEvent(const void *pLog)
{
    INT32 iRet = 0, iRetryTime = 0;
    ST_LOG_ENTRY * pLogEntry = NULL;

    if(NULL == pLog)
    {
        return OTLB_RT_INVALID_PARAM;
    }

    pLogEntry = (ST_LOG_ENTRY *)pLog;

    while(iRetryTime < 2)
    {
        /*采用消息队列来实现,若发送出错需重发一次*/
        /* 注意发送数据长度,否则接收端无法处理,错误信息:Argument list too long*/
        if(msgsnd(g_OutputMsgQId, pLogEntry, (sizeof(ST_LOG_ENTRY) + pLogEntry->uiLen), IPC_NOWAIT) == -1)
        {
            iRet = errno;
            /* 判断错误码是否是队列描述符无效, 如果是则重新获取队列描述符*/
            syslog(LOG_ERR, "OutputLib send data stream failed: Reason :%s", strerror(errno));

            if(1 == iRetryTime)
                break;
            else
                iRetryTime++;

            if((iRet == EIDRM) || (iRet == EINVAL))
            {
                if(OutputLib_Client_Init() != OTLB_RT_OK)
                {
                    syslog(LOG_ERR, "OutputLib_AddOutputEvent OutputLib_Client_Init failed!");
                    return OTLB_RT_MSGQ_ERROR;
                }

                continue;
            }

            if(1 == iRetryTime)
                continue;

            return OTLB_RT_MSGQ_ERROR;
        }

        iRetryTime = 2;
    }

    return OTLB_RT_OK;
}


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
INT32 OutputLib_OutputInfo(UINT32 level, UINT32 uiModId, const char *fmt, ...)
{
    char cBuffer[OTLB_INFO_BUFFER_SIZE] = {'\0'};
    ST_LOG_ENTRY *pLogEntry = NULL;
    va_list args;
    size_t infoLen = 0;

    /*入参检测*/
    if((level <= OTLB_LOG_MIN) || (level >= OTLB_LOG_MAX) || (uiModId <= MIN_MOD_NUM) || (uiModId >= MAX_MOD_NUM) || (NULL == fmt))
    {
        return OTLB_RT_INVALID_PARAM;
    }

    if(OutputLib_Client_Init() != OTLB_RT_OK)
    {
        syslog(LOG_ERR, "OutputLib_OutputInfo OutputLib_Client_Init failed!");
        return OTLB_RT_MSGQ_ERROR;
    }

    va_start(args, fmt);
    vsnprintf(cBuffer + sizeof(ST_LOG_ENTRY), (OTLB_INFO_BUFFER_SIZE - sizeof(ST_LOG_ENTRY)), fmt, args);
    va_end(args);
    /*保证字符串结尾有\n 存在*/
    infoLen = strlen(cBuffer + sizeof(ST_LOG_ENTRY));

    if(infoLen < OTLB_INFO_STRING_SIZE)
    {
        if(cBuffer[sizeof(ST_LOG_ENTRY) + infoLen - 1] != '\n')
        {
            cBuffer[sizeof(ST_LOG_ENTRY) + infoLen] = '\n';
            infoLen++;
        }
    }
    else
    {
        cBuffer[sizeof(ST_LOG_ENTRY) + infoLen - 1] = '\n';
    }

    /*拷贝头部数据*/
    pLogEntry = (ST_LOG_ENTRY *)cBuffer;
    pLogEntry->uiLevel = level;
    pLogEntry->uiModId = uiModId;
    pLogEntry->uiLen = strlen(cBuffer + sizeof(ST_LOG_ENTRY));

    /*增加事件队列*/
    if(OutputLib_AddOutputEvent(pLogEntry) != OTLB_RT_OK)
    {
        return OTLB_RT_ERROR;
    }

    return OTLB_RT_OK;
}

/********************************************************************
**函数名: OutputLib_OutputData
**输入:  UINT32 level --- 日志级别
**       UINT32 uiModId --- 调用此接口的模块号
**       const UINT8 *pName --- 指示当前打印的数据流内容
**       UINT32  len --- 数据流的长度
**输出:  OTLB_RT_OK --- 成功
**       其他--- 失败
**描述:  将数据流转换成16进制和ASCII字符打印输出
**       pData最长为1835字节，当超出1833字节时，‘}’会被覆盖
**作者:
**日期:
**********************************************************************/
INT32 OutputLib_OutputData(UINT32 level, UINT32 uiModId, const char *pName, const unsigned char *pData, const UINT32  len)
{
    ST_LOG_ENTRY *pLogEntry = NULL;
    UINT32  i = 0, j = 0;
    size_t dataLen = 0;

    INT32 nDataLenRet = 0;
    INT32 nLeftDataLen = OTLB_DATA_STREAM_BUFFER_SIZE;
    char * pBuffer = NULL;
    char *pDataStream = NULL;
    unsigned char c = 0;

    /*len 长度限制无效，删除*/
    if((level >= OTLB_LOG_MAX) || (level <= OTLB_LOG_MIN) || (uiModId >= MAX_MOD_NUM) || (uiModId <= MIN_MOD_NUM) || (NULL == pName) || (NULL == pData) || (0 == len))
    {
        return OTLB_RT_INVALID_PARAM;
    }

    if(OutputLib_Client_Init() != OTLB_RT_OK)
    {
        syslog(LOG_ERR, "OutputLib_OutputData OutputLib_Client_Init failed!");
        return OTLB_RT_MSGQ_ERROR;
    }

    if((pBuffer = OTLB_MALLOC(OTLB_DATA_STREAM_BUFFER_SIZE)) == NULL)
    {
        return OTLB_RT_MEM_ERROR;
    }

    memset(pBuffer, '\0', OTLB_DATA_STREAM_BUFFER_SIZE);
    pDataStream = pBuffer;

    /* 跳过头部 */
    pDataStream += sizeof(ST_LOG_ENTRY);
    nLeftDataLen -= sizeof(ST_LOG_ENTRY);
    /*填充输出信息*/
    nDataLenRet = snprintf(pDataStream, nLeftDataLen, "[%s]:\n{\n   ", pName);
    /* 这里不需要作返回值判断 */
    pDataStream += nDataLenRet;
    nLeftDataLen -= nDataLenRet;

    for(i = 0; i < len; i++)
    {
        nDataLenRet = snprintf(pDataStream, nLeftDataLen, "%02x ", (unsigned char)pData[i]);

        /*当返回值大于nLeftDataLen 表示缓冲区已满，剩余缓冲区大小应为0*/
        if(nDataLenRet >= nLeftDataLen)
        {
            /*实际传入的字节大小*/
            pDataStream += nLeftDataLen - 1;
            nLeftDataLen = 0;
            break;
        }
        else
        {
            pDataStream += nDataLenRet;
            nLeftDataLen -= nDataLenRet;
        }

        /*打印ASCII 字符*/
        if(((i + 1) % 16 == 0) && (i != 0))
        {
            nDataLenRet = snprintf(pDataStream, nLeftDataLen, "   ");

            /*当返回值大于nLeftDataLen 表示缓冲区已满，剩余缓冲区大小应为0*/
            if(nDataLenRet >= nLeftDataLen)
            {
                /*实际传入的字节大小*/
                pDataStream += nLeftDataLen - 1;
                nLeftDataLen = 0;
                break;
            }
            else
            {
                pDataStream += nDataLenRet;
                nLeftDataLen -= nDataLenRet;
            }

            for(; j <= i; j++)
            {
                if(32 < pData[j] && pData[j] < 126)
                {
                    c = pData[j];
                }
                else
                {
                    c = '.';
                }

                nDataLenRet = snprintf(pDataStream, nLeftDataLen, "%c", c);

                /*当返回值大于nLeftDataLen 表示缓冲区已满，剩余缓冲区大小应为0*/
                if(nDataLenRet >= nLeftDataLen)
                {
                    /*实际传入的字节大小*/
                    pDataStream += nLeftDataLen - 1;
                    nLeftDataLen = 0;
                    break;
                }
                else
                {
                    pDataStream += nDataLenRet;
                    nLeftDataLen -= nDataLenRet;
                }
            }

            nDataLenRet = snprintf(pDataStream, nLeftDataLen, "\n   ");

            /*当返回值大于nLeftDataLen 表示缓冲区已满，剩余缓冲区大小应为0*/
            if(nDataLenRet >= nLeftDataLen)
            {
                /*实际传入的字节大小*/
                pDataStream += nLeftDataLen - 1;
                nLeftDataLen = 0;
                break;
            }
            else
            {
                pDataStream += nDataLenRet;
                nLeftDataLen -= nDataLenRet;
            }
        }
    }

    /*保证最后一行打印16个字节数据*/
    if(i % 16 != 0)
    {
        int k = i;

        /*打印对齐*/
        while(k % 16 != 0)
        {
            nDataLenRet = snprintf(pDataStream, nLeftDataLen, "   ");

            /*当返回值大于nLeftDataLen 表示缓冲区已满，剩余缓冲区大小应为0*/
            if(nDataLenRet >= nLeftDataLen)
            {
                /*实际传入的字节大小*/
                pDataStream += nLeftDataLen - 1;
                nLeftDataLen = 0;
                break;
            }
            else
            {
                pDataStream += nDataLenRet;
                nLeftDataLen -= nDataLenRet;
            }

            k++;
        }

        /*打印ASSCI字符*/
        nDataLenRet = snprintf(pDataStream, nLeftDataLen, "   ");

        /*当返回值大于nLeftDataLen 表示缓冲区已满，剩余缓冲区大小应为0*/
        if(nDataLenRet >= nLeftDataLen)
        {
            /*实际传入的字节大小*/
            pDataStream += nLeftDataLen - 1;
            nLeftDataLen = 0;
        }
        else
        {
            pDataStream += nDataLenRet;
            nLeftDataLen -= nDataLenRet;
        }

        for(; j < i; j++)
        {
            if(32 < pData[j] && pData[j] < 126)
            {
                c = pData[j];
            }
            else
            {
                c = '.';
            }

            nDataLenRet = snprintf(pDataStream, nLeftDataLen, "%c", c);

            /*当返回值大于nLeftDataLen 表示缓冲区已满，剩余缓冲区大小应为0*/
            if(nDataLenRet >= nLeftDataLen)
            {
                /*实际传入的字节大小*/
                pDataStream += nLeftDataLen - 1;
                nLeftDataLen = 0;
                break;
            }
            else
            {
                pDataStream += nDataLenRet;
                nLeftDataLen -= nDataLenRet;
            }
        }

        nDataLenRet = snprintf(pDataStream, nLeftDataLen, "\n");

        /*当返回值大于nLeftDataLen 表示缓冲区已满，剩余缓冲区大小应为0*/
        if(nDataLenRet >= nLeftDataLen)
        {
            /*实际传入的字节大小*/
            pDataStream += nLeftDataLen - 1;
            nLeftDataLen = 0;
        }
        else
        {
            pDataStream += nDataLenRet;
            nLeftDataLen -= nDataLenRet;
        }
    }

    /* 字符串连接结束，这里不需要作返回值判断 */
    if(nLeftDataLen > 1)
    {
        snprintf(pDataStream, nLeftDataLen, "}\n");
    }

    /* 保证字符串结尾有\n 存在*/
    dataLen = strlen(pBuffer + sizeof(ST_LOG_ENTRY));

    if(dataLen < OTLB_DATA_STREAM_STRING_SIZE)
    {
        if(*(pBuffer + sizeof(ST_LOG_ENTRY) + dataLen - 1) != '\n')
        {
            *(pBuffer + sizeof(ST_LOG_ENTRY) + dataLen) = '\n';
            dataLen++;
        }
    }
    else
    {
        *(pBuffer + sizeof(ST_LOG_ENTRY) + dataLen - 1) = '\n';
    }

    /*拷贝头部数据*/
    pLogEntry = (ST_LOG_ENTRY *)pBuffer;
    pLogEntry->uiLevel = level;
    pLogEntry->uiModId = uiModId;
    pLogEntry->uiLen = strlen(pBuffer + sizeof(ST_LOG_ENTRY));

    /*增加事件队列*/
    if(OutputLib_AddOutputEvent(pLogEntry) != OTLB_RT_OK)
    {
        if(NULL != pLogEntry)
        {
            OTLB_FREE(pLogEntry);
            pLogEntry = NULL;
        }

        return OTLB_RT_ERROR;
    }

    OTLB_FREE(pLogEntry);
    pLogEntry = NULL;
    return OTLB_RT_OK;
}

/********************************************************************
**函数名: OutputLib_Init
**输入: 无
**输出: OTLB_RT_OK --- 成功
**      其他--- 失败
**描述: 初始化服务端模块
**作者:
**日期:
**********************************************************************/
INT32 OutputLib_Init()
{
    /*采用消息队列来实现*/
    if(OutputLib_Server_Init() != OTLB_RT_OK)
    {
        syslog(LOG_ERR, "OutputLib_Init creats msgQ failed!");
        return OTLB_RT_MSGQ_ERROR;
    }

    /*初始化syslog 连接*/
    openlog("Output_Task", LOG_CONS | LOG_PID, 0);

    /*创建任务*/
    if(OTLB_RT_MSGQ_ERROR == OutputLib_TakeOutputEvent())
    {
        closelog();
        return OTLB_RT_MSGQ_ERROR;
    }

    return OTLB_RT_OK;
}

/********************************************************************
**函数名: OutputLib_TerminalWrite
**输入:  unsigned int level  --- 日志级别
**       char *log  --- 日志信息
**       unsigned int length  --- 日志的长度
**输出:
**描述:  输出到终端的封装函数
**作者:
**日期:
**********************************************************************/
static INT32 OutputLib_TerminalWrite(const UINT32  level, const UINT32 uiModNum,
                                     const char *log, UINT32  length)
{
    struct utmp *pUtmp = NULL;
    UINT16  usTtyNameLen = 32;
    char strTtyName[32] = {'\0'};
    char strInfoHead[INFO_HEADER_LEN] = {'\0'};
    char pSysTime[12] = {'\0'};
    int fd = 0;
    INT32 nLogStrOffset = 0;

    if(NULL == log || 0 == length)
    {
        return OTLB_RT_ERROR;
    }

    OutputLib_GetSysTime(pSysTime);
    snprintf(strInfoHead, INFO_HEADER_LEN, "###%s#%s ", OutputLib_GetModNameByModNum(uiModNum), pSysTime);
    setutent();

    pUtmp = getutent();
    while(pUtmp != NULL)
    {
        if (pUtmp->ut_type == LOGIN_PROCESS || pUtmp->ut_type == USER_PROCESS)
        {
            snprintf(strTtyName, usTtyNameLen, "/dev/%s", pUtmp->ut_line);

            if((fd = open(strTtyName, O_RDWR | O_NONBLOCK)) == -1)
            {
                continue;
            }

            fcntl(fd, O_NONBLOCK);

            if(-1 == write(fd, strInfoHead, INFO_HEADER_LEN))
            {
                syslog(LOG_ERR, "Write data to terminal Failed !\n");
                syslog(LOG_ERR, "Write data to terminal Failed:error: %s !\n", strerror(errno));
                close(fd);
                continue;
            }

            nLogStrOffset = 0;

            while(nLogStrOffset < length)
            {
                if((length - nLogStrOffset) >= DATA_TO_TERMIN_LEN)
                {
                    if(-1 == write(fd, log + nLogStrOffset, DATA_TO_TERMIN_LEN))
                    {
                        syslog(LOG_ERR, "Write Failed: Reason:%s!\n ", strerror(errno));
                        break;
                    }

                    usleep(10000);
                    nLogStrOffset += DATA_TO_TERMIN_LEN;
                }
                else
                {
                    if(-1 == write(fd, log + nLogStrOffset, (length - nLogStrOffset)))
                    {
                        syslog(LOG_ERR, "Write Failed: Reason:%s!\n", strerror(errno));
                    }

                    nLogStrOffset = length;
                }
            }

            close(fd);
        }
        pUtmp = getutent();
    }

    endutent();
    return OTLB_RT_OK;
}

/********************************************************************
**函数名: OutputLib_TakeOutputEvent
**输入:
**输出:
**描述:  服务端处理消息
**作者:
**日期:
**********************************************************************/
static INT32 OutputLib_TakeOutputEvent(void)
{
    UINT32 nRet = 0;
    ST_LOG_ENTRY * pSerLogEntry = NULL;
    /*注册一个SIGTERM 信号，OutputTask进程收到此信号便释放内存并退出*/
    signal(SIGTERM, OutputLib_Signal_Handler);
    signal(SIGKILL, OutputLib_Signal_Handler);

    if((pServBuf = OTLB_MALLOC(OTLB_DATA_STREAM_BUFFER_SIZE)) == NULL)
    {
        return OTLB_RT_MEM_ERROR;
    }

    pSerLogEntry = (ST_LOG_ENTRY *)pServBuf;

    while(1)
    {
        memset(pServBuf, '\0', OTLB_DATA_STREAM_BUFFER_SIZE);

        /*提取输出项*/
        if((nRet = msgrcv(g_OutputMsgQId, pServBuf, OUTPUT_TASK_MAX_RECV_BYTE, 0, 0)) == -1)
        {
            /*如果提取不到消息队列, 记录syslog*/
            syslog(LOG_ERR, "OutputTask recieve message failed!Msg Id %d,  ERR Code %s", g_OutputMsgQId, strerror(errno));

            /*先删除消息队列, 再重新新建*/
            if(g_OutputMsgQId > 0)
            {
                msgctl(g_OutputMsgQId, IPC_RMID, 0);
            }

            /*重新创建消息队列*/
            if(OutputLib_Server_Init() != OTLB_RT_OK)
            {
                syslog(LOG_ERR, "OutputTask recreats msgQ failed!");
                break;
            }

            continue;
        }

        /*判断输出级别是否合法*/
        if((pSerLogEntry->uiLevel < OTLB_LOG_MIN) || (pSerLogEntry->uiLevel > OTLB_LOG_MAX))
        {
            continue;
        }

        /*判断模块号是否合法*/
        if((pSerLogEntry->uiModId < MIN_MOD_NUM) || (pSerLogEntry->uiModId > MAX_MOD_NUM))
        {
            continue;
        }

        /*输出到终端*/
        if(OTLB_RT_ERROR == OutputLib_TerminalWrite(pSerLogEntry->uiLevel, pSerLogEntry->uiModId, pServBuf + sizeof(ST_LOG_ENTRY), pSerLogEntry->uiLen))
        {
            continue;
        }

        /*如果输出级别是OTLB_LOG_EMERG，则记录日志*/
        if(OTLB_LOG_EMERG == pSerLogEntry->uiLevel)
        {
            syslog(LOG_ERR, pServBuf + sizeof(ST_LOG_ENTRY));
        }
    }

    if(NULL != pServBuf)
        OTLB_FREE(pServBuf);

    pServBuf = NULL;
    return OTLB_RT_MSGQ_ERROR;
}

/********************************************************************
**函数名: OutputLib_GetModNameByModNum
**输入:  UINT32 uiModNum---模块号
**输出:  NULL --- 获取模块名失败
**       字符串--- 获取到的模块名
**描述:  通过模块号获取模块名
**作者:
**日期:
**********************************************************************/
static char *OutputLib_GetModNameByModNum(UINT32 uiModNum)
{
    if((uiModNum <= MIN_MOD_NUM) || (uiModNum >= MAX_MOD_NUM))
        return NULL;
    else
        return g_apModInfo[uiModNum].ucModName;
}
/********************************************************************
**函数名: OutputLib_GetSysTime
**输入:  无
**输出:  NULL ---获取系统时间失败
**       字符串--- 获取到的系统时间
**描述:  获取系统当前时间
**作者:
**日期:
**********************************************************************/
static void OutputLib_GetSysTime(char * pTimeStr)
{
    struct tm *pTime;
    time_t pTimeVal;
    time(&pTimeVal);

    if((pTime = localtime(&pTimeVal)) == NULL)
        return;

    sprintf(pTimeStr, "[%02d:%02d:%02d]", pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
    return;
}

