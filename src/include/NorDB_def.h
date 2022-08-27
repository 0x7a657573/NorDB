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

#define NorDB_Magic	        0x0766
#define NorDB_RVer	        0x0001

#define nordb_FreeMark		0xFF
#define nordb_UnReadMark	0xF0
#define nordb_ReadMark		0x00

/*heap and memory management*/
#define nordb_malloc(x)	malloc(x)
#define nordb_free(x)	free(x)
#define nordb_memset	memset
#define nordb_memcpy	memcpy

/*file function*/

#endif /* NORDB_DEF_H_ */
