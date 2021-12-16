#include <stdio.h>
#include "log.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
// #include <stdio.h>
// #include <stdlib.h>

#include<cstring>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 7777
#define IPC_API_TAG "ipc_api_tag"
#define SERVER_SOCKET_CONECT_MAX 128
#define SERVER_FD_LISTEN_MAX 1024

#define IPC_CONN_MAX_CNT 3
#define IPC_REGIST_TYPE_MSG_LEN 12

enum IPC_CMD_TYPE
{
  IPC_TYPE_UNKNOWN,
  IPC_TYPE_REGIST_EVENT_TYPE,
  IPC_TYPE_SEND_MSG,
  IPC_TYPE_MAX
};

typedef int (* IPC_EVENT_API_CALLBACK)(int , void * , int);
extern int ipc_tcp_conn_init(IPC_EVENT_API_CALLBACK callback);
extern int ipc_regist_event_type(int type);
extern int ipc_send_msg(int type,char *data,int len);
extern int ipc_client_rec_loop();
