#ifndef __TIMELINE_CLOG_H____
#define __TIMELINE_CLOG_H____
#include "monitor_config.h"

#define TIMELINE_MONITOR_FILENAME "/usr/local/services/spp_upp-2.3/client/upp/conf/upp_monitor.conf"

class CUppMonitorConfig:public CConfig
{
public:
	CUppMonitorConfig():CConfig(TIMELINE_MONITOR_FILENAME) {};
};

#define TIMELINEMONLOG_V_ERROR(subid, reqinfo, tmrecord, result, opmod, opname, opids, svrip, args...) \
{\
	tmrecord.stop();\
	switch ( subid.cmd )\
	{\
		case TIMELINE_CMD_ADD_ALBUM:\
		case TIMELINE_CMD_MODIFY_ALBUM:\
		case TIMELINE_CMD_DEL_ALBUM:\
		case TIMELINE_CMD_UPLOAD_PHOTO:\
		case TIMELINE_CMD_ZZ_PHOTO_BYURL:\
		case TIMELINE_CMD_MODIFY_PHOTO:\
		case TIMELINE_CMD_SHIELD_PHOTO:\
		case TIMELINE_CMD_UNSHIELD_PHOTO:\
		case TIMELINE_CMD_DEL_PHOTO:\
		case TIMELINE_CMD_ZZ_PHOTO:\
		case TIMELINE_CMD_ADD_COMMENT:\
		case TIMELINE_CMD_DEL_COMMENT:\
		case TIMELINE_CMD_ADD_REPLY:\
		case TIMELINE_CMD_DEL_REPLY:\
		case TIMELINE_CMD_MODIFY_USER:\
		{\
			CUPPProcSingle::instance()->stCLogWrite.writeLogV(true, &subid, &reqinfo, &tmrecord, NS_QZDATA::QzDCLogDefs::DC_ERROR, __FILE__, __LINE__, \
				result, 1, opmod, opname, opids, svrip, args);\
			break;\
		}\
		\
		default:\
		{\
			CUPPProcSingle::instance()->stCLogRead.writeLogV(true, &subid, &reqinfo, &tmrecord, NS_QZDATA::QzDCLogDefs::DC_ERROR, __FILE__, __LINE__, \
				result, 1, opmod, opname, opids, svrip, args);\
			break;\
		}\
	}\
}

#define TIMELINEMONLOG_V_DEBUG(subid, reqinfo, tmrecord, result, opmod, opname, opids, svrip, args...) \
{\
	tmrecord.stop();\
	switch ( subid.cmd )\
	{\
		case TIMELINE_CMD_ADD_ALBUM: \
		case TIMELINE_CMD_MODIFY_ALBUM: \
		case TIMELINE_CMD_DEL_ALBUM: \
		case TIMELINE_CMD_UPLOAD_PHOTO: \
		case TIMELINE_CMD_ZZ_PHOTO_BYURL: \
		case TIMELINE_CMD_MODIFY_PHOTO: \
		case TIMELINE_CMD_SHIELD_PHOTO: \
		case TIMELINE_CMD_UNSHIELD_PHOTO: \
		case TIMELINE_CMD_DEL_PHOTO: \
		case TIMELINE_CMD_ZZ_PHOTO: \
		case TIMELINE_CMD_ADD_COMMENT: \
		case TIMELINE_CMD_DEL_COMMENT: \
		case TIMELINE_CMD_ADD_REPLY: \
		case TIMELINE_CMD_DEL_REPLY: \
		case TIMELINE_CMD_MODIFY_USER: \
		{\
			CUPPProcSingle::instance()->stCLogWrite.writeLogV(true, &subid, &reqinfo, &tmrecord, NS_QZDATA::QzDCLogDefs::DC_DEBUG, __FILE__, __LINE__, \
				result, 0, opmod, opname, opids, svrip, args);\
			break;\
		}\
\
		default: \
		{\
			CUPPProcSingle::instance()->stCLogRead.writeLogV(true, &subid, &reqinfo, &tmrecord, NS_QZDATA::QzDCLogDefs::DC_DEBUG, __FILE__, __LINE__, \
				result, 0, opmod, opname, opids, svrip, args);\
			break;\
		}\
	}\
}

#define TIMELINEMONLOG_V_INFO(subid, reqinfo, tmrecord, result, opmod, opname, opids, svrip, args...) \
{\
	tmrecord.stop();\
	switch ( subid.cmd )\
	{\
		case TIMELINE_CMD_ADD_ALBUM:\
		case TIMELINE_CMD_MODIFY_ALBUM:\
		case TIMELINE_CMD_DEL_ALBUM:\
		case TIMELINE_CMD_UPLOAD_PHOTO:\
		case TIMELINE_CMD_ZZ_PHOTO_BYURL:\
		case TIMELINE_CMD_MODIFY_PHOTO:\
		case TIMELINE_CMD_SHIELD_PHOTO:\
		case TIMELINE_CMD_UNSHIELD_PHOTO:\
		case TIMELINE_CMD_DEL_PHOTO:\
		case TIMELINE_CMD_ZZ_PHOTO:\
		case TIMELINE_CMD_ADD_COMMENT:\
		case TIMELINE_CMD_DEL_COMMENT:\
		case TIMELINE_CMD_ADD_REPLY:\
		case TIMELINE_CMD_DEL_REPLY:\
		case TIMELINE_CMD_MODIFY_USER:\
		{\
			CUPPProcSingle::instance()->stCLogWrite.writeLogV(true, &subid, &reqinfo, &tmrecord, NS_QZDATA::QzDCLogDefs::DC_INFO, __FILE__, __LINE__, \
				result, 0, opmod, opname, opids, svrip, args);\
			break;\
		}\
\
		default:\
		{\
			CUPPProcSingle::instance()->stCLogRead.writeLogV(true, &subid, &reqinfo, &tmrecord, NS_QZDATA::QzDCLogDefs::DC_INFO, __FILE__, __LINE__, \
				result, 0, opmod, opname, opids, svrip, args);\
			break;\
		}\
	}\
}

#endif//__TIMELINE_CLOG_H____
