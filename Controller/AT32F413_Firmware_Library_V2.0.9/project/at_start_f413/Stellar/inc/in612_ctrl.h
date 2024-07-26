#ifndef __IN612_CTRL_H__
#define __IN612_CTRL_H__

#include "stdio.h"
#include "at32f413.h"

enum{

	MODE_NORMAL = 0,
	MODE_TEST = 0x01,
	MODE_BOOT = 0x02,

//	IN612_CS_1 = 0x10,
//	IN612_CS_2 = 0x20,
//	IN612_CS_3 = 0x30,
//	IN612_CS_4 = 0x40,
};

int in612_ctrl_init(void);
void in612_ctrl_reset(int cs);
int in612_ctrl_get_mode(uint8_t buf[], int len);
int in612_ctrl_get_cs(void);
void in612_enter_boot_mode(int cs);
int in612_ctrl_parse(uint8_t buf[], int len, int cur_mode, int cur_cs, int *p_mode, int *p_cs);

#endif
