#ifndef __TIMELINE_PROC_H__
#define __TIMELINE_PROC_H__ 

#include "server_config.h"
#include "sppincl.h"
#include "singleton.h"
#include "timeline_msg.h"
#include "photo_unify_monitor_log.h"

USING_ASYNCFRAME_NS;

//SVR框架类
//1. 初始化SPP异步框架
//2. decode PDU请求包，填充@msg.pReqBody结构
//   请求执行类处理请求并将处理结果填充@msg.pRspBody)
//3. 读@msg.pRspBody，encode PDU响应包，并向客服端发响应包
class CTimeLineProc
{
public:
	CTimeLineProc();
	~CTimeLineProc();

	int HandleInit(const char *sConfFile, CServerBase *base);
    int HandleInput(blob_type *blob);
	int HandleProcess(unsigned flow, void* arg1, void* arg2);
    int HandleFini();
    
	static int InitState(CAsyncFrame *pFrame, CMsgBase *pMsg);
	static int FiniState(CAsyncFrame *pFrame, CMsgBase *pMsg);

	int InitTimeLineClog();

	/*clog相关*/
	photo_log::PhotoUnifyMonitorLogger stCLogRead;
	photo_log::PhotoUnifyMonitorLogger stCLogWrite;
	
private:
	
};

typedef CSingleton<CTimeLineProc> CTimeLineProcSingle;
#endif

