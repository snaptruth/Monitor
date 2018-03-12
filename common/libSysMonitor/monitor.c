#include "monitor.h"
#include "taskLib.h"
//#include <sys/timer.h>
#include "errCode.h"
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/syscall.h>
/*
 * 全局变量
 */

MONITOR_APP_INFO g_MonitorAppInfoTable[] =
{
//    {MONITOR_PROC_RESTART,     90000,    90000,    "PSM",  MSG_SRC_PSM,   "/usr/bin/psm&",   "reboot" , 0, 0, 0, 0, 0, {{0}, 0, 0}},
    {MONITOR_PROC_RESTART,     60000,    60000,    "sample",  MSG_SRC_SAMPLE,   "killall sample; /tmp/bin/sample &",   "reboot" , 0, 0, 0, 0, 0, {{0}, 0, 0}},
    // {MONITOR_PROC_RESTART,     90000,    90000,    "CLI",  MSG_SRC_CLI,   "/usr/bin/cli&",   "reboot" , 0, 0, 0, 0, 0, {{0}, 0, 0}},
//    {MONITOR_PROC_RESTART,     90000,    90000,    "BM",   MSG_SRC_BM ,   "/usr/bin/bm&" ,   "reboot"  , 0, 0, 0, 0, 0, {{0}, 0, 0}},
};

key_t sysMonitorMsgkey = SYSTEM_MONITOR_RCV_MSG_QUEUE_KEY;

INT32 monitorMsgQueue = 0;
int fpgaFd = 0;
watchDogPtr watchDogFun = NULL;

