#include "at32f413_board.h"
#include "at32f413_clock.h"
#include "usbd_core.h"
#include "cdc_class.h"
#include "cdc_desc.h"
#include "usbd_int.h"
#include "at32f413_int.h"

#include "usart_if.h"

usbd_core_type usb_core_dev;

/**
  * @brief  usb delay millisecond function.
  * @param  ms: number of millisecond delay
  * @retval none
  */
void usb_delay_ms(uint32_t ms)
{
  /* user can define self delay function */
  delay_ms(ms);
}

/**
  * @brief  usb delay microsecond function.
  * @param  us: number of microsecond delay
  * @retval none
  */
void usb_delay_us(uint32_t us)
{
  delay_us(us);
}

/**
  * @brief  this function handles usb interrupt.
  * @param  none
  * @retval none
  */
void USBFS_L_CAN1_RX0_IRQHandler(void)
{
  usbd_irq_handler(&usb_core_dev);
}

/**
  * @brief  this function handles usart2  and linecoding config.
  * @param  linecoding: linecoding value
  * @retval none
  */
void usb_usart_config( linecoding_type linecoding)
{
  usart_stop_bit_num_type usart_stop_bit;
  usart_data_bit_num_type usart_data_bit;
  usart_parity_selection_type usart_parity_select;

  /* enable the usart2 and gpio clock */
//  crm_periph_clock_enable(CRM_USART2_PERIPH_CLOCK, FALSE);
//  crm_periph_clock_enable(CRM_USART2_PERIPH_CLOCK, TRUE);

  /* stop bit */
  switch(linecoding.format)
  {
    case 0x0:
      usart_stop_bit = USART_STOP_1_BIT;
      break;
	/* to be used when transmitting and receiving data in smartcard mode */
    case 0x1:
      usart_stop_bit = USART_STOP_1_5_BIT;
      break;
    case 0x2:
      usart_stop_bit = USART_STOP_2_BIT;
      break;
    default :
      break;
  }
  /* data bits */
  switch(linecoding.data)
  {
    /* hardware usart not support data bits for 5/6/7 */
    case 0x5:
    case 0x6:
    case 0x7:
      break;
    case 0x8:
      usart_data_bit = USART_DATA_8BITS;
      break;
    /* hardware usart not support data bits for 16 */
    case 0x10:
      break;
    default :
      break;
  }
  /* parity */
  switch(linecoding.parity)
  {
    case 0x0:
      usart_parity_select = USART_PARITY_NONE;
      break;
    case 0x1:
      usart_parity_select = USART_PARITY_ODD;
      break;
    case 0x2:
      usart_parity_select = USART_PARITY_EVEN;
      break;
    /* hardware usart not support parity for mark and space */
    case 0x3:
    case 0x4:
      break;
    default :
      break;
  }

  /* usart config */
  usart_configuration(linecoding.bitrate, usart_data_bit, usart_stop_bit, usart_parity_select);
}

void usb_if_open(void)
{
  /* enable usb interrupt */
  nvic_irq_enable(USBFS_L_CAN1_RX0_IRQn, 5, 5);

  /* usb core init */
  usbd_core_init(&usb_core_dev, USB, &cdc_class_handler, &cdc_desc_handler, 0);

  /* enable usb pull-up */
  usbd_connect(&usb_core_dev);
}

uint16_t usb_if_rx(uint8_t *recv_data)
{
	return usb_vcp_get_rxdata(&usb_core_dev, recv_data);
}

error_status usb_if_tx(uint8_t *send_data, uint16_t len)
{
	//usb_delay_ms(8);
	return usb_vcp_send_data(&usb_core_dev, send_data, len);
}
