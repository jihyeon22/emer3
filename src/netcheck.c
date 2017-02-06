#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <linux/unistd.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netdb.h>
#include <netinet/ether.h>

#include <linux/ioctl.h>

#include <at/at_util.h>
#include <at/at_log.h>
#include <logd_rpc.h>
#include "board_system.h"

void network_device_up()
{
	printf("%s +--\n", __func__);
	
	create_watchdog("network_device_up", 100); //100sec
	system(NETIF_DOWN_CMD);
	send_at_cmd("at$$apcall=0");
	sleep(2);
	send_at_cmd("at$$apcall=1");
	system(NETIF_UP_CMD);
	watchdog_delete_id("network_device_up");
	
	send_at_cmd("AT$$LEDOFF=0");

	printf("%s +--\n", __func__);
	
}

void network_device_down()
{
	printf("%s ++\n", __func__);

	create_watchdog("network_device_down", 100); //100sec
	system(NETIF_DOWN_CMD);
	send_at_cmd("at$$apcall=0");
	watchdog_delete_id("network_device_down");

	printf("%s +--\n", __func__);
}

int is_net_device_active( )
{
	int             sock, sock_ret;
	struct ifreq    ifr;

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
	if (sock < 0) {
		printf("%s Get socket open error : %s\n", strerror(errno));
		LOGE(eSVC_COMMON, "%s Get socket open error : %s\n", strerror(errno));
		return 0;
	}

	sprintf((char *)&ifr.ifr_name, "%s", MDMC_NET_INTERFACE );

	// Get IP Adress
	sock_ret = ioctl(sock, SIOCGIFADDR, &ifr);
    
	if (sock_ret < 0) {
		printf("%s not found\n", MDMC_NET_INTERFACE);
		LOGE(eSVC_COMMON, "%s not found\n", MDMC_NET_INTERFACE);
		close(sock);
		return 0;
	}
    
   
	close(sock);
	return 1;
}
