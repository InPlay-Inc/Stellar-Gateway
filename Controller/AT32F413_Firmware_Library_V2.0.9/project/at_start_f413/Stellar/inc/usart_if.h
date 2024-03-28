#ifndef __USART_IF_H__
#define __USART_IF_H__
#include "stdio.h"
#include "at32f413.h"

//#define  usart_buffer_size  1200

typedef void (*usart_rx_cb)(int cs, uint8_t buf[], uint16_t len);

#ifdef USART_IF_INTERNAL
//uint8_t usart_rx_status[4] = {0};
//uint8_t usart_rx_buffer[4][usart_buffer_size]={{0},{0},{0},{0}};
//uint16_t usart_rx_data_len[4] = {0};
//uint8_t usart_tx_buffer[4][usart_buffer_size]={{0},{0},{0},{0}};
//uint16_t usart_tx_data_len[4] = {0};
#else
//extern uint8_t usart_rx_status[4];
//extern uint8_t usart_rx_buffer[4][usart_buffer_size];
//extern uint16_t usart_rx_data_len[4];
//extern uint8_t usart_tx_buffer[4][usart_buffer_size];
//extern uint16_t usart_tx_data_len[4];
#endif

//Function declaration
int usart_if_open(usart_rx_cb rx_cb);
int usart_if_rx(usart_rx_cb rx_cb);
void usart_if_tx(int idx, uint8_t *send_data, uint16_t len);
void usart_configuration(uint32_t baud_rate, usart_data_bit_num_type data_bit, usart_stop_bit_num_type stop_bit, usart_parity_selection_type usart_parity_select);

#endif
