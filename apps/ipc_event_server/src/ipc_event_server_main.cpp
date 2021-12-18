// #include "../h/server_tcp.h"

#include "server_tcp.h"
#include "TcpServer.h"


// TCP_CONNET_INFO tcp_connet_info[128];

int main(int argc, char ** argv){
    int log_res = Log_init();
    if (log_res < 0)
        printf("Log_init err.\n");
    LOGI(TAG,"this is server process");
    TcpServer mTcpServer = TcpServer::instance();
    mTcpServer.tcpServerInit();
    mTcpServer.socketFdSelectRunLoop();
    return 0;
}