include "timeline_comm.h"
#include "timeline_getphoto.h"
#include "storage_timeline_url_generator.h"


using namespace timeline;


template<typename Req, typename Rsp>
int CTimeLineGetPhotoState::AddAction(CTimeLineMsg *msg, CActionSet *pActionSet)
{
	if ((NULL == msg->pReqBody) || (NULL == msg->pRspBody)
		|| (NULL == msg->pBlobData) || (0 == msg->iBlobLen))
	{
		TIMELINE_ERROR("pReqBody[%p], pRspBody[%p], pBlobData[%p], iBlobLen[%d], errinfo=frame error", msg->pReqBody, msg->pRspBody, msg->pBlobData, msg->iBlobLen);
		return -1;
	}
	
	Req *pReq = (Req *)(msg->pReqBody);
	Rsp *pRsp= (Rsp *)(msg->pRspBody);

	/*添加Action*/
	CNonStateActionInfo *pAction = NULL;
	static CTimeLineGetPhotoAction cGetIndexUinAction;

	try
	{
		pAction = new CNonStateActionInfo(1024);
	}
	catch (...)
	{
		TIMELINE_ERROR("new AddAction exception, appid[%s], userid[%s], uin[%u], albumid[%s]",
			pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(),
			msg->iPostUin, pReq->sIndexUinId.c_str());

		pRsp->rspComm.iRet = QZONE::eSvrErr;
		pRsp->rspComm.sMsg = "spp internal err";
		return -2;
	}

	int index = 0;
	pAction->SetRouteID(TIMELINE_SVR_MODID, TIMELINE_SVR_CMDID);
	pAction->SetID(index);
	pAction->SetProto(ProtoType_TCP);
	pAction->SetActionType(ActionType_SendRecv_KeepAliveWithPending);
	pAction->SetTimeout(TIMELINE_SVR_TIMEOUT_MS);
	pAction->SetIActionPtr((IAction *)&cGetIndexUinAction);
	int ret = pActionSet->AddAction(pAction);
	if (0 != ret)
	{
		TIMELINE_ERROR("AddAction() err[%d], appid[%s], userid[%s], uin[%u], albumid[%s]",
			ret, pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(),
			msg->iPostUin, pReq->sIndexUinId.c_str());

		pRsp->rspComm.iRet = QZONE::eSvrErr;
		pRsp->rspComm.sMsg = "spp internal err";
		return -3;
	}

	TIMELINE_DEBUG("AddAction(), appid[%s], userid[%s], uin[%u], albumid[%s]",
			pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(),
			msg->iPostUin, pReq->sIndexUinId.c_str());

	return 0;
}

template<typename Req, typename Rsp>
int CTimeLineGetPhotoAction::GetIndexUinEncode(CTimeLineMsg *msg, char *buf, int &len)
{
	TIMELINE_DEBUG("Action Encode::cmd[%d]...", msg->iCmd);
	if ((NULL == msg->pReqBody) || (NULL == msg->pRspBody))
	{
		TIMELINE_ERROR("pReqBody=%p,pRspBody=%p, frame error", msg->pReqBody, msg->pRspBody);

		return -1;
	}

	Req *pReq = (Req *)(msg->pReqBody);
	Rsp *pRsp= (Rsp *)(msg->pRspBody);

	struct timeval ct;
	::gettimeofday(&ct,NULL);
	srand(ct.tv_usec);
	unsigned int seq = (unsigned)rand();	

	string sParent="";
	if (pReq->sParentIndexUinId.empty())
		sParent=pReq->reqComm.sUserId;
	else
		sParent=pReq->sParentIndexUinId;	

	char *timeline_asyn_buf = NULL;
	int timeline_asyn_len = 0;
	int ret = g_timeline_asyn_api->query_node_encode(seq, pReq->reqComm.sAppId,
								msg->iPostUin,
								pReq->reqComm.sUserId,
								sParent,
								pReq->sIndexUinId,
								&timeline_asyn_buf,
								timeline_asyn_len);
	if ( ret!=0 )
	{
		TIMELINE_ERROR("g_timeline_asyn_api->query_node_encode() err[%d], appid[%s], userid[%s], uin[%u]",
			ret, pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(),
			msg->iPostUin);

		/*释放异步api里面的buffer*/
		TIMELINE_RELEASE_PTR(timeline_asyn_buf);

		pRsp->rspComm.iRet = QZONE::eSvrErr;
		pRsp->rspComm.sMsg = "query_node_encode err";
		return ret;
	}

	if ( timeline_asyn_len>len )
	{
		TIMELINE_DEBUG("timeline_asyn_len[%d] > len[%d]", timeline_asyn_len, len);

		/*释放异步api里面的buffer*/
		TIMELINE_RELEASE_PTR(timeline_asyn_buf);
		
		return timeline_asyn_len;
	}

	/*架平异步打包api里面new了一个buffer，只好memcpy一下*/
	memcpy(buf, timeline_asyn_buf, timeline_asyn_len);
	len = timeline_asyn_len;

	/*释放异步api里面的buffer*/
	TIMELINE_RELEASE_PTR(timeline_asyn_buf);

	TIMELINE_DEBUG("Action Encode ok, timeline_asyn_len[%d]", timeline_asyn_len);
	return 0;
}

