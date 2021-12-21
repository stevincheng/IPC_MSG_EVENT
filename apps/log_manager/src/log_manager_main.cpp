#include "log_manager.h"

static char log_cache_buf[LOG_CACHE_BUF_MAX_LEN];
static int log_cache_len = 0;
static LOG_SAVE_FILE_INFO log_save_file_info;
int get_log_file_sn(char *file_name){
    int h = file_name[strlen(LOG_FILE_NAME_PREFIX)] - '0';
    int l = file_name[strlen(LOG_FILE_NAME_PREFIX)+1] - '0';
    return h * 10 + l;
}
int get_log_save_file_path(){
    if (strlen(log_save_file_info.log_dir) == 0){
        LOGI(TAG,"log_save_file_info.log_dir == NULL");
        return -1;
    }
    if (strlen(log_save_file_info.log_file_path) > 0){
        struct stat file_info;
        if (stat(log_save_file_info.log_file_path,&file_info) == -1){
            LOGE(TAG,"stat file info err,latest_file_path = %s",log_save_file_info.log_file_path);
        }else{
            if (file_info.st_size < LOG_ONE_FILE_MAX_SIZE){
                if (NULL == log_save_file_info.log_file_fp)
                    log_save_file_info.log_file_fp = fopen(log_save_file_info.log_file_path, "a+");
                return 0;
            }
        }
    }
    struct dirent *entry;
    if (log_save_file_info.log_dir_fd == NULL){
        log_save_file_info.log_dir_fd = opendir(log_save_file_info.log_dir);
    }
    int log_files_max_num = 0;
    if (log_save_file_info.log_dir_fd == NULL){
        printf("opendir err\n");
        return -1;
    }
    while (entry = readdir(log_save_file_info.log_dir_fd)){
        LOGI(TAG,"entry->d_name = %s",entry->d_name);
        if (strstr(entry->d_name,LOG_FILE_NAME_SUFFIX) != NULL){
            int log_file_sn = get_log_file_sn(entry->d_name);
            if (log_file_sn > log_files_max_num)
                log_files_max_num = log_file_sn;
        }
    }
    if (log_save_file_info.log_dir_fd){
        closedir(log_save_file_info.log_dir_fd);
        log_save_file_info.log_dir_fd = NULL;
    }
    if (strlen(log_save_file_info.log_file_path) == 0){
        if (log_files_max_num == 0)
            log_files_max_num = 1;
        sprintf(log_save_file_info.log_file_path,"%s/%s%02d%s",log_save_file_info.log_dir,LOG_FILE_NAME_PREFIX,log_files_max_num,LOG_FILE_NAME_SUFFIX);
        if (NULL == log_save_file_info.log_file_fp)
            log_save_file_info.log_file_fp = fopen(log_save_file_info.log_file_path, "a+");
        return 0;
    }
    log_files_max_num ++;
    if (log_files_max_num > LOG_FILE_MAX_NUM){
        char rm_path[256] = {0};
        sprintf(rm_path,"%s/%s%02d%s",log_save_file_info.log_dir,LOG_FILE_NAME_PREFIX,LOG_FILE_MIN_NUM,LOG_FILE_NAME_SUFFIX);
        if(remove(rm_path) == -1)
           LOGE(TAG,"remove file err,rm_path = %s",rm_path);
        for (int i = 2; i <= LOG_FILE_MAX_NUM; i++){
            char old_path[256] = {0};
            char new_path[256] = {0};
            sprintf(old_path,"%s/%s%02d%s",log_save_file_info.log_dir,LOG_FILE_NAME_PREFIX,i,LOG_FILE_NAME_SUFFIX);
            sprintf(new_path,"%s/%s%02d%s",log_save_file_info.log_dir,LOG_FILE_NAME_PREFIX,i-1,LOG_FILE_NAME_SUFFIX);
            if (access(old_path,F_OK) == -1){
                LOGI(TAG,"mv file no exisits : old_path = %s",old_path);
                continue;
            }
            if (rename(old_path,new_path) == -1)
                LOGE(TAG,"rename file err,old_path = %s new_path = %s",old_path,new_path);
        }
        log_files_max_num = LOG_FILE_MAX_NUM;
    }
    if (NULL != log_save_file_info.log_file_fp){
        fclose(log_save_file_info.log_file_fp);
        system("sync");
        log_save_file_info.log_file_fp = NULL;
    }
    memset(log_save_file_info.log_file_path,0,sizeof(log_save_file_info.log_file_path));
    sprintf(log_save_file_info.log_file_path,"%s/%s%02d%s",log_save_file_info.log_dir,LOG_FILE_NAME_PREFIX,log_files_max_num,LOG_FILE_NAME_SUFFIX);
    log_save_file_info.log_file_fp = fopen(log_save_file_info.log_file_path, "a+");
    return 0;
}
void *save_log_file_thread(void *arg){
    int cnt = 0;
    memset(log_cache_buf,0,LOG_CACHE_BUF_MAX_LEN);
    while (1){
        LOG_BUF_SHM* log_buf_shm = NULL;
        Lock_Log_Shm();
        log_buf_shm = Get_log_shm();
        if (NULL != log_buf_shm){
            memcpy(log_cache_buf+log_cache_len,log_buf_shm->log_buf,log_buf_shm->log_len);
            log_cache_len += log_buf_shm->log_len;
            memset(log_buf_shm->log_buf,0,sizeof(log_buf_shm->log_buf));
            log_buf_shm->log_len = 0;
        }
        Unlock_Log_Shm();
        cnt++;
        if ( (LOG_CACHE_BUF_MAX_LEN - log_cache_len < LOG_SHM_BUF_MAX_LEN)  //log_cache_buf 即将写满，需要写入文件保存
                || cnt >= SAVE_LOG_FILE_TIMEOUT ){   //超过5s没有写文件进行保存了，需要及时写入
            LOGI(TAG,"log_cache_buf no space or time out , fres space : %d cnt = %d",(LOG_CACHE_BUF_MAX_LEN - log_cache_len),cnt);
            get_log_save_file_path();
            if (log_save_file_info.log_file_fp)
                fwrite(log_cache_buf, 1, log_cache_len, log_save_file_info.log_file_fp);
            memset(log_cache_buf,0,LOG_CACHE_BUF_MAX_LEN);
            log_cache_len = 0;
            cnt = 0;
        }
        usleep(SAVE_LOG_FREQ_N_SEC);    //50ms循环一次，将共享内存中的log写入log_manager的log cache中
    }
}

int main(int argc, char ** argv){
    int log_res = Log_init();
    if (log_res < 0)
        printf("Log_init err.\n");
    char *path = NULL;
    path = getenv(IPC_MSG_LOG_DIR);
    memcpy(log_save_file_info.log_dir,path,strlen(path));
    LOGI(TAG,"this is log_manager process . log_save_file_info.log_dir = %s ",log_save_file_info.log_dir);
    pthread_t save_log_file_thread_t;
    pthread_create(&save_log_file_thread_t, NULL, save_log_file_thread, NULL);
    pthread_detach(save_log_file_thread_t);

    while (1)
    {
        sleep(10);
    }
    return 0;
}

