#ifndef __SPI_IF_H__
#define __SPI_IF_H__
#include "stdio.h"
#include "at32f413.h"

#define  spi_buffer_size  256

typedef enum  {
	SPI_STATE_IDLE = 0,
	SPI_STATE_BUSY,
	SPI_STATE_DONE,
}spi_state_t;

#ifdef INTERNAL
uint8_t spi_rx_buffer[spi_buffer_size]={0};
#else
extern uint8_t spi_rx_buffer[spi_buffer_size];
#endif

//Function declaration
void spi_if_open(void);
spi_state_t spi_if_trx(uint8_t cs);

#endif //__SPI_IF_H__