template<typename Req, typename Rsp>
int CTimeLineGetPhotoState::GetIndexUinDecode(CTimeLineMsg *msg, CActionSet *pActionSet, NODEINFO *p_node_info, unsigned int &iIndex)
{
	if ((NULL == msg->pReqBody) || (NULL == msg->pRspBody))
	{
		TIMELINE_ERROR("pReqBody=%p,pRspBody=%p, frame error", msg->pReqBody, msg->pRspBody);

		return -1;
	}

	Req *pReq = (Req *)(msg->pReqBody);
	Rsp *pRsp= (Rsp *)(msg->pRspBody);

	CActionSet::ActionSet &action_set = pActionSet->GetActionSet();
	if ( action_set.empty() )
	{
		pRsp->rspComm.iRet = QZONE::eSvrErr;
		pRsp->rspComm.sMsg = "spp internal err";
		return -1;
	}
	CActionInfo *pAction = *(action_set.begin());
	
	int action_err = 0;
	pAction->GetErrno(action_err);
	if ( action_err!=0 )
	{
		string dest_ip;
		unsigned short dest_port = 0;
		pAction->GetDestIp(dest_ip);
		pAction->GetDestPort(dest_port);
		TIMELINE_ERROR("dst[%s:%d], action_err[%d], appid[%s], userid[%s], uin[%u]",
			dest_ip.c_str(), dest_port, action_err,
			pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), msg->iPostUin);

		if ( action_err!=EEncodeFail )//非插件代码里面的错误
		{
		/*
		和aoxu确认了，在spp内部，action的HandleProcess函数是没判断返回值的
		在state里面无法知道action的HandleProcess的处理结果是如何，
		如果关心action的HandleProcess返回值，只有把action的HandleProcess代码挪到state的HandleProcess里面来
		*/
			char szSppErr[64] = {0};
			snprintf(szSppErr, sizeof(szSppErr), "spp err[%d]", action_err);
			pRsp->rspComm.iRet = QZONE::eSvrErr;
			pRsp->rspComm.sMsg = string(szSppErr);
		}
		return action_err;
	}

	char *buf = NULL;
	int len=0;
	pAction->GetBuffer(&buf, len);

	if ( (NULL==buf) || (len<=0) )
	{
		TIMELINE_ERROR("buf[%p], len[%d], invalid!", buf, len);
		pRsp->rspComm.iRet = QZONE::eSvrErr;
		pRsp->rspComm.sMsg = "spp internal err";
		return -3;
	}

	TimeLineMessage *pTimeLineMsg = NULL;
	int datalen_nouse = 0;
	unsigned int seq = 0;
	char *pBuf = const_cast<char *>(buf);
	int ret = g_timeline_asyn_api->GetMessage(pBuf, len, datalen_nouse, seq, &pTimeLineMsg);
	if ( ret!=0 )
	{
		TIMELINE_ERROR("g_timeline_asyn_api->GetMessage() err[%d], appid[%s], userid[%s], uin[%u], albumid[%s]",
			ret, pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), msg->iPostUin, pReq->sIndexUinId.c_str());

		pRsp->rspComm.iRet = QZONE::eSvrErr;
		pRsp->rspComm.sMsg = "timeline_asyn_api err";
		
		if ( NULL!=pTimeLineMsg )
		{
			g_timeline_asyn_api->ReleasePhotoMsg(pTimeLineMsg);
		}

		return ret;
	}
		
	int retcode = 0;
	unsigned int iTotal = 0;
	string albumid;
	ret = g_timeline_asyn_api->query_node_decode(pTimeLineMsg, iIndex, p_node_info, retcode);
	if ( 0==ret ){
		/*和johnsonli确认ret和retcode没有交集，为了下面写代码方便，给ret赋值retcode*/
		TIMELINE_DEBUG("ret=0, retcode[%d]", retcode);
		ret = retcode;
	}
	
	if ( NULL!=pTimeLineMsg )
	{
		g_timeline_asyn_api->ReleasePhotoMsg(pTimeLineMsg);
	}

	if (ret!=0)
	{
		if (API_TimeLineDataNotExist(ret))//查询的节点不存在
		{
			TIMELINE_DEBUG("album does not exist,, appid[%s], userid[%s], uin[%u], albumid[%s]",
				pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), pReq->sIndexUinId.c_str());
			pRsp->rspComm.iRet = QZONE::eTargetNotExist;
			pRsp->rspComm.sMsg = "album does not exist";
			return 0;
		}
		else
		{  	    
			TIMELINE_ERROR("g_timeline_asyn_api->query_node_decode() err[%d], appid[%s], userid[%s], uin[%u], albumid[%s]",
				ret, pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(),
				msg->iPostUin, pReq->sIndexUinId.c_str());
			pRsp->rspComm.iRet = QZONE::eTimeLineStoreErr;
			pRsp->rspComm.sMsg = "query_node_decode failed";
			return ret;
		}
	}

	if (p_node_info->node_type!=FOLDER_TYPE)
	{
		TIMELINE_ERROR("note_type err, note_type[%d],, appid[%s], userid[%s], uin[%u], albumid[%s]",
			p_node_info->node_type, pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), msg->iPostUin, pReq->sAlbumId.c_str());

		pRsp->rspComm.iRet = QZONE::eTimeLineStoreDataExc;
		pRsp->rspComm.sMsg = "node_type error";
		return -4;
	}
	
	return ret;
}

