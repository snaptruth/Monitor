#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sched.h>

#include "public.h"
#include "errCode.h"
#include "outputLib.h"
#include "taskLib.h"

//本地调试命令消息队列
key_t DbgMsgkey = Dbg_Msg_KEY;
int dbgMsgQueue = -1;

/**************************************************************************
** 函数名: initDbgMsgQueue
** 输  入:
** 输  出:
** 描  述: 初始化voice本地调试消息队列
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
INT32 initDbgMsgQueue()
{
    dbgMsgQueue = msgget(DbgMsgkey, IPC_CREAT | 0666);
    APP_DEBUG_INFO("dbgMsgQueue %d\n", dbgMsgQueue);
    if(dbgMsgQueue != -1)
    {
        return SYS_RC_OK;
    }
    APP_DEBUG_ERR("msgget err  %d ,%s\n", errno, strerror(errno));
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
INT32 sendDbgMsg(char* _command)
{
    INT32 result;
    if(_command == NULL)
    {
        return SYS_RC_INV_PARAM;
    }
    sCommandMsg *pSendCommand = (sCommandMsg*)_command;

    result = msgsnd(dbgMsgQueue, pSendCommand, sizeof(pSendCommand->str), IPC_NOWAIT);
    if(0 != result)
    {
        APP_DEBUG_ERR("sendDbgMsg err  %d, %s \n", errno, strerror(errno));
        return SYS_RC_ERR;
    }

    return SYS_RC_OK;
}
/**************************************************************************
** 函数名: sendDbgTestMsg
** 输  入:
** 输  出:
** 描  述: 发送测试消息到调试线程，用于线程监控
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
void* sendDbgTestMsg(void* pTmp)
{
    UINT32 temp = 0;
    sCommandMsg sendCommand;
    memset((void*)&sendCommand, 0, sizeof(sCommandMsg));
    sendCommand.type = COMMAND_MAX + 1;
    memcpy(sendCommand.str, &temp, sizeof(temp));
    memcpy((UINT32*)sendCommand.str + 1, &temp, sizeof(temp));
    memcpy((UINT32*)sendCommand.str + 2, "dbg test msg", strlen("dbg test msg"));
    sendDbgMsg((char*)&sendCommand);

    UNWARING(pTmp);
    return NULL;
}
/**************************************************************************
** 函数名: receDbgMsg
** 输  入:
** 输  出:
** 描  述: voice进程接收本地调试命令
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
INT32 receDbgMsg(char* _command)
{
    INT32 msg_len = 0;
    if(_command == NULL)
    {
        return SYS_RC_INV_PARAM;
    }
    sCommandMsg* pReceCommand;
    pReceCommand = (sCommandMsg*) _command;

    msg_len = taskMsgRec(dbgMsgQueue, pReceCommand, sizeof(pReceCommand->str), sendDbgTestMsg);
    if(msg_len == -1)
    {
        APP_DEBUG_ERR("receDbgMsg err %d :%s \n", errno, strerror(errno));
        return SYS_RC_ERR;
    }

    return SYS_RC_OK;
}
/**************************************************************************
** 函数名: receDbgMsgThread
** 输  入:
** 输  出:
** 描  述: 接收调试消息线程
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
void* receDbgMsgThread(void* arg)
{
    if(initDbgMsgQueue() != SYS_RC_OK)
    {
        APP_DEBUG_ERR("can't run voicedbg\n");
        return NULL;
    }

    sCommandMsg receCmd;
    INT32 result;

    taskRegisterPid();
    while(1)
    {
        taskRegister();
        memset((void*)&receCmd, 0, sizeof(sCommandMsg));
        result =  receDbgMsg((char*)&receCmd);

        if(receCmd.type - 1 != COMMAND_MAX)
        {
            APP_DEBUG_INFO("\nreceive command: %s\n", (char*)((UINT32*)receCmd.str + 2));
        }

        if(result == SYS_RC_OK)
        {
            switch(receCmd.type - 1)
            {
                case SHOW_VERSION_CMD:
                    APP_DEBUG_INFO("soft version:%s\n", g_version);
                    break;

                case SET_DEBUG_CONFIG_CMD:
                {
                    UINT32* pDebugType = (UINT32*)receCmd.str;

                    switch(*pDebugType)
                    {
                        case 0:
                            SetDbgInfoOn_Stdio();
                            break;
                        case 1:
                            SetDbgInfoOff_Stdio();
                            break;
                        case 2:
                            SetDbgErrOn_Stdio();
                            break;
                        case 3:
                            SetDbgErrOff_Stdio();
                            break;
                        case 4:
                            SetDbgInfoOn_Term();
                            break;
                        case 5:
                            SetDbgInfoOff_Term();
                            break;
                        case 6:
                            SetDbgErrOn_Term();
                            break;
                        case 7:
                            SetDbgErrOff_Term();
                            break;
                        case 8:
                            SetDbgAllOn_Term();
                            break;
                        case 9:
                            SetDbgAllOff_Term();
                        case 10:
                        {
                            char oldTtyStr[1] = {0};
                            char ttyStr[1] = {0};
                            char * tmpTtyStr = ttyname(1);
                            if(tmpTtyStr == NULL)
                            {
                                APP_DEBUG_RT("get current tty is NULL\n");
                            }
                            else
                            {
                                strcpy(oldTtyStr, tmpTtyStr);
                            }

                            if(strcmp(oldTtyStr, (char*)((UINT32*)receCmd.str + 1)) == 0)
                            {
                                APP_DEBUG_RT("not need redirect tty, just set debug flag\n");

                            }
                            else
                            {
                                memcpy(ttyStr, (UINT32*)receCmd.str + 1, 16);
                                int fd = open(ttyStr, O_WRONLY);
                                dup2(fd, 1);
                                APP_DEBUG_RT("old ttynam(1)  : %s\n", oldTtyStr);
                            }
                        }
                        SetDbgAllOn_Stdio();
                        break;
                        case 11:
                            SetDbgAllOff_Stdio();
                            break;
                        default:
                            APP_DEBUG_ERR("SET_DEBUG_CONFIG_CMD: pDebugType %d is error \n", *pDebugType);
                            break;
                    }
                    APP_DEBUG_INFO("IsDbgInfoOn_Stdio    : %d\n", IsDbgInfoOn_Stdio());
                    APP_DEBUG_INFO("IsDbgErrOn_Stdio     : %d\n", IsDbgErrOn_Stdio());
                    APP_DEBUG_INFO("IsDbgInfoOn_Term     : %d\n", IsDbgInfoOn_Term());
                    APP_DEBUG_INFO("IsDbgErrOn_Term      : %d\n", IsDbgErrOn_Term());

                }
                break;

                case SHOW_DEBUG_INFO_CMD:
                {
                    APP_DEBUG_INFO("IsDbgInfoOn_Stdio       : %d\n", IsDbgInfoOn_Stdio());
                    APP_DEBUG_INFO("IsDbgErrOn_Stdio        : %d\n", IsDbgErrOn_Stdio());
                    APP_DEBUG_INFO("IsDbgInfoOn_Term        : %d\n", IsDbgInfoOn_Term());
                    APP_DEBUG_INFO("IsDbgErrOn_Term         : %d\n", IsDbgErrOn_Term());


//                    APP_DEBUG_INFO("status_thread      runtimes %10d,  run_flag %d,  pause_flag %d,  alm_pause_flag %d,  per_pause_flag %d  \n",
//                                   status_thread_run_times, status_thread_run_flag, status_thread_pause_flag,
//                                   status_thread_alm_pause_flag, status_thread_per_pause_flag);

//                    APP_DEBUG_INFO("alm_thread         runtimes %10d,  run_flag %d,  pause_flag %d \n",
//                                   alm_thread_run_times, alm_thread_run_flag, alm_thread_pause_flag);

//                    APP_DEBUG_INFO("message_thread     runtimes %10d,  run_flag %d,  pause_flag %d \n",
//                                   message_thread_run_times, message_thread_run_flag, message_thread_pause_flag);

//                    APP_DEBUG_INFO("lcas_thread        runtimes %10d,  run_flag %d,  pause_flag %d \n",
//                                   lcas_thread_run_times, lcas_thread_run_flag, lcas_thread_pause_flag);

//                    APP_DEBUG_INFO("init_thread        runtimes %10d,  run_flag %d,  pause_flag %d \n",
//                                   init_thread_run_times, init_thread_run_flag, init_thread_pause_flag);

//                    APP_DEBUG_INFO("taskWatch_thread   runtimes %10d,  run_flag %d,  pause_flag %d,  watchFalg      %d,  monitorFlag %d \n",
//                                   0,0,0,getTaskWatchFalg(),getTaskMonitorFlag());

                }
                break;
                case SUSPEND_THREAD_CMD:
                {
                    UINT32* pThreadNum = (UINT32*)receCmd.str;
                    UINT32* pDetail = (UINT32*)receCmd.str + 1;
                    APP_DEBUG_INFO("recevice : pThreadNum %d,pDetail %d\n", *pThreadNum, *pDetail);
                    switch(*pThreadNum)
                    {
                        case 4:
                            setTaskWatchFalg(0);
                            switch(*pDetail)
                            {
                                case 0:
                                    setTaskMonitorFlag(0);
                                    break;
                                case 1:
                                    setTaskMonitorFlag(1);
                                    break;
                                default:
                                    APP_DEBUG_ERR("SUSPEND_THREAD_CMD: taskWatch_thread  MonitorFlag %d is error \n", *pDetail);
                                    break;
                            }
                            break;
                        default:
                            APP_DEBUG_ERR("SUSPEND_THREAD_CMD: pThreadNum %d is error \n", *pThreadNum);
                            break;
                    }
                }
                break;

                case RESTART_THREAD_CMD:
                {
                    UINT32* pThreadNum = (UINT32*)receCmd.str;
                    UINT32* pDetail = (UINT32*)receCmd.str + 1;
                    switch(*pThreadNum)
                    {
                        case 4:
                            setTaskWatchFalg(1);
                            switch(*pDetail)
                            {
                                case 0:
                                    setTaskMonitorFlag(0);
                                    break;
                                case 1:
                                    setTaskMonitorFlag(1);
                                    break;
                                default:
                                    APP_DEBUG_ERR("SUSPEND_THREAD_CMD: taskWatch_thread  MonitorFlag %d is error \n", *pDetail);
                                    break;
                            }
                            break;
                        default:
                            APP_DEBUG_ERR("SUSPEND_THREAD_CMD: pThreadNum %d is error \n", *pThreadNum);
                            break;
                    }
                }
                break;

                case SHOW_TASK_CMD :
                    taskShow();
                    break;

                case TEST_MONITOR:
                {
                    UINT32* pDebugType = (UINT32*)receCmd.str;
                    UINT32* pParam = (UINT32*)receCmd.str + 1;
                    APP_DEBUG_INFO("receive pDebugType %d,pParam %d\n", *pDebugType, *pParam);
                    switch(*pDebugType)
                    {
                        default:
                            APP_DEBUG_ERR("TEST_MONITOR: thread num %d is error \n", *pDebugType);
                            break;
                    }
                }
                break;

                case COMMAND_MAX:
                    //作为测试消息用
                    break;

                default:
                    APP_DEBUG_INFO("\n%s: receive wrong msg, msg type = %d\n", __func__, receCmd.type - 1);
                    break;
            }
        }
    }
    UNWARING(arg);
}



