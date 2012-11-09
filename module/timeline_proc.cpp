#include "timeline_proc.h"
#include "timeline_msg.h"
#include "timeline_define.h"
#include "timeline_photo_asyn_api.h"

#include "photo_unify_monitor_log.h"

#include "timeline_getphoto.h"

#include "timeline_comm.h"
#include "server_config.h"
#include "pdu_parser.h"
#include "qza_protocol.h"

#include "photo_net.h"

using namespace qzone;
using namespace timeline;

CTimeLineProc::CTimeLineProc()
{
}

CTimeLineProc::~CTimeLineProc()
{
}

int CTimeLineProc::HandleInit(const char *etc, CServerBase *base)
{
    int ret = 0;
    //FIXME: ����sConfFile����ȡ����
    ret = CONFIG->ParseFile(etc);
    if (0 != ret)
    {
        TimeLine_ERROR("ParseFile: ret=%d", ret);
        return -1;
    }

    //��ʼ���첽���
    ret = CAsyncFrame::Instance()->InitFrame2(base, MAX_PENDING_CONN_NUM);
    if (0 != ret) {TIMELINE_ERROR("InitFrame2: ret=%d", ret); return ret;}

    //ע��ص�����
    ret = CAsyncFrame::Instance()->RegCallback(CAsyncFrame::CBType_Init, CTimeLineProc::InitState);
    if (0 != ret) {TIMELINE_ERROR("RegCallback.CBType_Init: ret=%d", ret); return ret;}
    ret = CAsyncFrame::Instance()->RegCallback(CAsyncFrame::CBType_Fini, CTimeLineProc::FiniState);
    if (0 != ret) {TIMELINE_ERROR("RegCallback.CBType_Fini: ret=%d", ret); return ret;}

    }

    //�������������Ҫ������״̬IState�ӿ�   
    ret = CAsyncFrame::Instance()->AddState(TIMELINE_GETPHOTO, new CTimeLineGetPhotoState);
    if (0 != ret) {TIMELINE_ERROR("AddState: ret=%d,statid=%d", ret, TIMELINE_GETPHOTO); return ret;}    
    
	InitTimeLineClog();

	return ret;
}

int CTimeLineProc::InitTimeLineClog()
{
/*������:��ͨ�û�ֻ��error��alphaȫ��
 *д����:�����û�ȫ��
 *Ĭ������Ϊ"������"���ã�stCLogWrite�ٶ�ȡ���ø���
*/
	int config_ret = 0;

	config_ret = stCLogRead.initLogging("active.timelinesvr", NS_QZDATA::QzDCLogDefs::ASYNC_CALL, "gbk", "/data/log/spp_timeline-2.3/TIMELINE_dclog", TIMELINE_MONITOR_FILENAME);
	if ( config_ret!=0 )
	{
		TIMELINE_ERROR("stCLogRead.initLogging() err[%d]", config_ret);
		return config_ret;
	}
	
	config_ret = stCLogWrite.initLogging("active.timelinesvr", NS_QZDATA::QzDCLogDefs::ASYNC_CALL, "gbk", "/data/log/spp_timeline-2.3/TIMELINE_dclog", TIMELINE_MONITOR_FILENAME);
	if ( config_ret!=0 )
	{
		TIMELINE_ERROR("stCLogWrite.initLogging() err[%d]", config_ret);
		return config_ret;
	}

	CTimeLineMonitorConfig timeline_monitor_config;
	string str_dclog_switch_write = timeline_monitor_config["dclog_config_write.DCLOG_SWITCH"];
	string str_dclog_level_write = timeline_monitor_config["dclog_config_write.DCLOG_LEVEL"];

	int dclog_switch_write, dclog_level_write;
	if ( str_dclog_switch_write.empty() ){
		TIMELINE_ERROR("config empty!!dclog_config_write.DCLOG_SWITCH");
		dclog_switch_write = NS_QZDATA::QzDCLogDefs::DCLOG_REPORT_BY_LEVEL;
	} else {
		dclog_switch_write = atoi(str_dclog_switch_write.c_str());
	}

	if ( str_dclog_level_write.empty() ){
		TIMELINE_ERROR("config empty!!dclog_config_write.DCLOG_LEVEL");
		dclog_level_write = NS_QZDATA::QzDCLogDefs::DC_DEBUG;
	} else {
		dclog_level_write = atoi(str_dclog_level_write.c_str());
	}

	config_ret = stCLogWrite.setCustomLogging(static_cast<NS_QZDATA::QzDCLogDefs::DCLogSwitch>(dclog_switch_write),
		static_cast<NS_QZDATA::QzDCLogDefs::DCLogLevel>(dclog_level_write));
	if  ( config_ret!=0 )
	{
		TIMELINE_ERROR("stCLogWrite.setCustomLogging() err[%d], dclog_switch_write[%d], dclog_level_write[%d], str_dclog_switch_write[%s], str_dclog_level_write[%s]",
				config_ret, dclog_switch_write, dclog_level_write, str_dclog_switch_write.c_str(), str_dclog_level_write.c_str());

		return config_ret;
	}

	TIMELINE_DEBUG("stCLogWrite.setCustomLogging() ret[%d], dclog_switch_write[%d], dclog_level_write[%d], str_dclog_switch_write[%s], str_dclog_level_write[%s]",
		config_ret, dclog_switch_write, dclog_level_write, str_dclog_switch_write.c_str(), str_dclog_level_write.c_str());
	return 0;
}

