/*
 * sem_ll.c
 *
 *  Created on: Jul 18, 2021
 *      Author: zeus
 */
#include "sem_ll.h"



bool NorDB_sem_init(NorDB_sem_t *sem)
{
	sem_init(sem, 0, 1);
	return true;
}

void NorDB_sem_Lock(NorDB_sem_t *sem)
{
	sem_wait(sem);
}

void NorDB_sem_Unlock(NorDB_sem_t *sem)
{
	sem_post(sem);
}