int CTimeLineGetPhotoState::HandleEncode(CAsyncFrame *pFrame, CActionSet *pActionSet, CMsgBase *pMsg)
{
	CTimeLineMsg *msg = (CTimeLineMsg *)pMsg;
	msg->m_timer.start();
	TIMELINE_DEBUG("cmd[%d]...", msg->iCmd);
	
	if ((NULL == msg->pReqBody) || (NULL == msg->pRspBody))
	{
		TIMELINE_ERROR("pReqBody=%p,pRspBody=%p, frame error", msg->pReqBody, msg->pRspBody);

		return -1;
	}

	int ret = 0;
	switch ( msg->iCmd )
	{
		case TIMELINE_CMD_GET_ALBUM:
		{
			stGetIndexUinReq *pReq = (stGetIndexUinReq *)msg->pReqBody;
			stGetIndexUinRsp *pRsp = (stGetIndexUinRsp *)msg->pRspBody;
	
			ret = AddAction<stGetIndexUinReq, stGetIndexUinRsp>( msg, pActionSet );
			if ( ret!=0 )
			{
				TIMELINE_ERROR("TIMELINE_CMD_GET_ALBUM:AddAction() err[%d], appid[%s], userid[%s], uin[%u], albumid[%s], sParentAlbumId[%s], iGetPhotoCnt[%d]",
					ret, pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), msg->iPostUin,
					pReq->sAlbumId.c_str(), pReq->sParentAlbumId.c_str(), pReq->iGetPhotoCnt);
			}

			break;
		}
		case TIMELINE_CMD_MODIFY_ALBUM:
		{
			stModifyAlbumReq *pReq = (stModifyAlbumReq *)msg->pReqBody;
			stModifyAlbumRsp *pRsp = (stModifyAlbumRsp *)msg->pRspBody;

			if(pReq->sTitle.size() > TIMELINE_SIZE_ALBUM_NAME
				|| pReq->sDesc.size() > TIMELINE_SIZE_ALBUM_DESC)
			{
				TIMELINE_ERROR("TIMELINE_CMD_MODIFY_ALBUM:size error, title:%u, desc:%u, appid[%s], userid[%s], uin[%u], "
					"albumid[%s], sParentAlbumId[%s], sCoverId[%s], iPrivacy[%d], iType[%d]",
					pReq->sTitle.size(), pReq->sDesc.size(), pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), msg->iPostUin,
					pReq->sAlbumId.c_str(), pReq->sParentAlbumId.c_str(), pReq->sCoverId.c_str(), pReq->iPrivacy, pReq->iType);

				TIMELINEMONLOG_V_ERROR(msg->stSubId, msg->stHippoReqInfo, msg->m_timer, QZONE::eParamInvalid, "passive.storage", "itf_storage.get_album",
					"albumid:"+pReq->sAlbumId, 0,
					"TIMELINE_CMD_MODIFY_ALBUM:QZONE::eParamInvalid, appid[%s], userid[%s], uin[%u], "
					"albumid[%s], sParentAlbumId[%s], sCoverId[%s], iPrivacy[%d], iType[%d]",
					pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), msg->iPostUin,
					pReq->sAlbumId.c_str(), pReq->sParentAlbumId.c_str(), pReq->sCoverId.c_str(), pReq->iPrivacy, pReq->iType);
					
				pRsp->rspComm.iRet = QZONE::eParamInvalid;
				pRsp->rspComm.sMsg = "param error";
				return -1;
			}
    
			ret = AddAction<stModifyAlbumReq, stModifyAlbumRsp>( msg, pActionSet );
			if ( ret!=0 )
			{
				TIMELINE_ERROR("TIMELINE_CMD_MODIFY_ALBUM:AddAction() err[%d], appid[%s], userid[%s], uin[%u], "
				"albumid[%s], sParentAlbumId[%s], sCoverId[%s], iPrivacy[%d], iType[%d]",
					ret, pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), msg->iPostUin,
					pReq->sAlbumId.c_str(), pReq->sParentAlbumId.c_str(), pReq->sCoverId.c_str(), pReq->iPrivacy, pReq->iType);
			}

			break;
		}
		
		default:
			TIMELINE_ERROR("unknowed cmd[%d]", msg->iCmd);
			ret =  -1;
	}

	return ret;
}

