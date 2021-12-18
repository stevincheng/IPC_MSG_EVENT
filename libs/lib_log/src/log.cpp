
#include "log.h"

static int shmid;
static LOG_BUF_SHM* log_buf_shm = NULL;

void log_add_newline_symbol(char *data,int max_len){
    if (strlen(data) >= max_len)
    {
        if (data[max_len] != '\n')
            data[max_len] = '\n';
    }else{
        if (data[strlen(data)-1] != '\n')
            data[strlen(data)] = '\n';
    }
}
void log_to_shm(char *data,int len){
    // if (1)
    // {
    //     return;
    // }
    
    Lock_Log_Shm();
    int wr_len = 0;
    if (LOG_SHM_BUF_MAX_LEN - strlen(log_buf_shm->log_buf) > len){
        wr_len = len;
    }else{
        wr_len = LOG_SHM_BUF_MAX_LEN - strlen(log_buf_shm->log_buf);
        printf("log_shm_buf cur allow wr num too less, hope len = %d ; can wr_len = %d \n",len,wr_len);
    }
    memcpy(log_buf_shm->log_buf + strlen(log_buf_shm->log_buf),data,wr_len);
    Unlock_Log_Shm();
}
int log_output(const int level,const char *file_name,const int line_num,const char *tag,const char *format, ...){
    va_list args;
    char log_buf[LOG_BUF_MAX_LEN] = {0};

    // sprintf(log_buf,"%s %s %s:%d [%s] ",__DATE__,__TIME__,file_name,line_num,tag);
    sprintf(log_buf+strlen(log_buf),"%s ",__DATE__);
    sprintf(log_buf+strlen(log_buf),"%s ",__TIME__);
    sprintf(log_buf+strlen(log_buf),"%s:",file_name);
    sprintf(log_buf+strlen(log_buf),"%d ",line_num);
    sprintf(log_buf+strlen(log_buf),"[%s] ",tag);

    va_start(args, format);
    int ret = vsnprintf(log_buf + strlen(log_buf), LOG_BUF_MAX_LEN - strlen(log_buf), format, args);
    va_end(args);

    log_add_newline_symbol(log_buf,LOG_BUF_MAX_LEN);

    printf("%s",log_buf);
    log_to_shm(log_buf,strlen(log_buf));
    return 0;
}

int log_rawdata_hex(const int level,const char *file_name,const int line_num,
                    const char *tag,const char *data_prefix,const char *data,const int len){
    char log_buf[LOG_BUF_MAX_LEN] = {0};

    // sprintf(log_buf,"%s %s %s:%d [%s] %s ",__DATE__,__TIME__,file_name,line_num,tag,data_prefix);
    sprintf(log_buf+strlen(log_buf),"%s ",__DATE__);
    sprintf(log_buf+strlen(log_buf),"%s ",__TIME__);
    sprintf(log_buf+strlen(log_buf),"%s:",file_name);
    sprintf(log_buf+strlen(log_buf),"%d ",line_num);
    sprintf(log_buf+strlen(log_buf),"[%s] ",tag);
    sprintf(log_buf+strlen(log_buf),"%s ",data_prefix);

    int raw_data_max_len = (len + strlen(log_buf) < LOG_BUF_MAX_LEN) ? len : (LOG_BUF_MAX_LEN - strlen(log_buf));
    for (int i = 0; i < raw_data_max_len; i++)
    {
        sprintf(log_buf+strlen(log_buf),"%02x ",data[i]);
    }
    log_add_newline_symbol(log_buf,LOG_BUF_MAX_LEN);

    printf("%s",log_buf);
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
    key_t key = ftok("/home/stevin/user/IPC_MSG_EVENT/log_shm",66);
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

