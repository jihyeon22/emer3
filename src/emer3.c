#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <mdsapi/mds_api.h>
#include <at/at_util.h>
#include <at/at_log.h>
#include <at/watchdog.h>
#include <logd_rpc.h>


#include <mdsapi/mds_shm.h>

#include "board_system.h"

#include "sms_cmd.h"
#include "netcheck.h"

#define MAX_CMD_TMP_BUFF 512

#define OPT_STR__IS_NET_CONN_DISABLE	"-no_use_net"

int g_opt_is_net_conn = 1;

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
	char* filtered_sms = NULL;

	int cnt = 0;
	int target_len = 0;
	const char *s = msg;

	LOGT(eSVC_COMMON, "===============================================\n");
	LOGT(eSVC_COMMON, "emer sms callback function...\n");
	LOGD(eSVC_COMMON, "%s : phone_num [%s]\r\n", __func__,  phone_num);
	LOGD(eSVC_COMMON, "%s : recv_time [%s]\r\n",  __func__,  recv_time);
	LOGD(eSVC_COMMON, "%s : msg [%s]\r\n", __func__,  msg);	
	
	util_remove_cr(msg, target, strlen(msg));
	filtered_sms = target;
	// remove "[web....]"
	//printf(" strncasecmp => [%c] [%d]\r\n", target[0], strncasecmp(target+1,"web", strlen("web") ) );
	if (( target[0] == '[' ) && ( strncasecmp(target+1,"web", strlen("web") )  == 0 ) )
	{
		char* tmp_char = strstr(target+4,"]");

		if ( tmp_char != NULL )
		{
			tmp_char++;
			filtered_sms = tmp_char;
		}
	}


	LOGI(eSVC_COMMON, "parser target cmd is [%s] / [%s]\r\n", target, filtered_sms);
	LOGT(eSVC_COMMON, "===============================================\n");
	
	parse_model_sms(recv_time, phone_num, filtered_sms);
	
}


void chk_qcmap_proc()
{
	int ret = 0;

	ret = mds_api_proc_find("QCMAP_ConnectionManager");
	LOGD(eSVC_COMMON, "check qcmap proc [%d] \r\n", ret);
	if ( ret <= 0 )
	{
		LOGE(eSVC_COMMON, " >> force run qcmap proc \r\n");
		system("/usr/bin/QCMAP_ConnectionManager /etc/mobileap_cfg.xml &");
	}
}

void main(int argc, char* argv[])
{
	int network_max_check_count = 240; // when 5sec delay, 240 means 20min.
	int net_check_count = 0;
	int chk_qcmap_proc_count = 0;

	int net_wait_time = 5;

	int pid, sid;
	int count = 0;

	int i = 0;

    int led_onoff_cmd_flag = 0;

	// default is network conn.
	g_opt_is_net_conn = 1;

	logd_init();

	for ( i = 0 ; i < argc ; i ++ )
	{
		printf("argv [%d] => [%s]\r\n", i, argv[i]);
		if ( strncmp(argv[i], OPT_STR__IS_NET_CONN_DISABLE, strlen(OPT_STR__IS_NET_CONN_DISABLE)) == 0 )
		{
			printf("network disable !!\r\n" );
			g_opt_is_net_conn = 0;
		}
	}

#ifdef NO_USE_NETWORK
	printf("network disable !!\r\n" );
	g_opt_is_net_conn = 0;
#endif

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

	printf("start main looping. [%d]\n", g_opt_is_net_conn);
	net_check_count = 0;

	// emer main proc wait and run.
	sleep(20);

	/*
	// 강제로 led 켠다. 간혹가다가 혹시나, led off 상태에서 출고될까봐..
	if ( shm_data.test_mode == 0 )
		send_at_cmd("AT$$LEDOFF=0");
	else
		send_at_cmd("AT$$LEDOFF=1");
	*/
	// first check.
    if(!is_net_device_active()) {
        chk_qcmap_proc();  // if network disable when check process
        network_device_up();
    }

#ifndef NO_USE_NETWORK
    // network check...
    while(g_opt_is_net_conn)
    {
        // main routine...
        if(!is_net_device_active()) {
            chk_qcmap_proc();  // if network disable when check process
            network_device_up();
            net_wait_time = 5;
            net_check_count += 1;
            chk_qcmap_proc_count += 1;
        } else {
            net_wait_time = 30;	// normal stat : 30sec interval
            net_check_count = 0;
            chk_qcmap_proc_count = 0;

            if ( led_onoff_cmd_flag == 0)
            {
                SHM_GLOBAL_DATA_T shm_data = {0,};
                app_shm_get_global_data(&shm_data);
                if ( shm_data.test_mode == 0 )
                    send_at_cmd("AT$$LEDOFF=0");
                led_onoff_cmd_flag = 1;
            }
        }
        
        sleep(net_wait_time);

        if ( chk_qcmap_proc_count > 10 ) // 5sec * 30 = 150sec :: check qcmap proc 
        {
            chk_qcmap_proc();
            chk_qcmap_proc_count = 0;
        }

        if(net_check_count < network_max_check_count)
            watchdog_keepalive_id("emer.main");
        
        printf("emer main loop alive[%d/%d]\r\n", net_check_count, network_max_check_count);
        LOGD(eSVC_COMMON, "emer main loop alive[%d/%d]\r\n", net_check_count, network_max_check_count);
    }
    #endif
    // network do not check.
    printf("emer no check network!! do not conn network!!!!");

    while(1)
    {
        net_wait_time = 60;
        net_check_count = 0;

        LOGI(eSVC_COMMON, "emer ::: SKIP NETWORK CHK\r\n");
        sleep(net_wait_time);

        watchdog_keepalive_id("emer.main");
    }

}
