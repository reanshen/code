#ifndef __TIMELINE_PROC_H__
#define __TIMELINE_PROC_H__ 

#include "server_config.h"
#include "sppincl.h"
#include "singleton.h"
#include "timeline_msg.h"
#include "photo_unify_monitor_log.h"

USING_ASYNCFRAME_NS;

//SVR�����
//1. ��ʼ��SPP�첽���
//2. decode PDU����������@msg.pReqBody�ṹ
//   ����ִ���ദ�����󲢽����������@msg.pRspBody)
//3. ��@msg.pRspBody��encode PDU��Ӧ��������ͷ��˷���Ӧ��
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

	/*clog���*/
	photo_log::PhotoUnifyMonitorLogger stCLogRead;
	photo_log::PhotoUnifyMonitorLogger stCLogWrite;
	
private:
	
};

typedef CSingleton<CTimeLineProc> CTimeLineProcSingle;
#endif

