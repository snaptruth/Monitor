#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

#include "common.h"
#include "errCode.h"
#include "taskLib.h"
#include "public.h"


//保证只有一个线程执行
static INT32 RunStatInit(void)
{
    int nFd = 0;

    /*一定要保证/var/run目录存在*/
    system(MKDIR_APP_RUN_DIR);

    nFd = open(APP_STR_LOCK_FILE, O_CREAT | O_WRONLY, 0644);
    if(-1 == nFd)
    {
        APP_DEBUG_ERR("%s,%s: Unable to open lockfile %s: %s\n",
                         SAMPLE_APP, __func__, APP_STR_LOCK_FILE, strerror(errno));
        return SYS_RC_OPEN_FAILED;
    }

    if(-1 == lockf(nFd, F_TEST, 0))
    {
        APP_DEBUG_ERR("%s,%s: Lock file <%s> F_TEST Failure: '%s'\n",
                         SAMPLE_APP, __func__, APP_STR_LOCK_FILE, strerror(errno));
        APP_DEBUG_ERR("Maybe Process <%s> is Running ...\n", SAMPLE_APP);
        return SYS_RC_LOCK_TEST_ERR;
    }

    if(-1 == lockf(nFd, F_LOCK, 0))
    {
        APP_DEBUG_ERR("%s,%s: Lockfile %s LOCK error: %s\n",
                         SAMPLE_APP, __func__, APP_STR_LOCK_FILE, strerror(errno));
        return SYS_RC_LOCK_ERR;
    }

    return SYS_RC_OK;
}

int main()
{
    INT32 result = 0;

    /*设置打印输出到终端*/
//    SetDbgErrOn_Stdio();
//    SetDbgInfoOn_Stdio();
//    SetDbgInfoOn_Term();
    SetDbgErrOn_Term();
//    SetDbgAllOff_Term();
//    SetDbgAllOff_Stdio();
//    SetDbgAllOn_Stdio();
//    SetDbgAllOn_Term();

    //初始化线程记录表
    if(taskLibInit(20) == TASK_RT_ERROR)
    {
        APP_DEBUG_ERR("%s taskLibInit error: %s.\n", SAMPLE_APP, strerror(errno));
        return errno;
    }

    /* 保证只有一个进程运行 */
    result = RunStatInit();
    if(SYS_RC_OK != result)
    {
        return errno;
    }

    //开启调试线程
    if(taskSpawn("receDbgThread", TASK_DEFAULT_PRIORITY, TASK_DEFAULT_OPTION, TASK_DEFAULT_STACK_SIZE, 0, receDbgMsgThread, NULL) < 0)
    {
        return -1;
    }

    sleep(5);

    char cmd[512] = {0};
    sprintf(cmd, "logger sample: Pid %d Start Running ...... \n", getpid());
    system(cmd);

    //主线程进行线程监控
    UCHAR msgSrc = MSG_SRC_SAMPLE;
    taskWatch(&msgSrc);
    return 0;

}
