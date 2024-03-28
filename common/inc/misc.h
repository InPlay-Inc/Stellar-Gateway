#ifndef __MISC_H__
#define __MISC_H__

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifndef __AT32F413_H 
#include "cmsis_os.h"
#endif

#define ARRAY_LEN(array)   (sizeof((array))/sizeof((array)[0]))

#define APP_MALLOC(p, type, size)\
	p = (type*)malloc(size);\
	if (p==NULL){\
		printf("size=%d\n", size);\
		assert(p);\
	}\
	memset(p, 0, size);\

#define APP_MFREE(p)\
	if (p) free(p);\
	p = NULL;\

typedef struct {

	unsigned long msg_id;
	unsigned short msg_len;
	unsigned char msg_data[];

} msg_t;

#define MSG_AllOC(p, id, size)\
	p = NULL;\
	APP_MALLOC(p, msg_t, sizeof(msg_t)+size);\
	p->msg_id = id;\
	p->msg_len = size;\

#ifndef __AT32F413_H 
static __inline osStatus msg_put(osMessageQId msg_q, msg_t* p_msg, uint32_t tmo)
{
	osStatus status = osMessagePut(msg_q, (uint32_t)p_msg, tmo);
	if (status)
		printf("osMessagePut failed! (line:%d, res:%d)\n", __LINE__, status);

	return status;
}

static __inline msg_t* msg_get(osMessageQId msg_q)
{
	while(1) {
		osEvent evt = osMessageGet(msg_q, osWaitForever);
		if (evt.status == osEventMessage)
			return (msg_t*)evt.value.p;
	}
}
#endif

#define DUMP_HEX_DATA(str, p_val, length, base) \
{\
	uint8_t *p = (uint8_t*)p_val;\
	printf("\n%s(%d) = ", str, length);\
	for (int i=0;i<length;i++)\
	{\
		if (base == 'c')\
			if (p[i] == 'r')\
				break;\
			else\
				printf("%c", p[i]);\
		else if (base == 'x')\
			printf("%02X ", p[i]);\
	}\
	printf("\r\n");\
}

#define INB_API_CALL(func, ...) do { \
	if ((ret=func(##__VA_ARGS__))!=0) { \
		printf("%s (Err=0x%02X)\r\n", #func, ret); \
		goto out; \
		} \
	} while(false);

static __inline void str_to_byte_array(char *str, int reverse_order, uint8_t *p_out, int *p_out_len)
{
	int in_len = strlen(str);

	//Not enough memory for byte array
	if ( in_len > (*p_out_len)*2 ) {
		*p_out_len = 0;
		return;
	}

	for (int i=0;i<in_len;i++) {

		char c;

		if (reverse_order == 1) 
			c = str[in_len-i-1];
		else
			c = str[i];

		if (c>='0' && c<='9')
			p_out[i/2] += (c-'0')<<4*(i%2);
		else if (c>='a' && c<='f')
			p_out[i/2] += (0x0a+c-'a')<<4*(i%2);
		else if (c>='A' && c<='F')
			p_out[i/2] += (0x0a+c-'A')<<4*(i%2);
		else
		{
			*p_out_len = 0;
			return;
		}
	}
	*p_out_len = in_len/2;
}

static __inline void byte_array_to_str(uint8_t *p_in, int in_len, char *str, int reverse_order)
{
	uint8_t b;

	for (int i=0;i<in_len;str+=2,i++) {
		if (reverse_order == 1) 
			b = p_in[in_len-i-1];
		else
			b = p_in[i];

		sprintf(str, "%02X", b);
	}
}

#endif

