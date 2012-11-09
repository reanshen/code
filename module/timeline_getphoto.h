#ifndef __TIMELINE_GETPHOTO_H__
#define __TIMELINE_GETPHOTO_H__

#include "sppincl.h"
USING_ASYNCFRAME_NS;

//请求执行类
//调用存储层等接口完成具体操作，然后将结构填充到@msg.pRspBody
//NOTE: 同步请求只需要实现State类
class CTimeLineGetPhotoState : public IState
{
public:
    //告诉框架应该执行哪些动作
    virtual int HandleEncode(CAsyncFrame *pFrame, CActionSet *pActionSet, CMsgBase *pMsg);
    // 处理动作执行的结果
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

