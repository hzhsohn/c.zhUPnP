/*

作者:韩智鸿

han.zhihong@qq.com

*/

#include "zhUPnP_client.h"

#ifdef _DEBUG
#define DBG_INFO(x) //printf x
#else
#define DBG_INFO(x)
#endif

#define TAGSTART   "<%s>"
#define TAGEND     "</%s>"

#define UPNP_SEARCH_ADDR "239.255.255.250"
#define UPNP_SEARCH_PORT 1900

#define UPNP_MSG_BUF_SIZE 8192

#define UPPN_HTTP_VER  "HTTP/1.1"
#define XML_VERSION "1.0"

#define RootDeviceUDN "upnp:rootdevice"
#define UDNPREFIX "urn:schemas-upnp-org:device:"
#define GATEWAYDEVICE0 "InternetGatewayDevice"
#define GATEWAYDEVICE1 "WANDevice"
#define GATEWAYDEVICE2 "WANConnectionDevice"
#define UPNP_VER 1

#if 0
enum
{
    deviceType,
    presentationURL,
    friendlyName,
    manufacturer,
    manufacturerURL,
    modelDescription,
    modelName,
    modelNumber,
    UDN,
    UPC,
    serviceList,
    deviceList,
    ALL
};

enum
{
    POST,
    GET
};

char *Tag[ALL] = 
{
    "deviceType",
    "presentationURL",
    "friendlyName",
    "manufacturer",
    "manufacturerURL",
    "modelDescription",
    "modelName",
    "modelNumber",
    "UDN",
    "UPC",
    "serviceList",
    "deviceList"
};

typedef struct 
{
    int tag;
    char *tagName;
}XML_Device;

XML_Device deviceNodes[ALL] = 
{
    {deviceType, "deviceType"},
    {presentationURL, "presentationURL"},
    {friendlyName, "friendlyName"},
    {manufacturer, "manufacturer"},
    {manufacturerURL, "manufacturerURL"},
    {modelDescription, "modelDescription"},
    {modelName, "modelName"},
    {modelNumber, "modelNumber"},
    {UDN, "UDN"},
    {UPC, "UPC"},
    {serviceList, "serviceList"},
    {deviceList, "deviceList"}
};

typedef struct _UPnPService
{
    char* ServiceType;
    char* ServiceId;
    char* ControlURL;
    char* SubscriptionURL;
    char* SCPDURL;
    char* SubscriptionID;
    int MaxVersion;

    struct UPnPAction *Actions;
    struct UPnPStateVariable *Variables;
    struct UPnPDevice *Parent;
    struct UPnPService *Next;

}UPnPService;

typedef struct _UPnPStateVariable
{
    struct UPnPStateVariable *Next;
    struct UPnPService *Parent;

    char* Name;
    char **AllowedValues;
    int NumAllowedValues;
    char* Min;
    char* Max;
    char* Step;
}UPnPStateVariable;

typedef struct _UPnPAction
{
    char* Name;
    struct UPnPAction *Next;
}UPnPAction;

typedef struct _UPnPAllowedValue
{
    struct UPnPAllowedValue *Next;
    char* Value;
}UPnPAllowedValue;

typedef struct _UPnPDevice
{
    char *deviceType;
    char *presentationURL;
    char *friendlyName;
    char *manufacturer;
    char *manufacturerURL;
    char *modelDescription;
    char *modelName;
    char *modelNumber;
    char *UDN;
    char *UPC;

    struct _UPnPDevice *Parent;
    struct _UPnPDevice *EmbededDevices;
    struct _UPnPDevice *Next;
    UPnPService *Services;
}UPnPDevice;

typedef struct _XMLNode
{
    char *name;
    int nameLen;

    struct _XMLNode *parent;
    struct _XMLNode *next;
    struct _XMLNode *peer;
    
}XMLNode;

typedef struct
{
    char *name;
    int nameLen;
    char *value;
    int valueLen;

    XMLNode *parent;
    
}XML_PROPERTY;


