#include "common.h"
#include "errCode.h"


sErrToMsg errToMsgs[] =
{
    {SYS_RC_OK                  , SYS_RC_MSG_OK                      },
    {SYS_RC_INV_PARAM           , SYS_RC_MSG_INV_PARAM               },
    {SYS_RC_ERR                 , SYS_RC_MSG_ERR                     },
    {SYS_RC_CREAT_FAILED        , SYS_RC_MSG_CREAT_FAILED            },
    {SYS_RC_OPEN_FAILED         , SYS_RC_MSG_OPEN_FAILED             },
    {SYS_RC_INIT_FAILED         , SYS_RC_MSG_INIT_FAILED             },
    {SYS_RC_FIND_ERR            , SYS_RC_MSG_FIND_ERR                },
    {SYS_RC_GET_ERR             , SYS_RC_MSG_GET_ERR                 },
    {SYS_RC_INSERT_ERR          , SYS_RC_MSG_INSERT_ERR              },
    {SYS_RC_UPDATE_ERR          , SYS_RC_MSG_UPDATE_ERR              },
    {SYS_RC_DELETE_ERR          , SYS_RC_MSG_DELETE_ERR              },
    {SYS_RC_REGISTE_ERR         , SYS_RC_MSG_REGISTE_ERR             },
    {SYS_RC_MALLOC_ERR          , SYS_RC_MSG_MALLOC_ERR              },
    {SYS_RC_READ_ERR            , SYS_RC_MSG_READ_ERR                },
    {SYS_RC_WRITE_ERR           , SYS_RC_MSG_WRITE_ERR               },
    {SYS_RC_FILE_NOT_EXIST      , SYS_RC_MSG_FILE_NOT_EXIST          },
    {SYS_RC_FILE_STAT_ERR       , SYS_RC_MSG_FILE_STAT_ERR           },
    {SYS_RC_FILE_RM_ERR         , SYS_RC_MSG_FILE_RM_ERR             },
    {SYS_RC_FILE_TYPE_NOT_MATCH , SYS_RC_MSG_FILE_TYPE_NOT_MATCH     },
    {SYS_RC_RPC_CLNT_CREATE_ERR , SYS_RC_MSG_RPC_CLNT_CREATE_ERR     },
    {SYS_RC_RPC_CLNT_DESTROY_ERR, SYS_RC_MSG_RPC_CLNT_DESTROY_ERR    },
    {SYS_RC_CURL_EASY_INIT_ERR  , SYS_RC_MSG_CURL_EASY_INIT_ERR      },
    {SYS_RC_MD5_CHKSUM_ERR      , SYS_RC_MSG_MD5_CHKSUM_ERR          },
    {SYS_RC_LOCK_TEST_ERR       , SYS_RC_MSG_LOCK_TEST_ERR           },
    {SYS_RC_LOCK_ERR            , SYS_RC_MSG_LOCK_ERR                },
    {SYS_RC_PKG_INVALID         , SYS_RC_MSG_PKG_INVALID             },
};

//返回错误码的文字解释
char* RC_GetResultMsg_ByCode(int nResultCode)
{

    int i = 0;
    for(i = 0; i < (sizeof(errToMsgs) / sizeof(sErrToMsg)); i++)
    {
        if(errToMsgs[i].errCode == nResultCode)
        {
            return errToMsgs[i].errMsg;
        }
    }
    return "errCode not define\n";
}
