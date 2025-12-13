/*
 ============================================================================
 Name        : Ram_multithread_example.c
 Author      : Mohammad Mazarei
 Version     :
 Copyright   : Copyright 2020 Mohammad Mazarei This program is free software
 Description : Multi-thread demo for NorDB on RAM backend
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <NorDB.h>
#include <ll/Ram_ll.h>
#include "TestRoutines.h"

typedef struct
{
	NorDB_t *db;
	int writes;
	unsigned int seed;
	int id;
	struct shared_state *shared;
} writer_ctx_t;

typedef struct shared_state
{
	pthread_mutex_t lock;
	pthread_cond_t cond;
	int writers_total;
	int writers_done;
	uint32_t total_written;
	uint32_t total_read;
} shared_state_t;

static void *writer_thread(void *arg)
{
	writer_ctx_t *ctx = (writer_ctx_t *)arg;
	if (!ctx || !ctx->db || !ctx->shared)
		return NULL;

	uint8_t *tmp = (uint8_t *)malloc((size_t)GetDummyRecordSize());
	if (!tmp)
		return NULL;

	for (int i = 0; i < ctx->writes; i++)
	{
		/* build a deterministic, per-thread record so we can validate later */
		for (int j = 0; j < GetDummyRecordSize(); j++)
			tmp[j] = (uint8_t)(rand_r(&ctx->seed) & 0xFF);

		uint32_t w = NorDB_AddRecord(ctx->db, tmp);
		if (w == 0)
		{
			printf("Writer[%d]: write failed at i=%d (db full or write error)\n", ctx->id, i);
			break;
		}

		pthread_mutex_lock(&ctx->shared->lock);
		ctx->shared->total_written++;
		pthread_cond_signal(&ctx->shared->cond);
		pthread_mutex_unlock(&ctx->shared->lock);

		if ((i % 64) == 0)
			printf("Writer[%d]: wrote %d\n", ctx->id, i + 1);
	}

	pthread_mutex_lock(&ctx->shared->lock);
	ctx->shared->writers_done++;
	pthread_cond_broadcast(&ctx->shared->cond);
	pthread_mutex_unlock(&ctx->shared->lock);

	free(tmp);
	return NULL;
}

typedef struct
{
	NorDB_t *db;
	shared_state_t *shared;
} reader_ctx_t;

static void *reader_thread(void *arg)
{
	reader_ctx_t *ctx = (reader_ctx_t *)arg;
	if (!ctx || !ctx->db || !ctx->shared)
		return NULL;

	uint8_t *out = (uint8_t *)malloc((size_t)GetDummyRecordSize());
	if (!out)
		return NULL;

	while (1)
	{
		pthread_mutex_lock(&ctx->shared->lock);
		while ((ctx->shared->total_read >= ctx->shared->total_written) &&
			   (ctx->shared->writers_done < ctx->shared->writers_total))
		{
			pthread_cond_wait(&ctx->shared->cond, &ctx->shared->lock);
		}
		bool all_done = (ctx->shared->writers_done >= ctx->shared->writers_total);
		uint32_t should_read_more = (ctx->shared->total_read < ctx->shared->total_written) ? 1u : 0u;
		pthread_mutex_unlock(&ctx->shared->lock);

		if (!should_read_more)
		{
			if (all_done)
				break;
			continue;
		}

		uint32_t r = NorDB_ReadRecord(ctx->db, out);
		if (r != 0)
		{
			pthread_mutex_lock(&ctx->shared->lock);
			ctx->shared->total_read++;
			uint32_t rc = ctx->shared->total_read;
			pthread_mutex_unlock(&ctx->shared->lock);

			if ((rc % 64u) == 0u)
				printf("Reader: read %u\n", rc);
		}
		else
		{
			/* rare race: unread flipped between counters and actual DB state */
			pthread_mutex_lock(&ctx->shared->lock);
			pthread_cond_wait(&ctx->shared->cond, &ctx->shared->lock);
			pthread_mutex_unlock(&ctx->shared->lock);
		}
	}

	free(out);
	return NULL;
}

int main(int argc, char **argv)
{
	srand((unsigned)time(NULL));

	uint32_t sector_size = 4096;
	uint32_t sector_count = 8;
	int thread_count = 2;
	int writes_per_thread = 200;

	if (argc >= 2)
		sector_size = (uint32_t)strtoul(argv[1], NULL, 0);
	if (argc >= 3)
		sector_count = (uint32_t)strtoul(argv[2], NULL, 0);
	if (argc >= 4)
		thread_count = (int)strtol(argv[3], NULL, 0);
	if (argc >= 5)
		writes_per_thread = (int)strtol(argv[4], NULL, 0);

	if (thread_count < 1)
		thread_count = 1;
	if (thread_count > 8)
		thread_count = 8;
	if (writes_per_thread < 1)
		writes_per_thread = 1;

	NorDB_HWLayer *Ram_Hw = Ramll_Init((uint16_t)sector_size, (uint16_t)sector_count);
	if (!Ram_Hw)
	{
		printf("Error: Ramll_Init failed\n");
		return EXIT_FAILURE;
	}

	NorDB_t *DB = NorDB(Ram_Hw, (uint16_t)GetDummyRecordSize());
	if (!DB)
	{
		printf("Error: NorDB init failed\n");
		return EXIT_FAILURE;
	}

	printf("NorDB RAM Multi-thread Demo\n");
	printf("\tSectors: %u\n", DB->DB_ll->SectorNumber);
	printf("\tSectorSize: %u\n", DB->DB_ll->SectorSize);
	printf("\tRecordPerSector: %u\n", DB->Record_NumberInSector);
	printf("\tThreads: %d\n", thread_count);
	printf("\tWrites/Thread: %d\n", writes_per_thread);

	pthread_t threads[8];
	writer_ctx_t ctx[8];
	pthread_t rthread;
	reader_ctx_t rctx;
	shared_state_t shared;

	pthread_mutex_init(&shared.lock, NULL);
	pthread_cond_init(&shared.cond, NULL);
	shared.writers_total = thread_count;
	shared.writers_done = 0;
	shared.total_written = 0;
	shared.total_read = 0;

	rctx.db = DB;
	rctx.shared = &shared;
	if (pthread_create(&rthread, NULL, reader_thread, &rctx) != 0)
	{
		printf("Error: pthread_create failed (reader)\n");
		return EXIT_FAILURE;
	}

	for (int i = 0; i < thread_count; i++)
	{
		ctx[i].db = DB;
		ctx[i].writes = writes_per_thread;
		ctx[i].seed = (unsigned int)time(NULL) ^ (unsigned int)(i * 2654435761u);
		ctx[i].id = i;
		ctx[i].shared = &shared;
		if (pthread_create(&threads[i], NULL, writer_thread, &ctx[i]) != 0)
		{
			printf("Error: pthread_create failed (i=%d)\n", i);
			return EXIT_FAILURE;
		}
	}

	for (int i = 0; i < thread_count; i++)	
		pthread_join(threads[i], NULL);

	pthread_join(rthread, NULL);

	pthread_mutex_destroy(&shared.lock);
	pthread_cond_destroy(&shared.cond);

	printf("\nThreads done. total_written=%u total_read=%u unread_now=%u\n",
			shared.total_written,
			shared.total_read,
			NorDB_Get_TotalUnreadRecord(DB));
	printf("Done\n");
	return EXIT_SUCCESS;
}