typedef struct
{
    char searchTarget[64];
    char host[16];
    
}UPnPSearchPara;
#endif


char *SearchFormat = 
"M-SEARCH * HTTP/1.1\r\n\
MX: 5\r\n\
ST: %s\r\n\
HOST: 239.255.255.250:1900\r\n\
MAN: \"ssdp:discover\"\r\n\
Content-Length: 0\r\n\
\r\n";

char *GetFormat = 
"GET /%s HTTP/1.1\r\n\
Accept: text/xml, application/xml\r\n\
User-Agent: HC-NVS\r\n\
Host: %s:%d\r\n\
Connection: Keep-Alive\r\n\
Cache-Control: max-age=0\r\n\
\r\n";

    char *HttpFormat =
"POST %s HTTP/1.1\r\n\
Host: %s:%d\r\n\
SOAPACTION: \"%s#%s\"\r\n\
CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n\
Content-Length: %d\r\n\r\n";


    char *SoapFormat =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n\
<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n\
<s:Body>\r\n<u:%s xmlns:u=\"%s\">\r\n%s</u:%s>\r\n</s:Body>\r\n</s:Envelope>\r\n";

static char controlUrl[1024] ="";
static char controlService[1024] = "";
static char deviceAddr[1024] = "";
static char deviceIP[16] = "";
static char externalIP[16] = "";
static int devicePort;
static char postUrl[512] = "";
static char location[128] = "";

int UPnP_GetPropertyByName(char *xml, char *name, char *value)
{
    char *startTag = NULL;
    char *endTag = NULL;
    //int start;
    //int end;
    char *p1 = NULL;
    char *p2 = NULL;
    //int valueLen;
    
    startTag = (char*)malloc(strlen(name)+3);
    if(!startTag)
        return 0;
    endTag = (char*)malloc(strlen(name)+4);
    if(!endTag)
        return 0;

    sprintf(startTag, "<%s>", name);
    sprintf(endTag, "</%s>", name);

    if(!(p1 = strstri(xml, startTag)))
    {
        free(startTag);
        free(endTag);
        return 0;
    }
    if(!(p2 = strstri(p1, endTag)))
    {
        free(startTag);
        free(endTag);
        return 0;
    }

    //size enough
    memcpy(value, p1+strlen(startTag), p2-p1-strlen(startTag));
    value[p2-p1-strlen(startTag)] = '\0';
    //printf("%s\n",value);
    free(startTag);
    free(endTag);

    return strlen(value);
}

bool UPnP_InvokeCmd(char *cmd, char *response, int responsesize)
{
    SOCKET upnp_sock;
    int ret;
    int len;
    int contentLen;
    int headLen;
    char *p = NULL;
	char buf[1023];
	int buf_pos;

    if(!cmd||!response)
        return false;

    len = strlen(cmd);
    
    if(!zhSockInit(&upnp_sock,ZH_SOCK_TCP)){return false;}
    if(!zhSockSetNonBlocking(upnp_sock,false)){DBG_INFO("SetNonBlocking Fail..!!");return false;}
	if(!zhSockSetReuseAddr(upnp_sock,true)){DBG_INFO("Reuse Addr Fail..!!");return false;}
   // if(!zhSockBindAddr(upnp_sock,NULL,wBindPort)){return false;}
    //printf("after link host.\n");
	if(!zhSockConnect(upnp_sock,deviceIP, devicePort,0))
    {
        DBG_INFO(("upnp link device create socket failed.\n"));
        return ERROR;
    }

    /*Send  packet*/
    ret = send(upnp_sock, cmd, len,0);
    if(ret ==SOCKET_ERROR)
    {
        DBG_INFO(("UPnP send failed.\n"));
        goto __failed;
    }

	buf_pos=0;
    while(1)
	{
		memset(buf,0,sizeof(buf));
		len = recv(upnp_sock,buf , sizeof(buf),0);
		if(len < 0)
		{
			DBG_INFO(("receive error.\n"));
			goto __failed;
		}

		memcpy(&response[buf_pos],buf,len);
		buf_pos+=len;

		p = strstri(response, "\r\n\r\n");
		if(p)
		{
			break;
		}
		
	}
    headLen = p-response+strlen("\r\n\r\n");
    
    p = strstri(response, "Content-Length:");
    if(p)
    {
        sscanf(p, "%*[^0-9]%d", &contentLen);
        while(buf_pos<contentLen+headLen)
        {
            ret = recv(upnp_sock,buf,sizeof(buf),0);
            if(ret ==SOCKET_ERROR)
            {
                DBG_INFO(("UPnP Read XML content failed.\n"));
                goto __failed;
            }
			memcpy(&response[buf_pos],buf,ret);
			buf_pos+=ret;
        }
    }
    
    DBG_INFO(("<<<<<<<<<<<:\n%s\n", response));

    if(upnp_sock>=0)
    {
        zhSockClose(upnp_sock);
        upnp_sock = -1;
    }
    
    if((p = strstri(response, "200 OK")) == NULL)
    {
        return false;
    }

    return true;

    __failed: 
    if(upnp_sock>=0)
    {
        zhSockClose(upnp_sock);
        upnp_sock = -1;
    }
    return false;

}

