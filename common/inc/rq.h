#ifndef __RQ_H__
#define __RQ_H__

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#include "cmsis_os.h"

typedef struct msg_def  {
	unsigned short sz;
	unsigned short count;
	unsigned short push_index;
	unsigned short pop_index;
	bool rq_full;
	unsigned char *rq_buf;
} rq_def_t;

#if defined (osObjectsExternal)  // object is external
#define RQ_DEF(name, sz, count, buf)   \
extern rq_def_t rq_##name;
#else                            // define the object
#define RQ_DEF(name, sz, count, buf)   \
rq_def_t rq_##name = \
{\
	sz,count,0,0,false, buf,\
};
#endif

#define RQ(name) \
&rq_##name

//int rq_init(int size);
int rq_push(rq_def_t *rq, void *pdata, int size);
int rq_pop(rq_def_t *rq, void *p_data, int size);
int rq_clear(rq_def_t *rq);

typedef struct {
	unsigned char *rq_buf;
	unsigned short sz;
}seg_t;

typedef struct {
	unsigned int sz;
	unsigned int count;
	int push_index;
	int pop_index;
	bool rq_full;
	seg_t seg[2];
} rq_mmu_def_t;

#if defined (osObjectsExternal)  // object is external
#define RQ_MMU_DEF(name, sz, count, seg0_buf, seg0_sz, seg1_buf, seg1_sz)   \
extern rq_mmu_def_t rq_mmu_##name;
#else                          // define the object
#define RQ_MMU_DEF(name, sz, count, seg0_buf, seg0_sz, seg1_buf, seg1_sz)   \
rq_mmu_def_t rq_mmu_##name = \
{\
	sz,count,0,0,false, {{seg0_buf,seg0_sz},{seg1_buf,seg1_sz}} \
};
#endif

#define RQ_MMU(name) \
&rq_mmu_##name

int rq_mmu_push(rq_mmu_def_t *rq, void *pdata, int size);
int rq_mmu_pop(rq_mmu_def_t *rq, void *p_data, int size);

#endif
