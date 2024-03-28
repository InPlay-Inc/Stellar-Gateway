/* includes ------------------------------------------------------------------*/
#include "at32f413.h"

#include "wl_share.h"

#include "usb_if.h"
#include "usart_if.h"
#include "misc.h"
#include "rq.h"

enum {
	U1_CS = 0,
	U3_CS,
	U6_CS,
	U8_CS,
	CS_MAX,
};

#define TRX_CS		U8_CS

typedef enum
{
	GW_STATE_NONE,
	GW_STATE_READY,
	GW_STATE_RUN,
} in612_state_t;

//static in612_state_t in_state[4] = {IN612_STATE_NONE,IN612_STATE_NONE,IN612_STATE_NONE,IN612_STATE_NONE};
static int gw_state = GW_STATE_NONE;
static int gw_cs = 0;
static cmd_Configure_t gw_cfg;
static uint16_t gw_sync_intv = 0;

uint8_t wl_msg_rq_buf[MAX_FRAME_SZ*(200+1)];
RQ_DEF(wl_msg, MAX_FRAME_SZ, 200, wl_msg_rq_buf);

//wl_msg_t *g_cmd;
//wl_msg_t *g_rsp;
//uint8_t g_rsp_buf[256];
//int g_rsp_len;

typedef union
{
	ble_node_id_t ble_id;
	sdr_node_id_t sdr_id;
}__attribute__ ((packed)) node_id_t;

struct _filter_table_
{
//	union
//	{
//		ble_node_id_t ble_id;
//		sdr_node_id_t sdr_id;
//	}__attribute__ ((packed)) u;
	node_id_t node_id;
	uint16_t time_stamp; //in unit of 10ms
}__attribute__ ((packed)) node_filter_table[MAX_NODE_COUNT]={0};

static int filter_count=0;

static bool search_node_filter_table(node_id_t *p_node, int *p_idx)
{
	bool found=false;
	
	int start_idx, end_idx, mid_idx;

	start_idx = 0;
	end_idx = filter_count;
	mid_idx = (start_idx+end_idx)/2;
	
	while(end_idx>start_idx)
	{
		int res = memcmp(&node_filter_table[mid_idx].node_id, p_node, sizeof(node_id_t));
		
		if (res == 0)
		{
			found = true;
			*p_idx = mid_idx;
			break;
		}
		else if (res < 0)
		{//node_filter_table[mid_idx].u.ble_addr < &p_adv->node_id
			start_idx = mid_idx+1;
		}
		else 
		{//node_filter_table[mid_idx].u.ble_addr > &p_adv->node_id
			end_idx = mid_idx;
		}
		mid_idx = (start_idx+end_idx)/2;
	}
	if (!found)
		*p_idx = mid_idx;
	
	return found;
}

static void insert_node_filter_table(int idx, node_id_t *p_node, uint32_t time)
{
	if (filter_count < MAX_NODE_COUNT)
	{
		int sz = (filter_count-idx)*sizeof(struct _filter_table_);
		memmove(&node_filter_table[idx+1], &node_filter_table[idx], sz);
		node_filter_table[idx].time_stamp = time;
		memcpy(&node_filter_table[idx].node_id, p_node, sizeof(node_id_t));
	
		filter_count++;
	}

//	printf("==============================================\n");
//	for (int i=0;i<filter_count;i++){
//		printf("[%d]: %02x%02x%02x%02x%02x%02x , 0x%04x \n", i, \
//				node_filter_table[i].node_id.ble_id.bd_addr[0],node_filter_table[i].node_id.ble_id.bd_addr[1],node_filter_table[i].node_id.ble_id.bd_addr[2],
//				node_filter_table[i].node_id.ble_id.bd_addr[3],node_filter_table[i].node_id.ble_id.bd_addr[4],node_filter_table[i].node_id.ble_id.bd_addr[5], node_filter_table[i].time_stamp );
//	}
//	printf("==============================================\n");
}

