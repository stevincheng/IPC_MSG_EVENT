
#include "log.h"

static int shmid;
static LOG_BUF_SHM* log_buf_shm = NULL;
static char *log_shm_path;

void log_add_newline_symbol(char *data,int max_len){
    if (strlen(data) >= max_len){
        if (data[max_len] != '\n')
            data[max_len] = '\n';
    }else{
        if (data[strlen(data)-1] != '\n')
            data[strlen(data)] = '\n';
    }
}
void log_get_level_string(char *level_s,const int level){
    switch (level){
        case LOG_LVL_ERROR:
            memcpy(level_s,LOG_LEVEL_STRING_ERR,sizeof(LOG_LEVEL_STRING_ERR));
            break;
        case LOG_LVL_WARN:
            memcpy(level_s,LOG_LEVEL_STRING_WARN,sizeof(LOG_LEVEL_STRING_WARN));
            break;
        case LOG_LVL_INFO:
            memcpy(level_s,LOG_LEVEL_STRING_INFO,sizeof(LOG_LEVEL_STRING_INFO));
            break;
        case LOG_LVL_DEBUG:
            memcpy(level_s,LOG_LEVEL_STRING_DEBUG,sizeof(LOG_LEVEL_STRING_DEBUG));
            break;
        default:
            memcpy(level_s,LOG_LEVEL_STRING_DEBUG,sizeof(LOG_LEVEL_STRING_DEBUG));
            break;
    }
}
void log_get_cur_time_string(char *time_s){
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    sprintf(time_s,"%d-%d-%d %d:%d:%d",p->tm_year+1900,p->tm_mon+1,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
}
void log_to_shm(char *data,int len){
    Lock_Log_Shm();
    int wr_len = 0;
    if (LOG_SHM_BUF_MAX_LEN - log_buf_shm->log_len > len){
        wr_len = len;
    }else{
        wr_len = LOG_SHM_BUF_MAX_LEN - log_buf_shm->log_len;
        printf("log_shm_buf cur allow wr num too less, hope len = %d ; can wr_len = %d \n",len,wr_len);
    }
    memcpy(log_buf_shm->log_buf + log_buf_shm->log_len,data,wr_len);
    log_buf_shm->log_len += wr_len;
    Unlock_Log_Shm();
}
int log_output(const int level,const char *file_name,const int line_num,const char *tag,const char *format, ...){
    va_list args;
    char log_buf[LOG_BUF_MAX_LEN] = {0};

    char time_string[100] = {0};
    log_get_cur_time_string(time_string);
    sprintf(log_buf+strlen(log_buf),"%s ",time_string);

    sprintf(log_buf+strlen(log_buf),"%s:",file_name);
    sprintf(log_buf+strlen(log_buf),"%d",line_num);
    sprintf(log_buf+strlen(log_buf),"[%s]",tag);

    char log_lvl_string[10] = {0};
    log_get_level_string(log_lvl_string,level);
    sprintf(log_buf+strlen(log_buf),"[%s] ",log_lvl_string);

    va_start(args, format);
    int ret = vsnprintf(log_buf + strlen(log_buf), LOG_BUF_MAX_LEN - strlen(log_buf), format, args);
    va_end(args);

    log_add_newline_symbol(log_buf,LOG_BUF_MAX_LEN);

    printf("%s",log_buf);
    // if (level <= LOG_LVL_INFO)
        log_to_shm(log_buf,strlen(log_buf));
    return 0;
}

int log_rawdata_hex(const int level,const char *file_name,const int line_num,
                    const char *tag,const char *data_prefix,const char *data,const int len){
    char log_buf[LOG_BUF_MAX_LEN] = {0};

    char time_string[100] = {0};
    log_get_cur_time_string(time_string);
    sprintf(log_buf+strlen(log_buf),"%s ",time_string);

    sprintf(log_buf+strlen(log_buf),"%s:",file_name);
    sprintf(log_buf+strlen(log_buf),"%d",line_num);
    sprintf(log_buf+strlen(log_buf),"[%s]",tag);

    char log_lvl_string[10] = {0};
    log_get_level_string(log_lvl_string,level);
    sprintf(log_buf+strlen(log_buf),"[%s] ",log_lvl_string);

    sprintf(log_buf+strlen(log_buf),"%s ",data_prefix);

    int raw_data_max_len = (len + strlen(log_buf) < LOG_BUF_MAX_LEN) ? len : (LOG_BUF_MAX_LEN - strlen(log_buf));
    for (int i = 0; i < raw_data_max_len; i++)
    {
        sprintf(log_buf+strlen(log_buf),"%02x ",data[i]);
    }
    log_add_newline_symbol(log_buf,LOG_BUF_MAX_LEN);

    printf("%s",log_buf);
    if (level <= LOG_LVL_INFO)
        log_to_shm(log_buf,strlen(log_buf));
    return 0;
}
int Lock_Log_Shm(void){   
    int ret = -1;
    if(NULL != log_buf_shm){
        ret = pthread_mutex_lock(&(log_buf_shm->mutexLock));
        if(ret != 0)
        {
            printf("ret=%d,lock err:%s\n",ret,strerror(ret));
            if(ret == EOWNERDEAD)
                pthread_mutex_consistent(&(log_buf_shm->mutexLock));
        }
    }
    return ret;
}

int Unlock_Log_Shm(void){
    int ret = -1;
    if(NULL != log_buf_shm){
        ret = pthread_mutex_unlock(&(log_buf_shm->mutexLock));
        if(ret != 0)
            printf("ret=%d,unlock err:%s\n",ret,strerror(ret));
    }
    return ret;
}

LOG_BUF_SHM* Get_log_shm(void)
{
    return log_buf_shm;
}

int Log_init(){
    int ret = 0;
    log_shm_path = getenv(LOG_SHM_PATH);
    printf("log_shm_path = %s\n",log_shm_path);
    key_t key = ftok(log_shm_path,66);
    if(key < 0){
        printf("get logshm key error:%s\n",strerror(errno));
        ret = -1;
    }else{
        shmid = shmget(key,sizeof(LOG_BUF_SHM),IPC_CREAT | 0666);
        if(shmid < 0)
        {                  
            printf("get logshm error:%s\n",strerror(errno));
            ret = -1;
        }else{
            log_buf_shm = (LOG_BUF_SHM*)shmat(shmid,NULL,0);
            if((void *)-1 == (void *)log_buf_shm)
            {
                log_buf_shm = NULL;
                printf("logshm shmat error:%s\n",strerror(errno));
                ret = -1;
            }
        }    
    }
    return ret;
}

