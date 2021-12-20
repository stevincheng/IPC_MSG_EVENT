#ifndef __server_tcp_H__
#define __server_tcp_H__

#include <stdio.h>
#include "log.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "sys_config.h"

#define TAG "log_manager"
#define LOG_FILE "/log_1.log"
#define LOG_CACHE_BUF_MAX_LEN   (LOG_BUF_MAX_LEN * 40)

#define SAVE_LOG_FREQ_N_SEC (50*1000) //shm log to log manager save freq (unit : ns)
#define SAVE_LOG_FILE_TIMEOUT ((1000*1000/SAVE_LOG_FREQ_N_SEC) * 1) //最低5s保存log到文件

int save_log_file(char *data,int data_len,char *file_name);

// void ota_upgrade_result(int result);
// typedef struct
// {
//     unsigned char mode;
//     unsigned int timeout;
// }EXL4_TBOX_OTA_MODE;

#endif