static bool filter_duplicate(node_id_t *p_node, uint16_t time_stamp)
{
	int idx;
	_Bool found = search_node_filter_table(p_node, &idx);
	if (found)
	{
		if (abs(node_filter_table[idx].time_stamp - time_stamp) < 20 )//200ms differece
		{
//			for (int i=0;i<6;i++)
//				printf("%02x", p_node->ble_id.bd_addr[i]);
//			printf(": skiped 0x%04x\n", time_stamp);
			return true;
		}
		
		node_filter_table[idx].time_stamp = time_stamp;
	}
	else
		insert_node_filter_table(idx, p_node, time_stamp);

	return false;
}

int gw_init( uint8_t ble_addr[6], uint32_t sdr_bcst_access_address, uint32_t sdr_ucst_access_address)
{
//	cmd_Configure_t cfg;

	// U1/U3/U6: MODULE_SCANNER, 
//	cfg.module_type = MODULE_SCANNER;
//	memcpy( cfg.ble_addr, ble_addr, 6);
//	cfg.scanner_access_address = sdr_bcst_access_address;
//	cfg.transceiver_access_address = sdr_ucst_access_address;

//	for (int i=0;i<3;i++) {

//		usart_if_tx(0, (uint8_t*)&cfg, sizeof(cfg));

//	}

//	cfg.module_type = MODULE_TRANSCEIVER;
//	usart_if_tx(0, (uint8_t*)&cfg, sizeof(cfg));

	return 0;
}

int gw_mng_handle_host_msg(uint8_t buf[], int data_len)
{
	wl_msg_t *p_msg = (wl_msg_t*)buf;

	switch( p_msg->msg_id)
	{
	case Cmd_Reset:
		{
			for (int i=0;i<CS_MAX;i++) {
				usart_if_tx(i, (uint8_t*)p_msg, sizeof(wl_msg_t)+sizeof(cmd_RESET_t));
			}
		} break;
	case Cmd_Configure:
		{
			cmd_Configure_t *p_cfg = (cmd_Configure_t *)p_msg->data;
			if (p_cfg->module_type == MODULE_SCANNER) {
				usart_if_tx(U1_CS, (uint8_t*)p_msg, sizeof(wl_msg_t)+sizeof(cmd_Configure_t));
				usart_if_tx(U3_CS, (uint8_t*)p_msg, sizeof(wl_msg_t)+sizeof(cmd_Configure_t));
				usart_if_tx(U6_CS, (uint8_t*)p_msg, sizeof(wl_msg_t)+sizeof(cmd_Configure_t));
			} else {
				usart_if_tx(U8_CS, (uint8_t*)p_msg, sizeof(wl_msg_t)+sizeof(cmd_Configure_t));
			}
		} break;
	case Cmd_Start_Ctrl:
		{
			cmd_Start_Ctrl_t *p_ctrl = (cmd_Start_Ctrl_t *)p_msg->data;
			if (p_ctrl->module == MODULE_SCANNER) {
				usart_if_tx(U1_CS, (uint8_t*)p_msg, sizeof(wl_msg_t)+sizeof(cmd_Start_Ctrl_t));
				usart_if_tx(U3_CS, (uint8_t*)p_msg, sizeof(wl_msg_t)+sizeof(cmd_Start_Ctrl_t));
				usart_if_tx(U6_CS, (uint8_t*)p_msg, sizeof(wl_msg_t)+sizeof(cmd_Start_Ctrl_t));
			} else {
				usart_if_tx(U8_CS, (uint8_t*)p_msg, sizeof(wl_msg_t)+sizeof(cmd_Start_Ctrl_t));
			}
		} break;
	case Cmd_Filter:
		{
			cmd_Filter_t *p_filter = (cmd_Filter_t*)p_msg->data;
			if (p_filter->len != 0) {
				usart_if_tx(U1_CS, (uint8_t*)p_msg, sizeof(wl_msg_t)+sizeof(cmd_Start_Ctrl_t));
				usart_if_tx(U3_CS, (uint8_t*)p_msg, sizeof(wl_msg_t)+sizeof(cmd_Start_Ctrl_t));
				usart_if_tx(U6_CS, (uint8_t*)p_msg, sizeof(wl_msg_t)+sizeof(cmd_Start_Ctrl_t));
			}
		} break;
	case Cmd_SDR_Mcst:
	case Cmd_SDR_Ucst:
	case Cmd_BLE_Connect:
	case Cmd_BLE_GATT_SDP:
	case Cmd_BLE_GATT_Read:
	case Cmd_BLE_GATT_Write:
		{
			usart_if_tx(U8_CS, (uint8_t*)p_msg, data_len);
		} break;
	default:
		break;
	}

done:
	return 0;
}

