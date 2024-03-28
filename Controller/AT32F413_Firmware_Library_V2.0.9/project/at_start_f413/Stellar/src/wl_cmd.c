/* includes ------------------------------------------------------------------*/
#include "at32f413.h"

#include "wl_share.h"
#include "usart_if.h"
//#include "misc.h"

//#include "semphr.h"

//#include "usart_task.h"

//typedef struct _wl_cmd_ {
//	struct _wl_cmd_ 	*next;

//	wl_node_t 		node_id;
//	wl_msg_rsp_handler_t rsp_handler;
//	wl_msg_t		wl_msg;
//} wl_cmd_t;

//typedef struct {
//	wl_cmd_t		*head;
//} wl_cmd_list_t;

//wl_cmd_list_t g_wl_cmds;

typedef struct {

	usart_type* scanner_usartx[3];
	usart_type* transceiver_usartx;

	int scanner_chip_idx[3];
	int scanner_state[3];
	int transceiver_chip_idx;
	int transceiver_state;
} wl_cmd_t;

wl_cmd_t g_wl_cmd = {
	.scanner_usartx = {USART1, USART2, USART3},
	.transceiver_usartx = UART4,
	.scanner_state = {0,0,0},
	.transceiver_state = 0,
};

/*
	Internal functions
*/
//static int insert_to_list(wl_cmd_t *p_cmd)
//{
//	wl_cmd_t *next = g_wl_cmds.head;

//	p_cmd->next = NULL;

//	if (next == NULL) {
//		g_wl_cmds.head = p_cmd;
//	} else {
//		while(1) {
//			if (next->next == NULL) {
//	
//				next->next = p_cmd;
//				return 0;
//			}

//			next = next->next;
//		}
//	}

//	return -1;
//}

//static wl_cmd_t * search_from_list(wl_node_t *p_node_id)
//{
//	wl_cmd_t *next;

//	next = g_wl_cmds.head;

//	while(next) {
//		if (next && !memcmp(&next->node_id, p_node_id, sizeof(wl_node_t)) )
//			break;

//		next = next->next;
//	}

//	return next;
//}

//static wl_cmd_t * pop_from_list(wl_node_t *p_node_id)
//{
//	wl_cmd_t *last;
//	wl_cmd_t *next;

//	last = NULL;
//	next = g_wl_cmds.head;

//	while(next) {
//		if (next && !memcmp(&next->node_id, p_node_id, sizeof(wl_node_t)) ) {
//			if (last == NULL)
//				g_wl_cmds.head = next->next;
//			else
//				last->next = next->next;
//			break;
//		}

//		last = next;
//		next = next->next;
//	}
//	return next;
//}

/*
	APIs
*/
int wl_msg_send(int cs, wl_msg_t *p_msg, uint16_t len)
{
	if (cs < 3) {
		if (g_wl_cmd.scanner_state[cs] == 0) {

			g_wl_cmd.scanner_state[cs] = 1;
			usart_send_data(g_wl_cmd.scanner_usartx[cs], (uint8_t*)p_msg, len );

			return 0;
		}
	} else {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
		if (g_wl_cmd.transceiver_state == 0) {
			g_wl_cmd.transceiver_state = 1;
			usart_send_data(g_wl_cmd.transceiver_usartx, (uint8_t*)p_msg, len );

			return 0;
		}
	}

	return -1;
}
