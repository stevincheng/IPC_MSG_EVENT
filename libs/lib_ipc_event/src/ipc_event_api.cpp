#include "ipc_event_config.h"

static int ipc_msg_fd = -1;
static IPC_EVENT_API_CALLBACK ipc_msg_callback;

static pthread_mutex_t ipc_msg_rcv_mutex;
static pthread_cond_t ipc_msg_rcv_condition;
static char ipc_rec_buf[BUFSIZ];

static void * ipc_msg_rcv_thread(void *arg);

int ipc_tcp_conn_init(IPC_EVENT_API_CALLBACK callback){
    LOGI(IPC_API_TAG,"ipc_tcp_conn_init");
    memset(ipc_rec_buf,0,sizeof(ipc_rec_buf));
    ipc_msg_callback = callback;

    pthread_cond_init(&ipc_msg_rcv_condition, NULL);
    pthread_mutex_init(&ipc_msg_rcv_mutex, NULL); 
    pthread_t ip_msg_rec_thread_t;
    pthread_create(&ip_msg_rec_thread_t, NULL, ipc_msg_rcv_thread, NULL);
    pthread_detach(ip_msg_rec_thread_t);

    ipc_msg_fd = socket(AF_INET,SOCK_STREAM,0);
    if (-1 == ipc_msg_fd)
    {
        LOGI(IPC_API_TAG,"init socket fd err.")
        return -1;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET,SERVER_IP,&server_addr.sin_addr.s_addr);
    socklen_t sock_len = sizeof(server_addr);
    int ipc_conn_cnt = 0;
    while (1)
    {
        int con_res = connect(ipc_msg_fd,(struct sockaddr *)&server_addr,sock_len);
        if (-1 == con_res)
        {
            ipc_conn_cnt++;
            LOGI(IPC_API_TAG,"IPC conn err,wait ipc server init... ipc_conn_cnt = %d",ipc_conn_cnt);
            if (ipc_conn_cnt >= IPC_CONN_MAX_CNT)
            {
                LOGI(IPC_API_TAG,"IPC conn err too many times....");
                close(ipc_msg_fd);
                return -1;
            }
            sleep(1);
        }else{
            LOGI(IPC_API_TAG,"IPC conn sucess.");
            break;
        }
    }
    return 0;
}
void init_ipc_msg_head(char *data,int msg_len,int cmd_type,int ipc_event_type){

    uint32_t len = htonl((uint32_t)msg_len);
    memcpy(data,(void*)&len,sizeof(int));   //整个ipc_msg 长度

    uint32_t cmd = htonl((uint32_t)cmd_type);
    memcpy(data+4,(void*)&cmd,sizeof(int)); //ipc_msg类型  IPC_TYPE_REGIST_EVENT_TYPE：注册消息类型 IPC_TYPE_SEND_MSG 发送ipc消息

    uint32_t event_type = htonl((uint32_t)ipc_event_type);
    memcpy(data+8,(void*)&event_type,sizeof(int));  //进程间消息type

}
int ipc_regist_event_type(int type){
    LOGI(IPC_API_TAG,"regist_ipc_event_type type = %x",type);
    char ipc_msg[IPC_REGIST_TYPE_MSG_LEN] = {0};
    init_ipc_msg_head(ipc_msg,IPC_REGIST_TYPE_MSG_LEN,IPC_TYPE_REGIST_EVENT_TYPE,type);
    int send_res = send(ipc_msg_fd,ipc_msg,sizeof(ipc_msg),MSG_NOSIGNAL);
    if (send_res == -1)
    {
        LOGI(IPC_API_TAG,"regist_ipc_event_type send ipc msg err.");
    }
    return 0;
}
int ipc_send_msg(int type,char *data,int data_len){
    char ipc_msg[BUFSIZ] = {0};
    init_ipc_msg_head(ipc_msg,data_len + IPC_REGIST_TYPE_MSG_LEN,IPC_TYPE_SEND_MSG,type);
    // printf("================\n");
    // for (size_t i = 0; i < data_len+IPC_REGIST_TYPE_MSG_LEN; i++)
    // {
    //     printf("%02x ",ipc_msg[i]);
    // }
    // printf("\n");
    // printf("*******************\n");

    memcpy(ipc_msg+IPC_REGIST_TYPE_MSG_LEN,data,data_len);
    // LOGI(IPC_API_TAG,"ipc_send_msg data_len = %d",data_len);
    // for (size_t i = 0; i < (data_len+IPC_REGIST_TYPE_MSG_LEN); i++)
    // {
    //     printf("%02x ",ipc_msg[i]);
    // }
    // printf("\n");
    
    int send_res = send(ipc_msg_fd,ipc_msg,data_len+IPC_REGIST_TYPE_MSG_LEN,MSG_NOSIGNAL);
    if (send_res == -1)
    {
        LOGI(IPC_API_TAG,"send_ipc_msg send ipc msg err.");
        return -1;
    }
    return 0;
}
int parse_ipc_cmd_int(char *data){
    int value = 0;
    memcpy(&value,data,sizeof(int));
    value = ntohl(value);
    return value;
}
int is_ipc_rec_buf_empty(){
    return !(ipc_rec_buf[0]&ipc_rec_buf[1]&ipc_rec_buf[2]&ipc_rec_buf[3]);
}
static void * ipc_msg_rcv_thread(void *arg){
    while (1)
    {
        pthread_mutex_lock(&ipc_msg_rcv_mutex);
        if (is_ipc_rec_buf_empty())
        {
            pthread_cond_wait(&ipc_msg_rcv_condition, &ipc_msg_rcv_mutex);
        }
        int type = parse_ipc_cmd_int(ipc_rec_buf+8);
        int data_len = parse_ipc_cmd_int(ipc_rec_buf) - IPC_REGIST_TYPE_MSG_LEN;
        ipc_msg_callback(type,ipc_rec_buf+IPC_REGIST_TYPE_MSG_LEN,data_len);
        pthread_mutex_unlock(&ipc_msg_rcv_mutex);
    }
}
    
int ipc_client_rec_loop(){
    if (ipc_msg_fd < 0)
        return -1;
    
    fd_set client_fd_set;
    FD_ZERO(&client_fd_set);	
    FD_SET(ipc_msg_fd , &client_fd_set);
    while (1)
    {
        int ret = select(ipc_msg_fd + 1, &client_fd_set, NULL, NULL, NULL);
        if (ret <= 0){
            LOGI(IPC_API_TAG,"client_ipc_rec_loop select err.");
        }else{
            pthread_mutex_lock(&ipc_msg_rcv_mutex);
            int n = read(ipc_msg_fd,ipc_rec_buf,sizeof(ipc_rec_buf));
            if (n > 0){
                pthread_cond_signal(&ipc_msg_rcv_condition);
            }
            pthread_mutex_unlock(&ipc_msg_rcv_mutex);
        }
        
    }
    
}