int CTimeLineProc::HandleInput(blob_type *blob)
{    
    TConnExtInfo *extinfo = (TConnExtInfo *)blob->extdata;

    if (blob->len < (int)sizeof(QZAHEAD))
	{
	    //���ݰ���δ��������������������գ�
	    TIMELINE_DEBUG("HandleInput: blob->len=%d(<%d),info=continue recv",
	            blob->len, sizeof(QZAHEAD));
		return 0;
	}

	QZAHEAD* qzahead = (QZAHEAD*)blob->data;
	int iPackLen =qzahead->GetPackLen();
	if (blob->len > iPackLen)
	{
		TIMELINE_ERROR("remote=%s,msg=pack too big, blob_len:%d, pack_len:%u", 
		    inet_ntoa(*(struct in_addr*)&extinfo->remoteip_), blob->len, iPackLen);
		return -3;
	}
	else if (blob->len < iPackLen)
	{	    
	    //���ݰ���δ��������������������գ�
	    TIMELINE_DEBUG("HandleInput: blob->len=%d(<%d), ip:%s, info=continue recv",
	            blob->len, iPackLen, inet_ntoa(*(struct in_addr*)&extinfo->remoteip_));
		return 0;
	}
	TIMELINE_DEBUG("proxy: %d bytes received", iPackLen);
	
	return iPackLen;
}

int CTimeLineProc::HandleProcess(unsigned flow, void *arg1, void *arg2)
{
    TIMELINE_DEBUG("");
    
    //���ݿ���󣬽ṹ��ο�tcommu.h
    blob_type *blob = (blob_type *)arg1;
    TConnExtInfo *extinfo = (TConnExtInfo *)blob->extdata;
    //��������������
    CServerBase *base = (CServerBase *)arg2;
    //������Դ��ͨѶ�������
    CTCommu *commu = (CTCommu *)blob->owner;
   
    //���msg�ṹ
    CTimeLineMsg *msg = new CTimeLineMsg;
    msg->SetServerBase(base);
    msg->SetTCommu(commu);
    msg->SetFlow(flow);
    msg->SetInfoFlag(false);
    msg->extinfo = *extinfo;

    int ret = msg->Req2Msg(blob->data, blob->len);
	if (0 != ret)
	{
	    TIMELINE_ERROR("Req2Msg: ret=%d", ret);
	    delete msg;
	    msg = NULL;
	    //���ʧ�ܣ�����<0�������ر����ӣ�
        return -1;
	}

    //�����첽��ܴ�������
    ret = CAsyncFrame::Instance()->Process(msg);
    if (0 != ret)
	{
	    TIMELINE_ERROR("Process: ret=%d,cmd=%d", ret, msg->iCmd);
	}

	return 0;
}

int CTimeLineProc::HandleFini()
{
    TIMELINE_DEBUG("");
    
    CAsyncFrame::Instance()->FiniFrame();
    CAsyncFrame::Destroy();

    return 0;
}

int CTimeLineProc::InitState(CAsyncFrame *pFrame, CMsgBase *pMsg)
{
    //��ܿ�ʼ��������ʱ�ص������ķ���ֵ�ǵ�һ�������״̬ID��Ĭ��IDΪ1��
    CTimeLineMsg *msg = (CTimeLineMsg *)pMsg;
    TIMELINE_DEBUG("cmd=%d", msg->iCmd);
    return msg->GetFirstStat();
}

int CTimeLineProc::FiniState(CAsyncFrame *pFrame, CMsgBase *pMsg)
{
    //��ܽ���������ʱ�ص���IState::HandleProcess����ֵ<=0ʱ������ǰ��������ϣ�
    //��ʱ����ܻ�ص��������������һ������£�������ص���������Ը�ǰ�˿ͻ��˻ذ���
    int ret = 0;
    CTimeLineMsg *msg = (CTimeLineMsg *)pMsg;

    TIMELINE_DEBUG("FiniState, cmd=%d", msg->iCmd);

    //����Ӧ���İ���
	char *pRspBody = NULL;
	uint32_t iRspBodyLen = 0;
	qzone::CPduParser oPduParser;
    ret = msg->Msg2Rsp(oPduParser, pRspBody, iRspBodyLen);
	if (ret!=0)
	{
		TIMELINE_ERROR("Msg2Rsp failed, ret=%d", ret);
		return -1;
	}

    TIMELINE_DEBUG("head len=%d, body_len:%d, body:%p", msg->m_pQzaHead->GetHeadLen(), iRspBodyLen, pRspBody);

	//������Ӧ���İ�ͷ����
	msg->m_pQzaHead->SetPackLen(msg->m_pQzaHead->GetHeadLen() + iRspBodyLen);

	//��Ϸ��ذ�
    blob_type blob;
	blob.len=msg->m_pQzaHead->GetHeadLen() + iRspBodyLen;
	blob.data=(char*)malloc(blob.len);
	if (blob.data==NULL)
	{
		TIMELINE_ERROR("malloc failed, len=%d", blob.len);
		return -1;
	}
	memcpy(blob.data, msg->m_pQzaHead, msg->m_pQzaHead->GetHeadLen());
	memcpy(blob.data + msg->m_pQzaHead->GetHeadLen(), pRspBody, iRspBodyLen);

	ret = msg->SendToClient(blob);
    if (0 != ret)
    {
        TIMELINE_ERROR("SendToClient failed, ret=%d, blob.len:%d, cmd:%d", ret, blob.len, msg->iCmd);
    }
	else
	{
	    TIMELINE_DEBUG("client <----- blob.len:%d, cmd:%d", blob.len, msg->iCmd);
	}

	free(blob.data);
	
  	return 0;
}