//Called from USART_Interrupt
//void gw_mng_usart_rx_cb(int cs, uint8_t rx_buf[], int rx_len)
//{
//	wl_msg_t *p_msg = (wl_msg_t*)rx_buf;
//	int offset = 0;

//	while(rx_len >= 3) {

//		int len = get_msg_len(p_msg->msg_id);
//		if (len == 0) {
//			printf("(%d)",p_msg->msg_id);
//			rx_len = 0;
//			continue;
//		}

//		if (p_msg->msg_id < Cmd_max) {

//			switch(p_msg->msg_id)
//			{
//			case Cmd_Configure:
//			case Cmd_Start_Ctrl:
//			case Cmd_Filter:
//			//SDR
//			case Cmd_SDR_Mcst:
//			case Cmd_SDR_Ucst:
//			//BLE
//			case Cmd_BLE_Connect:
//			case Cmd_BLE_Disconnect:
//			case Cmd_BLE_GATT_Write:
//				//These are fixed length rsp from IN612
//				rq_push(RQ(wl_msg), rx_buf+offset, len);
//				break;
//			case Cmd_BLE_Adv:
//			case Cmd_BLE_GATT_Service:
//			case Cmd_BLE_GATT_Notify:
//				//As peripheral
//				//TBD
//				break;
//			case Cmd_BLE_GATT_SDP:
//			case Cmd_BLE_GATT_Read:
//				//These are variable length rsp from IN612
//				break;
//			}

//		} else {
//	
//			switch(p_msg->msg_id)
//			{
//			case Evt_sdr_bcst:
//			case Evt_ble_adv:
//			case Evt_ble_connection:
//				rq_push(RQ(wl_msg), rx_buf+offset, len);
//				break;
//			case Evt_sdr_ucst:
//			case Evt_ble_data:
//				//These are variable length event from IN612
//				break;
//			}
//		}

//		rx_len -= len;
//		offset += len;
//	}
//}

void gw_mng_usart_rx_cb(int cs, uint8_t rx_buf[], int rx_len)
{	
	int offset = 0;
	int len;

	while(rx_len >= 3) {

		wl_msg_t *p_msg = (wl_msg_t*)(rx_buf+offset);

		switch(p_msg->msg_id)
		{
		case Cmd_Reset:
			len = 0;
			break;
		case Cmd_Configure:
			len = sizeof(wl_msg_t)+sizeof(rsp_Configure_t);
			break;
		case Cmd_Start_Ctrl:
			len = sizeof(wl_msg_t)+sizeof(rsp_Start_Ctrl_t);
			break;
		case Cmd_Filter:
			len = sizeof(wl_msg_t)+sizeof(rsp_Filter_t);
			break;
		case Cmd_SDR_Mcst:
			len = sizeof(wl_msg_t)+sizeof(rsp_SDR_Mcst_t);
			break;
		case Cmd_SDR_Ucst:
			len = sizeof(wl_msg_t)+sizeof(rsp_SDR_Ucst_t);
			break;
		//BLE Central
		case Cmd_BLE_Connect:
			len = sizeof(wl_msg_t)+sizeof(rsp_BLE_Connect_t);
			break;
		case Cmd_BLE_GATT_SDP:
			{
				rsp_GATT_SDP_t *p_rsp = (rsp_GATT_SDP_t *)p_msg->data;
				len = sizeof(wl_msg_t)+sizeof(rsp_GATT_SDP_t)+p_rsp->len;
			} break;
		case Cmd_BLE_GATT_Read:
			{
				rsp_GATT_Read_t *p_rsp = (rsp_GATT_Read_t*)p_msg->data;
				len = sizeof(wl_msg_t)+sizeof(rsp_GATT_SDP_t)+p_rsp->len;
			} break;
		case Cmd_BLE_GATT_Write:
			len = sizeof(wl_msg_t)+sizeof(rsp_GATT_Write_t);
			break;
		//BLE Peripheral
		case Cmd_BLE_Adv:
			len = sizeof(wl_msg_t)+sizeof(rsp_BLE_Adv_t);
			break;
		case Cmd_BLE_GATT_Service:
			len = sizeof(wl_msg_t)+sizeof(rsp_GATT_Svc_t);
			break;
		case Cmd_BLE_GATT_Notify:
			len = sizeof(wl_msg_t)+sizeof(rsp_GATT_Notify_t);
			break;
		//Event
		case Evt_sdr_bcst:
		case Evt_ble_adv:
			len = MAX_FRAME_SZ;
			break;
		case Evt_sdr_ucst:
			{
				wl_evt_sdr_ucst_t *p_evt = (wl_evt_sdr_ucst_t *)p_msg->data;
				len = sizeof(wl_msg_t)+sizeof(wl_evt_sdr_ucst_t)+p_evt->len;
			} break;
		case Evt_ble_data:
			{
				wl_evt_ble_data_t *p_evt = (wl_evt_ble_data_t*)p_msg->data;
				len = sizeof(wl_msg_t)+sizeof(wl_evt_ble_data_t)+p_evt->len;
			} break;
		case Evt_ble_connection:
			len = sizeof(wl_msg_t)+sizeof(Evt_ble_connection);
			break;
		default:
			len = 0;
			break;
		}

		if (len == 0) {
			for (int i=0;i<rx_len;i++)
				printf("%02x ",rx_buf[i]);
			printf("\n");
			return;
		}

		if ( p_msg->msg_id == Cmd_BLE_GATT_SDP || 
			 p_msg->msg_id == Cmd_BLE_GATT_Read || 
			 p_msg->msg_id == Evt_sdr_ucst || 
			 p_msg->msg_id == Evt_ble_data) {

			//These are variable length rsp or event from IN612
			while( len ) {
				int sz = len<=MAX_FRAME_SZ ? len : MAX_FRAME_SZ;
				rq_push(RQ(wl_msg), rx_buf+offset, sz);
				len -= sz;
				rx_len -= sz;
				offset += sz;
			}
		} else {
			rq_push(RQ(wl_msg), rx_buf+offset, len);
			rx_len -= len;
			offset += len;
		}
	}
}

