#ifndef __TIMELINE_GETPHOTO_H__
#define __TIMELINE_GETPHOTO_H__

#include "sppincl.h"
USING_ASYNCFRAME_NS;

//����ִ����
//���ô洢��Ƚӿ���ɾ��������Ȼ�󽫽ṹ��䵽@msg.pRspBody
//NOTE: ͬ������ֻ��Ҫʵ��State��
class CTimeLineGetPhotoState : public IState
{
public:
    //���߿��Ӧ��ִ����Щ����
    virtual int HandleEncode(CAsyncFrame *pFrame, CActionSet *pActionSet, CMsgBase *pMsg);
    // ������ִ�еĽ��
    virtual int HandleProcess(CAsyncFrame *pFrame, CActionSet *pActionSet, CMsgBase *pMsg);

	template<typename Req, typename Rsp>
	int AddAction(CTimeLineMsg *msg, CActionSet *pActionSet);

	template<typename Req, typename Rsp>
	int GetPhotoDecode(CTimeLineMsg *msg, CActionSet *pActionSet, timeline::NODEINFO *p_node_info, unsigned int &iIndex);

};

class CTimeLineGetPhotoAction : public IAction
{
public:
    virtual int HandleEncode(CAsyncFrame *pFrame, char *buf, int &len, CMsgBase *pMsg);
    virtual int HandleInput(CAsyncFrame *pFrame, const char *buf, int len, CMsgBase *pMsg);
    virtual int HandleProcess(CAsyncFrame *pFrame, const char *buf, int len, CMsgBase *pMsg);
    virtual int HandleError(CAsyncFrame *pFrame, int err_no, CMsgBase *pMsg);

	template<typename Req, typename Rsp>
	int GetPhotoEncode(CTimeLineMsg *msg, char *buf, int &len);
};

#endif  //__TIMELINE_GETPHOTO_H__

