/**
  **************************************************************************
  * @file     main.c
  * @brief    main program
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#include "at32f413_board.h"
#include "at32f413_clock.h"
#include "usbd_core.h"
#include "cdc_class.h"
#include "cdc_desc.h"
#include "usbd_int.h"
#include "at32f413_int.h"

#include "usb_if.h"
#include "usart_if.h"
#include "spi_if.h"

/** @addtogroup AT32F413_periph_examples
  * @{
  */

/** @addtogroup 413_USB_device_vcp_loopback USB_device_vcp_loopback
  * @{
  */

enum{

	MODE_NORMAL = 0,
	MODE_TEST,
	MODE_BOOT,
};

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
				printf(, "%c", p[i]);\
		else if (base == 'x')\
			printf(, "%02X ", p[i]);\
	}\
	printf(, "\r\n");\
}

uint8_t usb_buffer[1200];

int in612_cs = -1;
int mode;

//Function Declaration of in612_ctrl
int in612_ctrl_init(void);
void in612_ctrl_enter_boot_mode(int cs);
void in612_ctrl_reset(int cs);
_Bool in612_ctrl_check_cmd_ready(uint8_t buf[], int len);
void in612_enter_boot_mode(int cs);
int in612_ctrl_get_cs(void);

//Function Declaration of gateway
int gw_mng_usart_rx_cb(int cs, uint8_t rx_buf[], int rx_len);
int gw_mng_handle_host_msg(uint8_t buf[], int len);
int gw_mng_handle_wl_msg(uint8_t **p);

void usart_send_data(usart_type* usart_x, uint8_t *send_data, uint16_t len);
//uint16_t usart_receive_data(void);

/**
  * @brief  usb 48M clock select
  * @param  clk_s:USB_CLK_HICK, USB_CLK_HEXT
  * @retval none
  */
static void usb_clock48m_select(usb_clk48_s clk_s)
{
  if(clk_s == USB_CLK_HICK)
  {
    crm_usb_clock_source_select(CRM_USB_CLOCK_SOURCE_HICK);

    /* enable the acc calibration ready interrupt */
    crm_periph_clock_enable(CRM_ACC_PERIPH_CLOCK, TRUE);

    /* update the c1\c2\c3 value */
    acc_write_c1(7980);
    acc_write_c2(8000);
    acc_write_c3(8020);

    /* open acc calibration */
    acc_calibration_mode_enable(ACC_CAL_HICKTRIM, TRUE);
  }
  else
  {
    switch(system_core_clock)
    {
      /* 48MHz */
      case 48000000:
        crm_usb_clock_div_set(CRM_USB_DIV_1);
        break;

      /* 72MHz */
      case 72000000:
        crm_usb_clock_div_set(CRM_USB_DIV_1_5);
        break;

      /* 96MHz */
      case 96000000:
        crm_usb_clock_div_set(CRM_USB_DIV_2);
        break;

      /* 120MHz */
      case 120000000:
        crm_usb_clock_div_set(CRM_USB_DIV_2_5);
        break;

      /* 144MHz */
      case 144000000:
        crm_usb_clock_div_set(CRM_USB_DIV_3);
        break;

      /* 168MHz */
      case 168000000:
        crm_usb_clock_div_set(CRM_USB_DIV_3_5);
        break;

      /* 192MHz */
      case 192000000:
        crm_usb_clock_div_set(CRM_USB_DIV_4);
        break;

      default:
        break;

    }
  }
}

static void periph_clock_init(void)
{
  /* GPIO clock*/
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);

  /* USART clock*/
  crm_periph_clock_enable(CRM_USART1_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_USART2_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_USART3_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_UART4_PERIPH_CLOCK, TRUE);

  /* enable dma1 clock */
  crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);

  /* select usb 48m clcok source */
  usb_clock48m_select(USB_CLK_HEXT);

  /* enable usb clock */
  crm_periph_clock_enable(CRM_USB_PERIPH_CLOCK, TRUE);

  /* SPI clock */
  crm_periph_clock_enable(CRM_SPI1_PERIPH_CLOCK, TRUE);
}


void usart_data_rcvd(int cs, uint8_t buf[], uint16_t len)
{
	if (mode == MODE_NORMAL) {
		gw_mng_usart_rx_cb(cs, buf, len);
	} else { //Test mode || Boot mode

		/* send data to host */
		if(usb_if_tx(buf, len) != SUCCESS)
			printf("usb_vcp_send_data failed!\n");
	}
}


/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  int i;
  uint16_t data_len;

  /* config nvic priority group */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);

  system_clock_config();
  at32_board_init();
  periph_clock_init();

  printf("stellar_gateway_board_v2 started...\n");

  usart_if_open(usart_data_rcvd);
  usb_if_open();
  //spi_if_open();

/*
	WAKEUP_IN/SLEEP_IND  |       MODE
	=======================================
		   00			 |      Normal
		   01            |      Bypass
		   10            |      Bypass
		   11            |  IN612 Download
*/
  mode = in612_ctrl_init();

  for (int cs=0;cs<4;cs++)
    in612_ctrl_reset(cs);

  if (mode == 1) {//Test mode, all data from VCP will be passed to each IN612L
	in612_cs = in612_ctrl_get_cs();
  } else 
    in612_cs = -1;

  printf("mode = %x cs = %d\n", mode, in612_cs);

  while(1) {
    /* 1. SPI*/
    spi_state_t spi_state;
    for (int i=0;i<2;i++) {
      spi_state = spi_if_trx(i);
      if (spi_state == SPI_STATE_DONE) {

        //DUMP_HEX_DATA("spi_rx: ", spi_rx_buffer, spi_buffer_size, 'x');
      }
    }

    /* 2. USART*/
	uint8_t *p_out = NULL;
	data_len = gw_mng_handle_wl_msg(&p_out);
	if (data_len != 0 && p_out ) {
		/* send data to host */
		if(usb_if_tx(p_out, data_len) != SUCCESS)
		printf("usb_vcp_send_data failed!\n");
	}

    /*3. USB Virtual Port*/
    /* get usb vcp receive data */
	data_len = usb_if_rx(usb_buffer);
    /* send data to hardware usart */
    if(data_len > 0)
    {
	  switch(mode) {
      case MODE_TEST:
        if (in612_cs != -1)
          usart_if_tx(in612_cs, usb_buffer, data_len);
        break;
      case MODE_BOOT:
        if ( in612_ctrl_check_cmd_ready(usb_buffer, data_len) == TRUE) {
          if (in612_cs != in612_ctrl_get_cs()) {
            in612_cs = in612_ctrl_get_cs();
            in612_enter_boot_mode(in612_cs);
            mode = MODE_BOOT;
            printf("mode = %x cs = %d\n", mode, in612_cs);
          }
		}
        usart_if_tx(in612_cs, usb_buffer, data_len);
        break;
      case MODE_NORMAL:
        printf(" <== ");
        for (int i=0;i<data_len;i++)
          printf("%02x ", usb_buffer[i]);
        printf("\n");
        if ( in612_ctrl_check_cmd_ready(usb_buffer, data_len) == TRUE) {
          in612_cs = in612_ctrl_get_cs();
          in612_enter_boot_mode(in612_cs);
          mode = MODE_BOOT;
          printf("mode = %x cs = %d\n", mode, in612_cs);
		} else {
          gw_mng_handle_host_msg(usb_buffer, data_len);
        }
      }
    }
  }

}

/**
  * @}
  */

/**
  * @}
  */
