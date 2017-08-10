/*

作者:韩智鸿

han.zhihong@qq.com

*/

#ifndef __ZH_C_NET_UPNP_H__
#define __ZH_C_NET_UPNP_H__

#include "socket.h"


#ifdef __cplusplus
extern "C"{
#endif

//端口的结构信息
typedef struct _Tzh_UPNP_PORTMAPINFO
{
    char ExternalIP[16];
    unsigned short ExternalPort;
    int Protocol;//0:TCP 1:UDP
    char InternalIP[16];
    unsigned short InternalPort;
    int Enabled;//
    char Description[64];
    long Duration;
}Tzh_UPNP_PORTMAPINFO;


//查找是否存在UPNP设备
bool zhUPnP_FindDevice();

//获取设备描述
bool zhUPnP_GetDeviceDescription();

//获取外网已经存在的地址映射
int zhUPnP_GetExternalIPAddress();

//判断网关设备是否正常
bool zhUPnP_IsGatewayOK();
//获取IP信息
char* zhUPnP_GetExternalIP();
char* zhUPnP_GetDeviceIP();

/*
添加一个映射
	RemoteHose 默认为空
	ExternalPort 外网端口
	PortMappingProtocal 映射的协议 ,填"TCP" 或 "UDP"一定要大写 
	InternalPort 内网端口
	InternalClient 内网映射的主机地址
	PortMappingDescription 映射信息描述
	PortMappingEnable 端口映射启用状态 默认为 1
	PortMappingLeaseDuration 默认为 0
*/
bool zhUPnP_AddPortMapping(const char *RemoteHose, 
                        unsigned short ExternalPort,
                        const char *PortMappingProtocal,
                        unsigned short InternalPort,
                        const char *InternalClient,
                        const char *PortMappingDescription,
                        int PortMappingEnable,
                        unsigned long PortMappingLeaseDuration);

/*
删除一个映射
	RemoteHose 默认为空
	ExternalPort 外网端口
	PortMappingProtocal 映射的协议 ,填"TCP" 或 "UDP"一定要大写
*/
bool zhUPnP_DeletePortMapping(const char *RemoteHose, 
						   unsigned short ExternalPort,
                           const char *PortMappingProtocal);

//获取映射端口的信息
bool zhUPnP_GetPortMappingInfo( int PortMappingIndex, Tzh_UPNP_PORTMAPINFO *info);


#ifdef __cplusplus
}
#endif
#endif
