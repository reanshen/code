#ifndef __TIMELINE_MSG_H__
#define __TIMELINE_MSG_H__

#include "spp_incl/MsgBase.h"
#include "sppincl.h"
#include "pdu_header.h"
#include "pdu_parser.h"
#include "qza_protocol.h"
#include "jce/Jce.h"
#include "photo_err_code.h"
#include "photo_define.h"
#include "timeline_photo_asyn_api.h"
#include "qz_dclog_api.h"
#include "timeline_macro.h"
#include "qza_timeline_jce.h"

USING_ASYNCFRAME_NS;

const int TIMELINESTATE_FINISH 		= 0;  			//����״̬

const int TIMELINESTATE_ADD_ALBUM 	= 1;  			//�������
const int TIMELINESTATE_LIST_ALBUM 	= 2;    		//��ȡ����б�
const int TIMELINESTATE_MODIFY_ALBUM = 3;            //�޸����
const int TIMELINESTATE_DEL_ALBUM	= 4;			//ɾ�����
const int TIMELINESTATE_GET_ALBUM	= 5;			//��ȡ���
const int TIMELINESTATE_GET_USER 	= 6;			//��ȡ�û�

const int TIMELINESTATE_UPLOAD_PHOTO	= 11;  			//�ϴ���Ƭ
const int TIMELINESTATE_LIST_PHOTO   = 12;			//��ȡ��Ƭ�б�	
const int TIMELINESTATE_MODIFY_PHOTO = 13;           //�޸���Ƭ
const int TIMELINESTATE_DEL_PHOTO    = 14;           //ɾ����Ƭ
const int TIMELINESTATE_GET_PHOTO    = 15;			//��ȡ��Ƭ
const int TIMELINESTATE_ZZ_PHOTO    = 16;			//ת����Ƭ
const int TIMELINESTATE_ZZ_PHOTO_BYURL    = 17;			//����url ת����Ƭ

const int TIMELINESTATE_LIST_ALBUM_2_PHOTOLIST = 101;//��ȡ����б����ȡÿ��������Ƭ�б�
const int TIMELINESTATE_GET_ALBUM_2_PHOTOLIST = 102; //��ѯ�����Ϣ����ȡ������Ƭ�б�
const int TIMELINESTATE_LIST_PHOTO_2_LATEST_COMMENT =  103;//ͼƬ�б����ȡÿ��ͼ�������������


const int TIMELINESTATE_LIST_COMMENT    = 201;	//��ȡ����
const int TIMELINESTATE_ADD_COMMENT    = 202;	//�������
const int TIMELINESTATE_DEL_COMMENT    = 203;	//ɾ������
const int TIMELINESTATE_ADD_REPLY    = 204;		//��ӻظ�
const int TIMELINESTATE_DEL_REPLY    = 205;		//ɾ���ظ�
const int TIMELINESTATE_GET_COMMENT    = 206;	//��ȡ����

const int TIMELINESTATE_ADD_FEED = 301;          //��feed ��TIMELINE feed svr

const int TIMELINESTATE_SHIELD_PHOTO = 401;      //����ͼƬ

const int TIMELINESTATE_GET_ROOT_DIR = 501;
const int TIMELINESTATE_CREATE_ROOT_DIR = 502;

/*------------------VIP State-------------------*/
const int TIMELINESTATE_GET_IMBITMAP = 9527;


#ifdef _TIMELINE_SVR_DEBUG_
    #include <sstream>
    #define PrintPduStruct(pdu) \
    do {\
        ostringstream oss;\
        (pdu).display(oss);\
        TIMELINE_DEBUG("pdustruct=%s,len=%u\n%s", (pdu).className().c_str(), oss.str().size(), oss.str().c_str());\
    }while(0)
#else
    #define PrintPduStruct(pdu) do{}while(0)
#endif

class CTimeLineMsg : public CMsgBase
{
public:
	CTimeLineMsg();
	~CTimeLineMsg();

    int Req2Msg(char *pData, const uint32_t iLen);
    int Msg2Rsp(qzone::CPduParser &oPduParser, char *&buf, uint32_t &iLen);

    void SetBlobDataPtr(char *blob_data){ pBlobData=blob_data; }
    void SetBlobDataLen(int blob_len){ iBlobLen=blob_len; }
	
    int GetFirstStat();

	int GetUserIMBitmapFlag(unsigned int uiUin, unsigned long long &ullFlag);

	template<typename Req, typename Rsp>
	void TimeLineFillCLogReqInfo();

	NS_QZDATA::TimeRecord m_timer;
	
protected:

	template<typename Req, typename Rsp>
	int SetBody(char *pData, uint32_t iLen)
	{
	    qzone::CPduParser oPduParser;
        Req *pReqBody_ = new Req;
        Rsp *pRspBody_ = new Rsp;
        pReqBody = pReqBody_;
        pRspBody = pRspBody_;
        pRspBody_->rspComm.iRet = QZONE::eSvrErr;
        pRspBody_->rspComm.sMsg = "timeline err";

        int ret = oPduParser.Decode<Req>(pData, iLen, *(Req *)pReqBody_, false);
	        
		if ( ret!=0 ){
			return ret;
		}

		PrintPduStruct(*pReqBody_);

		TimeLineFillCLogReqInfo<Req, Rsp>();

		stSubId.cmd = iCmd;

		return 0;
	}

    template<typename Rsp>
    int GetBody(qzone::CPduParser &oPduParser, char *&pData, uint32_t &iLen, int tryLen = 1024)
    {
        int ret = oPduParser.Encode<Rsp>(*(Rsp *)pRspBody, pData, iLen, tryLen, false);
        return ret;
    }

    template<typename Req, typename Rsp>
    int DelBody()
    {
        if(pReqBody)
        {
            Req *pReqBody_ = (Req *)pReqBody;
            delete pReqBody_;
            pReqBody = NULL;
        }
        if(pRspBody)
        {
            Rsp *pRspBody_ = (Rsp *)pRspBody;
            delete pRspBody_;
            pRspBody = NULL;
        }

        if (m_pQzaHead)
		{
			free(m_pQzaHead);
			m_pQzaHead=NULL;
		}

        return 0;
    }

public:
    short iCmd;
    TConnExtInfo extinfo;
	void *pReqBody;
	void *pRspBody;
    unsigned int iPostUin;
    int iPlat;	  //ƽ̨ID�������������ҵ����0����
    int iSource;  //��ԴID�������������ҵ����0����

	void *pBlobData;
	int iBlobLen;

    /*����state֮�䴫������*/
	timeline::NODEINFO stAlbumNodeInfo;
	timeline::NODEINFO stPhotoNodeInfo;
	string sPicData;

	unsigned int flow;

	NS_QZDATA::CustomSubLogId stSubId;
	NS_QZDATA::HippoRequestInfo stHippoReqInfo;

    QZAHEAD *m_pQzaHead;	//���������ͷ��,�Ա�ذ�ʱʹ��
};

#endif

