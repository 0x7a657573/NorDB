/*
 * ReadAndWrite.h
 *
 *  Created on: Jul 18, 2021
 *      Author: zeus
 */
#ifndef READ_WRITE_H_
#define READ_WRITE_H_

#include <NorDB.h>
#include <ll/SerialNorFlash_ll.h>
#include "TestConfig.h"

int RunTest(bool(*test)(NorDB_t*, char*, int), NorDB_t* DB, int count);

int GetDummyRecordSize(void);
bool FullFill_Test(NorDB_t *DB, char *name, int count);
bool RoundRobin_Test(NorDB_t *DB, char *name, int count);
bool OverWrite_Test(NorDB_t *DB, char *name, int count);
bool WriteRead_Time_Test(NorDB_t *DB, char *name, int count);
bool DeleteDB_Test(NorDB_t *DB, char *name, int count);
bool Erase_Test(NorDB_t *DB, char *name, int count);
#endif /* READ_WRITE_H_ */