bool zhUPnP_GetDeviceDescription()
{
    int ret;
    char *p  = NULL;
    char *p1 = NULL;
    char value[1024];
    char request[1024];
    char tempbuf[10240];
    char URLBase[1024];

    /*build GET packet*/
    sprintf(request, GetFormat, postUrl, deviceIP,devicePort);
    ret = strlen(request);
    request[ret] = '\0';
    DBG_INFO((">>>>>>>> \n%s\n", request));
    
    memset(tempbuf, 0, 10240);
    ret = UPnP_InvokeCmd(request, tempbuf, 10240);
    if(!ret)
    {
        DBG_INFO(("UPnP invoke Get description failed.\n"));
        return false;
    }
    /*Response state is 200 OK ?*/
    if((p = strstri(tempbuf, "200 OK")) == NULL)
        return false;

    /*Parse Xml, get service contral URL*/
    ret = UPnP_GetPropertyByName(tempbuf, "URLBase", URLBase);
    //printf("ret = %d\n", ret);
    if(ret <= 0)
    {

        sprintf(URLBase, "http://%s:%d", deviceIP, devicePort);
    }
    //if(URLBase[strlen(URLBase)-1] != '/')
    //{
    //    strcat(URLBase, "/");
    //}
    DBG_INFO(("URLBase is :%s.\n",URLBase));

    p = strstri(tempbuf, "<deviceType>");
    UPnP_GetPropertyByName(p, "deviceType", value);
    if(!strcmp(value, "urn:schemas-upnp-org:device:InternetGatewayDevice:1"))
    {
        p1 = strstri(p+strlen(value)+25, "<deviceType>");
        UPnP_GetPropertyByName(p1, "deviceType", value);
        //printf("%s.\n", value);
        if(!strcmp(value, "urn:schemas-upnp-org:device:WANDevice:1"))
        {
            p = strstri(p1+strlen(value)+25, "<deviceType>");
            UPnP_GetPropertyByName(p, "deviceType", value);
            if(!strcmp(value, "urn:schemas-upnp-org:device:WANConnectionDevice:1"))
            {
                p1 = strstri(p+strlen(value)+25, "<serviceType>");
                UPnP_GetPropertyByName(p1, "serviceType", value);
                if(!strcmp(value, "urn:schemas-upnp-org:service:WANIPConnection:1")
                    ||!strcmp(value, "urn:schemas-upnp-org:service:WANPPPConnection:1"))
                {
                    strcpy(controlService, value);
                    p = strstri(p1+strlen(value), "<controlURL>");
                    ret = UPnP_GetPropertyByName(p, "controlURL", value);
                    if(ret == 0)
                        return false;
                    strcpy(postUrl, value);
                    strcat(URLBase, value);
                    strcpy(controlUrl, URLBase);
                    DBG_INFO(("controlUrl is %s.\n", controlUrl));
                    //sscanf(URLBase, "%*[^0-9]%[^:]:%d/%s", ip, &port, postUrl);
                    //printf("%s.\n", postUrl);
                    return true;
                }
            }
        }
    }

    return false;
        
}

