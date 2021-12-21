#include "TcpServer.h"
TcpServer TcpServer::instance()
{
    static TcpServer m_instance;
    return m_instance;
}

TcpServer::TcpServer()
{
    // connected_client_cnt = 1;
}
void TcpServer::tcpServerInit(){
    listen_fd = socket(AF_INET,SOCK_STREAM,0);
    int opt = 1;
    setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET,SERVER_IP,&server_addr.sin_addr.s_addr);
    int bind_res = bind(listen_fd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    if (bind_res == -1)
    {
        LOGI(TAG,"socket bind err.");
        return;
    }
    
    int listen_res = listen(listen_fd,SERVER_SOCKET_CONECT_MAX);
    if (listen_res == -1)
    {
        LOGI(TAG,"socket listen err.");
        return;
    }

    max_fd = listen_fd;
    for (size_t i = 0; i < FD_SETSIZE; i++)
    {
        client_fd_list[i] = -1;
    }
    FD_ZERO(&all_fd_set);
    FD_SET(listen_fd,&all_fd_set);
    LOGI(TAG,"tcpServerInit listen_fd = %d max_fd = %d",listen_fd,max_fd);
}

void TcpServer::socketFdSelectRunLoop(){
    LOGI(TAG,"socketFdSelectRunLoop .");
    while (1){
        ready_fd_set = all_fd_set;
        nready = select(max_fd + 1,&ready_fd_set,NULL,NULL,NULL);
        if (nready < 0){
            LOGI(TAG,"TCP server slect err.");
            return;
        }
        if (FD_ISSET(listen_fd,&ready_fd_set)) {  //有新连接
            socklen_t addr_len = sizeof(client_addr);
            conn_fd = accept(listen_fd,(struct sockaddr *)&client_addr,&addr_len);
            FD_SET(conn_fd,&all_fd_set);
            if (conn_fd > max_fd)
                max_fd = conn_fd;
            
            char client_ip[BUFSIZ];
            inet_ntop(AF_INET,&(client_addr.sin_addr.s_addr),client_ip,sizeof(client_ip));
            int client_port = ntohs(client_addr.sin_port);
            LOGI(TAG,"new connect : fd = %d ip = %s port = %d",conn_fd,client_ip,client_port);

            for (size_t i = 0; i < FD_SETSIZE; i++){
                if ( client_fd_list[i] < 0 ){
                    client_fd_list[i] = conn_fd;
                    break;
                }
            }
        }
        for (int i = 0; i < FD_SETSIZE; i++){
            if ( client_fd_list[i] < 0) continue;
            if (FD_ISSET(client_fd_list[i],&ready_fd_set)){ //有信息传递
                memset(read_fd_buf,0,sizeof(read_fd_buf));
                int n = read(client_fd_list[i],read_fd_buf,sizeof(read_fd_buf));
                if (n == 0){    //客户端断开了
                    LOGI(TAG,"client disconnect ,fd = %d",client_fd_list[i]);
                    close(client_fd_list[i]);
                    FD_CLR(client_fd_list[i],&all_fd_set);
                    client_fd_list[i] = -1;
                }else if (n > 0){   //读到新数据
                    parseReadFdBuff(read_fd_buf,n,client_fd_list[i]);
                }
            }
        }
    }
    if (listen_fd > 0)   
        close(listen_fd);
}
void TcpServer::parseReadFdBuff(char *buf,int len,int from_fd){
    char data[BUFSIZ] = {0};
    memcpy(data,buf,len);

    LOG_RAW_DATA_HEX(TAG,"parseReadFdBuff buf = ",data,len);
    int index = 0;
    while (1)
    {
        int cmd_len = parseIpcCmdInt(data + index);
        if (index + cmd_len <= len)
        {
            parseIpcCmd(data + index,cmd_len,from_fd);
        }
        index = index + cmd_len;
        if (index >= len)
        {
            // LOGI(TAG,"parseReadFdBuff will out buf size.");
            break;
        }
    }
}
int TcpServer::parseIpcCmdInt(char *data){
    int value = 0;
    memcpy(&value,data,sizeof(int));
    value = ntohl(value);
    return value;
}
void TcpServer::parseIpcCmd(char *data,int len,int from_fd){

    // LOG_RAW_DATA_HEX(TAG,"parseIpcCmd buf = ",data,len);
    int ipc_server_event_type = parseIpcCmdInt(data + 4);
    switch (ipc_server_event_type)
    {
        case IPC_TYPE_REGIST_EVENT_TYPE:{
            IPC_EVENT_TYPE_FD ipc_event_type_fd;
            ipc_event_type_fd.event_type = parseIpcCmdInt(data + 8);
            ipc_event_type_fd.fd = from_fd;
            ipc_type_fd_vector.push_back(ipc_event_type_fd);
            break;
        }
        case IPC_TYPE_SEND_MSG:{
            int ipc_msg_type = parseIpcCmdInt(data + 8);
            int data_len = len - IPC_REGIST_TYPE_MSG_LEN;
            for (IPC_EVENT_TYPE_FD_VECTOR::iterator it = ipc_type_fd_vector.begin();it!=ipc_type_fd_vector.end();it++)
            {
                if (it->event_type == ipc_msg_type){
                    // LOGI(TAG,"this is a event of send msg.... from_fd = %d  to_fd = %d ipc_msg_type = %08x",from_fd,it->fd,ipc_msg_type);
                    // LOGI(TAG,"ipc_msg_type = %08X buf = %s",ipc_msg_type,data+12);
                    send(it->fd,data,len,MSG_NOSIGNAL);
                }
            }
            break;
        }
    
        default:
            break;
    }
    return;
}

