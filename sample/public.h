#ifndef PUBLIC_H
#define PUBLIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "outputLib.h"

#define PUBLIC_OK   0
#define PUBLIC_ERR -1

#define g_version            "pa1.00"
#define MKDIR_APP_RUN_DIR    "mkdir -p /var/run"
#define APP_STR_LOCK_FILE    "/var/run/sample.pid"
#define SAMPLE_APP           "SAMPLE"

typedef enum
{
    DEBUG_INFO_TO_TERM     = 0x01,
    DEBUG_ERR_TO_TERM      = 0x02,
    DEBUG_ALL_TERM         = 0x03, /* 0x01 + 0x02*/

    DEBUG_INFO_TO_STDIO    = 0x04,
    DEBUG_ERR_TO_STDIO     = 0x08,
    DEBUG_ALL_STDIO        = 0x0C, /* 0x04 + 0x8*/
} eDebugEnum;

#define APP_DEBUG_INFO(str, args...)\
    do{\
        if(IsDbgInfoOn_Stdio())\
        {\
            printf(str, ## args);\
            fflush(stdout);\
            break;\
        }\
        if(IsDbgInfoOn_Term())\
        {\
            OutputLib_OutputInfo(OTLB_LOG_DEBUG, SAMPLE_MOD_NUM, str, ## args);\
        }\
    }while(0)

#define APP_DEBUG_INFO_DATA(msg, packet, len)\
    do\
    {\
        if(IsDbgInfoOn_Term())\
        {\
            OutputLib_OutputData(OTLB_LOG_DEBUG, SAMPLE_MOD_NUM,msg, packet, len);\
        }\
    }while(0)

/* 错误提示信息调试输出接口 */
#define APP_DEBUG_ERR(str, args...)\
    do\
    {\
        if(IsDbgErrOn_Stdio())\
        {\
            printf(str, ## args);\
            fflush(stdout);\
            break;\
        }\
        if(IsDbgErrOn_Term())\
        {\
            OutputLib_OutputInfo(OTLB_LOG_EMERG, SAMPLE_MOD_NUM, str, ## args);\
        }\
    }while(0)

/* 实时输出debug信息 */
#define APP_DEBUG_RT(str, args...)\
    do\
    {\
        /*INT32 result=0;*/\
        if(IsDbgInfoOn_Stdio())\
        {\
            printf(str, ## args);\
            fflush(stdout);\
            break;\
        }\
        /*result=OutputLib_OutputInfo(OTLB_LOG_EMERG, SAMPLE_MOD_NUM, str, ## args);*/\
        OutputLib_OutputInfo(OTLB_LOG_EMERG, SAMPLE_MOD_NUM, str, ## args);\
    }while(0)


/*外部函数声明*/
extern int  IsDbgInfoOn_Stdio(void);
extern void SetDbgInfoOn_Stdio(void);
extern void SetDbgInfoOff_Stdio(void);
extern int  IsDbgErrOn_Stdio(void);
extern void SetDbgErrOn_Stdio(void);
extern void SetDbgErrOff_Stdio(void);

extern void SetDbgAllOn_Stdio(void);
extern void SetDbgAllOff_Stdio(void);

extern int  IsDbgInfoOn_Term(void);
extern void SetDbgInfoOn_Term(void);
extern void SetDbgInfoOff_Term(void);
extern int  IsDbgErrOn_Term(void);
extern void SetDbgErrOn_Term(void);
extern void SetDbgErrOff_Term(void);

extern void SetDbgAllOn_Term(void);
extern void SetDbgAllOff_Term(void);

extern void LMDB_GetDbgValue(UINT32* pValue);
extern void LMDB_SetDbgValue(UINT32 nValue);
extern void LMDB_StripEnterChar(char* pStrings, int nLen);



//调试命令字
typedef enum
{
    SHOW_VERSION_CMD = 0,
    SET_DEBUG_CONFIG_CMD,
    SHOW_DEBUG_INFO_CMD,
    SUSPEND_THREAD_CMD,
    RESTART_THREAD_CMD,
    SHOW_TASK_CMD,
    TEST_MONITOR,
    GET_SCHEDULER,
    HELP_CMD,
    COMMAND_MAX,
} sCommandType;

//调试消息长度
#define DBG_MSG_LEN 200

//调试消息队列key值
#define Dbg_Msg_KEY 8001

//发送调试命令消息结构
typedef struct
{
    INT32 type;
    char str[DBG_MSG_LEN];
} sCommandMsg;

//调试命令结构
typedef struct
{
    sCommandType type;
    char* str;
    void (*fun)(INT32);
} sCommand;

//调试命令数组
extern sCommand debugCommands[COMMAND_MAX];

//调试命令函数列表
extern void searchCommand(char* pCmd);
extern void sendCommand(int num, UINT32 arg1, UINT32 arg2);
extern void show_version(INT32);
extern void showCommand(INT32);
extern void set_debug_cmd(INT32);
extern void suspend_thread_cmd(INT32);
extern void restart_thread_cmd(INT32);
extern void show_dbginfo_cmd(INT32);
extern void show_task_cmd(INT32);
extern void test_monitor(INT32);
extern void get_scheduler(INT32);
extern void showCommand(INT32);
extern void show_cur_low_mac(INT32);
extern void show_his_low_mac(INT32);
extern void show_tmp_low_mac(INT32);
extern void show_kickoff_mac(INT32);
extern void show_no_kickoff_mac(INT32);
extern void show_his_kickoff_mac(INT32);
extern void show_kernel_probe_info(INT32);
extern void set_probe_local_number(INT32);

#define RESULT_STR_PRINT(RETURN_VALUE)\
    do {\
        printf("file:%s,func:%s,line%d,err_str:%s",__FILE__,__func__,__LINE__,bm_result[RETURN_VALUE]);\
    } while (0)


#define RESULT_HEX_PRINT(RETURN_VALUE)\
    do {\
        printf("file:%s,func:%s,line%d,err_hex:0x%x",__FILE__,__func__,__LINE__,RETURN_VALUE);\
    } while (0)


INT32 initClientDbgMsgQueue();
INT32 sendDbgMsg(char* _command);

INT32 initServerDbgMsgQueue();
extern void* receDbgMsgThread(void*);

#define DEAL_WITH_INIT() INT32 BREAK_FALG=0;
#define DEAL_WITH_SCANF_ERR() \
{\
    UINT8 flag = 0;\
    char c;\
    while((c = getchar()) != '\n' && c != EOF)\
    {\
        /*输入异常时的处理*/\
        if(c == 'q' && flag == 0)\
        {\
            while((c = getchar()) != '\n' && c != EOF)\
            {\
            }\
            return;\
        }\
        flag++;\
    }\
}
#define DEAL_WITH_SCANF_ERR_BREAK() \
{\
    UINT8 flag = 0;\
    char c;\
    while((c = getchar()) != '\n' && c != EOF)\
    {\
        /*输入异常时的处理*/\
        if(c == 'q' && flag == 0)\
        {\
            while((c = getchar()) != '\n' && c != EOF)\
            {\
            }\
            BREAK_FALG=1;\
            break;\
        }\
        flag++;\
    }\
}

#define DEAL_WITH_SCANF_ERR_BREAK_END() \
    if(BREAK_FALG==1)\
    {\
        break;\
    }

#define UNWARING(x) (x=x)

#ifdef __cplusplus
}
#endif

#endif // PUBLIC_H