int zhUPnP_GetExternalIPAddress()
{
    char out[10240];
    char temp[10240];

    char *p = NULL;
    char *p1 = NULL;
    int len;
    int ret;

    sprintf(temp, SoapFormat,
               "GetExternalIPAddress",
               controlService,
               "",
               "GetExternalIPAddress");
    len = strlen(temp);
    
    sprintf(out, HttpFormat,
              postUrl,
              deviceIP,
              devicePort,
              controlService,
              "GetExternalIPAddress",
              len);
    
    strcat(out, temp);
    len = strlen(out);
    DBG_INFO((">>>>>>>>>>>>\n%s.\n", out));

    memset(temp, 0, 10240);
    ret = UPnP_InvokeCmd(out, temp, 10240);
    if(ret!=true)
    {
        DBG_INFO(("UPnP invoke GetExternalIP failed.\n"));
        return false;
    }
    
    if((p = strstri(temp, "200 OK")) == NULL)
    {
        return false;
    }

    if((p = strstri(temp, "<NewExternalIPAddress"))!=NULL)
    {
        p1 = strstri(p, ">");
        sscanf(p1, "%*[^0-9]%[^<]", externalIP);
    }
    
    return true;
    
}

bool zhUPnP_IsGatewayOK()
{
	return (strlen(externalIP) != 0)&&(strlen(deviceIP) != 0);
}

char* zhUPnP_GetExternalIP()
{
	return externalIP;
}

char* zhUPnP_GetDeviceIP()
{
	return deviceIP;
}


bool zhUPnP_GetPortMappingInfo( int PortMappingIndex, Tzh_UPNP_PORTMAPINFO *info)
{
    char out[10240];
    char temp[10240];

    char *p = NULL;
    char *p1 = NULL;
    int len;
    int ret;

    char Action_GetPort_Format[64] = {"<NewPortMappingIndex>%d</NewPortMappingIndex>"};

    sprintf(out, Action_GetPort_Format, PortMappingIndex);
    len = strlen(out);

    sprintf(temp, SoapFormat, "GetGenericPortMappingEntry", controlService, out, "GetGenericPortMappingEntry");
    len = strlen(temp);

    sprintf(out, HttpFormat,
                     postUrl,
                     deviceIP,
                     devicePort,
                     controlService,
                     "GetGenericPortMappingEntry",
                     len);

    strcat(out, temp);
    len = strlen(out);
    DBG_INFO((">>>>>>>>>>>>\n%s.\n", out));

    memset(temp, 0, 10240);
    ret = UPnP_InvokeCmd(out, temp, 10240);
    if(ret != true)
    {

        DBG_INFO(("UPnP invoke GetPortMappingInfo failed.\n"));
        return false;
    }
    
    if((p = strstri(temp, "200 OK")) == NULL)
    {
        return false;
    }
    else
    {
        if((p = strstri(temp, "<NewExternalPort")) != NULL)
        {
            p1 = strstri(p, ">");
            sscanf(p1, "%*[^0-9]%d", &info->ExternalPort);
        }
        if((p = strstri(temp, "<NewProtocol"))!=NULL)
        {
            p1 = strstri(p, ">");
            if(!strncmp("TCP", p1+1, 3))
                info->Protocol = 0;
            else if(!strncmp("UDP", p1+1, 3))
                info->Protocol = 1;
        }
        if((p = strstri(temp, "<NewInternalPort"))!=NULL)
        {
            p1 = strstri(p, ">");
            sscanf(p1, "%*[^0-9]%d", &info->InternalPort);
        }
        if((p = strstri(temp, "<NewInternalClient"))!=NULL)
        {
            p1 = strstri(p, ">");
            sscanf(p1+1, "%[^<]", info->InternalIP);
        }
        if((p = strstri(temp, "<NewEnabled"))!=NULL)
        {
            p1 = strstri(p, ">");
            sscanf(p1, "%*[^0-9]%d", &info->Enabled);
        }
        if((p = strstri(temp, "<NewPortMappingDescription"))!=NULL)
        {
            p1 = strstri(p ,">");
            sscanf(p1+1, "%[^<][64]", info->Description);
        }
        if((p = strstri(temp, "<NewLeaseDuration>"))!=NULL)
        {
            p1 = strstri(p, ">");
            sscanf(p1, "%*[^0-9]%d", &info->Duration);
        }

    }

    return true;

}

