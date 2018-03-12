#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sched.h>

#include "public.h"
#include "errCode.h"
#include "taskLib.h"
#include "outputLib.h"

//命令行列表
sCommand debugCommands[COMMAND_MAX] =
{
    {SHOW_VERSION_CMD           , "show_version"          , show_version           },
    {SET_DEBUG_CONFIG_CMD       , "set_debug"             , set_debug_cmd          },
    {SHOW_DEBUG_INFO_CMD        , "show_dbginfo"          , show_dbginfo_cmd       },
    {SUSPEND_THREAD_CMD         , "suspend_thread"        , suspend_thread_cmd     },
    {RESTART_THREAD_CMD         , "restart_thread"        , restart_thread_cmd     },
    {SHOW_TASK_CMD              , "show_task"             , show_task_cmd          },
    {TEST_MONITOR               , "test_monitor"          , test_monitor           },
    {GET_SCHEDULER              , "get_scheduler"         , get_scheduler          },
    {HELP_CMD                   , "help"                  , showCommand            },
};

//本地调试命令消息队列
key_t DbgMsgkey = Dbg_Msg_KEY;
int dbgMsgQueue = -1;

/**************************************************************************
** 函数名: init_debug
** 输  入:
** 输  出:
** 描  述: 初始化测试函数
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
void init_debug()
{
//    /*申请共享内存*/
//    int shm_id;
//    shm_id = shmget(alm_key, 4096, 0666 | IPC_CREAT);
//    if(shm_id == -1)
//    {
//        perror("STATUS_Thread shmget error");
//        return ;
//    }
//    /*获取共享内存地址*/
//    alm_mem_map = (sALM_MEM*)shmat(shm_id, NULL, 0);

}
/**************************************************************************
** 函数名: searchCommand
** 输  入:
** 输  出:
** 描  述: 按输入的字符串搜索命令行列表，如果找到，执行相关命令
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
void searchCommand(char* pCmd)
{
    int i = 0;
    for(i = 0; i < COMMAND_MAX; i++)
    {
        if(strcmp(pCmd, debugCommands[i].str) == 0)
        {
            printf("catch it\n");
            if(debugCommands[i].fun != NULL)
            {
                debugCommands[i].fun(i);
            }
            return;
        }
    }
    printf("not support , try again\n");
}
/**************************************************************************
** 函数名: sendCommand
** 输  入:
** 输  出:
** 描  述: 发送命令到main进程
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
void sendCommand(int num, UINT32 arg1, UINT32 arg2)
{
    sCommandMsg sendCommand;
    memset((void*)&sendCommand, 0, sizeof(sCommandMsg));
    sendCommand.type = num + 1;
    memcpy(sendCommand.str, &arg1, sizeof(arg1));
    memcpy((UINT32*)sendCommand.str + 1, &arg2, sizeof(arg2));
    memcpy((UINT32*)sendCommand.str + 2, debugCommands[num].str, strlen(debugCommands[num].str));
//    printf("sendCommand: num %d,arg1: 0X%X, arg2: 0X%X\n",num,arg1,arg2);
    sendDbgMsg((char*)&sendCommand);
}
/**************************************************************************
** 函数名:showCommand
** 输  入:
** 输  出:
** 描  述: 显示支持的命令
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
void showCommand(int num)
{
    int i = 0;
    printf("supports commands:%d\n", num);
    for(i = 0; i < COMMAND_MAX; i++)
    {
        printf("%02d : %s\n", i, debugCommands[i].str);
    }

}

/**************************************************************************
** 函数名: show_task_cmd
** 输  入:
** 输  出:
** 描  述: 显示线程运行情况
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
void show_task_cmd(int num)
{
    sendCommand(num, 0, 0);
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
void show_version(int num)
{
    sendCommand(num, 0, 0);
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
void set_debug_cmd(INT32 num)
{
    printf("Please input the debugtype \n");
    printf("SetDbgInfoOn_Stdio   0   \n");
    printf("SetDbgInfoOff_Stdio  1   \n");
    printf("SetDbgErrOn_Stdio    2   \n");
    printf("SetDbgErrOff_Stdio   3   \n");
    printf("SetDbgInfoOn_Term    4   \n");
    printf("SetDbgInfoOff_Term   5   \n");
    printf("SetDbgErrOn_Term     6   \n");
    printf("SetDbgErrOff_Term    7   \n");
    printf("SetDbgAllOn_Term     8   \n");
    printf("SetDbgAllOff_Term    9   \n");
    printf("SetDbgAllOn_Stdio    10  \n");
    printf("SetDbgAllOff_Stdio   11  \n");
    printf("Input \'q\'' to up level \n");

    while(1)
    {
//        DEAL_WITH_INIT();
        int result = 0;
        UINT32 debugType = 0XFF;

        result = scanf("%d", &debugType);

        if(result == -1)
        {
            printf("input error try again\n");
            continue;
        }
        DEAL_WITH_SCANF_ERR() ;

        if(debugType <= 12)
        {
            if(debugType == 10)
            {
                char ttyStr[16] = {0};
                char *tmpTtyStr = ttyname(1);
                if(tmpTtyStr == NULL)
                {
                    printf("get current tty is NULL,can't set SetDbgAllOn_Stdio\n");
                    continue;
                }
                strcpy(ttyStr, tmpTtyStr);
                printf("ttyname(0):%s:%d\n", tmpTtyStr, atoi(ttyStr));

                sCommandMsg sendCommand;
                memset((void*)&sendCommand, 0, sizeof(sCommandMsg));
                sendCommand.type = num + 1;
                memcpy((UINT32*)sendCommand.str + 0, &debugType, sizeof(UINT32));
                memcpy((UINT32*)sendCommand.str + 1, &ttyStr   , 16);
                memcpy((UINT32*)sendCommand.str + 16, debugCommands[num].str, strlen(debugCommands[num].str));
                sendDbgMsg((char*)&sendCommand);
            }
            else
            {
                sendCommand(num, debugType, 0);
            }
        }
        else
        {
            printf("debugType %d is out of range\n", debugType);
            printf("Please input the debugtype \n");
            printf("SetDbgInfoOn_Stdio   0   \n");
            printf("SetDbgInfoOff_Stdio  1   \n");
            printf("SetDbgErrOn_Stdio    2   \n");
            printf("SetDbgErrOff_Stdio   3   \n");
            printf("SetDbgInfoOn_Term    4   \n");
            printf("SetDbgInfoOff_Term   5   \n");
            printf("SetDbgErrOn_Term     6   \n");
            printf("SetDbgErrOff_Term    7   \n");
            printf("SetDbgAllOn_Term     8   \n");
            printf("SetDbgAllOff_Term    9   \n");
            printf("SetDbgAllOn_Stdio    10  \n");
            printf("SetDbgAllOff_Stdio   11  \n");
            printf("Input \'q\'' to up level \n");
        }
        taskDelay(100 * 1000);
        printf("set_debug_cmd,try again or  \'q\' to up level \n");
    }
}
/**************************************************************************
** 函数名: show_dbginfo_cmd
** 输  入:
** 输  出:
** 描  述: 显示调试信息
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
void show_dbginfo_cmd(INT32 num)
{
    sendCommand(num, 0, 0);
}
/**************************************************************************
** 函数名: suspend_thread_cmd
** 输  入:
** 输  出:
** 描  述: 暂停子线程执行
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
void suspend_thread_cmd(INT32 num)
{
    printf("Please input the thread you want suspend: \n");
    printf("taskwatch  thread    4 \n");
    printf("Input \'q\'' to up level \n");

    while(1)
    {
        DEAL_WITH_INIT();
        int result = 0;
        UINT32 threadNum = 0XFF;

        result = scanf("%d", &threadNum);

        if(result == -1)
        {
            printf("input error try again\n");
            continue;
        }
        DEAL_WITH_SCANF_ERR() ;

        if(threadNum <= 6)
        {

            if(threadNum == 4)
            {
                while(1)
                {
                    UINT32 detail;
                    printf("please input monnitor flag: 0 don't send monittor message ,1 send monittor message,or \'q' to up level\n");
                    result = scanf("%X", &detail);

                    if(result == -1)
                    {
                        printf("input error try again\n");
                        continue;
                    }

                    DEAL_WITH_SCANF_ERR_BREAK() ;
                    DEAL_WITH_SCANF_ERR_BREAK_END();
                    if(detail <= 1)
                    {
                        sendCommand(num, threadNum, detail);
                    }
                    else
                    {
                        printf("monnitor flag %d is out of range,try again\n", detail);
                    }
                }
            }
            else
            {
                sendCommand(num, threadNum, 0);
            }
        }
        else
        {
            printf("threadNum %d is out of range\n", threadNum);
            printf("Please input the thread you want suspend: \n");
            printf("taskwatch  thread    4 \n");
            printf("Input \'q\'' to up level \n");
        }

        taskDelay(100 * 1000);
        printf("suspend_thread_cmd,try again or  \'q\' to up level \n");
    }
}
/**************************************************************************
** 函数名: restart_thread_cmd
** 输  入:
** 输  出:
** 描  述: 重启子线程执行
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
void restart_thread_cmd(INT32 num)
{
    printf("Please input the thread you want restart: \n");
    printf("taskwatch  thread    4 \n");
    printf("Input \'q\'' to up level \n");

    while(1)
    {
        DEAL_WITH_INIT();
        int result = 0;
        UINT32 threadNum = 0XFF;

        result = scanf("%d", &threadNum);

        if(result == -1)
        {
            printf("input error try again\n");
            continue;
        }
        DEAL_WITH_SCANF_ERR() ;

        if(threadNum <= 6)
        {
            if(threadNum == 4)
            {
                while(1)
                {
                    UINT32 detail;
                    printf("please input monnitor flag: 0 don't send monittor message ,1 send monittor message,or \'q' to up level\n");
                    result = scanf("%X", &detail);

                    if(result == -1)
                    {
                        printf("input error try again\n");
                        continue;
                    }

                    DEAL_WITH_SCANF_ERR_BREAK() ;
                    DEAL_WITH_SCANF_ERR_BREAK_END();
                    if(detail <= 1)
                    {
                        sendCommand(num, threadNum, detail);
                    }
                    else
                    {
                        printf("monnitor flag %d is out of range,try again\n", detail);
                    }
                }
            }
            else
            {
                sendCommand(num, threadNum, 0);
            }
        }
        else
        {
            printf("threadNum %d is out of range\n", threadNum);
            printf("Please input the thread you want restart: \n");
            printf("taskwatch  thread    4 \n");
            printf("Input \'q\'' to up level \n");
        }

        taskDelay(100 * 1000);
        printf("restart_thread_cmd,try again or  \'q\' to up level \n");
    }
}
/**************************************************************************
** 函数名: test_monitor
** 输  入:
** 输  出:
** 描  述: 测试监控功能
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
void test_monitor(INT32 num)
{
    printf("Please input the test thread num\n");
    printf("apcommThread                  0 \n");
    printf("probeThread                   1 \n");
    printf("Input \'q\'' to up level \n");

    while(1)
    {
        int result = 0;
        UINT32 debugType = 0XFF;

        result = scanf("%d", &debugType);

        if(result == -1)
        {
            continue;
        }
        DEAL_WITH_SCANF_ERR() ;


        if(debugType < 3)
        {
            sendCommand(num, debugType, 0);
        }
        else
        {
            printf("test thread num %d is out of range\n", debugType);
            printf("input error try again\n");
            printf("Please input the test thread num\n");
            printf("apcommThread                  0 \n");
            printf("probeThread                   1 \n");
            printf("Input \'q\'' to up level \n");
        }
        taskDelay(100 * 1000);
        printf("set_debug_cmd,try again or  \'q\' to up level \n");
    }
}
/**************************************************************************
** 函数名: get_scheduler
** 输  入:
** 输  出:
** 描  述:
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
void get_scheduler(INT32 num)
{
    printf("Please input the PID num\n");
    printf("Input \'q\'' to up level \n");
    UNWARING(num);
    while(1)
    {
        int result = 0;
        UINT32 debugType = 0XFF;

        result = scanf("%d", &debugType);

        if(result == -1)
        {
            continue;
        }
        DEAL_WITH_SCANF_ERR() ;

        int nRet = 0;
        nRet = sched_getscheduler(debugType);
        APP_DEBUG_INFO("PID %d,sched_getscheduler: %d\n", debugType, nRet);

        taskDelay(100 * 1000);
        printf("get_scheduler,try again or  \'q\' to up level \n");
    }
}

/**************************************************************************
** 函数名: initDbgMsgQueue
** 输  入:
** 输  出:
** 描  述: 初始化本地调试消息队列
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
INT32 initClientDbgMsgQueue()
{
    dbgMsgQueue = msgget(DbgMsgkey,  IPC_CREAT | 0666);
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
** 描  述: 本地发送调试命令到进程
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
** 函数名: main
** 输  入:
** 输  出:
** 描  述: 调试程序主函数
** 作  者: xiaodong
** 日  期:
** 修  改:
** 备  注:
**************************************************************************/
int main(void)
{
    /*设置打印输出到stdio*/
    SetDbgInfoOn_Stdio();
    SetDbgErrOn_Stdio();
//    SetDbgInfoOn_Term();
//    SetDbgErrOn_Term();

    if(initClientDbgMsgQueue() != SYS_RC_OK)
    {
        APP_DEBUG_INFO("can't run voicedbg\n");
        return -1;
    }

    init_debug();

    char s[80];
    while(1)
    {
        printf("\napcomm debug,please input the command:\n");
        memset(s, 0, sizeof(s));
        fgets(s, sizeof(s), stdin);
        memset(s + strlen(s) - 1, 0, 1);

        if(strcmp("q", s) == 0 || strcmp("quit", s) == 0)
        {
            printf("exit apcomm debug\n");
            break;
        }
        searchCommand(s);
    }
    return 0;
}