int CTimeLineGetPhotoState::HandleProcess(CAsyncFrame *pFrame, CActionSet *pActionSet, CMsgBase *pMsg)
{
/*处理结果*/
	CTimeLineMsg *msg = (CTimeLineMsg *)pMsg;

	int ret = 0;
	unsigned int iIndex = 0;
	NODEINFO node_info;

	CActionSet::ActionSet &action_set = pActionSet->GetActionSet();
	if ( action_set.empty() )
	{
		TIMELINE_ERROR("action_set is empty!!!");
		return TIMELINESTATE_FINISH;
	}
	CActionInfo *pAction = *(action_set.begin());

	int action_err = 0;
	string dest_ip;
	unsigned short dest_port = 0;
	pAction->GetDestIp(dest_ip);
	pAction->GetDestPort(dest_port);
	pAction->GetErrno(action_err);
		
	switch ( msg->iCmd )
	{
		case TIMELINE_CMD_GET_ALBUM:
		{
			stGetIndexUinReq *pReq = (stGetIndexUinReq *)msg->pReqBody;
			stGetIndexUinRsp *pRsp = (stGetIndexUinRsp *)msg->pRspBody;
	
			ret = GetIndexUinDecode<stModifyAlbumReq, stModifyAlbumRsp>( msg, pActionSet, &node_info, iIndex );
			if ( ret!=0 )
			{
				TIMELINE_ERROR("TIMELINE_CMD_GET_ALBUM:GetIndexUinDecode() err[%d], appid[%s], userid[%s], uin[%u], albumid[%s], sParentAlbumId[%s], iGetPhotoCnt[%d]",
					ret, pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), msg->iPostUin,
					pReq->sAlbumId.c_str(), pReq->sParentAlbumId.c_str(), pReq->iGetPhotoCnt);

				TIMELINEMONLOG_V_ERROR(msg->stSubId, msg->stHippoReqInfo, msg->m_timer, ret, "passive.storage", "itf_storage.getuin",
					"albumid:"+pReq->sAlbumId, ntohl( inet_addr(dest_ip.c_str())),
					"TIMELINE_CMD_GET_ALBUM:GetIndexUinDecode() err[%d], dst[%s:%d], appid[%s], userid[%s], uin[%u], "
					"albumid[%s], sParentAlbumId[%s], iGetPhotoCnt[%d]",
					ret, dest_ip.c_str(), dest_port, pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), msg->iPostUin,
					pReq->sAlbumId.c_str(), pReq->sParentAlbumId.c_str(), pReq->iGetPhotoCnt);

				return TIMELINESTATE_FINISH;
			}

			pRsp->iIndex = iIndex;

			stTimeLineAlbum TimeLineAlbum;

			TimeLineAlbum.sAlbumId=node_info.node_key;
			TimeLineAlbum.iCreateUin=node_info.folder_node.uin;
			TimeLineAlbum.iCreateTime=node_info.folder_node.createtime;
			TimeLineAlbum.iUpdateTime=node_info.folder_node.updatetime;
			TimeLineAlbum.iLastUploadTime=node_info.folder_node.lastuploadtime;
			TimeLineAlbum.iDiskUsed=node_info.folder_node.diskused;
			TimeLineAlbum.iPhotoCnt=node_info.folder_node.file_num;
			TimeLineAlbum.iAlbumCnt=node_info.folder_node.folder_num;

			stTimeLineAlbumBiz TimeLineAlbumBiz;
			if (node_info.biz_blob.size()!=0)
			{
				qzone::CPduParser pduParser;
				ret = pduParser.Decode<stTimeLineAlbumBiz>(node_info.biz_blob.data(), 
											node_info.biz_blob.size(), 
											TimeLineAlbumBiz,
											false);
				if (ret!=0)
				{
					TIMELINE_ERROR("pduParser.Decode() failed ,ret[%d],, appid[%s], userid[%s], uin[%u], albumid[%s], sParentAlbumId[%s], iGetPhotoCnt[%d]",
						ret, pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), msg->iPostUin,
						pReq->sAlbumId.c_str(), pReq->sParentAlbumId.c_str(), pReq->iGetPhotoCnt);

					pRsp->rspComm.iRet = QZONE::eTimeLineStoreDataExc;
					pRsp->rspComm.sMsg = "decode stTimeLineAlbumBiz error";
					return TIMELINESTATE_FINISH;		
				}
			}
			else
			{
				TIMELINE_ERROR("node_info.biz_blob.size()=0, appid[%s], userid[%s], uin[%u], albumid[%s], sParentAlbumId[%s], iGetPhotoCnt[%d]",
					pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), msg->iPostUin,
					pReq->sAlbumId.c_str(), pReq->sParentAlbumId.c_str(), pReq->iGetPhotoCnt);
			}	

			TimeLineAlbum.sTitle=TimeLineAlbumBiz.sTitle;
			TimeLineAlbum.sDesc=TimeLineAlbumBiz.sDesc;
			if (TimeLineAlbumBiz.sCoverId.empty())
			{
				TimeLineAlbum.sCoverUrl.clear();
			}
			else
			{
				unsigned int updateTime;
				parse_group_photo_coverpid(TimeLineAlbumBiz.sCoverId, TimeLineAlbum.sCoverId, updateTime);
				if(TimeLineAlbum.sCoverId.empty())
				{
					TimeLineAlbum.sCoverId = TimeLineAlbumBiz.sCoverId;
				}

				storage_generate_normal_timeline_url(
					pReq->reqComm.sAppId,
					node_info.node_key,
					TimeLineAlbum.sCoverId,
					TimeLineAlbum.sCoverUrl);
				TIMELINE_DEBUG("URL=%s", TimeLineAlbum.sCoverUrl.c_str());
			}

			TimeLineAlbum.iPrivacy=TimeLineAlbumBiz.iPrivacy;
			TimeLineAlbum.iType=TimeLineAlbumBiz.iType;
			TimeLineAlbum.iCommentCnt=TimeLineAlbumBiz.iCommentCnt;
			TimeLineAlbum.vecComment.clear();
			TimeLineAlbum.mapExt=TimeLineAlbumBiz.mapExt;

			pRsp->rspComm.iRet = QZONE::eSucc;
			pRsp->rspComm.sMsg = "success";
			pRsp->iRowCnt=1;
			pRsp->TimeLineAlbum=TimeLineAlbum;

			TIMELINEMONLOG_V_DEBUG(msg->stSubId, msg->stHippoReqInfo, msg->m_timer, 0, "passive.storage", "itf_storage.get_album",
				"albumid:"+pReq->sAlbumId, ntohl( inet_addr(dest_ip.c_str())),
				"TIMELINE_CMD_GET_ALBUM:GetIndexUinDecode() err[%d], dst[%s:%d], appid[%s], userid[%s], uin[%u], "
				"albumid[%s], sParentAlbumId[%s], iGetPhotoCnt[%d]",
				ret, dest_ip.c_str(), dest_port, pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), msg->iPostUin,
				pReq->sAlbumId.c_str(), pReq->sParentAlbumId.c_str(), pReq->iGetPhotoCnt);
					
			return TIMELINESTATE_GET_ALBUM_2_PHOTOLIST;
		}
		case TIMELINE_CMD_MODIFY_ALBUM:
		{
			stModifyAlbumReq *pReq = (stModifyAlbumReq *)msg->pReqBody;
			stModifyAlbumRsp *pRsp = (stModifyAlbumRsp *)msg->pRspBody;
  
			ret = GetIndexUinDecode<stModifyAlbumReq, stModifyAlbumRsp>( msg, pActionSet, &(msg->stAlbumNodeInfo), iIndex );
			if ( ret!=0 )
			{
				TIMELINE_ERROR("TIMELINE_CMD_MODIFY_ALBUM:GetIndexUinDecode() err[%d], appid[%s], userid[%s], uin[%u], "
					"albumid[%s], sParentAlbumId[%s], sCoverId[%s], iPrivacy[%d], iType[%d]",
					ret, pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), msg->iPostUin,
					pReq->sAlbumId.c_str(), pReq->sParentAlbumId.c_str(), pReq->sCoverId.c_str(), pReq->iPrivacy, pReq->iType);

				TIMELINEMONLOG_V_ERROR(msg->stSubId, msg->stHippoReqInfo, msg->m_timer, ret, "passive.storage", "itf_storage.getuin",
					"albumid:"+pReq->sAlbumId, ntohl( inet_addr(dest_ip.c_str())),
					"TIMELINE_CMD_MODIFY_ALBUM:GetIndexUinDecode() err[%d], dst[%s:%d], appid[%s], userid[%s], uin[%u], "
					"albumid[%s], sParentAlbumId[%s], sCoverId[%s], iPrivacy[%d], iType[%d]",
					ret, dest_ip.c_str(), dest_port, pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), msg->iPostUin,
					pReq->sAlbumId.c_str(), pReq->sParentAlbumId.c_str(), pReq->sCoverId.c_str(), pReq->iPrivacy, pReq->iType);
			}

			TIMELINEMONLOG_V_DEBUG(msg->stSubId, msg->stHippoReqInfo, msg->m_timer, 0, "passive.storage", "itf_storage.getuin",
				"albumid:"+pReq->sAlbumId, ntohl( inet_addr(dest_ip.c_str())),
				"TIMELINE_CMD_MODIFY_ALBUM:GetIndexUinDecode() err[%d], dst[%s:%d], appid[%s], userid[%s], uin[%u], "
				"albumid[%s], sParentAlbumId[%s], sCoverId[%s], iPrivacy[%d], iType[%d]",
				ret, dest_ip.c_str(), dest_port, pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), msg->iPostUin,
				pReq->sAlbumId.c_str(), pReq->sParentAlbumId.c_str(), pReq->sCoverId.c_str(), pReq->iPrivacy, pReq->iType);
			return TIMELINESTATE_MODIFY_ALBUM;
		}
		
		default:
			TIMELINE_ERROR("unknowed cmd[%d]", msg->iCmd);
			return TIMELINESTATE_FINISH;
	} 
}

