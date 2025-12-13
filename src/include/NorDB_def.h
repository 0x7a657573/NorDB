/*
 * NorDB_def.h
 *
 *  Created on: Jul 18, 2021
 *      Author: zeus
 */

#ifndef NORDB_DEF_H_
#define NORDB_DEF_H_
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define NorDB_Magic	        0x0766
#define NorDB_RVer	        0x0002

#define nordb_FreeMark		0b11
#define nordb_UnReadMark	0b10
#define nordb_ReadMark		0b00

/*Log Founction*/
#define NORDB_LOG_LEVEL_ERROR 1
#define NORDB_LOG_LEVEL_WARN  2
#define NORDB_LOG_LEVEL_INFO  3
#define NORDB_LOG_LEVEL_DEBUG 4

#if defined(NORDB_ENABLE_LOG)
#ifndef NORDB_LOG_PRINTF
#define NORDB_LOG_PRINTF printf
#endif
#ifndef NORDB_LOG_LEVEL
#define NORDB_LOG_LEVEL NORDB_LOG_LEVEL_INFO
#endif

#if (NORDB_LOG_LEVEL >= NORDB_LOG_LEVEL_ERROR)
#define err_log(...)  do { NORDB_LOG_PRINTF(__VA_ARGS__); } while (0)
#else
#define err_log(...)  do { } while (0)
#endif

#if (NORDB_LOG_LEVEL >= NORDB_LOG_LEVEL_WARN)
#define warn_log(...) do { NORDB_LOG_PRINTF(__VA_ARGS__); } while (0)
#else
#define warn_log(...) do { } while (0)
#endif

#if (NORDB_LOG_LEVEL >= NORDB_LOG_LEVEL_INFO)
#define info_log(...) do { NORDB_LOG_PRINTF(__VA_ARGS__); } while (0)
#else
#define info_log(...) do { } while (0)
#endif

#if (NORDB_LOG_LEVEL >= NORDB_LOG_LEVEL_DEBUG)
#define debug_log(...) do { NORDB_LOG_PRINTF(__VA_ARGS__); } while (0)
#else
#define debug_log(...) do { } while (0)
#endif
#else
#define err_log(...)   do { } while (0)
#define warn_log(...)  do { } while (0)
#define info_log(...)  do { } while (0)
#define debug_log(...) do { } while (0)
#endif

/*heap and memory management*/
#define nordb_malloc(x)	malloc(x)
#define nordb_free(x)	free(x)
#define nordb_memset	memset
#define nordb_memcpy	memcpy

/*file function*/
#define nordb_FILE  FILE
#define nordb_fopen(path,mode)          fopen(path,mode)
#define nordb_fclose(fd)                fclose(fd)
#define nordb_fseek(fd,offset,whence)   fseek(fd,offset,whence)
#define nordb_ftell(fd)                 ftell(fd)
#define nordb_fflush(fd)                fflush(fd)
#define nordb_fwrite(fd,ptr,size)       fwrite(ptr,size,1,fd)
#define nordb_fread(fd,ptr,size)        fread(ptr,size,1,fd)

#endif /* NORDB_DEF_H_ */
