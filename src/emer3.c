#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <at/at_util.h>
#include <at/at_log.h>
#include <at/watchdog.h>
#include <logd_rpc.h>

#include "sms_cmd.h"
#include "netcheck.h"

#define MAX_CMD_TMP_BUFF 512

int util_remove_cr(const char *s, char* target, int target_len)
{
	int cnt = 0;

	while (*s)
	{
		//printf("strlen [%c]\r\n" ,*s);
		if ( ( *s != '\r' ) && ( *s != '\n' ) )
		{
			target[cnt] = *s;
			cnt++;
			
			if (cnt > target_len)
				return -1;
		}
		s++;
	}
	//printf("strlen count [%d]\r\n" ,cnt);
	return cnt;
}

void sms_proc(const char* phone_num, const char* recv_time, const char* msg)
{

	//_parse_sms(buffer);
	
	char target[MAX_CMD_TMP_BUFF]={0,};
	int cnt = 0;
	int target_len = 0;
	const char *s = msg;

	LOGT(eSVC_COMMON, "===============================================\n");
	LOGT(eSVC_COMMON, "emer sms callback function...\n");
	LOGD(eSVC_COMMON, "%s : phone_num [%s]\r\n", __func__,  phone_num);
	LOGD(eSVC_COMMON, "%s : recv_time [%s]\r\n",  __func__,  recv_time);
	LOGD(eSVC_COMMON, "%s : msg [%s]\r\n", __func__,  msg);	
	
	util_remove_cr(msg, target, strlen(msg));
	
	LOGI(eSVC_COMMON, "parser target cmd is [%s]\r\n", target);
	LOGT(eSVC_COMMON, "===============================================\n");
	
	parse_model_sms(recv_time, phone_num, target);
	
}

void main()
{
	int network_max_check_count = 512; //when 5sec delay, 512 means 42min.
	int net_check_count = 0;
	int net_wait_time = 5;

	int pid, sid;
	int count = 0;

	logd_init();

	// make deamon Process...
	pid = fork();
	while (pid < 0 )
	{
		perror("fork error : ");
		pid = fork();
		if (count == 10)
			exit(0);
		sleep(10);
		count++;
	}
	if (pid > 0)
		exit(EXIT_SUCCESS);

	sid = setsid();
	if (sid < 0)
		exit(EXIT_FAILURE);

	chdir("/");

    mds_api_stackdump_init();
    
	printf("at_listener_open call.\n");
	at_listener_open();

	printf("at_open call.\n");
	at_open(AT_LIB_TARGET_DEV, NULL, sms_proc, NULL);

	printf("create_watchdog call.\n");
	create_watchdog("emer.main", 3*60); //3min

	printf("start main looping.\n");
	net_check_count = 0;
	while(1)
	{
		
		// main routine...
		if(!is_net_device_active()) {
			network_device_up();
			net_wait_time = 5;
			net_check_count += 1;
		} else {
			net_wait_time = 60;
			net_check_count = 0;
		}
		
		sleep(net_wait_time);
	
		if(net_check_count < network_max_check_count)
			watchdog_keepalive_id("emer.main");
		
		printf("emer main loop alive[%d/%d]\r\n", net_check_count, network_max_check_count);
		LOGD(eSVC_COMMON, "emer main loop alive[%d/%d]\r\n", net_check_count, network_max_check_count);
	}
}