int CTimeLineGetPhotoAction::HandleEncode(CAsyncFrame *pFrame, char *buf, int &len, CMsgBase *pMsg)
{
/*架平异步打包接口*/
	CTimeLineMsg *msg = (CTimeLineMsg *)pMsg;
	TIMELINE_DEBUG("cmd[%d]...", msg->iCmd);
	if ((NULL == msg->pReqBody) || (NULL == msg->pRspBody))
	{
		TIMELINE_ERROR("pReqBody=%p,pRspBody=%p, frame error", msg->pReqBody, msg->pRspBody);

		return -1;
	}

	int ret = 0;
	switch ( msg->iCmd )
	{
		case TIMELINE_CMD_GET_ALBUM:
		{
			stGetIndexUinReq *pReq = (stGetIndexUinReq *)msg->pReqBody;
			stGetIndexUinRsp *pRsp = (stGetIndexUinRsp *)msg->pRspBody;
			
			ret = GetIndexUinEncode<stGetIndexUinReq, stGetIndexUinRsp>( msg, buf, len );
			if ( ret!=0 )
			{
				TIMELINE_ERROR("TIMELINE_CMD_GET_ALBUM:GetIndexUinEncode() err[%d], appid[%s], userid[%s], uin[%u], albumid[%s], sParentAlbumId[%s], iGetPhotoCnt[%d]",
					ret, pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), msg->iPostUin,
					pReq->sAlbumId.c_str(), pReq->sParentAlbumId.c_str(), pReq->iGetPhotoCnt);
			}

			break;
		}
		case TIMELINE_CMD_MODIFY_ALBUM:
		{
			stModifyAlbumReq *pReq = (stModifyAlbumReq *)msg->pReqBody;
			stModifyAlbumRsp *pRsp = (stModifyAlbumRsp *)msg->pRspBody;

			ret = GetIndexUinEncode<stModifyAlbumReq, stModifyAlbumRsp>( msg, buf, len );
			
			if ( ret!=0 )
			{
				TIMELINE_ERROR("TIMELINE_CMD_MODIFY_ALBUM:GetIndexUinEncode() err[%d], appid[%s], userid[%s], uin[%u], "
					"albumid[%s], sParentAlbumId[%s], sCoverId[%s], iPrivacy[%d], iType[%d]",
					ret, pReq->reqComm.sAppId.c_str(), pReq->reqComm.sUserId.c_str(), msg->iPostUin,
					pReq->sAlbumId.c_str(), pReq->sParentAlbumId.c_str(), pReq->sCoverId.c_str(), pReq->iPrivacy, pReq->iType);
			}

			break;
		}
		
		default:
			TIMELINE_ERROR("unknowed cmd[%d]", msg->iCmd);
			ret = -1;
	}
	
	return ret;
}

int CTimeLineGetPhotoAction::HandleInput(CAsyncFrame *pFrame, const char *buf, int len, CMsgBase *pMsg)
{
/*检查响应包完整性*/
	unsigned pkt_len = 0;
	
	int ret_len = asn_complete_func(buf, len, pkt_len);
	TIMELINE_DEBUG("Action Input, ret_len[%d], len[%d], pkt_len[%d]", ret_len, len, pkt_len);

	if ( 0==ret_len )
	{//空包或者包不完整
		return 0;
	}
	else if ( ret_len<0 )
	{//包有问题
		TIMELINE_ERROR("Action Input:invalid packet, ret_len[%d]", ret_len);
		return -1;
	}

	/*恭喜，收包完成*/
	return ret_len;
}

int CTimeLineGetPhotoAction::HandleProcess(CAsyncFrame *pFrame, const char *buf, int len, CMsgBase *pMsg)
{
	return 0;
}

int CTimeLineGetPhotoAction::HandleError(CAsyncFrame *pFrame, int err_no, CMsgBase *pMsg)
{
/*出错处理*/
	TIMELINE_ERROR("err_no[%d]", err_no);
	return 0;
}

