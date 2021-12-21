#include "client_b.h"

int tcp_fd;
void *ipc_msg_test_thread(void *arg){
    sleep(2);
    while (1)
    {
        // sleep(3);
        usleep(1000*500);
        char input_buf[BUFSIZ] = "client_b_0";
        char input_buf_1[BUFSIZ] = "client_b_1";
        char input_buf_2[BUFSIZ] = "client_b_2";
        char input_buf_3[BUFSIZ] = "client_b_3";
        ipc_send_msg(IPC_MSG_TYPE_2,input_buf,strlen(input_buf));
        ipc_send_msg(IPC_MSG_TYPE_22,input_buf_1,strlen(input_buf_1));
        ipc_send_msg(IPC_MSG_TYPE_4,input_buf_2,strlen(input_buf_2));
        ipc_send_msg(IPC_MSG_TYPE_44,input_buf_3,strlen(input_buf_3));
    }
}
static const int ipc_event_type_list[] = {
    IPC_MSG_TYPE_1,
    IPC_MSG_TYPE_11,
    IPC_MSG_TYPE_3,
    IPC_MSG_TYPE_33,
};
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
static int ipc_msg_event_callback(int type, void * usr_data, int usr_data_length){
    char *data = (char *) usr_data;
    LOGI(TAG,"type = %08x usr_data_length = %d  data = %s",type,usr_data_length,data);
}
int main(int argc, char ** argv){
    int log_res = Log_init();
    if (log_res < 0)
        printf("Log_init err.\n");
    LOGI(TAG,"this is client_a process = %s \n",CLIENT_B_S);

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
    //     ipc_send_msg(IPC_MSG_TYPE_22,(char *)CLIENT_B_S,strlen(CLIENT_B_S));
    // }
    return 0;
}