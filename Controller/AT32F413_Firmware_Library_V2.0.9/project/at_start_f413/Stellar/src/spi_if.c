#include "at32f413_board.h"
#include "at32f413_clock.h"
#include "at32f413_int.h"
#define INTERNAL
#include "spi_if.h"

typedef struct
{
	spi_type 					*spi_base;
	
	gpio_type 					*clk_gpio_base;
	uint16_t 					clk_pin;
	gpio_type 					*miso_gpio_base;
	uint16_t 					miso_pin;
	gpio_type 					*mosi_gpio_base;
	uint16_t 					mosi_pin;
	gpio_type 					*cs_gpio_base[2];
	uint16_t					cs_pin[2];
	gpio_type 					*slv_ready_gpio_base[2];
	uint16_t					slv_ready_pin[2];

//	IRQn_Type 					usart_irq;

	dma_channel_type			*tx_dma_chn_base;
	IRQn_Type 					tx_dma_irq;
	uint8_t 					tx_flex_channelx;
	dma_flexible_request_type 	tx_flexible_request;

	dma_channel_type 			*rx_dma_chn_base;
	IRQn_Type 					rx_dma_irq;
	uint8_t 					rx_flex_channelx;
	dma_flexible_request_type 	rx_flexible_request;

} spi_cfg_t;

spi_cfg_t spi_cfg = {
//	.spi_base = 		SPI2,

//	.clk_gpio_base =		GPIOB,
//	.clk_pin = 				GPIO_PINS_13,
//	.miso_gpio_base = 		GPIOB,
//	.miso_pin = 			GPIO_PINS_14,
//	.mosi_gpio_base = 		GPIOB,
//	.mosi_pin = 			GPIO_PINS_15,
//	.cs_gpio_base = 		{GPIOA,		  GPIOC},
//	.cs_pin = 				{GPIO_PINS_4, GPIO_PINS_5},
//	.slv_ready_gpio_base =  {GPIOC, 	  GPIOA},
//	.slv_ready_pin = 		{GPIO_PINS_6, GPIO_PINS_1},
	.spi_base = 		SPI1,

	.clk_gpio_base =		GPIOA,
	.clk_pin = 				GPIO_PINS_5,
	.miso_gpio_base = 		GPIOA,
	.miso_pin = 			GPIO_PINS_6,
	.mosi_gpio_base = 		GPIOA,
	.mosi_pin = 			GPIO_PINS_7,
	.cs_gpio_base = 		{GPIOA,		  GPIOC},
	.cs_pin = 				{GPIO_PINS_4, GPIO_PINS_5},
	.slv_ready_gpio_base =  {GPIOC, 	  GPIOA},
	.slv_ready_pin = 		{GPIO_PINS_6, GPIO_PINS_1},

	.tx_dma_chn_base = 		DMA1_CHANNEL6,
	.tx_dma_irq = 			DMA1_Channel6_IRQn,
	.tx_flex_channelx = 	FLEX_CHANNEL6,
	.tx_flexible_request = 	DMA_FLEXIBLE_SPI1_TX,

	.rx_dma_chn_base = 		DMA1_CHANNEL7,	
	.rx_dma_irq	= 			DMA1_Channel7_IRQn,
	.rx_flex_channelx = 	FLEX_CHANNEL7,
	.rx_flexible_request = 	DMA_FLEXIBLE_SPI1_RX,
};

uint8_t spi_rx_status[2] = {0};
uint8_t spi_tx_buffer[spi_buffer_size]={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc};
uint16_t spi_rx_data_len = 0;

/**
  * @brief  this function handles dma1 channel7 handler.
  * @param  none
  * @retval none
  */
void DMA1_Channel7_IRQHandler(void)
{
  if(dma_flag_get(DMA1_FDT7_FLAG)) {
    dma_flag_clear(DMA1_FDT7_FLAG);
	dma_channel_enable(spi_cfg.tx_dma_chn_base, FALSE);
    dma_channel_enable(DMA1_CHANNEL7, FALSE);
    for (int i=0;i<2;i++) {
      if ( spi_rx_status[i] == SPI_STATE_BUSY) {
		spi_rx_status[i] = SPI_STATE_DONE;
		gpio_bits_set(spi_cfg.cs_gpio_base[i], spi_cfg.cs_pin[i]);
		spi_enable(spi_cfg.spi_base, FALSE);
        break;
      }
	}
  }
}

/**
  * @brief  spi configuration.
  * @param  none
  * @retval none
  */
