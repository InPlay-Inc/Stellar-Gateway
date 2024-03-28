#ifndef __USB_IF_H__
#define __USB_IF_H__
#include "stdio.h"
#include "at32f413.h"

//Function declaration
void usb_if_open(void);
uint16_t usb_if_rx(uint8_t *recv_data);
error_status usb_if_tx(uint8_t *send_data, uint16_t len);

#endif //__USB_IF_H__