int gw_mng_handle_wl_msg(void)
{
	uint8_t data[MAX_FRAME_SZ] = {0};
	wl_msg_t *p_msg = (wl_msg_t*)data;
	int len = 0;

	if ( rq_pop(RQ(wl_msg), data, MAX_FRAME_SZ) != -1) {

		switch(p_msg->msg_id)
		{
		case Cmd_Reset:
			len = 0;
			break;
		case Cmd_Configure:
			len = sizeof(wl_msg_t)+sizeof(rsp_Configure_t);
			break;
		case Cmd_Start_Ctrl:
			len = sizeof(wl_msg_t)+sizeof(rsp_Start_Ctrl_t);
			break;
		case Cmd_Filter:
			len = sizeof(wl_msg_t)+sizeof(rsp_Filter_t);
			break;
		case Cmd_SDR_Mcst:
			len = sizeof(wl_msg_t)+sizeof(rsp_SDR_Mcst_t);
			break;
		case Cmd_SDR_Ucst:
			len = sizeof(wl_msg_t)+sizeof(rsp_SDR_Ucst_t);
			break;
		//BLE Central
		case Cmd_BLE_Connect:
			len = sizeof(wl_msg_t)+sizeof(rsp_BLE_Connect_t);
			break;
		case Cmd_BLE_GATT_SDP:
			{
				rsp_GATT_SDP_t *p_rsp = (rsp_GATT_SDP_t *)p_msg->data;
				len = sizeof(wl_msg_t)+sizeof(rsp_GATT_SDP_t)+p_rsp->len;
			} break;
		case Cmd_BLE_GATT_Read:
			{
				rsp_GATT_Read_t *p_rsp = (rsp_GATT_Read_t*)p_msg->data;
				len = sizeof(wl_msg_t)+sizeof(rsp_GATT_SDP_t)+p_rsp->len;
			} break;
		case Cmd_BLE_GATT_Write:
			len = sizeof(wl_msg_t)+sizeof(rsp_GATT_Write_t);
			break;
		//BLE Peripheral
		case Cmd_BLE_Adv:
			len = sizeof(wl_msg_t)+sizeof(rsp_BLE_Adv_t);
			break;
		case Cmd_BLE_GATT_Service:
			len = sizeof(wl_msg_t)+sizeof(rsp_GATT_Svc_t);
			break;
		case Cmd_BLE_GATT_Notify:
			len = sizeof(wl_msg_t)+sizeof(rsp_GATT_Notify_t);
			break;
		//Event
		case Evt_sdr_bcst:
		case Evt_ble_adv:
			len = MAX_FRAME_SZ;
			break;
		case Evt_sdr_ucst:
			{
				wl_evt_sdr_ucst_t *p_evt = (wl_evt_sdr_ucst_t *)p_msg->data;
				len = sizeof(wl_msg_t)+sizeof(wl_evt_sdr_ucst_t)+p_evt->len;
			} break;
		case Evt_ble_data:
			{
				wl_evt_ble_data_t *p_evt = (wl_evt_ble_data_t*)p_msg->data;
				len = sizeof(wl_msg_t)+sizeof(wl_evt_ble_data_t)+p_evt->len;
			} break;
		case Evt_ble_connection:
			len = sizeof(wl_msg_t)+sizeof(Evt_ble_connection);
			break;
		default:
			len = 0;
			break;
		}

		if ( p_msg->msg_id == Cmd_BLE_GATT_SDP || 
			 p_msg->msg_id == Cmd_BLE_GATT_Read || 
			 p_msg->msg_id == Evt_sdr_ucst || 
			 p_msg->msg_id == Evt_ble_data) {

			//These are variable length rsp or event from IN612
			uint8_t *p = NULL;
			int offset = 0;

			APP_MALLOC(p, uint8_t, len);
			while(len) {
				int sz = len<=MAX_FRAME_SZ ? len : MAX_FRAME_SZ;
				memcpy(p+offset, data, sz);
				len -= sz;
				offset += sz;

				if (len)
					rq_pop(RQ(wl_msg), data, MAX_FRAME_SZ);
			}
			p_msg = (wl_msg_t*)p;
		}

		/* send data to host */
		if ( len ) {
			if(usb_if_tx((uint8_t*)p_msg, len) != SUCCESS)
				printf("usb_vcp_send_data failed!\n");
		}

		if ((uint32_t)p_msg != (uint32_t)data)
			APP_MFREE(p_msg);

	}//if (rq_pop...)
}

