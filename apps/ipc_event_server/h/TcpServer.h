#ifndef __TcpServer_H__
#define __TcpServer_H__

#include <stdio.h>
#include "server_tcp.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
// #include <poll.h>
#include <vector>
#include "ipc_event_config.h"



typedef struct{
    int event_type;
    int fd;
}IPC_EVENT_TYPE_FD;
typedef std::vector<IPC_EVENT_TYPE_FD> IPC_EVENT_TYPE_FD_VECTOR;

extern void *tcp_server_connect_thread(void *arg);
class TcpServer
{
    private:
        // int connected_client_cnt;
        int listen_fd; 
        int conn_fd;
        int max_fd;
        fd_set all_fd_set;
        fd_set ready_fd_set;
        int nready;
        int client_fd_list[FD_SETSIZE];
        char read_fd_buf[BUFSIZ];
        struct sockaddr_in server_addr;
        struct sockaddr_in client_addr;
        IPC_EVENT_TYPE_FD_VECTOR ipc_type_fd_vector;
        // std::vector<IPC_EVENT_TYPE_FD> ipc_type_fd_vector;

    public:
        
        static TcpServer instance();
        TcpServer();
        void tcpServerInit();
        void socketFdSelectRunLoop();
        void parseReadFdBuff(char *data,int len,int from_fd);
        int parseIpcCmdInt(char *data);
        void parseIpcCmd(char *data,int len,int from_fd);
};

#endif