void spi_if_open(void)
{
  gpio_init_type gpio_initstructure;
  gpio_default_para_init(&gpio_initstructure);

  /* master sck pin */
  gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
  gpio_initstructure.gpio_pull           = GPIO_PULL_DOWN;
  gpio_initstructure.gpio_mode           = GPIO_MODE_MUX;
  gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_initstructure.gpio_pins           = spi_cfg.clk_pin;
  gpio_init(spi_cfg.clk_gpio_base, &gpio_initstructure);

  /* master mosi pin */
  gpio_initstructure.gpio_pull           = GPIO_PULL_DOWN;
  gpio_initstructure.gpio_mode           = GPIO_MODE_MUX;
  gpio_initstructure.gpio_pins           = spi_cfg.mosi_pin;
  gpio_init(spi_cfg.mosi_gpio_base, &gpio_initstructure);

  /* master miso pin */
  gpio_initstructure.gpio_pull           = GPIO_PULL_UP;
  gpio_initstructure.gpio_mode           = GPIO_MODE_INPUT;
  gpio_initstructure.gpio_pins           = spi_cfg.miso_pin;
  gpio_init(spi_cfg.miso_gpio_base, &gpio_initstructure);

  /* master cs pin */
  for (int i=0;i<2;i++) {
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_initstructure.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
    gpio_initstructure.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_initstructure.gpio_pins = spi_cfg.cs_pin[i];
    gpio_initstructure.gpio_pull = GPIO_PULL_NONE;
    gpio_init(spi_cfg.cs_gpio_base[i], &gpio_initstructure);
    gpio_bits_set(spi_cfg.cs_gpio_base[i], spi_cfg.cs_pin[i]);
  }

  /* slave ready pin */
  for (int i=0;i<2;i++) {
    gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_initstructure.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
    gpio_initstructure.gpio_mode = GPIO_MODE_INPUT;
    gpio_initstructure.gpio_pins = spi_cfg.slv_ready_pin[i];
    gpio_initstructure.gpio_pull = GPIO_PULL_UP;
    gpio_init(spi_cfg.slv_ready_gpio_base[i], &gpio_initstructure);
//  gpio_exint_line_config(spi_cfg.rx_rdy_io[0].int_port, spi_cfg.rx_rdy_io[0].int_pin);
//  nvic_irq_enable(spi_cfg.rx_rdy_io[0].int_type, 0, 0);
  }

  spi_init_type spi_init_struct;
  spi_default_para_init(&spi_init_struct);

  spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
  spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
  spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_32;
  spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
  spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
  spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_HIGH;
  spi_init_struct.clock_phase = SPI_CLOCK_PHASE_2EDGE;
  spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
  spi_init(spi_cfg.spi_base, &spi_init_struct);

  spi_i2s_dma_transmitter_enable(spi_cfg.spi_base, TRUE);
  spi_i2s_dma_receiver_enable(spi_cfg.spi_base, TRUE);
  //spi_enable(spi_cfg.spi_base, TRUE);

  dma_init_type dma_init_struct;
  dma_default_para_init(&dma_init_struct);

  /* spi tx */
  dma_reset(spi_cfg.tx_dma_chn_base);
  dma_init_struct.buffer_size = spi_buffer_size;
  dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
  dma_init_struct.memory_base_addr = (uint32_t)spi_tx_buffer;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.peripheral_base_addr = (uint32_t)&spi_cfg.spi_base->dt;// 0x4000380C;
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = DMA_PRIORITY_MEDIUM;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init(spi_cfg.tx_dma_chn_base, &dma_init_struct);
//  dma_interrupt_enable(spi_cfg.tx_dma_chn_base, DMA_FDT_INT, TRUE);
  /* dma channel interrupt nvic init */
//  nvic_irq_enable(spi_cfg.tx_dma_irq, 0, 0);
  /* config flexible dma for SPI tx */
  dma_flexible_config((dma_type*)((uint32_t)spi_cfg.tx_dma_chn_base&0xFFFFFF00), spi_cfg.tx_flex_channelx, spi_cfg.tx_flexible_request);
  //dma_channel_enable(spi_cfg.tx_dma_chn_base, TRUE);

  /* spi rx */
  dma_reset(spi_cfg.rx_dma_chn_base);
  dma_init_struct.buffer_size = spi_buffer_size;
  dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
  dma_init_struct.memory_base_addr = (uint32_t)spi_rx_buffer;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.peripheral_base_addr = (uint32_t)&spi_cfg.spi_base->dt;// 0x4000380C;
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = DMA_PRIORITY_MEDIUM;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init(spi_cfg.rx_dma_chn_base, &dma_init_struct);
  /* enable transfer full data intterrupt */
  dma_interrupt_enable(spi_cfg.rx_dma_chn_base, DMA_FDT_INT, TRUE);
  /* dma channel interrupt nvic init */
  nvic_irq_enable(spi_cfg.rx_dma_irq, 0, 0);
  /* config flexible dma for SPI rx */
  dma_flexible_config((dma_type*)((uint32_t)spi_cfg.rx_dma_chn_base&0xFFFFFF00), spi_cfg.rx_flex_channelx, spi_cfg.rx_flexible_request);
  //dma_channel_enable(spi_cfg.rx_dma_chn_base, TRUE);
}

spi_state_t spi_if_trx(uint8_t cs)
{
  if (spi_rx_status[cs] == SPI_STATE_IDLE) {
//flag_status st = gpio_input_data_bit_read(spi_cfg.slv_ready_gpio_base[cs], spi_cfg.slv_ready_pin[cs]);
//printf("cs %d:[%p, %d] = %d\n", cs, spi_cfg.slv_ready_gpio_base[cs], spi_cfg.slv_ready_pin[cs], st);
    if (gpio_input_data_bit_read(spi_cfg.slv_ready_gpio_base[cs], spi_cfg.slv_ready_pin[cs]) == RESET) {
printf("start cs %d rx \n", cs);
      memset(spi_rx_buffer, 0, spi_buffer_size);
      spi_cfg.tx_dma_chn_base->dtcnt = 12;//spi_buffer_size;
   	  dma_channel_enable(spi_cfg.tx_dma_chn_base, TRUE);

      spi_cfg.rx_dma_chn_base->dtcnt = 12;//spi_buffer_size;
      spi_cfg.rx_dma_chn_base->maddr = (uint32_t)spi_rx_buffer;
   	  dma_channel_enable(spi_cfg.rx_dma_chn_base, TRUE);

      spi_rx_status[cs] = SPI_STATE_BUSY;

//delay_ms(100);

      gpio_bits_reset(spi_cfg.cs_gpio_base[cs], spi_cfg.cs_pin[cs]);
      spi_enable(spi_cfg.spi_base, TRUE);
    }
  } else if (spi_rx_status[cs] == SPI_STATE_DONE) {

delay_ms(100);
    spi_rx_status[cs] = SPI_STATE_IDLE;
    return SPI_STATE_DONE;
  }

  return spi_rx_status[cs];
}