//int gw_mng_handle_wl_msg(uint8_t **p)
//{
//	uint8_t data[MAX_FRAME_SZ] = {0};
//	wl_msg_t *p_msg = (wl_msg_t*)data;
//	int out_len = 0;

//	if ( rq_pop(RQ(wl_msg), data, MAX_FRAME_SZ) != -1) {
////putchar('p');
//		out_len = get_msg_len(p_msg->msg_id);

//		switch(p_msg->msg_id)
//		{
//		case Evt_sdr_bcst:
//		case Evt_ble_adv:
//			{
//				uint16_t time_stamp;
//				node_id_t *p_node;
//				wl_evt_sdr_bcst_t *p_sdr_bcst = (wl_evt_sdr_bcst_t*)p_msg->data;
//				wl_evt_ble_adv_t *p_ble_adv = (wl_evt_ble_adv_t*)p_msg->data;
//				out_len = 0;
//				
//				if (p_msg->msg_id == Evt_sdr_bcst) {
//					p_node = (node_id_t*)&p_sdr_bcst->node_id;
//					time_stamp = ((wl_evt_sdr_bcst_t*)p_msg->data)->time_stamp;
//				} else {
//					p_node = (node_id_t*)&p_ble_adv->node_id;
//					time_stamp = ((wl_evt_ble_adv_t*)p_msg->data)->time_stamp;
//				}
//		
//				if ( filter_duplicate(p_node, time_stamp) == false ) {
//					out_len = MAX_FRAME_SZ;
//				}
//			} break;
//		case 
//		}

//		/* send data to host */
//		if ( out_len ) {
//			for (int i=0;i<10;i++) {
//				if(usb_if_tx(data, out_len) == SUCCESS) {
//					//printf("usb_vcp_send_data done (%d)!\n", i);
//					break;
//				}

//				if (i == 10-1)
//					printf("usb_vcp_send_data failed!\n");
//			}
//				
//		}

//	}//if (rq_pop...)
//}
