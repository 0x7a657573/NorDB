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

#define nordb_FreeMark		0xFF
#define nordb_UnReadMark	0xF0
#define nordb_ReadMark		0x00

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