bool zhUPnP_AddPortMapping(const char *RemoteHose, 
                        unsigned short ExternalPort,
                        const char *PortMappingProtocal,
                        unsigned short InternalPort,
                        const char *InternalClient,
                        const char *PortMappingDescription,
                        int PortMappingEnable,
                        unsigned long PortMappingLeaseDuration)
{
    char out[10240];
    char temp[10240];

    char *p = NULL;
    int len;
    int ret;

    char Action_AddPort_Format[512] = 
{"<NewRemoteHost></NewRemoteHost>\r\n\
<NewExternalPort>%d</NewExternalPort>\r\n\
<NewProtocol>%s</NewProtocol>\r\n\
<NewInternalPort>%d</NewInternalPort>\r\n\
<NewInternalClient>%s</NewInternalClient>\r\n\
<NewEnabled>1</NewEnabled>\r\n\
<NewPortMappingDescription>%s</NewPortMappingDescription>\r\n\
<NewLeaseDuration>%d</NewLeaseDuration>\r\n"};

    sprintf(out, Action_AddPort_Format,
              ExternalPort,
              PortMappingProtocal,
              InternalPort, 
              InternalClient,
              PortMappingDescription,
              PortMappingLeaseDuration);
    len = strlen(out);
    
    sprintf(temp, SoapFormat,
               "AddPortMapping",
               controlService,
               out,
               "AddPortMapping");
    len = strlen(temp);
    
    sprintf(out, HttpFormat,
              postUrl,
              deviceIP,
              devicePort,
              controlService,
              "AddPortMapping",
              len);
    
    strcat(out, temp);
    len = strlen(out);
    DBG_INFO((">>>>>>>>>>>>\n%s.\n", out));

    memset(temp, 0, sizeof(temp));
    ret = UPnP_InvokeCmd(out, temp, sizeof(temp));
    if(ret != true)
    {
        DBG_INFO(("UPnP invoke AddPortMapping failed.\n"));
        return false;
    }
    
    if((p = strstri(temp, "200 OK")) == NULL)
        return false;
    
    return true;

}

bool zhUPnP_DeletePortMapping(const char *RemoteHose, 
						   unsigned short ExternalPort,
                           const char *PortMappingProtocal)
{
    char out[10240];
    char temp[10240];

    char *p = NULL;
    int len;
    int ret;

    char Action_AddPort_Format[512] = 
{"<NewRemoteHost></NewRemoteHost>\r\n\
<NewExternalPort>%d</NewExternalPort>\r\n\
<NewProtocol>%s</NewProtocol>\r\n"};

    sprintf(out, Action_AddPort_Format,
              ExternalPort,
              PortMappingProtocal);
    len = strlen(out);
    
    sprintf(temp, SoapFormat,
               "DeletePortMapping",
               controlService,
               out,
               "DeletePortMapping");
    len = strlen(temp);
    
    sprintf(out, HttpFormat,
              postUrl,
              deviceIP,
              devicePort,
              controlService,
              "DeletePortMapping",
              len);
    
    strcat(out, temp);
    len = strlen(out);
    DBG_INFO((">>>>>>>>>>>>\n%s.\n", out));

    memset(temp, 0, sizeof(temp));
    ret = UPnP_InvokeCmd(out, temp, sizeof(temp));
    if(ret != true)
    {
        DBG_INFO(("UPnP invoke DeletePortMapping failed.\n"));
        return false;
    }
    
    if((p = strstri(temp, "200 OK")) == NULL)
        return false;
    
    return true;

}


