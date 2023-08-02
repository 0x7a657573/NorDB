/*
 * TestConfig.h
 *
 *  Created on: Jul 18, 2021
 *      Author: zeus
 */
#ifndef TEST_CONFIG_H_
#define TEST_CONFIG_H_

#define WINDOWS     0
#define LINUX       1
#define FREERTOS    2

#define HOST_TARGET LINUX

#if HOST_TARGET == WINDOWS || HOST_TARGET == LINUX
    #include <time.h>
    
#elif HOST_TARGET == FREERTOS

#else

#endif

double GetTime(void);

#endif /* TEST_CONFIG_H_ */
