#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define LOG_BUF_MAX_LEN     1024

enum LOGL_EVEL
{
    LOG_LVL_ERROR,
    LOG_LVL_WARN ,
    LOG_LVL_INFO ,
    LOG_LVL_DEBUG,
    LOG_LVL_AMOUNT
};

extern int log_output(const int level,const char *file_name,const int line_num,const char *tag,const char *format,...);

extern int log_rawdata_hex(const int level,const char *file_name,const int line_num,const char *tag,const char *data_prefix,const char *data,int len);

#ifndef LOG_RAW_DATA_HEX
#define LOG_RAW_DATA_HEX(LOG_TAG,DATA_PREFIX,DATA,LEN,...) do {\
        (log_rawdata_hex(LOG_LVL_INFO,__FILE__,__LINE__,LOG_TAG,DATA_PREFIX,DATA,LEN)); \
    }while(0)
#endif


#ifndef LOGI
#define LOGI(LOG_TAG,...) do {\
        (log_output(LOG_LVL_INFO,__FILE__,__LINE__,LOG_TAG,##__VA_ARGS__)); \
    }while(0)
#endif

// #define LOGI(LOG_TAG,format, args...)\
//     do {\
//         printf("%s %s %s:%d [%s] ",__DATE__,__TIME__,__FILE__,__LINE__,LOG_TAG);\
//         printf(format ,##args);\
//         printf("\n");\
//     } while(0);


#endif