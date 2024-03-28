#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
//#include "cmsis_os.h"
#include "rq.h"

//static int16_t push_index=0;
//static int16_t pop_index=0;
//static uint8_t *rq_buf=NULL;
//static int rq_size;
//static bool rq_full=false;
//osMutexDef(rq_mutex);
//static osMutexId rq_mutex_id=NULL;
	
//int rq_init(int size)
//{
//	rq_buf = malloc(size);
//	if (!rq_buf)
//		return -1;

//	rq_size = size;
//	
//	rq_mutex_id = osMutexCreate(osMutex(rq_mutex));
//	if (!rq_mutex_id)
//		return -1;

//	return 0;
//}

int rq_push(rq_def_t *rq, void *pdata, int size)
{
	if (rq->rq_full)
	{
putchar('x');
		return -1;
	}
	
	memset(rq->rq_buf+rq->push_index, 0, rq->sz);
	memcpy(rq->rq_buf+rq->push_index, pdata, size);
	rq->push_index = (rq->push_index+rq->sz)%(rq->count*rq->sz);

	if (rq->push_index == rq->pop_index)
		rq->rq_full = true;

	return 0;
}
int rq_pop(rq_def_t *rq, void *p_data, int size)
{
	if (rq->pop_index==rq->push_index && !rq->rq_full)
	{
		return -1;
	}

	memcpy(p_data, rq->rq_buf+rq->pop_index, size);
	rq->pop_index = (rq->pop_index+rq->sz)%(rq->count*rq->sz);
	
	rq->rq_full = false;

	return 0;
}
int rq_clear(rq_def_t *rq)
{
	rq->push_index=0;
	rq->pop_index=0;
	rq->rq_full=false;
	return 0;
}
