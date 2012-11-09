#include "upp_msg.h"
#include "upp_macro.h"
#include "upp_define.h"

#include "qzdata_pub_struct.h"
#include "qzbitmap_l5client.h"
#include "qzbitmap_api.h"

#include "panel_auth.h"
#include "ptlogin_app_id.h"

using namespace upp;

CUPPMsg::CUPPMsg()
{
    iCmd = 0;
    pReqBody = NULL;
    pRspBody = NULL;
	pBlobData= NULL;
	iBlobLen= 0;
	m_pQzaHead = NULL;
    
    memset(&extinfo, 0, sizeof(TConnExtInfo));
}

CUPPMsg::~CUPPMsg()
{
    int ret = 0;
    switch (iCmd)
    {
    case UPP_CMD_ADD_ALBUM:
        ret = DelBody<stAddAlbumReq,stAddAlbumRsp>();
        break;
    case UPP_CMD_LIST_ALBUM_BYIDX:
        ret = DelBody<stListAlbumByIndexReq,stListAlbumRsp>();
        break;
    case UPP_CMD_MODIFY_ALBUM:
        ret = DelBody<stModifyAlbumReq,stModifyAlbumRsp>();
        break;
    case UPP_CMD_DEL_ALBUM:
        ret = DelBody<stDelAlbumReq,stDelAlbumRsp>();
        break;
	case UPP_CMD_GET_ALBUM:
		ret = DelBody<stGetAlbumReq,stGetAlbumRsp>();
		break;
		
    case UPP_CMD_LIST_PHOTO_BYIDX:
        ret = DelBody<stListPhotoByIndexReq,stListPhotoRsp>();
        break;
    case UPP_CMD_UPLOAD_PHOTO:
        ret = DelBody<stUploadPhotoReq,stUploadPhotoRsp>();
        break;
    case UPP_CMD_MODIFY_PHOTO:
        ret = DelBody<stModifyPhotoReq,stModifyPhotoRsp>();
        break;
    case UPP_CMD_DEL_PHOTO:
        ret = DelBody<stDelPhotoReq,stDelPhotoRsp>();
		break;
	case UPP_CMD_GET_PHOTO:
		ret = DelBody<stGetPhotoReq,stGetPhotoRsp>();
        break;
	case UPP_CMD_ZZ_PHOTO:
		ret = DelBody<stZzPhotoReq, stZzPhotoRsp>();
		break;
    case UPP_CMD_ZZ_PHOTO_BYURL:
		ret = DelBody<stZzPhotoByUrlReq, stZzPhotoByUrlRsp>();
		break;
    case UPP_CMD_LIST_COMMENT:
        ret = DelBody<stListCommentReq,stListCommentRsp>();
        break;
    case UPP_CMD_ADD_COMMENT:
        ret = DelBody<stAddCommentReq,stAddCommentRsp>();
        break;
    case UPP_CMD_DEL_COMMENT:
        ret = DelBody<stDelCommentReq,stDelCommentRsp>();
        break;
    case UPP_CMD_ADD_REPLY:
        ret = DelBody<stAddReplyReq,stAddReplyRsp>();
        break;
    case UPP_CMD_DEL_REPLY:
        ret = DelBody<stDelReplyReq,stDelReplyRsp>();
        break;
    case UPP_CMD_GET_COMMENT:
        ret = DelBody<stGetCommentReq,stGetCommentRsp>();
        break;
    case UPP_CMD_GET_USER:
        ret = DelBody<stGetUserReq,stGetUserRsp>();
        break;
    case UPP_CMD_MODIFY_USER:
        ret = DelBody<stModifyUserReq,stModifyUserRsp>();
        break;
        
    case UPP_CMD_SHIELD_PHOTO:
    case UPP_CMD_UNSHIELD_PHOTO:
        ret = DelBody<stShieldPhotoReq,stShieldPhotoRsp>();
        break;
    default:
        ret = -1;
        UPP_ERROR("cmd=%d illage", iCmd);
        break;
    }

    if (0 != ret)
        UPP_ERROR("DelBody: ret=%d,cmd=%d", ret, iCmd);
}