// void TcpServer::tcpServerInit(){
//     int socket_fd,conn_fd;
//     socket_fd = socket(AF_INET,SOCK_STREAM,0);

//     int opt = 1;
//     setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

//     struct sockaddr_in server_addr,client_addr;
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(SERVER_PORT);
//     inet_pton(AF_INET,SERVER_IP,&server_addr.sin_addr.s_addr);
//     bind(socket_fd,(struct sockaddr *)&server_addr,sizeof(server_addr));
//     listen(socket_fd,SERVER_SOCKET_CONECT_MAX);

//     struct pollfd client_fd[SERVER_FD_LISTEN_MAX];
//     for (size_t i = 1; i < SERVER_FD_LISTEN_MAX; i++)
//     {
//         client_fd[i].fd = -1;
//     }

//     int poll_res = 0;
//     while (1)
//     {
//         LOGI(TAG,"connected_client_cnt = %d",connected_client_cnt);
//         if (connected_client_cnt >= SERVER_FD_LISTEN_MAX - 1)
//         {
//             LOGI(TAG,"client reach limit,not need listen new connect.");
//             client_fd[0].fd = -1;
//         }else{
//             client_fd[0].fd = socket_fd;
//             client_fd[0].events = POLLIN;
//         }
        
//         poll_res = poll(client_fd,connected_client_cnt,-1);
//         if (client_fd[0].revents & POLLIN)                      /*有新的连接事件*/
//         {
//             socklen_t addr_len = sizeof(client_addr);
//             conn_fd = accept(socket_fd,(struct sockaddr *)&client_addr,&addr_len);
//             char client_ip[BUFSIZ];
//             inet_ntop(AF_INET,&(client_addr.sin_addr.s_addr),client_ip,sizeof(client_ip));
//             int client_port = ntohs(client_addr.sin_port);
//             LOGI(TAG,"new connect : fd = %d ip = %s port = %d",conn_fd,client_ip,client_port);
//             for (size_t m = 1; m < SERVER_FD_LISTEN_MAX; m++)   /*将客户端加入poll监听*/
//             {
//                 if (client_fd[m].fd == -1)
//                 {
//                     client_fd[m].fd = conn_fd;
//                     client_fd[m].events = POLLIN;
//                     connected_client_cnt ++;
//                     break;
//                 }
//             }
//         }
//         for (size_t i = 1; i < SERVER_FD_LISTEN_MAX; i++)
//         {
//             if (client_fd[i].fd < 0)
//             {
//                 continue;
//             }
//             if (client_fd[i].revents & POLLIN)          /*客户端有发送数据*/
//             {
//                 char read_client_buf[BUFSIZ] = {0};
//                 int n = read(client_fd[i].fd,read_client_buf,BUFSIZ);
//                 if (n > 0)
//                 {
//                     // LOGI(TAG,"read_client_buf = %s  n = %d",read_client_buf,n);
//                     // for (size_t j = 0; j < n; j++)
//                     // {
//                     //     read_client_buf[j] = toupper(read_client_buf[j]);
//                     // }
//                     // write(client_fd[i].fd,read_client_buf,n);
//                     printf("ser_ver read n = %d buf : ",n);
//                     for (size_t i = 0; i < n; i++)
//                     {
//                         printf("%x ",read_client_buf[i]);
//                     }
//                     printf("\n");

//                 }else{                                  /*读取客户端数据失败*/
//                     if (n == 0){
//                         LOGI(TAG,"clinet closed.");
//                     }else{
//                         LOGI(TAG,"read client data err.");
//                     }
//                     close(client_fd[i].fd);
//                     client_fd[i].fd = -1;
//                     connected_client_cnt --;
//                 }
//             }
//         }
//     }
// }
