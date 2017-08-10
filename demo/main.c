
#include "../upnp/zhUPnP_client.h"

bool PrintPortMap()
{
    int i = 0;
    int ret;
    Tzh_UPNP_PORTMAPINFO info;

    printf("\n\n---------------------------------------------------------------\n");
    printf("This Internet Gateway Device's UPnP PortMapping Information is :\n");
    printf("---------------------------------------------------------------\n");
    printf("\n%s   %s   %s      %s         %s   %s \n", "Protocol", "ExternalPort", "InternalIP", "InternalPort", "Description", "Ena");

    while(1)
    {
        ret = zhUPnP_GetPortMappingInfo(i++, &info);
        if(ret != true)
        {
            printf("---------------------------------------------------------------\n\n");
            break;
        }
        else
        {
            printf("%5s%13d%20s%12d%24.22s%5d\n", (info.Protocol == 0)?"TCP":"UDP", info.ExternalPort, info.InternalIP, info.InternalPort, info.Description, info.Enabled);
        }
    }

    return true;
}

int main(int argc, char *argv[])
{
    int ret;
    int i;
    
    char cmd;

    for(i=0;i<5;i++)
    {
        ret = zhUPnP_FindDevice();
        if(ret == true)
        {
            zhUPnP_GetDeviceDescription();
            break;
        }
    }
    
    if(ret != true)
    {
        printf("\nThere is no Internet Gateway Device finded.You can try again later.Quit.\n");
        return false;
    }
    zhUPnP_GetExternalIPAddress();

    if(zhUPnP_IsGatewayOK())
    {
        printf("Find A Internet Gateway Device. External IP is %s. Internal IP is %s.\n", zhUPnP_GetExternalIP(), zhUPnP_GetDeviceIP());
    }
    else
    {
        printf("There is no Internet Gateway Device finded.You can try again later.Quit.\n");
        return false;
    }
    
	//ÊäÈëÖ¸Áî
	while(1)
	{
		printf("\r\n*******************************\r\n");
		printf("a = insert new port\r\n");
		printf("d = delete port\r\n");
		printf("i = printf all port information\r\n");
		printf("*******************************\r\n");
		printf("\r\nplease input cmd:");
		fflush(stdin);
		scanf( "%c", &cmd);

		switch(cmd)
		{
			case 'a':
				{
					if(zhUPnP_AddPortMapping("",2323, "TCP", 2323, "192.168.1.116", "Han.zh-UPnP", 1, 0))
					{
						printf("AddPortMapping Action Successed.  protocol=tcp port=2323 descript=Han.zh-UPnP\n");
					}
					else
					{
						printf("AddPortMapping Action Fail.\n");
					}
				}
				break;
			case 'd':
				{
					if(zhUPnP_DeletePortMapping("", 2323, "TCP"))
					{
						printf("DeletePortMapping successed. port=2323\n");
					}
					else
					{
						printf("DeletePortMapping fail. port=2323\n");
					}
				}
				break;
			case 'i':
				PrintPortMap();
				break;		
			default:
				printf("NO such cmd. use upnp -h get help info.\n");
				break;
		}
        
	}
    return 0;
}