/**************************************************************************
** 函数名:
** 输  入:
** 输  出:
** 描  述:
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
void showMonitor()
{
    UINT32 uiCount;
    const char szFormat[] = {"%-2d    %-12s    0x%-8x    %-8d    %-8d    0x%-8x    0x%-8x    0x%-8x    0x%-8x\n\r"};

    /*遍历打印当前的任务列表*/
    SYSMONITOR_DEBUG_ERR("\nId    Name          ThreadId      RegCnt    LstRegCnt      rstCnt        IdChCnt        Time1        Time2\n\r");
    SYSMONITOR_DEBUG_ERR("------------------------------------------------------------------------------------------------------\n\r");

    INT32 tableNum = sizeof(g_MonitorAppInfoTable) / sizeof(MONITOR_APP_INFO);

    for(uiCount = 0; uiCount < tableNum; uiCount++)
    {
        MONITOR_APP_INFO *appInfo = g_MonitorAppInfoTable + uiCount;

        INT32 lastestIndex = 0 ;
        INT32 curIndex     = 0 ;
        if(appInfo->resetTimeTable.resetNum >= RECORD_RESET_TIME_MUX)
        {
            lastestIndex = appInfo->resetTimeTable.resetIndex;
            curIndex     = appInfo->resetTimeTable.resetIndex == 0 ? RECORD_RESET_TIME_MUX - 1 : appInfo->resetTimeTable.resetIndex - 1;
        }
        else
        {
            lastestIndex = appInfo->resetTimeTable.resetIndex - 2;
            curIndex     = appInfo->resetTimeTable.resetIndex - 1;
        }

        unsigned int time1 = 0;
        unsigned int time2 = 0;
        if(curIndex >=0 )
        {
            time1 = (unsigned int)appInfo->resetTimeTable.resetTime[curIndex];
        }
        if(lastestIndex >= 0)
        {
            time2 = (unsigned int)appInfo->resetTimeTable.resetTime[lastestIndex];
        }

        SYSMONITOR_DEBUG_ERR(szFormat, appInfo->msgSrc, appInfo->pAppName, (unsigned int)appInfo->pid,
                              appInfo->appRegCount, appInfo->lastAppRegCount,  appInfo->restartAppCount, appInfo->appIdChangeTimes,
                              time1, time2);
    }
    SYSMONITOR_DEBUG_ERR("------------------------------------------------------------------------------------------------------\n\r");
    return ;
}
/**************************************************************************
** 函数名:
** 输  入:
** 输  出:
** 描  述:
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
INT32 initMonitorMsgque()
{
    monitorMsgQueue = msgget(sysMonitorMsgkey, IPC_CREAT | 0660);
    if(monitorMsgQueue != -1)
    {
        return SYS_RC_OK;
    }
    SYSMONITOR_DEBUG_ERR("%s:msgget err%d,%s\n", __func__, errno, strerror(errno));
    return SYS_RC_ERR;
}

/**************************************************************************
** 函数名: sendDbgMsg
** 输  入:
** 输  出:
** 描  述: 本地发送调试命令到voice进程
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
INT32 sendMonitorMsg(UCHAR msgSrc)
{
    INT32 result;

    MONITOR_MSG sendMonitorMsg ;
    sendMonitorMsg.type = MONITOR_MSG_TYPE;
    sendMonitorMsg.payload.msgSrc = msgSrc;
    sendMonitorMsg.payload.pidNum = getpid();

    result = msgsnd(monitorMsgQueue, &sendMonitorMsg, sizeof(sendMonitorMsg.payload), IPC_NOWAIT);
    if(0 != result)
    {
        SYSMONITOR_DEBUG_ERR("%s: err:%d, %s \n",  __func__, errno, strerror(errno));
        return SYS_RC_ERR;
    }

    return SYS_RC_OK;
}
/**************************************************************************
** 函数名:
** 输  入:
** 输  出:
** 描  述:
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
INT32 receMonitorMsg()
{
    INT32 result = 0;
    MONITOR_MSG receMonitorMsg;
    receMonitorMsg.type = MONITOR_MSG_TYPE;

    result = msgrcv(monitorMsgQueue, &receMonitorMsg, sizeof(MONITOR_MSG), 0, 0);
    if(result == -1)
    {
        SYSMONITOR_DEBUG_ERR("%s: err:%d,%s\n", __func__, errno, strerror(errno));
        return SYS_RC_ERR;
    }
    else
    {
//        SYSMONITOR_DEBUG_INFO("%s: type:%d,payload src:%d,payload pid:0x%x\n", __func__, receMonitorMsg.type, receMonitorMsg.payload.msgSrc, receMonitorMsg.payload.pidNum);
    }

    int i = 0;
    INT32 tableNum = sizeof(g_MonitorAppInfoTable) / sizeof(MONITOR_APP_INFO);
    for(i = 0; i < tableNum; i++)
    {
        MONITOR_APP_INFO *appInfo = g_MonitorAppInfoTable + i;

        //找到注册的任务
        if(receMonitorMsg.payload.msgSrc == appInfo->msgSrc)
        {
            //当前任务注册数目累加
            appInfo->appRegCount++;

            //如果注册消息的PID不等于上一次记录的PID，说明进程重启过
            if(receMonitorMsg.payload.pidNum != appInfo->pid && appInfo->pid != 0)
            {
                //累加PID变化次数
                appInfo->appIdChangeTimes++;

                //记录当前的复位时间
                time_t curTime;
                curTime = time(NULL);
                appInfo->resetTimeTable.resetTime[appInfo->resetTimeTable.resetIndex] = curTime;
                appInfo->resetTimeTable.resetNum++;
                appInfo->resetTimeTable.resetIndex++;
                if(appInfo->resetTimeTable.resetIndex == RECORD_RESET_TIME_MUX)
                {
                    appInfo->resetTimeTable.resetIndex = 0;
                }
//                SYSMONITOR_DEBUG_INFO("\napp reset: resetTime:%d, resetNum:%d, resetIndex:%d, receMonitorMsg.payload.pidNum:%d，appInfo->pid:%d\n",
//                                      appInfo->resetTimeTable.resetTime[appInfo->resetTimeTable.resetIndex-1],
//                                      appInfo->resetTimeTable.resetNum,
//                                      appInfo->resetTimeTable.resetIndex,
//                                      receMonitorMsg.payload.pidNum,
//                                      appInfo->pid);
            }

            //记录此次消息的PID号
            appInfo->pid = receMonitorMsg.payload.pidNum;
        }
    }

    return SYS_RC_OK;
}
/**************************************************************************
** 函数名: monitor_200ms_process()
** 输  入:
** 输  出:
** 描  述: 200毫秒定时器用于喂狗,会产生SIGALRM信号
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
void monitor_200ms_process(union sigval v)
{
    if(watchDogFun != NULL)
    {
        watchDogFun();
    }

    //进程注册统计
    INT32 i = 0;

    INT32 tableNum = sizeof(g_MonitorAppInfoTable) / sizeof(MONITOR_APP_INFO);
    for(i = 0; i < tableNum; i++)
    {
        MONITOR_APP_INFO *appInfo = g_MonitorAppInfoTable + i;

        //更新剩余检测时间，减喂狗周期，预设值是200毫秒,curCheckInterval必须设置为200毫秒的倍数
        appInfo->curCheckInterval -= FEED_DOG_INTERVAL;

        //当剩余检测时间为0时，说明进程的检测周期到了
        if(appInfo->curCheckInterval == 0)
        {
            //更新剩余检测时间等于检测周期
            appInfo->curCheckInterval = appInfo->checkInterval;

            //如果复位次数达到RECORD_RESET_TIME_MUX次数，同时第一次复位和最后一次复位的时间小于RESET_TIME_GAP的门限
            //说明进程复位频繁，需要复位
            //这里默认30分钟内5次复位
            if(appInfo->resetTimeTable.resetNum >= RECORD_RESET_TIME_MUX  || appInfo->restartAppCount >= RECORD_RESET_TIME_MUX)
            {
                INT32 lastestIndex = appInfo->resetTimeTable.resetIndex;
                INT32 curIndex = appInfo->resetTimeTable.resetIndex == 0 ? RECORD_RESET_TIME_MUX - 1 : appInfo->resetTimeTable.resetIndex - 1;

                if((appInfo->resetTimeTable.resetTime[curIndex] - appInfo->resetTimeTable.resetTime[lastestIndex]) <= RESET_TIME_GAP)
                {
                    //复位系统
                    showMonitor();
                    SYSMONITOR_DEBUG_ERR("app:%s,appInfo->pRebootCmd:%s\n", appInfo->pAppName, appInfo->pRebootCmd);
                    system(appInfo->pRebootCmd);
                    return;
                }
            }

            //如果当前检测注册的次数和上次检测注册的数目一致说明，任务出现异常需要复位
            if(appInfo->lastAppRegCount == appInfo->appRegCount)
            {
                if(appInfo->procFlag == MONITOR_PROC_RESTART)
                {
                    //复位进程
                    showMonitor();
                    SYSMONITOR_DEBUG_ERR("app:%s,appInfo->pRestartCmd:%s\n", appInfo->pAppName, appInfo->pRestartCmd);
                    system(appInfo->pRestartCmd);
                }

                if(appInfo->procFlag == MONITOR_PROC_REBOOT)
                {
                    //复位系统
                    showMonitor();
                    SYSMONITOR_DEBUG_ERR("app:%s,appInfo->pRebootCmd:%s\n", appInfo->pAppName, appInfo->pRebootCmd);
                    system(appInfo->pRebootCmd);
                    return;
                }

                //主动复位进程数累加
                appInfo->restartAppCount++;
            }

            //更新上一次任务注册次数
            appInfo->lastAppRegCount = appInfo->appRegCount;
        }


    }
    return;
}

/**************************************************************************
** 函数名: systemMonitorInit()
** 输  入:
** 输  出:
** 描  述: 注册200毫秒定时器用于喂狗,以及监控统计,200毫秒周期会产生SIGALRM信号
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/

INT32  systemMonitorInit()
{
    INT32 i = 0;
    INT32 tableNum = sizeof(g_MonitorAppInfoTable) / sizeof(MONITOR_APP_INFO);
    for(i = 0; i < tableNum; i++)
    {
        memset(&g_MonitorAppInfoTable[i].resetTimeTable, 0, sizeof(g_MonitorAppInfoTable[i].resetTimeTable));
    }

    //posix 定时器
    timer_t timerid;

    struct sigevent evp;
    evp.sigev_notify = SIGEV_THREAD;
    evp.sigev_notify_function = monitor_200ms_process;
    evp.sigev_notify_attributes=NULL;
    evp.sigev_value.sival_ptr=&timerid;

    INT32 ret = timer_create(CLOCK_REALTIME, &evp, &timerid);
    if(ret != 0)
    {
        SYSMONITOR_DEBUG_ERR("Set timer error. %s \n", strerror(errno));
        return SYS_RC_ERR;
    }

    struct itimerspec value;
    value.it_value.tv_sec = 3;
    value.it_value.tv_nsec = 0;
    value.it_interval.tv_sec = 0 ;
    value.it_interval.tv_nsec = FEED_DOG_INTERVAL * 1000 * 1000;
    timer_settime(timerid, 0, &value, NULL);

    return SYS_RC_OK;
}

/**************************************************************************
** 函数名:
** 输  入:
** 输  出:
** 描  述:
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
void registerWatchDog(watchDogPtr fun)
{
    watchDogFun = fun;
}
/**************************************************************************
** 函数名: monitor_rcv_queue_msg_sevice
** 输  入:
** 输  出:
** 描  述: 接收进程注册消息
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
void* monitor_rcv_queue_msg_sevice(void* arg)
{
    if(initMonitorMsgque() == SYS_RC_ERR)
    {
        SYSMONITOR_DEBUG_ERR("%s:initMsg err\n", __func__);
        return NULL;
    }

    while(1)
    {
        //接收消息
        receMonitorMsg(monitorMsgQueue);
    }
    return NULL;
}


