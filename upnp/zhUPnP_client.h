/*

����:���Ǻ�

han.zhihong@qq.com

*/

#ifndef __ZH_C_NET_UPNP_H__
#define __ZH_C_NET_UPNP_H__

#include "socket.h"


#ifdef __cplusplus
extern "C"{
#endif

//�˿ڵĽṹ��Ϣ
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


//�����Ƿ����UPNP�豸
bool zhUPnP_FindDevice();

//��ȡ�豸����
bool zhUPnP_GetDeviceDescription();

//��ȡ�����Ѿ����ڵĵ�ַӳ��
int zhUPnP_GetExternalIPAddress();

//�ж������豸�Ƿ�����
bool zhUPnP_IsGatewayOK();
//��ȡIP��Ϣ
char* zhUPnP_GetExternalIP();
char* zhUPnP_GetDeviceIP();

/*
���һ��ӳ��
	RemoteHose Ĭ��Ϊ��
	ExternalPort �����˿�
	PortMappingProtocal ӳ���Э�� ,��"TCP" �� "UDP"һ��Ҫ��д 
	InternalPort �����˿�
	InternalClient ����ӳ���������ַ
	PortMappingDescription ӳ����Ϣ����
	PortMappingEnable �˿�ӳ������״̬ Ĭ��Ϊ 1
	PortMappingLeaseDuration Ĭ��Ϊ 0
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
ɾ��һ��ӳ��
	RemoteHose Ĭ��Ϊ��
	ExternalPort �����˿�
	PortMappingProtocal ӳ���Э�� ,��"TCP" �� "UDP"һ��Ҫ��д
*/
bool zhUPnP_DeletePortMapping(const char *RemoteHose, 
						   unsigned short ExternalPort,
                           const char *PortMappingProtocal);

//��ȡӳ��˿ڵ���Ϣ
bool zhUPnP_GetPortMappingInfo( int PortMappingIndex, Tzh_UPNP_PORTMAPINFO *info);


#ifdef __cplusplus
}
#endif
#endif
