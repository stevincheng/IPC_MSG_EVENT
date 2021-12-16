// #include "../h/server_tcp.h"

#include "server_tcp.h"
#include "TcpServer.h"


// TCP_CONNET_INFO tcp_connet_info[128];

int main(int argc, char ** argv){
    LOGI(TAG,"this is server process");
    TcpServer mTcpServer = TcpServer::instance();
    mTcpServer.tcpServerInit();
    mTcpServer.socketFdSelectRunLoop();
    return 0;
}