bool zhUPnP_FindDevice()
{
    int len, ret;
    char *p = NULL;
    char *p1 = NULL;
    SOCKET upnp_localsock;
    struct sockaddr_in qAddr, addr;
    char Location[128];
    char buf[UPNP_MSG_BUF_SIZE] = {0};
	int addrlen;

    memset((char*)&qAddr,0, sizeof(qAddr));
    qAddr.sin_family = AF_INET;
    qAddr.sin_port = htons(UPNP_SEARCH_PORT);
    qAddr.sin_addr.s_addr = inet_addr(UPNP_SEARCH_ADDR);

    /*create udp sock*/
    /*join multicast*/
	zhSockInit(&upnp_localsock,ZH_SOCK_UDP);
	if(zhSockSetReuseAddr(upnp_localsock,true))
    if(false==zhSockBindAddr(upnp_localsock,NULL,33445))
    {
        DBG_INFO(("UPnP join failed.\n"));
        goto __failed;
    }

    /*build and send search(IGD) msg */
    sprintf(buf,  SearchFormat, "upnp:rootdevice"/*"urn:schemas-upnp-org:device:InternetGatewayDevice:1"*/);
    len = strlen(buf);
    DBG_INFO((">>>>>>>>>>:\n%s\n",buf));
        
    ret = zhSockSendTo(upnp_localsock, buf, len, &qAddr);
    ret = zhSockSendTo(upnp_localsock, buf, len, &qAddr);
    if(ret == ERROR)
    {
        DBG_INFO(("UPnP send search packet failed.\n"));
        goto __failed;
    }

    
    /*receive response for search*/
    ret = zhSockCanRead(upnp_localsock,0,7000);
    if(ret <= 0)
    {
        DBG_INFO(("sock 7s timeout .\n"));
        goto __failed;
    }
    memset(buf, 0, UPNP_MSG_BUF_SIZE);
	addrlen=sizeof(struct sockaddr_in);
    ret = zhSockRecvFrom(upnp_localsock, buf, UPNP_MSG_BUF_SIZE, &addr,&addrlen);
    if(ret <= 0)
    {
        DBG_INFO(("UPnP receive response for search faield.\n"));
        goto __failed;
    }
    buf[ret] = '\0';
    DBG_INFO(("<<<<<<<<<\n%s\n",buf));

    /*search session over, close the socket*/
    if(upnp_localsock>=0)
    {
		zhSockClose(upnp_localsock);
        upnp_localsock = -1;
    }

    /*sample  is http OK*/
    if(strstri(buf, "200 OK") == NULL)
    {
        DBG_INFO(("UPnP receive bad http response. failed.\n"));
        return ERROR;
    }
    
    /*find Location value*/
    p = strstri(buf, "Location");
    if(!p)
        return ERROR;
    p1 = strstri(p, "\r\n");
    if(!p1)
        return ERROR;
    memcpy(Location, p, p1-p);
    Location[p1-p] = '\0';
    DBG_INFO(("UPnP get Location: %s.\n", Location));

    //get description accroding host:port\path
    strcpy(deviceAddr, Location);


    p = strstri(Location, "http://");
    if(!p)
    {
        DBG_INFO(("Location not viladate"));
        return false;
    }
    //parse ip port post
    sscanf(p, "%*[^0-9]%[^:]:%d/%s", deviceIP, &devicePort, postUrl);
    DBG_INFO(("Device location %s:%d/%s\n", deviceIP, devicePort, postUrl));

    return true;

    __failed: 
    if(upnp_localsock>=0)
    {
        zhSockClose(upnp_localsock);
        upnp_localsock = -1;
    }
    return false;
        
}
