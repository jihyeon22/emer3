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

#include <mdsapi/mds_shm.h>

void network_device_up()
{
	SHM_GLOBAL_DATA_T shm_data = {0,};

    static time_t last_cycle = 0;
    time_t cur_time = 0;

	struct timeval tv;
	struct tm ttm;

	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &ttm);
    cur_time = mktime(&ttm);

    app_shm_get_global_data(&shm_data);
	if ( shm_data.test_mode == 1 )
	{
		printf("%s test mod set... do nothing .. return \n", __func__);
		LOGI(eSVC_COMMON, "%s test mod set... do nothing .. return\r\n", __func__);
		return ;
	}
	else 
	{
		printf("%s test mod not set...\n", __func__);
	}

	printf("%s +--\n", __func__);

	create_watchdog("network_device_up", 100); //100sec

    // 너무 자주할경우 아예 안붙을것 같다는거지..
    // 30 sec interval network chk
    // 불리는 주기가 불규칙, 때문에 시간계산하여 30초마다 한번씩 불리도록
    {
        if(last_cycle == 0)
            last_cycle = cur_time;

        if ( cur_time < last_cycle )
            last_cycle = cur_time;

        if( (cur_time - last_cycle) < 30 )
        {
            printf("network check interval skip... [%d][%d]\r\n", (cur_time - last_cycle), 30);
            return 0;
        }

        last_cycle = cur_time;
    }

	system(NETIF_DOWN_CMD);
	send_at_cmd("at$$apcall=0");
	sleep(2);
	send_at_cmd("at$$apcall=1");
	system(NETIF_UP_CMD);
	watchdog_delete_id("network_device_up");
	
	// sync 가 가끔 안맞을 때가 있다.
	// 한번더 cmd 를 write 한다.
	app_shm_get_global_data(&shm_data);
	if ( shm_data.test_mode == 0 )
		send_at_cmd("AT$$LEDOFF=0");
	else
		send_at_cmd("AT$$LEDOFF=1");

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
