/*
 * sem_ll.h
 *
 *  Created on: Jul 18, 2021
 *      Author: zeus
 */

#ifndef LL_SEM_LL_H_
#define LL_SEM_LL_H_
#include <stdint.h>
#include <stdbool.h>
#include <semaphore.h>

typedef sem_t	NorDB_sem_t;

bool NorDB_sem_init(NorDB_sem_t *sem);
void NorDB_sem_Lock(NorDB_sem_t *sem);
void NorDB_sem_Unlock(NorDB_sem_t *sem);

#endif /* LL_SEM_LL_H_ */
