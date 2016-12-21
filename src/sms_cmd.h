#ifndef __SMS_CMD_H__
#define __SMS_CMD_H__

#define MAX_CMD_TMP_BUFF 512

typedef struct
{
	int index;
    char * cmd;
    int (*proc_func)(int argc, char* argv[], const char* phonenum);
}SMS_CMD_FUNC_T;

typedef enum
{
	eSMS_CMD_ENABLE_ECHO,
	eSMS_CMD_ENABLE_NORMAL,
	eSMS_CMD_DISABLE,
	eSMS_CMD_REQ_FACKTORY_RESET,
	eSMS_CMD_REQ_RESET,
	eSMS_CMD_GET_IMG_INFO,
	eSMS_CMD_REQ_MAIN_APP_VER,
	eSMS_CMD_REQ_RUN_LINUX_APP,
	eSMS_CMD_SET_RUN_MODE,
	eSMS_CMD_REQ_BOOT_WD_STAT,
	MAX_SMS_CMD,
}SMS_CMD_INDEX;

int parse_model_sms(const char *time, const char *phonenum, char *sms);
static int _sms_cmd_enable_echo(int argc, char* argv[], const char* phonenum);
static int _sms_cmd_enable_normal(int argc, char* argv[], const char* phonenum);
static int _sms_cmd_disable(int argc, char* argv[], const char* phonenum);
static int _sms_cmd_req_reset(int argc, char* argv[], const char* phonenum);
static int _sms_cmd_get_version(int argc, char* argv[], const char* phonenum);
static int _sms_cmd_req_main_app_ver(int argc, char* argv[], const char* phonenum);
static int _sms_cmd_req_factory_reset(int argc, char* argv[], const char* phonenum);
static int _sms_cmd_req_run_linux_app(int argc, char* argv[], const char* phonenum);
static int _sms_cmd_set_run_mode(int argc, char* argv[], const char* phonenum);
static int _sms_cmd_req_factory_reset(int argc, char* argv[], const char* phonenum);
static int _sms_cmd_req_boot_wd_stat(int argc, char* argv[], const char* phonenum);

#endif