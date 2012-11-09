//必须包含spp的头文件
#include "sppincl.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "timeline_proc.h"
#include "timeline_macro.h"

#define MODULE_PROC_NUM	"module_proc_num"

CServerBase *g_ServerBase = NULL;

//初始化方法（可选实现）
//arg1:	配置文件
//arg2:	服务器容器对象
//返回0成功，非0失败
extern "C" int spp_handle_init(void* arg1, void* arg2)
{
    //插件自身的配置文件
    const char *etc = (const char*)arg1;
    //服务器容器对象
    CServerBase *base = (CServerBase*)arg2; 
    base->log_.LOG_P(LOG_DEBUG, "spp_handle_init: etc=%s,svrtype=%d\n", etc, base->servertype());
    //建立一个统计项, 统计策略为累加
    //base->stat_.init_statobj(MODULE_PROC_NUM, STAT_TYPE_SUM);

    //便于调用LOG_P_ALL()等打log
    if  (!g_ServerBase)
        g_ServerBase = base;
    
	if (base->servertype() == SERVER_TYPE_WORKER)
	{
		//初始化
		return CUPPProcSingle::instance()->HandleInit(etc, base);
	}
	
    return 0;
}

//数据接收（必须实现）
//flow:	请求包标志
//arg1:	数据块对象
//arg2:	服务器容器对象
//返回正数表示数据已经接收完整且该值表示数据包的长度，0值表示数据包还未接收完整，负数表示出错
extern "C" int spp_handle_input(unsigned flow, void* arg1, void* arg2)
{
    //数据块对象，结构请参考tcommu.h
    blob_type* blob = (blob_type*)arg1;
    //extinfo有扩展信息
    TConnExtInfo* extinfo = (TConnExtInfo*)blob->extdata;
    //服务器容器对象
    CServerBase* base = (CServerBase*)arg2;

    UPP_DEBUG("spp_handle_input: \n");
	//检查请求包合法性，返回包长度
    return CUPPProcSingle::instance()->HandleInput(blob);
}

//路由选择（可选实现）
//flow:	请求包标志
//arg1:	数据块对象
//arg2:	服务器容器对象
//返回值表示worker的组号
extern "C" int spp_handle_route(unsigned flow, void* arg1, void* arg2)
{
    //服务器容器对象
    CServerBase* base = (CServerBase*)arg2;
    base->log_.LOG_P(LOG_DEBUG, "spp_handle_route, %d\n", flow);

    //FIXME: 暂时只支持一个worker组
    return 1;
}

//数据处理（必须实现）
//flow:	请求包标志
//arg1:	数据块对象
//arg2:	服务器容器对象
//返回0表示成功，非0失败
extern "C" int spp_handle_process(unsigned flow, void* arg1, void* arg2)
{
	//解请求包，填充CMsgBase，然后交给异步框架处理
    return CUPPProcSingle::instance()->HandleProcess(flow, arg1, arg2);
}

//析构资源（可选实现）
//arg1:	保留参数
//arg2:	服务器容器对象
extern "C" void spp_handle_fini(void* arg1, void* arg2)
{
    //服务器容器对象
    CServerBase* base = (CServerBase*)arg2;
    base->log_.LOG_P(LOG_DEBUG, "spp_handle_fini\n");
	
	if (base->servertype() == SERVER_TYPE_WORKER)
    {
        //析构框架资源
        CUPPProcSingle::instance()->HandleFini();
    }
}


