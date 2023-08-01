#include <TestConfig.h>

#if HOST_TARGET == WINDOWS || HOST_TARGET == LINUX

    double GetTime(void)
    {
        struct timespec CurrentTime;
         clock_gettime(0, &CurrentTime);
         return CurrentTime.tv_sec + CurrentTime.tv_nsec/1000000000.0;
    }
    
#elif HOST_TARGET == FREERTOS

#else

#endif

