#include <stdlib.h>
#include <string.h>

#include "at32f413_board.h"
#include "at32f413_clock.h"
#include "at32f413_int.h"

#define USART_IF_INTERNAL
#include "usart_if.h"

#include "rq.h"

typedef struct
{
	usart_type					*usart_base;
	
	gpio_type 					*tx_gpio_base;
	uint16_t 					tx_pin;
	gpio_type 					*rx_gpio_base;
	uint16_t 					rx_pin;
	
	IRQn_Type					usart_irq;

	dma_channel_type 			*dma_chn_base;
	IRQn_Type					dma_irq;
	uint8_t 					flex_channelx;
	dma_flexible_request_type 	flexible_request;

} usart_cfg_t;

usart_cfg_t usart_cfg[4] = {
	[0] = { // U1
			.usart_base 	= USART3,
			.tx_gpio_base 	= GPIOB,			
			.tx_pin 		= GPIO_PINS_10,
			.rx_gpio_base 	= GPIOB,
			.rx_pin 		= GPIO_PINS_11,
			.usart_irq		= USART3_IRQn,
			.dma_chn_base	= DMA1_CHANNEL1,
			.dma_irq 		= DMA1_Channel1_IRQn,
			.flex_channelx	= FLEX_CHANNEL1,
			.flexible_request = DMA_FLEXIBLE_UART3_RX,
		  },
	[1] = { // U3
			.usart_base 	= USART2,
			.tx_gpio_base 	= GPIOA,
			.tx_pin 		= GPIO_PINS_2,
			.rx_gpio_base 	= GPIOA,
			.rx_pin 		= GPIO_PINS_3,
			.usart_irq		= USART2_IRQn,
			.dma_chn_base	= DMA1_CHANNEL2,
			.dma_irq 		= DMA1_Channel2_IRQn,
			.flex_channelx	= FLEX_CHANNEL2,
			.flexible_request = DMA_FLEXIBLE_UART2_RX,
		  },
	[2] = { //U6
			.usart_base 	= USART1,
			.tx_gpio_base 	= GPIOA,
			.tx_pin 		= GPIO_PINS_9,
			.rx_gpio_base 	= GPIOA,
			.rx_pin 		= GPIO_PINS_10,
			.usart_irq		= USART1_IRQn,
			.dma_chn_base	= DMA1_CHANNEL3,
			.dma_irq 		= DMA1_Channel3_IRQn,
			.flex_channelx	= FLEX_CHANNEL3,
			.flexible_request = DMA_FLEXIBLE_UART1_RX,
		  },
	[3] = { //U8
			.usart_base 	= UART4,
			.tx_gpio_base 	= GPIOC,
			.tx_pin 		= GPIO_PINS_10,
			.rx_gpio_base 	= GPIOC,
			.rx_pin 		= GPIO_PINS_11,
			.usart_irq		= UART4_IRQn,
			.dma_chn_base	= DMA1_CHANNEL4,
			.dma_irq 		= DMA1_Channel4_IRQn,
			.flex_channelx	= FLEX_CHANNEL4,
			.flexible_request = DMA_FLEXIBLE_UART4_RX,
		  },
};

#define  usart_dma_buf_sz  1200
#define  usart_rcvd_buf_sz (usart_dma_buf_sz*4)

static uint8_t usart_dma_buf[4][usart_dma_buf_sz]={{0},{0},{0},{0}};

//uint8_t usart_rcvd_buf[];

static usart_rx_cb g_rx_cb;

static void DMA1_Interrupt(int idx)
{
	uint32_t dma_fdt_flag[4] = {DMA1_FDT1_FLAG, DMA1_FDT2_FLAG, DMA1_FDT3_FLAG, DMA1_FDT4_FLAG};

	if(dma_flag_get(dma_fdt_flag[idx])) {

		printf("**DMA FDT!**\n");

		//usart_rx_status[idx] = 1;
		//usart_rx_data_len[idx] = dma_data_number_get(DMA1_CHANNEL1);
		dma_flag_clear(dma_fdt_flag[idx]);
		//dma_channel_enable(DMA1_CHANNEL1, FALSE);
	}
}

/**
  * @brief  this function handles dma1 channel1 handler.
  * @param  none
  * @retval none
  */
void DMA1_Channel1_IRQHandler(void)
{
  DMA1_Interrupt(0);
}

/**
  * @brief  this function handles dma1 channel2 handler.
  * @param  none
  * @retval none
  */
void DMA1_Channel2_IRQHandler(void)
{
  DMA1_Interrupt(1);
}

/**
  * @brief  this function handles dma1 channel3 handler.
  * @param  none
  * @retval none
  */
void DMA1_Channel3_IRQHandler(void)
{
  DMA1_Interrupt(2);
}

/**
  * @brief  this function handles dma1 channel4 handler.
  * @param  none
  * @retval none
  */
void DMA1_Channel4_IRQHandler(void)
{
  DMA1_Interrupt(3);
}

