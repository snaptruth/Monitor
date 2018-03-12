#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "public.h"
#include "errCode.h"

/* 全局调试开关*/
static UINT32 g_nDebugVal = 0;

/**
 *     函数功能：判断设置Info信息输出到标准终端是否打开
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
int IsDbgInfoOn_Stdio(void)
{
    return (g_nDebugVal & DEBUG_INFO_TO_STDIO);
}

/**
 *     函数功能：打开Info信息输出到标准终端的开关
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
void SetDbgInfoOn_Stdio(void)
{
    g_nDebugVal |= DEBUG_INFO_TO_STDIO;
    return;
}

/**
 *     函数功能：关闭Info信息输出到标准终端的开关
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
void SetDbgInfoOff_Stdio(void)
{
    g_nDebugVal &= ~DEBUG_ERR_TO_STDIO;
    return;
}

/**
 *     函数功能：判断设置Err信息输出到标准终端是否打开
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
int IsDbgErrOn_Stdio(void)
{
    return (g_nDebugVal & DEBUG_ERR_TO_STDIO);
}

/**
 *     函数功能：打开Err信息输出到标准终端的开关
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
void SetDbgErrOn_Stdio(void)
{
    g_nDebugVal |= DEBUG_ERR_TO_STDIO;
    return;
}

/**
 *     函数功能：关闭Err信息输出到标准终端的开关
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
void SetDbgErrOff_Stdio(void)
{
    g_nDebugVal &= ~DEBUG_INFO_TO_STDIO;
    return;
}

/**
 *     函数功能：打开所有信息输出到标准终端的开关
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
void SetDbgAllOn_Stdio(void)
{
    g_nDebugVal = DEBUG_ALL_STDIO;
    return;
}

/**
 *     函数功能：关闭所有信息输出到标准终端的开关
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
void SetDbgAllOff_Stdio(void)
{
    g_nDebugVal &= ~DEBUG_ALL_STDIO;
    return;
}

/**
 *     函数功能：判断设置Info信息输出到Term是否打开
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
int IsDbgInfoOn_Term(void)
{
    return (g_nDebugVal & DEBUG_INFO_TO_TERM);
}

/**
 *     函数功能：打开设置Info信息输出到Term的开关
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
void SetDbgInfoOn_Term(void)
{
    g_nDebugVal |= DEBUG_INFO_TO_TERM;
    return;
}

/**
 *     函数功能：关闭设置Info信息输出到Term的开关
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
void SetDbgInfoOff_Term(void)
{
    g_nDebugVal &= ~DEBUG_INFO_TO_TERM;
    return;
}

/**
 *     函数功能：判断设置Err信息输出到Term是否打开
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
int IsDbgErrOn_Term(void)
{
    return (g_nDebugVal & DEBUG_ERR_TO_TERM);
}

/**
 *     函数功能：打开设置Err信息输出到Term的开关
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
void SetDbgErrOn_Term(void)
{
    g_nDebugVal |= DEBUG_ERR_TO_TERM;
    return;
}

/**
 *     函数功能：关闭设置Err信息输出到Term
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
void SetDbgErrOff_Term(void)
{
    g_nDebugVal &= ~DEBUG_ERR_TO_TERM;
    return;
}

/**
 *     函数功能：打开设置所有信息输出到Term的开关
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
void SetDbgAllOff_Term(void)
{
    g_nDebugVal &= ~DEBUG_ALL_TERM;
    return;
}

/**
 *     函数功能：关闭设置所有信息输出到Term的开关
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
void SetDbgAllOn_Term(void)
{
    g_nDebugVal = DEBUG_ALL_TERM;
    return;
}

/**
 *     函数功能：获取debug值
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
void LMDB_GetDbgValue(UINT32* pValue)
{
    if(NULL != pValue)
    {
        *pValue = g_nDebugVal;
    }
}

/**
 *     函数功能：设置DEBUG值
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
void LMDB_SetDbgValue(UINT32 nValue)
{
    g_nDebugVal = nValue;
}

/**
 *     函数功能：去除结尾回车'\r' , '\n' 字符串
 *     @param
 *     @return
 *     @note
 *     @note    作   者   :
 *     @note    修改内容   : 新生成函数
 */
void LMDB_StripEnterChar(char* pStrings, int nLen)
{
    int i = 0;

    if(NULL == pStrings)
    {
        return;
    }

    for(i = 0; i < nLen; i++)
    {
        if(('\n' == pStrings[i]) ||
                ('\r' == pStrings[i]))
        {
            pStrings[i] = '\0';
        }
    }
}