template<typename Req, typename Rsp>
void CUPPMsg::UppFillCLogReqInfo()
{
	if ((NULL == pReqBody) || (NULL == pRspBody))
	{
		UPP_ERROR("pReqBody=%p,pRspBody=%p, frame error", pReqBody, pRspBody);

		return;
	}

    Req *pReq = (Req *)pReqBody;
        Rsp *pRsp = (Rsp *)pRspBody;
        
	stHippoReqInfo.opuin = iPostUin;
	if ( "newphoto"==pReq->reqComm.sAppId )
	{
		stHippoReqInfo.touin = atoll(pReq->reqComm.sUserId.c_str());
	}
	else {
		//群组类,没有tuin，而hippo流水查询页面上面只支持tuin查询，只能这么来防止丢掉流水
		stHippoReqInfo.touin = iPostUin;
	}

	stHippoReqInfo.userip = 0;
	stHippoReqInfo.plat = iPlat;
	stHippoReqInfo.source = iSource;
	stHippoReqInfo.appid = PHOT_ID;
	stHippoReqInfo.auth_type = NS_QZDATA::ENUM_AUTH_TYPE_SVR;
	stHippoReqInfo.auth_key = "";
	stHippoReqInfo.ptlogin_id = PHOT_ID + APPID_BEGIN;
	stHippoReqInfo.ptlogin_state = 0;//0则标识鉴权通过 
	//stHippoReqInfo.opuin_is_alpha = is_alpha;	

}


