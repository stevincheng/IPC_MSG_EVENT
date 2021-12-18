#include "log_manager.h"

static char log_cache_buf[LOG_CACHE_BUF_MAX_LEN];
static char *log_dir_path;
static char log_file_path[256];
void *save_log_file_thread(void *arg){
    int cnt = 0;
    memset(log_cache_buf,0,LOG_CACHE_BUF_MAX_LEN);
    while (1){
        LOG_BUF_SHM* log_buf_shm = NULL;
        Lock_Log_Shm();
        log_buf_shm = Get_log_shm();
        if (NULL != log_buf_shm){
            memcpy(log_cache_buf+strlen(log_cache_buf),log_buf_shm->log_buf,strlen(log_buf_shm->log_buf));
            memset(log_buf_shm->log_buf,0,sizeof(log_buf_shm->log_buf));
        }
        Unlock_Log_Shm();
        cnt++;
        if ( (LOG_CACHE_BUF_MAX_LEN - strlen(log_cache_buf) < LOG_SHM_BUF_MAX_LEN)  //log_cache_buf 即将写满，需要写入文件保存
                || cnt >= 20 * 10 ){   //超过10s没有写文件进行保存了，需要及时写入
            LOGI(TAG,"log_cache_buf no space or time out , fres space : %d cnt = %d",(LOG_CACHE_BUF_MAX_LEN - strlen(log_cache_buf)),cnt);
            save_log_file(log_cache_buf,strlen(log_cache_buf),log_file_path);
            system("sync");
            memset(log_cache_buf,0,LOG_CACHE_BUF_MAX_LEN);
            cnt = 0;
        }
        usleep(50*1000);    //50ms循环一次，将共享内存中的log写入log_manager的log cache中
    }
}
int main(int argc, char ** argv){
    int log_res = Log_init();
    if (log_res < 0)
        printf("Log_init err.\n");
    
    log_dir_path = getenv(IPC_MSG_LOG_DIR);
    memset(log_file_path,0,sizeof(log_file_path));
    LOGI(TAG,"this is log_manager process . log_dir_path = %s ",log_dir_path);
    memcpy(log_file_path,log_dir_path,strlen(log_dir_path));
    memcpy(log_file_path+strlen(log_file_path),LOG_FILE,sizeof(LOG_FILE));
    LOGI(TAG,"log_file_path = %s",log_file_path);

    pthread_t save_log_file_thread_t;
    pthread_create(&save_log_file_thread_t, NULL, save_log_file_thread, NULL);
    pthread_detach(save_log_file_thread_t);

    while (1)
    {
        sleep(10);
    }
    return 0;
}
int save_log_file(char *data,int data_len,char *file_name){

    FILE *fp;
    size_t nLen = 0;
    int nRet;
    if (NULL == file_name){
        LOGI(TAG,"write file err file name is null.");
        return -1;
    }
    fp = fopen(file_name, "a+");
    if (0 == fp){
        LOGI(TAG,"open file err, file name : %s",file_name);
        return -1;
    }
    nLen = fwrite(data, 1, data_len, fp);
    fclose(fp);
    return (int)nLen;
}
