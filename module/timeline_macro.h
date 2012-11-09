#ifndef __TIMELINE_MACRO__
#define __TIMELINE_MACRO__

#include "server_config.h"
#include "singleton.h"
#include "serverbase.h"

#define MIN_UIN 10000

#define TIMELINE_SIZE_ALBUM_NAME 600
#define TIMELINE_SIZE_ALBUM_DESC 600
#define TIMELINE_SIZE_PIC_NAME 600
#define TIMELINE_SIZE_PIC_DESC 1200
#define TIMELINE_SIZE_PIC_BODY 11000000	//图片最大尺寸，约10M

//相册，图片，评论一次请求最大拉取数
#define TIMELINE_MAX_ALBUM_REQ_NUM    100
#define TIMELINE_MAX_PHOTO_REQ_NUM     100
#define TIMELINE_MAX_COMMENT_REQ_NUM   100

//配置项
#define CONFIG CSingleton<CServerConf>::instance()
#define MAX_PENDING_CONN_NUM atoi((*CONFIG)["server.max_penging_conn"].c_str())

#define TIMELINE_SVR_TIMEOUT_MS atoi((*CONFIG)["timeline_api_server.timeout"].c_str())
#define TIMELINE_SVR_UPLOAD_TIMEOUT_MS atoi((*CONFIG)["timeline_api_server.upload_timeout_ms"].c_str())

#define TIMELINE_SVR_MODID atoi((*CONFIG)["timeline_api_server.modid"].c_str())
#define TIMELINE_SVR_CMDID atoi((*CONFIG)["timeline_api_server.cmdid"].c_str())

#define TIMELINE_UGC_APPID_CF (*CONFIG)["ugc.appid"]
#define TIMELINE_UGC_APPKEY_CF (*CONFIG)["ugc.appkey"]
#define TIMELINE_UGC_TIMEOUT_MS atoi((*CONFIG)["ugc.timeout_ms"].c_str())
#define TIMELINE_UGC_MODID atoi((*CONFIG)["ugc.modid"].c_str())
#define TIMELINE_UGC_CMDID atoi((*CONFIG)["ugc.cmdid"].c_str())

#define TIMELINE_FEEDS_TIMEOUT_MS (atoi((*CONFIG)["timeline_feeds_server.timeout_ms"].c_str()))
#define TIMELINE_FEEDS_MODID (atoi((*CONFIG)["timeline_feeds_server.modid"].c_str()))
#define TIMELINE_FEEDS_CMDID (atoi((*CONFIG)["timeline_feeds_server.cmdid"].c_str()))

#define TIMELINE_IMBITMAP_TIMEOUT_MS (atoi((*CONFIG)["qzdata.imbitmap_timeout_ms"].c_str()))
#define TIMELINE_IMBITMAP_MODID (atoi((*CONFIG)["qzdata.imbitmap_modid"].c_str()))
#define TIMELINE_IMBITMAP_CMDID (atoi((*CONFIG)["qzdata.imbitmap_cmdid"].c_str()))

#define TIMELINE_DOWNLOAD_PROXY_MODID (atoi((*CONFIG)["download_proxy.modid"].c_str()))
#define TIMELINE_DOWNLOAD_PROXY_CMDID (atoi((*CONFIG)["download_proxy.cmdid"].c_str()))
#define TIMELINE_DOWNLOAD_PROXY_TIMEOUT_MS (atoi((*CONFIG)["download_proxy.timeout_ms"].c_str()))

//timeline 参数限制
#define TIMELINE_PARAM_LIST_ALBUM_MAX 50
#define TIMELINE_PARAM_LIST_PHOTO_MAX 50

#define TIMELINE_RELEASE_PTR(p) \
	if ( p!=NULL)\
	{\
		delete []p;\
		p = NULL;\
	}

//LOG
//<!--LOG_TRACE = 0 LOG_DEBUG=1 LOG_NORMAL=2 LOG_ERROR=3 LOG_FATAL=4 LOG_NONE=5 -->
#ifdef _SPP_COMM_SERVERBASE_H_
	extern spp::comm::CServerBase *g_ServerBase;

	#ifndef TIMELINE_DEBUG
	#define TIMELINE_DEBUG(fmt, args...) \
		if (g_ServerBase) g_ServerBase->log_.LOG_P_ALL(1, fmt "\n", ##args)
	#endif

	#ifndef TIMELINE_ERROR
	#define TIMELINE_ERROR(fmt, args...) \
		if (g_ServerBase) g_ServerBase->log_.LOG_P_ALL(3, fmt "\n", ##args)
	#endif
#else
	#error "needs spp_incl first"
#endif	//_SPP_COMM_SERVERBASE_H_
#endif  //__TIMELINE_MACRO__

