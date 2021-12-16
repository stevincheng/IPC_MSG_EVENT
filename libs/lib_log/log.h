#include <stdio.h>

#define LOGI(LOG_TAG,format, args...)\
    do {\
        printf("%s %s %s:%d [%s] ",__DATE__,__TIME__,__FILE__,__LINE__,LOG_TAG);\
        printf(format ,##args);\
        printf("\n");\
    } while(0);
