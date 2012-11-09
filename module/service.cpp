//�������spp��ͷ�ļ�
#include "sppincl.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "timeline_proc.h"
#include "timeline_macro.h"

#define MODULE_PROC_NUM	"module_proc_num"

CServerBase *g_ServerBase = NULL;

//��ʼ����������ѡʵ�֣�
//arg1:	�����ļ�
//arg2:	��������������
//����0�ɹ�����0ʧ��
extern "C" int spp_handle_init(void* arg1, void* arg2)
{
    //�������������ļ�
    const char *etc = (const char*)arg1;
    //��������������
    CServerBase *base = (CServerBase*)arg2; 
    base->log_.LOG_P(LOG_DEBUG, "spp_handle_init: etc=%s,svrtype=%d\n", etc, base->servertype());
    //����һ��ͳ����, ͳ�Ʋ���Ϊ�ۼ�
    //base->stat_.init_statobj(MODULE_PROC_NUM, STAT_TYPE_SUM);

    //���ڵ���LOG_P_ALL()�ȴ�log
    if  (!g_ServerBase)
        g_ServerBase = base;
    
	if (base->servertype() == SERVER_TYPE_WORKER)
	{
		//��ʼ��
		return CUPPProcSingle::instance()->HandleInit(etc, base);
	}
	
    return 0;
}

//���ݽ��գ�����ʵ�֣�
//flow:	�������־
//arg1:	���ݿ����
//arg2:	��������������
//����������ʾ�����Ѿ����������Ҹ�ֵ��ʾ���ݰ��ĳ��ȣ�0ֵ��ʾ���ݰ���δ����������������ʾ����
extern "C" int spp_handle_input(unsigned flow, void* arg1, void* arg2)
{
    //���ݿ���󣬽ṹ��ο�tcommu.h
    blob_type* blob = (blob_type*)arg1;
    //extinfo����չ��Ϣ
    TConnExtInfo* extinfo = (TConnExtInfo*)blob->extdata;
    //��������������
    CServerBase* base = (CServerBase*)arg2;

    UPP_DEBUG("spp_handle_input: \n");
	//���������Ϸ��ԣ����ذ�����
    return CUPPProcSingle::instance()->HandleInput(blob);
}

//·��ѡ�񣨿�ѡʵ�֣�
//flow:	�������־
//arg1:	���ݿ����
//arg2:	��������������
//����ֵ��ʾworker�����
extern "C" int spp_handle_route(unsigned flow, void* arg1, void* arg2)
{
    //��������������
    CServerBase* base = (CServerBase*)arg2;
    base->log_.LOG_P(LOG_DEBUG, "spp_handle_route, %d\n", flow);

    //FIXME: ��ʱֻ֧��һ��worker��
    return 1;
}

//���ݴ�������ʵ�֣�
//flow:	�������־
//arg1:	���ݿ����
//arg2:	��������������
//����0��ʾ�ɹ�����0ʧ��
extern "C" int spp_handle_process(unsigned flow, void* arg1, void* arg2)
{
	//������������CMsgBase��Ȼ�󽻸��첽��ܴ���
    return CUPPProcSingle::instance()->HandleProcess(flow, arg1, arg2);
}

//������Դ����ѡʵ�֣�
//arg1:	��������
//arg2:	��������������
extern "C" void spp_handle_fini(void* arg1, void* arg2)
{
    //��������������
    CServerBase* base = (CServerBase*)arg2;
    base->log_.LOG_P(LOG_DEBUG, "spp_handle_fini\n");
	
	if (base->servertype() == SERVER_TYPE_WORKER)
    {
        //���������Դ
        CUPPProcSingle::instance()->HandleFini();
    }
}