static void USART_interrupt(int idx)
{
  //usart_type *usart_x[4] = {USART1,USART2,USART3,UART4};
  dma_channel_type *dma1_channel[4] = {DMA1_CHANNEL1, DMA1_CHANNEL2, DMA1_CHANNEL3, DMA1_CHANNEL4};

  if (usart_cfg[idx].usart_base->ctrl1_bit.idleien != RESET) {

    if (usart_cfg[idx].usart_base->sts_bit.idlef == 1) {

      int rx_len = usart_dma_buf_sz-dma_data_number_get(dma1_channel[idx]);
      usart_dma_buf[idx][rx_len] = usart_data_receive(usart_cfg[idx].usart_base);

	  dma_channel_enable(dma1_channel[idx], FALSE);
      dma1_channel[idx]->dtcnt = usart_dma_buf_sz;
      dma1_channel[idx]->maddr = (uint32_t)&usart_dma_buf[idx][0];

//printf("cs=%d: (%d) ", idx, rx_len );
//for (int i=0;i<rx_len;i++)
//	printf("%02x ", usart_dma_buf[idx][i]);
//printf("\n");
//putchar('i');

		g_rx_cb(idx, usart_dma_buf[idx], rx_len);

	  dma_channel_enable(dma1_channel[idx], TRUE);
     
//      usart_rx_status[idx] = 1;
    }
  }
}

/**
  * @brief  this function handles usart1 Tx interrupt handler.
  * @param  none
  * @retval none
  */
void USART1_IRQHandler(void)
{
  USART_interrupt(2);
}

/**
  * @brief  this function handles usart2 Tx interrupt handler.
  * @param  none
  * @retval none
  */
void USART2_IRQHandler(void)
{
  USART_interrupt(1);
}

/**
  * @brief  this function handles usart3 Tx interrupt handler.
  * @param  none
  * @retval none
  */
void USART3_IRQHandler(void)
{
  USART_interrupt(0);
}

/**
  * @brief  this function handles usart4 Tx interrupt handler.
  * @param  none
  * @retval none
  */
void UART4_IRQHandler(void)
{
  USART_interrupt(3);
}

/**
  * @brief  config usart
  * @param  none
  * @retval none
  */
void usart_configuration(uint32_t baud_rate, usart_data_bit_num_type data_bit, usart_stop_bit_num_type stop_bit, usart_parity_selection_type usart_parity_select)
{
  gpio_init_type gpio_init_struct;
  dma_init_type dma_init_struct;
  static uint32_t last_baud_rate = 0;

  if (last_baud_rate == baud_rate)
    return;
  last_baud_rate = baud_rate;

  printf("uart configuration\n");

  for (int i=0;i<4;i++) {
	gpio_default_para_init(&gpio_init_struct);

    /* configure the usart tx pin */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = usart_cfg[i].tx_pin;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(usart_cfg[i].tx_gpio_base, &gpio_init_struct);

    /* configure the usart_x rx pin */
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins = usart_cfg[i].rx_pin;
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init(usart_cfg[i].rx_gpio_base, &gpio_init_struct);

    /* configure usart param */
    usart_init(usart_cfg[i].usart_base, baud_rate, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_parity_selection_config(usart_cfg[i].usart_base, usart_parity_select);
    usart_transmitter_enable(usart_cfg[i].usart_base, TRUE);
    usart_receiver_enable(usart_cfg[i].usart_base, TRUE);
    usart_dma_receiver_enable(usart_cfg[i].usart_base, TRUE);
    usart_interrupt_enable(usart_cfg[i].usart_base, USART_IDLE_INT, TRUE);
    usart_enable(usart_cfg[i].usart_base, TRUE);
	/* config usart nvic interrupt */
    nvic_irq_enable(usart_cfg[i].usart_irq, 0, 0);

    /* dma1 channel for rx configuration */
    dma_reset(usart_cfg[i].dma_chn_base);
    dma_default_para_init(&dma_init_struct);
    dma_init_struct.buffer_size = usart_dma_buf_sz;
    dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_base_addr = (uint32_t)&usart_dma_buf[i][0];
    dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
    dma_init_struct.memory_inc_enable = TRUE;
    dma_init_struct.peripheral_base_addr = (uint32_t)&usart_cfg[i].usart_base->dt;
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.priority = DMA_PRIORITY_MEDIUM;
    dma_init_struct.loop_mode_enable = FALSE;
    dma_init(usart_cfg[i].dma_chn_base, &dma_init_struct);
    /* enable transfer full data intterrupt */
    //dma_interrupt_enable(usart_cfg[i].dma_chn_base, DMA_FDT_INT, TRUE);
    /* dma1 channel1 interrupt nvic init */
    //nvic_irq_enable(usart_cfg[i].dma_irq, 0, 0);
    /* config flexible dma for usart rx */
    dma_flexible_config((dma_type*)((uint32_t)usart_cfg[i].dma_chn_base&0xFFFFFF00), usart_cfg[i].flex_channelx, usart_cfg[i].flexible_request);    

    dma_channel_enable(usart_cfg[i].dma_chn_base, TRUE); /* usart rx begin dma receiving */
  }
}

/**
  * @brief  this function handles usart2 send data.
  * @param  send_data: pointer to data
  * @param  len: data len
  * @retval none
  */
void usart_if_tx(int idx, uint8_t *send_data, uint16_t len)
{
  uint16_t index = 0;
  for(index = 0; index < len; index++ )
  {
    do
    {
      ;
    }while(usart_flag_get(usart_cfg[idx].usart_base, USART_TDBE_FLAG) == RESET);
	usart_data_transmit(usart_cfg[idx].usart_base, send_data[index]);
  }
}

int usart_if_open(usart_rx_cb rx_cb)
{
	g_rx_cb = rx_cb;

	return 0;

}
