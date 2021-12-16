#include "client_a.h"

// int tcp_fd;
// void *screen_input(void *arg){
//     LOGI(TAG," screen_input . tcp_fd = %d",tcp_fd);
//     while (1)
//     {
//         char input_buf[BUFSIZ] = "client_a";
//         // fgets(input_buf,sizeof(input_buf),stdin);
//         LOGI(TAG,"input_buf = %s",input_buf);
//         write(tcp_fd,input_buf,strlen(input_buf));
//         sleep(1);
//     }
// }
// void *read_tcp_fd(void *arg){
//     LOGI(TAG," read_tcp_fd . tcp_fd = %d",tcp_fd);
//     char buf[BUFSIZ] = {0};
//     while (1)
//     {
//         memset(buf,0,sizeof(buf));
//         int n = read(tcp_fd,buf,sizeof(buf));
//         LOGI(TAG,"buf = %s  n = %d",buf,n);
//         // sleep(1);
//     }
    
// }

void *ipc_msg_test_thread(void *arg){
    while (1)
    {
        char input_buf[BUFSIZ] = "client_a";
        sleep(2);
        ipc_send_msg(IPC_MSG_TYPE_11,input_buf,strlen(input_buf));
    }
}

static const int ipc_event_type_list[] = {
    IPC_MSG_TYPE_2,
    IPC_MSG_TYPE_22,
    IPC_MSG_TYPE_4,
    IPC_MSG_TYPE_44,
};

static int ipc_msg_event_callback(int type, void * usr_data, int usr_data_length){
    char *data = (char *) usr_data;
    LOGI(TAG,"type = %x usr_data_length = %d data = %s",type,usr_data_length,data);
    // for (size_t i = 0; i < usr_data_length; i++)
    // {
    //     printf("%02x ",data[i]);
    // }
    // printf("\n");
    
}
int ipc_evt_type_regist(){
    for (size_t i = 0; i < sizeof(ipc_event_type_list)/sizeof(ipc_event_type_list[0]); i++)
    {
        int res = ipc_regist_event_type(ipc_event_type_list[i]);
        if (-1 == res)
        {
            LOGI(TAG,"ipc_evt_type_regist err : type = %d",ipc_event_type_list[i]);
        }
    }
    return 0;
}
int main(int argc, char ** argv){
    LOGI(TAG,"this is client_a process = %s",CLIENT_A_S);
    int ipc_init_res = ipc_tcp_conn_init(ipc_msg_event_callback);
    if (-1 == ipc_init_res)
    {
        LOGI(TAG,"ipc conn err,exit process.");
        return -1;
    }
    ipc_evt_type_regist();
    pthread_t ip_msg_tes_thread_t;
    pthread_create(&ip_msg_tes_thread_t, NULL, ipc_msg_test_thread, NULL);
    pthread_detach(ip_msg_tes_thread_t);

    ipc_client_rec_loop();

    // while (1)
    // {
    //     sleep(2);
        
    // }
    return 0;
}