int CUPPMsg::Req2Msg(char *pData, const uint32_t iLen)
{
	if (iLen < int(sizeof(QZAHEAD)))
	{
	    //数据包还未接收完整
	    UPP_ERROR("packate is not complete, len=%u", iLen);
		return -1;
	}

	QZAHEAD* qzahead = (QZAHEAD*)pData;
	const unsigned int iPackLen =qzahead->GetPackLen();
	const unsigned int iHeadLen = qzahead->GetHeadLen();
	if (iLen < iPackLen || iPackLen < iHeadLen)
	{
	    //数据包还未接收完整
	    UPP_ERROR("packate is not complete, len=%u, packlen:%u, headlen:%u", iLen, iPackLen, iHeadLen);
		return -1;
	}

	char *pBody = qzahead->GetBody();
	int iBodyLen = iPackLen - iHeadLen;

    //主命令字是在qza svr使用，这里不需要
    short iMainCmd;
   	qzahead->GetCmd(iMainCmd, iCmd);
   	iPostUin = qzahead->GetClientUin();
   	iPlat = qzahead->_detail_info._type_platform;
   	iSource = ntohl(qzahead->_detail_info._type_source);
   	
    int ret = 0;
    switch (iCmd)
    {
    case UPP_CMD_ADD_ALBUM:
        ret = SetBody<stAddAlbumReq,stAddAlbumRsp>(pBody, iBodyLen);
        break;
    case UPP_CMD_LIST_ALBUM_BYIDX:
        ret = SetBody<stListAlbumByIndexReq,stListAlbumRsp>(pBody, iBodyLen);
        break;
    case UPP_CMD_MODIFY_ALBUM:
        ret = SetBody<stModifyAlbumReq,stModifyAlbumRsp>(pBody, iBodyLen);
        break;
    case UPP_CMD_DEL_ALBUM:
        ret = SetBody<stDelAlbumReq,stDelAlbumRsp>(pBody, iBodyLen);
        break;
	case UPP_CMD_GET_ALBUM:
		ret = SetBody<stGetAlbumReq,stGetAlbumRsp>(pBody, iBodyLen);
		break;
		
    case UPP_CMD_LIST_PHOTO_BYIDX:
        ret = SetBody<stListPhotoByIndexReq,stListPhotoRsp>(pBody, iBodyLen);
        break;
    case UPP_CMD_UPLOAD_PHOTO:
        ret = SetBody<stUploadPhotoReq,stUploadPhotoRsp>(pBody, iBodyLen);
        break;
    case UPP_CMD_MODIFY_PHOTO:
        ret = SetBody<stModifyPhotoReq,stModifyPhotoRsp>(pBody, iBodyLen);
        break;
    case UPP_CMD_DEL_PHOTO:
        ret = SetBody<stDelPhotoReq,stDelPhotoRsp>(pBody, iBodyLen);
        break;
	case UPP_CMD_GET_PHOTO:
		ret = SetBody<stGetPhotoReq,stGetPhotoRsp>(pBody, iBodyLen);
		break;
	case UPP_CMD_ZZ_PHOTO:
		ret = SetBody<stZzPhotoReq,stZzPhotoRsp>(pBody, iBodyLen);
		break;
	case UPP_CMD_ZZ_PHOTO_BYURL:
		ret = SetBody<stZzPhotoByUrlReq, stZzPhotoByUrlRsp>(pBody, iBodyLen);
		break;	
    case UPP_CMD_LIST_COMMENT:
        ret = SetBody<stListCommentReq,stListCommentRsp>(pBody, iBodyLen);
        break;
    case UPP_CMD_ADD_COMMENT:
        ret = SetBody<stAddCommentReq,stAddCommentRsp>(pBody, iBodyLen);
        break;
    case UPP_CMD_DEL_COMMENT:
        ret = SetBody<stDelCommentReq,stDelCommentRsp>(pBody, iBodyLen);
        break;
    case UPP_CMD_ADD_REPLY:
        ret = SetBody<stAddReplyReq,stAddReplyRsp>(pBody, iBodyLen);
        break;
    case UPP_CMD_DEL_REPLY:
        ret = SetBody<stDelReplyReq,stDelReplyRsp>(pBody, iBodyLen);
        break;
    case UPP_CMD_GET_COMMENT:
        ret = SetBody<stGetCommentReq,stGetCommentRsp>(pBody, iBodyLen);
        break;
    case UPP_CMD_GET_USER:
        ret = SetBody<stGetUserReq,stGetUserRsp>(pBody, iBodyLen);
        break;
    case UPP_CMD_MODIFY_USER:
        ret = SetBody<stModifyUserReq,stModifyUserRsp>(pBody, iBodyLen);
        break;

    case UPP_CMD_SHIELD_PHOTO:
    case UPP_CMD_UNSHIELD_PHOTO:
        ret = SetBody<stShieldPhotoReq,stShieldPhotoRsp>(pBody, iBodyLen);
        break;

    default:
        ret = -1;
        UPP_ERROR("cmd=%d illage", iCmd);
        break;
    }

    if (0 != ret)
    {
        UPP_ERROR("SetBody: ret=%d,cmd=%d", ret, iCmd);
        return ret;
    }

    m_pQzaHead=(QZAHEAD*)malloc(qzahead->GetHeadLen());
	if (!m_pQzaHead)
	{
        UPP_ERROR("malloc failed, len:%u", qzahead->GetHeadLen());
		return -1;
	}
	memcpy(m_pQzaHead, qzahead, qzahead->GetHeadLen());

    return 0;
}

