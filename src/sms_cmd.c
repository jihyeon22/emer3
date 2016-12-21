#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sms_cmd.h"
#include "at/at_util.h"

int reset_chk_cnt=0;

static int mds_sms_hook_cmd_echo=0;
static int mds_sms_hook_enable = 0;

static SMS_CMD_FUNC_T sms_cmd_func[] =
{
	{eSMS_CMD_ENABLE_ECHO,        "neom2m.open+echo", _sms_cmd_enable_echo},
	{eSMS_CMD_ENABLE_NORMAL,      "neom2m.open",      _sms_cmd_enable_normal},
	{eSMS_CMD_DISABLE,            "neom2m.close",     _sms_cmd_disable},
	{eSMS_CMD_REQ_FACKTORY_RESET, "facreset",         _sms_cmd_req_factory_reset},	
	{eSMS_CMD_REQ_RESET,          "reset",            _sms_cmd_req_reset},
	{eSMS_CMD_GET_IMG_INFO,       "version",          _sms_cmd_get_version},
	{eSMS_CMD_REQ_MAIN_APP_VER,   "appver",           _sms_cmd_req_main_app_ver},	
	{eSMS_CMD_REQ_RUN_LINUX_APP,  "run",              _sms_cmd_req_run_linux_app},	
	{eSMS_CMD_SET_RUN_MODE,       "mode",             _sms_cmd_set_run_mode},	
	{eSMS_CMD_REQ_BOOT_WD_STAT,   "bwdstat",          _sms_cmd_req_boot_wd_stat},
	{0,                           NULL,               NULL},
};

int parse_model_sms(const char *time, const char *phonenum, char *sms)
{
//	int msg_type = 0;
	int ret = 0;
	
	int model_argc = 0;
	char *model_argv[32] = {0};

	char *base = 0;
	int len = 0;
	int i = 0;
	
	int found_cmd = 0;

	base = sms;
	len = strlen(sms);
	
	model_argv[model_argc++] = base;

	while(model_argc <= 32 && len--) 
	{
		switch(*base) {
			case ',':
				*base = '\0';
				model_argv[model_argc] = base + 1;
				model_argc++;
				break;
			default:
				break;
		}
		base++;
	}


	if(model_argv[0] == NULL)
	{
		return -1;
	}

	// find function..
	//for(i = 0; i < MAX_SMS_CMD; i++)
	//{
	//	if  (!( strstr ( model_argv[0], sms_cmd_func[i].cmd ) == NULL) )
	//	{
	//		found_cmd = 1;
	//		break;
	//	}
	//}
	printf("model_argv[0] = %s\n", model_argv[0]);
	i = 0;
	while(1) {
		char* p_cmd_start = NULL;

		if(sms_cmd_func[i].cmd == NULL) {
			printf("no detection cmd\n");
			break;
		}
		
		p_cmd_start = strstr(model_argv[0], sms_cmd_func[i].cmd);

		if( p_cmd_start != NULL)
		{
			printf("get cmd [%s] / [%s]\r\n", model_argv[0] , sms_cmd_func[i].cmd);
			if ( strncmp(model_argv[0], sms_cmd_func[i].cmd, strlen(sms_cmd_func[i].cmd) ) == 0 )
			{
				printf("model_argv[0] is [%s]\r\n", model_argv[0]);
				found_cmd = 1;
				break;
			}
		}
		i += 1;
	}

	if (found_cmd)
	{
		if(sms_cmd_func[i].proc_func != NULL)
			sms_cmd_func[i].proc_func(model_argc, model_argv, phonenum);
	}
	
	return ret;
}




static int _sms_cmd_enable_echo(int argc, char* argv[], const char* phonenum)
{
	mds_sms_hook_enable = 1;
	mds_sms_hook_cmd_echo = 1;

	at_send_sms( phonenum, "system.open" );
	//send_sms ( phonenum, "system.open");

	return 0;
}


static int _sms_cmd_enable_normal(int argc, char* argv[], const char* phonenum)
{
	mds_sms_hook_enable = 1;
	mds_sms_hook_cmd_echo = 0;

	return 0;
}

static int _sms_cmd_disable(int argc, char* argv[], const char* phonenum)
{
	if (mds_sms_hook_cmd_echo == 1)
		//at_send_sms ( phonenum, "system.close");
	
	mds_sms_hook_cmd_echo = 0;
	mds_sms_hook_enable = 0;
	
	return 0;
}

static int _sms_cmd_req_reset(int argc, char* argv[], const char* phonenum)
{
	char ret_str[80] = {0,};
	int power_off_cnt = 0;
			
	if ( mds_sms_hook_enable == 0)
		return 0;
	
	if (mds_sms_hook_cmd_echo == 1)
	{
		sprintf( ret_str, "cmd: reset - cnt [%d] \r", reset_chk_cnt++);
		at_send_sms ( phonenum, (const char*)ret_str );
	}
	
	//mds_sms_hook_cmd_reset(15);     // 10 sec after reset...
	while(1)
	{
		system("poweroff &");
		sleep(10);
		if(power_off_cnt++ > 10) {
			system("echo c > /proc/sysrq-trigger");
		}
	}
	
	return 0;
}


