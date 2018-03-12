#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "outputLib.h"
#include "errCode.h"

#define OUTPUT_PRINT printf


#define OUTPUT_TASK_LOCK_FILE "/var/run/output_task.lock"


/**************************************************************************//**
* function: OutputTask_RunLockFileInit
* @brief 锁文件初始化，如果文件已经被锁，说明已经在运行，则不再运行，防止多个业务实体同时运行
* @param
* @return
* @retval INT
*
* @note history:
*     @note    date:
*     @note    author:
*     @note    content:   锁文件初始化，如果文件已经被锁，说明已经在运行，则不再运行，防止多
*                         个业务实体同时运行
******************************************************************************/
int OutputTask_RunLockFileInit(const char* strFileName, int* pFd)
{
    int  nFd = 0;

    if((NULL == strFileName) || (NULL == pFd))
    {
        OUTPUT_PRINT("%s %d: invalid param.\n", __FUNCTION__, __LINE__);
        return SYS_RC_INV_PARAM;
    }

    nFd = open(strFileName, O_CREAT | O_WRONLY, 0644);
    if(-1 == nFd)
    {
        OUTPUT_PRINT("%s %d: Unable to open lockfile %s: %s\n",
                     __FUNCTION__, __LINE__, strFileName, strerror(errno));
        return SYS_RC_OPEN_FAILED;
    }

    if(-1 == lockf(nFd, F_TEST, 0))
    {
        OUTPUT_PRINT("file '%s' is locked\n", strFileName);
        OUTPUT_PRINT("It's already running...\n");
        return SYS_RC_ERR;
    }

    if(-1 == lockf(nFd, F_LOCK, 0))
    {
        OUTPUT_PRINT("lock file '%s' error: %s\n", strFileName, strerror(errno));
        return SYS_RC_ERR;
    }

    *pFd = nFd;
    return SYS_RC_OK;
}

/**************************************************************************//**
* function: OutputTask_RunLockFileFinit
* @brief 锁文件反初始化，释放文件锁
* @param
* @return
* @retval INT
*
* @note history:
*     @note    date:
*     @note    author:
*     @note    content:   锁文件反初始化，释放文件锁
******************************************************************************/
int OutputTask_RunLockFileFinit(const char* pLockFileName, int nFd)
{
    if(-1 == lockf(nFd, F_ULOCK, 0))
    {
        OUTPUT_PRINT("unlock file '%s' error: %s\n",
                     pLockFileName, strerror(errno));
        return SYS_RC_ERR;
    }

    close(nFd);

    return SYS_RC_OK;
}


int main()
{
    int nRet = 0;
    int nFd = 0;

    /* 锁文件初始化，如果文件已经被锁，说明已经在运行，则不再运行，防止多个业务实体同时运行*/
    nRet = OutputTask_RunLockFileInit(OUTPUT_TASK_LOCK_FILE, &nFd);
    if(SYS_RC_OK != nRet)
    {
        return nRet;
    }

    char cmd[512] = {0};
    sprintf(cmd, "logger output: Pid %d Start Running ...... \n", getpid());
    system(cmd);

    if(OTLB_RT_OK != OutputLib_Init())
    {
        OUTPUT_PRINT("%s %d %s OutputLib Init Failed!\n", __FILE__, __LINE__, __FUNCTION__);
        return OTLB_RT_ERROR;
    }

    OutputTask_RunLockFileFinit(OUTPUT_TASK_LOCK_FILE, nFd);


    return 0;
}