int CUPPMsg::Msg2Rsp(qzone::CPduParser &oPduParser, char *&buf, uint32_t &iLen)
{
    int ret = 0; 
    switch (iCmd)
    {
    case UPP_CMD_ADD_ALBUM:
        ret = GetBody<stAddAlbumRsp>(oPduParser, buf, iLen, 1024);
        break;
    case UPP_CMD_LIST_ALBUM_BYIDX:
        ret = GetBody<stListAlbumRsp>(oPduParser, buf, iLen, 1024 * 100);
        break;
    case UPP_CMD_MODIFY_ALBUM:
        ret = GetBody<stModifyAlbumRsp>(oPduParser, buf, iLen, 1024);
        break;
    case UPP_CMD_DEL_ALBUM:
        ret = GetBody<stDelAlbumRsp>(oPduParser, buf, iLen, 1024);
        break;
	case UPP_CMD_GET_ALBUM:
		ret = GetBody<stGetAlbumRsp>(oPduParser, buf, iLen, 1024*100);
		break;
		
    case UPP_CMD_LIST_PHOTO_BYIDX:
        ret = GetBody<stListPhotoRsp>(oPduParser, buf, iLen, 1024 * 100);
        break;
    case UPP_CMD_UPLOAD_PHOTO:
        ret = GetBody<stUploadPhotoRsp>(oPduParser, buf, iLen, 1024);
        break;
    case UPP_CMD_MODIFY_PHOTO:
        ret = GetBody<stModifyPhotoRsp>(oPduParser, buf, iLen, 1024);
        break;
    case UPP_CMD_DEL_PHOTO:
        ret = GetBody<stDelPhotoRsp>(oPduParser, buf, iLen, 1024);
        break;
    case UPP_CMD_GET_PHOTO:
        ret = GetBody<stGetPhotoRsp>(oPduParser, buf, iLen, 1024);
        break;
    case UPP_CMD_ZZ_PHOTO:
        ret = GetBody<stZzPhotoRsp>(oPduParser, buf, iLen, 1024);
        break;
    case UPP_CMD_ZZ_PHOTO_BYURL:
        ret = GetBody<stZzPhotoByUrlRsp>(oPduParser, buf, iLen, 1024);
        break;
		
    case UPP_CMD_LIST_COMMENT:
        ret = GetBody<stListCommentRsp>(oPduParser, buf, iLen, 1024 * 100);
        break;
    case UPP_CMD_ADD_COMMENT:
        ret = GetBody<stAddCommentRsp>(oPduParser, buf, iLen, 1024);
        break;
    case UPP_CMD_DEL_COMMENT:
        ret = GetBody<stDelCommentRsp>(oPduParser, buf, iLen, 1024);
        break;
    case UPP_CMD_ADD_REPLY:
        ret = GetBody<stAddReplyRsp>(oPduParser, buf, iLen, 1024);
        break;
    case UPP_CMD_DEL_REPLY:
        ret = GetBody<stDelReplyRsp>(oPduParser, buf, iLen, 1024);
        break;
    case UPP_CMD_GET_COMMENT:
        ret = GetBody<stGetCommentRsp>(oPduParser, buf, iLen, 1024 * 100);
        break;
    case UPP_CMD_GET_USER:
        ret = GetBody<stGetUserRsp>(oPduParser, buf, iLen, 1024 * 100);
        break;
    case UPP_CMD_MODIFY_USER:
        ret = GetBody<stModifyUserRsp>(oPduParser, buf, iLen, 1024);
        break;
    case UPP_CMD_SHIELD_PHOTO:
    case UPP_CMD_UNSHIELD_PHOTO:
        ret = GetBody<stShieldPhotoRsp>(oPduParser, buf, iLen, 1024);
        break;
    default:
        //不回包，直接断开连接
        ret = -1;
        UPP_ERROR("cmd=%d illage", iCmd);
        break;
    }
    if (0 != ret)
    {
        UPP_ERROR("GetBody: ret=%d,cmd=%d", ret, iCmd);
    }
    return ret;
}