static int _sms_cmd_get_version(int argc, char* argv[], const char* phonenum)
{
	char ret_str[80] = {0,};
	char nickname[128] = {0,};
	
	if ( mds_sms_hook_enable == 0)
		return 0;

	//smd_echo_transmit(ret_str, strlen(ret_str));
	if (mds_sms_hook_cmd_echo == 1)
	{
		//get_mds_img_nick(nickname, 128);
		//sprintf( ret_str, "cmd : version \n %s", nickname);
		//send_sms ( phonenum, ret_str );
	}
	
	return 0;
}



static int _sms_cmd_req_main_app_ver(int argc, char* argv[], const char* phonenum)
{
	char ret_str[80] = {0,};
	char app_ver[68] = {0,};
	
	if ( mds_sms_hook_enable == 0)
		return 0;

	//smd_echo_transmit(ret_str, strlen(ret_str));
	if (mds_sms_hook_cmd_echo == 1)
	{
		//get_mds_app_ver(app_ver, 68);
		//sprintf( ret_str, "cmd : appver \n %s", app_ver);
		//send_sms ( phonenum, ret_str );
	}
	
	return 0;
}




static int _sms_cmd_req_run_linux_app(int argc, char* argv[], const char* phonenum)
{
	char ret_str[80] = {0,};
	char cmd_buff[128] = {0,};
	
	// printf("%s> mds_sms_hook_enable[%d] argc[%d]\n", __func__, mds_sms_hook_enable, mds_sms_hook_enable);
	// printf("%s> strlen(argv[1]) = %d\n", __func__, strlen(argv[1]));

	if ( mds_sms_hook_enable == 0)
		return 0;
	
	if (argc < 2)
		return 0;
	
	if (strlen(argv[1]) <= 0)
		return 0;
	
	strncpy(ret_str, argv[1], strlen(argv[1]));
	
	sprintf(cmd_buff, "%s &", ret_str);

	//printf("%s> cmd_buff = %s\n", cmd_buff);
	//system(ret_str);
	system(cmd_buff);
	
	if (mds_sms_hook_cmd_echo == 1)
		at_send_sms ( phonenum, ret_str );
	
	return 0;
}

static int _sms_cmd_set_run_mode(int argc, char* argv[], const char* phonenum)
{
	char ret_str[80] = {0,};
	
	if ( mds_sms_hook_enable == 0)
		return 0;
	
	if (argc != 2)
		return 0;
	
	// 0 : clear linux flag
	if (! strncmp(argv[1], "0" , strlen("0")))
	{
		//mds_api_linux_boot_set( DO_NOT_RUN_LINUX );
		//sprintf(ret_str, "set mode - clear success : do not run linux ");
	}
	// 1 : set linux flag
	else if (! strncmp(argv[1], "1" , strlen("1")))
	{
		//mds_api_linux_boot_set( RUN_LINUX );
		//sprintf(ret_str, "set mode - set success : run linux");
	}
	// 2 : check linux flag
	else if (! strncmp(argv[1], "2" , strlen("2")))
	{
		/*
		if ( mds_api_linux_boot() == DO_NOT_RUN_LINUX )
		{ // do not linux run
			sprintf(ret_str, "check mode - do not run linux");
		}
		else
		{
			sprintf(ret_str, "check mode - run linux");
		}
		*/
	}
	else
	{
		sprintf(ret_str, "invalid mode argument");
	}
	
	if (mds_sms_hook_cmd_echo == 1)
    	//send_sms ( phonenum, ret_str );
		;
	
	return 0;
}

static int _sms_cmd_req_factory_reset(int argc, char* argv[], const char* phonenum)
{
	char ret_str[80] = {0,};
	int ret = 0;
	int count = 0;
	
	if ( mds_sms_hook_enable == 0)
		return 0;
	
	if (argc != 2)
		return 0;
	
	// 0 : fac reset flag clear 
	if (! strncmp(argv[1], "0" , strlen("0")))
	{
		// 1 : factory reset flag set..
		// 0 : factory reset flag clear...
		//ret = mds_factory_reset_set(1);
		//sprintf(ret_str,"factory reset flag set (0x%x)", ret);
	}
	// 1 : fac reset flag set
	else if (! strncmp(argv[1], "1" , strlen("1")))
	{
		//ret = mds_factory_reset_get_cnt(&count);
		//sprintf(ret_str,"factory reset count = [%d] (0x%x)", count , ret);
	}

	if (mds_sms_hook_cmd_echo == 1)
		//send_sms ( phonenum, ret_str );
		;
	
	return 0;
}

static int _sms_cmd_req_boot_wd_stat(int argc, char* argv[], const char* phonenum)
{
	char ret_str[80] = {0,};
	
	if ( mds_sms_hook_enable == 0)
		return 0;
	
	if (mds_sms_hook_cmd_echo == 1)
	{
		//sprintf(ret_str,"bwd e (%d) f (%d)/(%d)\r\n", g_boot_watchdog_boot_chk_enable, g_boot_watchdog_fail_cnt, MDS_WATCHDOG_BOOT_FAIL_MAX_CNT);
		//send_sms ( phonenum, ret_str );
	}
	
	return 0;
}


