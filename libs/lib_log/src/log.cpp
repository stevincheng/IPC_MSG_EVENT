
#include "log.h"

int log_output(const int level,const char *file_name,const int line_num,const char *tag,const char *format, ...){
    va_list args;
    char log_buf[LOG_BUF_MAX_LEN] = {0};

    sprintf(log_buf,"%s %s %s:%d [%s] ",__DATE__,__TIME__,file_name,line_num,tag);
    va_start(args, format);
    int ret = vsnprintf(log_buf + strlen(log_buf), LOG_BUF_MAX_LEN - strlen(log_buf), format, args);
    va_end(args);
    if (log_buf[strlen(log_buf)] != '\n')
    {
        log_buf[strlen(log_buf)] = '\n';
    }
    
    printf("%s",log_buf);
    return 0;
}

int log_rawdata_hex(const int level,const char *file_name,const int line_num,const char *tag,const char *data_prefix,const char *data,int len){

    char log_buf[LOG_BUF_MAX_LEN] = {0};

    sprintf(log_buf,"%s %s %s:%d [%s] %s ",__DATE__,__TIME__,file_name,line_num,tag,data_prefix);

    for (int i = 0; i < len; i++)
    {
        sprintf(log_buf+strlen(log_buf),"%02x ",data[i]);
    }

    if (log_buf[strlen(log_buf)] != '\n')
        log_buf[strlen(log_buf)] = '\n';

    printf("%s",log_buf);
    return 0;
}