#if 0//获取重要bitmap改成异步之前的版本
int CUPPMsg::GetFirstStat()
{
	switch (iCmd)
	{
		case UPP_CMD_ADD_ALBUM:
			return UPPSTATE_ADD_ALBUM;
			
		case UPP_CMD_LIST_ALBUM_BYIDX:
			return UPPSTATE_LIST_ALBUM;

/*修改相册，要先查询*/
		case UPP_CMD_GET_ALBUM:
		case UPP_CMD_MODIFY_ALBUM:
			return UPPSTATE_GET_ALBUM;
			
		case UPP_CMD_DEL_ALBUM:
			return UPPSTATE_DEL_ALBUM;
		
		case UPP_CMD_LIST_PHOTO_BYIDX:
			return UPPSTATE_LIST_PHOTO;
			
		case UPP_CMD_UPLOAD_PHOTO:
		case UPP_CMD_ZZ_PHOTO_BYURL:
			return UPPSTATE_GET_ROOT_DIR;

/*修改照片，要先查询*/
		case UPP_CMD_MODIFY_PHOTO:
		case UPP_CMD_GET_PHOTO:
		case UPP_CMD_SHIELD_PHOTO:
		case UPP_CMD_UNSHIELD_PHOTO:
			return UPPSTATE_GET_PHOTO;
			
		case UPP_CMD_DEL_PHOTO:
			return UPPSTATE_DEL_PHOTO;
				
		case UPP_CMD_ZZ_PHOTO:
			return UPPSTATE_ZZ_PHOTO;

/*ugc评论回复*/
		case UPP_CMD_LIST_COMMENT:
			return UPPSTATE_LIST_COMMENT;
		case UPP_CMD_ADD_COMMENT:
			return UPPSTATE_ADD_COMMENT;
		case UPP_CMD_DEL_COMMENT:
			return UPPSTATE_DEL_COMMENT;
		case UPP_CMD_ADD_REPLY:
			return UPPSTATE_ADD_REPLY;
		case UPP_CMD_DEL_REPLY:
			return UPPSTATE_DEL_REPLY;
		case UPP_CMD_GET_COMMENT:
			return UPPSTATE_GET_COMMENT;

		case UPP_CMD_GET_USER:
			return UPPSTATE_GET_USER;
		case UPP_CMD_MODIFY_USER:

		default:
		UPP_ERROR("cmd=%d illage", iCmd);
	}

	return UPPSTATE_FINISH;
}
#endif


int CUPPMsg::GetFirstStat()
{
	switch (iCmd)
	{
	//读操作均要获取imbitmap，看看是否alpha用户需要上报全量流水
		case UPP_CMD_LIST_ALBUM_BYIDX:
		case UPP_CMD_GET_ALBUM:
		case UPP_CMD_LIST_PHOTO_BYIDX:
		case UPP_CMD_GET_PHOTO:
		case UPP_CMD_LIST_COMMENT:
		case UPP_CMD_GET_COMMENT:
		case UPP_CMD_GET_USER:
			return UPPSTATE_GET_IMBITMAP;
			
		case UPP_CMD_ADD_ALBUM:
			return UPPSTATE_ADD_ALBUM;

		case UPP_CMD_MODIFY_ALBUM:
			return UPPSTATE_GET_ALBUM;
			
		case UPP_CMD_DEL_ALBUM:
			return UPPSTATE_DEL_ALBUM;
			
		case UPP_CMD_UPLOAD_PHOTO:
		case UPP_CMD_ZZ_PHOTO_BYURL:
			return UPPSTATE_GET_ROOT_DIR;

/*修改照片，要先查询*/
		case UPP_CMD_MODIFY_PHOTO:
		case UPP_CMD_SHIELD_PHOTO:
		case UPP_CMD_UNSHIELD_PHOTO:
			return UPPSTATE_GET_PHOTO;
			
		case UPP_CMD_DEL_PHOTO:
			return UPPSTATE_DEL_PHOTO;
				
		case UPP_CMD_ZZ_PHOTO:
			return UPPSTATE_ZZ_PHOTO;

/*ugc评论回复*/
		case UPP_CMD_ADD_COMMENT:
			return UPPSTATE_ADD_COMMENT;
		case UPP_CMD_DEL_COMMENT:
			return UPPSTATE_DEL_COMMENT;
		case UPP_CMD_ADD_REPLY:
			return UPPSTATE_ADD_REPLY;
		case UPP_CMD_DEL_REPLY:
			return UPPSTATE_DEL_REPLY;

		case UPP_CMD_MODIFY_USER:

		default:
		UPP_ERROR("cmd=%d illage", iCmd);
	}

	return UPPSTATE_FINISH;
}


