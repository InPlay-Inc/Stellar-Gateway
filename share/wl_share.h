/**
 ****************************************************************************************
 *
 * @file wl_share.h
 *
 * @brief header file for both Node, TRX Module and Base Controller in the system
 *
 * Copyright (C) Inplay Technologies Inc. 2022-2024
 *
 ****************************************************************************************
 */
 #ifndef WL_SHARE_H
#define WL_SHARE_H

#ifdef __AT32F413_H
#include "at32f413.h"
#else
#include "cmsis_os.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//#include "ble_app.h"

#define STELLA_MAGIC_WORD 0x494E  //"IN"

//#define MAX_NODE_COUNT 		1000//201
//#define MAX_FRAME_SZ	33
//#define MAX_SDR_BCST_PAYLOAD_LEN 22
//#define MAX_BLE_ADV_PAYLOAD_LEN 21
//#define MAX_SDR_UCST_PAYLOAD_LEN 23
#define MAX_NODE_COUNT 		1000
#define MAX_FRAME_SZ	43
#define MAX_SDR_BCST_PAYLOAD_LEN 32
#define MAX_BLE_ADV_PAYLOAD_LEN 31	//For iBeacon 27 bytes
#define MAX_SDR_UCST_PAYLOAD_LEN 35


#define PKT_BCST_ID 			0x0505			// Advertising Packet broadcast Id
#define PKT_PER_BCST_ID 	0x0506			// Periodic Advertising Packet broadcast Id
#define PKT_UCST_ID 			0x0507			// Packet unicast Id

//#define WL_NODE_MSG_ID 0x574c4e6d //"WLNm"

//#define MSG_WL_BASE (*(uint32_t*)"msgB")

#define BASE_STATE_NODE_REQ_TRAN 0x0001
#define BASE_STATE_NORMAL 0x0000

//#define BASE_ID 0x1e1193a0

/*************************************************************************************************
These are packages download from Transceiver
*************************************************************************************************/
#define PKT_NODE_LOC_ID			0x0A01
#define PKT_NODE_PRI_ID			0x0A02
#define PKT_NODE_MCAST_ID		0x0A04
#define PKT_NODE_DUMMY			0xFFFF


#define SIZE_OF_TYPE_EQUAL_TO(type,size)\
static inline char size_of_##type##_equal_to_##size()\
{\
    char __dummy1[sizeof(type)-size];\
    char __dummy2[size-sizeof(type)];\
    return __dummy1[-1]+__dummy2[-1];\
}

#ifndef __lpcode
#define __lpcode __attribute__((section("LPCODE")))
#endif
#ifndef __shmem
#define __shmem __attribute__((section("SHMEM")))
#endif

#define MAGIC_WORD 0x494E  //"IN"

//#define MAX_NODE_COUNT 		1000//201
//#define MAX_FRAME_SZ	33
//#define MAX_BCST_PAYLOAD_LEN 22
//#define MAX_UCST_PAYLOAD_LEN 23

#define PKT_BCST_ID 			0x0505			// Advertising Packet broadcast Id
#define PKT_PER_BCST_ID 	0x0506			// Periodic Advertising Packet broadcast Id
#define PKT_UCST_ID 			0x0507			// Packet unicast Id

enum module_type{
	MODULE_SCANNER=0,							// base stations perform SDR scan only
	MODULE_TRANSCEIVER,						// base station performs BLE advertising & BLE periodic advertising 
};

enum wl_msg_id {
	//General
	Cmd_Reset = 1,
	Cmd_Configure,
	Cmd_Start_Ctrl,
	Cmd_Filter,

	//SDR
	Cmd_SDR_Mcst,
	Cmd_SDR_Ucst,
	
	//BLE Central
	Cmd_BLE_Connect = 7,
	Cmd_BLE_GATT_SDP,
	Cmd_BLE_GATT_Read,
	Cmd_BLE_GATT_Write,

	//BLE Peripheral
	Cmd_BLE_Adv,
	Cmd_BLE_GATT_Service,
	Cmd_BLE_GATT_Notify,

	Cmd_max,

	Evt_sdr_bcst = 100,
	Evt_ble_adv,
	Evt_sdr_ucst,
	Evt_ble_data,
	Evt_ble_connection,

	Msg_max,

	//Internal msg id
	Evt_net,

//	EVT_cmp,
//	EVT_data,
//	EVT_net,
//	EVT_sdr_adv,
//	EVT_ble_adv,

//	CMD_statistic,
//	CMD_bconn,
//	CMD_ble_gatt,
};

/*
 * node definitions
*/
typedef enum {
	NODE_CLASS_NONE,
	NODE_CLASS_SDR,
	NODE_CLASS_BLE,
} wl_node_class_t;

typedef enum {
	BLE_NONE_CONNECTABLE,
	BLE_CONNECTABLE,
} ble_node_type_t;

/// node type is either base station or peripheral station
typedef enum {
	NODE_TYPE_NODE_BCST_ONLY = 0x04,
	NODE_TYPE_NODE_UCST 	 = 0x06,
} sdr_node_type_t;

typedef struct {
	struct _virtual_id_ {
		uint32_t 	virtual_addr;
		uint8_t 	group_id;
	}__attribute__ ((packed)) virtual_id;

	///@see sdr_node_type_t 
	uint8_t sdr_node_type;

}__attribute__ ((packed)) sdr_node_id_t;

typedef struct {
	uint8_t bd_addr[6];
}__attribute__ ((packed)) ble_node_id_t;

typedef struct {
	///@see wl_node_class_t
	uint8_t node_class;

	union {
		sdr_node_id_t sdr_node_id;
		ble_node_id_t ble_node_id;
	}__attribute__ ((packed)) u;

}__attribute__ ((packed)) wl_node_t;

/*
 * wl command structures
*/
// 1. Reset
typedef struct {
	uint8_t option;
}__attribute__ ((packed)) cmd_RESET_t;

// 2. Configure
//typedef struct {
//	uint32_t sdr_bcst_aa;
//}__attribute__ ((packed)) cmd_SCNR_Configure_t;

//typedef struct {
//	uint32_t sdr_bcst_aa;
//	uint32_t sdr_ucst_aa;
//	uint16_t sdr_gw_addr;
//}__attribute__ ((packed)) cmd_TRX_Configure_t;

typedef struct {
	uint16_t 	result;
}__attribute__ ((packed)) rsp_Configure_t;

typedef struct {
	uint8_t module_type;

	uint32_t sdr_bcst_aa;
	uint32_t sdr_ucst_aa;
	uint16_t sdr_gw_addr;

//	union {
//		cmd_SCNR_Configure_t 	scnr_cfg;
//		cmd_TRX_Configure_t 	trx_cfg;
//	}__attribute__ ((packed)) cfg;

}__attribute__ ((packed)) cmd_Configure_t;

// 3. Start/Stop Control
typedef struct {
	uint8_t		sdr_chn;
	uint16_t 	sdr_scan_wnd;
	uint8_t		ble_chn;
	uint16_t 	ble_scan_wnd;
	uint8_t		phy;
}__attribute__ ((packed)) SCNR_Start_Ctrl_t;

typedef struct {
	uint16_t 	sync_intv;
}__attribute__ ((packed)) TRX_Start_Ctrl_t;

typedef struct {
	uint8_t module;

	union {
		SCNR_Start_Ctrl_t scnr_ctrl;
		TRX_Start_Ctrl_t trx_ctrl;
	}__attribute__ ((packed)) ctrl;
	
}__attribute__ ((packed)) cmd_Start_Ctrl_t;

typedef struct {
	uint16_t 	result;
}__attribute__ ((packed)) rsp_Start_Ctrl_t;

// 4. Set Scanner Filter
typedef struct {
	uint8_t len;
	uint8_t data[];
}__attribute__ ((packed)) cmd_Filter_t;

typedef struct {
	uint16_t 	result;
}__attribute__ ((packed)) rsp_Filter_t;

// 1. SDR Multicast Data
typedef struct {
	uint8_t group_id;
	uint16_t duration;
	uint8_t len;
	uint8_t data[];
}__attribute__ ((packed)) cmd_SDR_Mcst_t;

typedef struct {
	uint16_t result;
}__attribute__ ((packed)) rsp_SDR_Mcst_t;

// 2. SDR Peer-to-Peer Send Data
typedef struct {
	sdr_node_id_t node_id;
	uint16_t tmo;
	uint8_t len;
	uint8_t data[];
}__attribute__ ((packed)) cmd_SDR_Ucst_t;

typedef struct {
	uint16_t result;
	sdr_node_id_t node_id;
}__attribute__ ((packed)) rsp_SDR_Ucst_t;

// 1. BLE Advertising
typedef struct {
	uint16_t 	adv_intv;
	uint16_t 	duration;
}__attribute__ ((packed)) BLE_Adv_param_t;

typedef struct {
	BLE_Adv_param_t	param;
}__attribute__ ((packed)) cmd_BLE_Adv_t;

typedef struct {
	uint16_t 	result;
}__attribute__ ((packed)) rsp_BLE_Adv_t;

// 2 BLE Connect
typedef struct {
	uint16_t 		conn_intv;
	uint16_t 		spv_tmo;
	uint16_t 		latency;
}__attribute__ ((packed)) BLE_Connect_param_t;

typedef struct {
	uint16_t		tmo;
	ble_node_id_t	node_id;
	BLE_Connect_param_t param;
}__attribute__ ((packed)) cmd_BLE_Connect_t;

typedef struct {
	uint16_t		result;
	ble_node_id_t	node_id;
}__attribute__ ((packed)) rsp_BLE_Connect_t;

// 3 BLE Disconnect
//typedef struct {
//	uint16_t		tmo;
//	ble_node_id_t	node_id;
//}__attribute__ ((packed)) cmd_BLE_Disconnect_t;

//typedef struct {
//	uint16_t		result;
//	ble_node_id_t	node_id;
//}__attribute__ ((packed)) rsp_BLE_Disconnect_t;

// 4 GATT Create Service
typedef struct {
	uint8_t 		svc_uuid[16];
}__attribute__ ((packed)) GATT_Svc_param_t;

typedef struct {
	GATT_Svc_param_t	param;
}__attribute__ ((packed)) cmd_GATT_Svc_t;

typedef struct {
	uint16_t 		result;
	uint16_t 		hdl;
}__attribute__ ((packed)) rsp_GATT_Svc_t;

// 5 GATT Service Discovery
typedef struct {
	uint8_t 		svc_uuid[16];
}__attribute__ ((packed)) GATT_SDP_param_t;

typedef struct {
	ble_node_id_t	node_id;
	GATT_SDP_param_t param;
}__attribute__ ((packed)) cmd_GATT_SDP_t;

typedef struct {
	uint16_t		result;
	ble_node_id_t	node_id;
	uint16_t 		len;
	uint8_t 		data[];
}__attribute__ ((packed)) rsp_GATT_SDP_t;

// 6 GATT Read
typedef struct {
	uint16_t 		hdl;
	uint8_t 		offset;
	uint8_t 		len;
	uint8_t 		data[];
}__attribute__ ((packed)) GATT_Read_param_t;

typedef struct {
	ble_node_id_t	node_id;
	GATT_Read_param_t param;
}__attribute__ ((packed)) cmd_GATT_Read_t;

typedef struct {
	int 			result;
	ble_node_id_t	node_id;
	uint8_t 		len;
	uint8_t 		data[];
}__attribute__ ((packed)) rsp_GATT_Read_t;

// 7. GATT Write
typedef struct {
	uint16_t 		hdl;
	uint8_t 		offset;
	uint8_t 		len;
	uint8_t 		data[];
}__attribute__ ((packed)) GATT_Write_param_t;

typedef struct {
	ble_node_id_t	node_id;
	GATT_Write_param_t param;
}__attribute__ ((packed)) cmd_GATT_Write_t;

typedef struct {
	uint16_t 		result;
	ble_node_id_t	node_id;
}__attribute__ ((packed)) rsp_GATT_Write_t;

// 8. GATT Notify
typedef struct {
	uint16_t 		hdl;
	uint8_t 		len;
	uint8_t 		data[];
}__attribute__ ((packed)) GATT_Notify_param_t;

typedef struct {
	ble_node_id_t 		node_id;
	uint16_t 			tmo;
	GATT_Notify_param_t param;
}__attribute__ ((packed)) cmd_GATT_Notify_t;

typedef struct {
	uint16_t 			result;
	ble_node_id_t 		node_id;
}__attribute__ ((packed)) rsp_GATT_Notify_t;

//typedef struct {
//	uint8_t 		op_code;
//	uint16_t		op_tmo;
//	ble_node_id_t	node_id;

//	union {
//		cmd_ble_op_connect_t 		conn_op;
//		cmd_ble_op_disconnect_t 	disc_op;
//		cmd_ble_op_gatt_sdp_t		sdp_op;
//		cmd_ble_op_gatt_read_t		rd_op;
//		cmd_ble_op_gatt_write_t 	wr_op;
//	}__attribute__ ((packed)) u;

//}__attribute__ ((packed)) cmd_BLE_OP_t;

//typedef struct {
//	ble_node_id_t	node_id;
//	uint8_t 		op_code;
//	uint16_t 		result;
//	uint16_t 		len;
//	uint8_t 		data[];
//}__attribute__ ((packed)) rsp_BLE_OP_t;

//typedef struct {
//	union {
//		ble_op_connect_t 		conn_op;
//		ble_op_disconnect_t 	disc_op;
//		ble_op_gatt_sdp_t		sdp_op;
//		ble_op_gatt_read_t		rd_op;
//		ble_op_gatt_write_t 	wr_op;
//	}__attribute__ ((packed)) u;

//}__attribute__ ((packed)) wl_msg_BLE_OP_t;

typedef struct {
	sdr_node_id_t node_id;
	int8_t rssi;
	uint16_t time_stamp;
	uint8_t len;
	uint8_t payload[MAX_SDR_BCST_PAYLOAD_LEN];
}__attribute__ ((packed)) wl_evt_sdr_bcst_t;

typedef struct {
	ble_node_id_t node_id;
	int8_t rssi;
	uint16_t time_stamp;
	uint8_t flag;
	uint8_t len;
	uint8_t payload[MAX_BLE_ADV_PAYLOAD_LEN];
}__attribute__ ((packed)) wl_evt_ble_adv_t;

typedef struct {
	sdr_node_id_t node_id;
//	uint8_t  data_type;
	uint8_t len;
	uint8_t data[];
}__attribute__ ((packed)) wl_evt_sdr_ucst_t;

typedef struct {
	ble_node_id_t 	node_id;
	uint8_t 		connection;
}__attribute__ ((packed)) wl_evt_ble_connection_t;

typedef struct {
	ble_node_id_t 	node_id;
	uint16_t 		handler;
	uint8_t 		len;
	uint8_t 		data[];
}__attribute__ ((packed)) wl_evt_ble_data_t;

typedef struct {
	sdr_node_id_t 	node_id;
	uint8_t 		reason;
}__attribute__ ((packed)) wl_evt_net_t;

typedef struct {

	/// @see enum wl_msg_id
	uint8_t msg_id;

	//uint16_t len;
	uint8_t data[];
	
}__attribute__ ((packed)) wl_msg_t;

//typedef struct {
//	uint8_t msg_id;

//	union {
//		rsp_Configure_t 		rsp_Configure;
//		rsp_Start_Ctrl_t 		rsp_Start_Ctrl;
//		rsp_Filter_t			rsp_Filter;
//		rsp_SDR_Mcst_t			rsp_SDR_Mcst;
//		rsp_SDR_Ucst_t			rsp_SDR_Ucst;
//		rsp_BLE_Adv_t			rsp_BLE_Adv;
//		rsp_BLE_Connect_t		rsp_BLE_Connect;
//		rsp_BLE_Disconnect_t	rsp_BLE_Disconnect;
//		rsp_GATT_Svc_t			rsp_GATT_Svc;
//		rsp_GATT_SDP_t			rsp_GATT_SDP;
//		rsp_GATT_Read_t			rsp_GATT_Read;
//		rsp_GATT_Write_t		rsp_GATT_Write;
//		rsp_GATT_Notify_t		rsp_GATT_Notify;
//	
//		uint16_t 				result;
//	}__attribute__ ((packed)) u;
//}__attribute__ ((packed)) wl_rsp;

typedef void (*wl_msg_rsp_handler_t)(wl_msg_t *p_msg);

/*************************************************************************************************
data between Transceiver & Controller
*************************************************************************************************/
enum
{
    /// No error
    WL_ERR_NO_ERROR                                                               = 0x00,

    // ----------------------------------------------------------------------------------
    // -------------------------  ATT Specific Error ------------------------------------
    // ----------------------------------------------------------------------------------
    /// No error
    WL_ATT_ERR_NO_ERROR                                                               = 0x00,
    /// 0x01: Handle is invalid
    WL_ATT_ERR_INVALID_HANDLE                                                         = 0x01,
    /// 0x02: Read permission disabled
    WL_ATT_ERR_READ_NOT_PERMITTED                                                     = 0x02,
    /// 0x03: Write permission disabled
    WL_ATT_ERR_WRITE_NOT_PERMITTED                                                    = 0x03,
    /// 0x04: Incorrect PDU
    WL_ATT_ERR_INVALID_PDU                                                            = 0x04,
    /// 0x05: Authentication privilege not enough
    WL_ATT_ERR_INSUFF_AUTHEN                                                          = 0x05,
    /// 0x06: Request not supported or not understood
    WL_ATT_ERR_REQUEST_NOT_SUPPORTED                                                  = 0x06,
    /// 0x07: Incorrect offset value
    WL_ATT_ERR_INVALID_OFFSET                                                         = 0x07,
    /// 0x08: Authorization privilege not enough
    WL_ATT_ERR_INSUFF_AUTHOR                                                          = 0x08,
    /// 0x09: Capacity queue for reliable write reached
    WL_ATT_ERR_PREPARE_QUEUE_FULL                                                     = 0x09,
    /// 0x0A: Attribute requested not existing
    WL_ATT_ERR_ATTRIBUTE_NOT_FOUND                                                    = 0x0A,
    /// 0x0B: Attribute requested not long
    WL_ATT_ERR_ATTRIBUTE_NOT_LONG                                                     = 0x0B,
    /// 0x0C: Encryption size not sufficient
    WL_ATT_ERR_INSUFF_ENC_KEY_SIZE                                                    = 0x0C,
    /// 0x0D: Invalid length of the attribute value
    WL_ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN                                              = 0x0D,
    /// 0x0E: Operation not fit to condition
    WL_ATT_ERR_UNLIKELY_ERR                                                           = 0x0E,
    /// 0x0F: Attribute requires encryption before operation
    WL_ATT_ERR_INSUFF_ENC                                                             = 0x0F,
    /// 0x10: Attribute grouping not supported
    WL_ATT_ERR_UNSUPP_GRP_TYPE                                                        = 0x10,
    /// 0x11: Resources not sufficient to complete the request
    WL_ATT_ERR_INSUFF_RESOURCE                                                        = 0x11,
    /// 0x80: Application error (also used in PRF Errors)
    WL_ATT_ERR_APP_ERROR                                                              = 0x80,

    // ----------------------------------------------------------------------------------
    // -------------------------- L2C Specific Error ------------------------------------
    // ----------------------------------------------------------------------------------
    /// Message cannot be sent because connection lost. (disconnected)
    WL_L2C_ERR_CONNECTION_LOST                                                        = 0x30,
    /// Invalid PDU length exceed MTU
    WL_L2C_ERR_INVALID_MTU_EXCEED                                                     = 0x31,
    /// Invalid PDU length exceed MPS
    WL_L2C_ERR_INVALID_MPS_EXCEED                                                     = 0x32,
    /// Invalid Channel ID
    WL_L2C_ERR_INVALID_CID                                                            = 0x33,
    /// Invalid PDU
    WL_L2C_ERR_INVALID_PDU                                                            = 0x34,
    /// Connection refused - no resources available
    WL_L2C_ERR_NO_RES_AVAIL                                                           = 0x35,
    /// Connection refused - insufficient authentication
    WL_L2C_ERR_INSUFF_AUTHEN                                                          = 0x36,
    /// Connection refused - insufficient authorization
    WL_L2C_ERR_INSUFF_AUTHOR                                                          = 0x37,
    /// Connection refused - insufficient encryption key size
    WL_L2C_ERR_INSUFF_ENC_KEY_SIZE                                                    = 0x38,
    /// Connection Refused - insufficient encryption
    WL_L2C_ERR_INSUFF_ENC                                                             = 0x39,
    /// Connection refused - LE_PSM not supported
    WL_L2C_ERR_LEPSM_NOT_SUPP                                                         = 0x3A,
    /// No more credit
    WL_L2C_ERR_INSUFF_CREDIT                                                          = 0x3B,
    /// Command not understood by peer device
    WL_L2C_ERR_NOT_UNDERSTOOD                                                         = 0x3C,
    /// Credit error, invalid number of credit received
    WL_L2C_ERR_CREDIT_ERROR                                                           = 0x3D,
    /// Channel identifier already allocated
    WL_L2C_ERR_CID_ALREADY_ALLOC                                                      = 0x3E,


    // ----------------------------------------------------------------------------------
    // -------------------------- GAP Specific Error ------------------------------------
    // ----------------------------------------------------------------------------------
    /// Invalid parameters set
    WL_GAP_ERR_INVALID_PARAM                                                          = 0x40,
    /// Problem with protocol exchange, get unexpected response
    WL_GAP_ERR_PROTOCOL_PROBLEM                                                       = 0x41,
    /// Request not supported by software configuration
    WL_GAP_ERR_NOT_SUPPORTED                                                          = 0x42,
    /// Request not allowed in current state.
    WL_GAP_ERR_COMMAND_DISALLOWED                                                     = 0x43,
    /// Requested operation canceled.
    WL_GAP_ERR_CANCELED                                                               = 0x44,
    /// Requested operation timeout.
    WL_GAP_ERR_TIMEOUT                                                                = 0x45,
    /// Link connection lost during operation.
    WL_GAP_ERR_DISCONNECTED                                                           = 0x46,
    /// Search algorithm finished, but no result found
    WL_GAP_ERR_NOT_FOUND                                                              = 0x47,
    /// Request rejected by peer device
    WL_GAP_ERR_REJECTED                                                               = 0x48,
    /// Problem with privacy configuration
    WL_GAP_ERR_PRIVACY_CFG_PB                                                         = 0x49,
    /// Duplicate or invalid advertising data
    WL_GAP_ERR_ADV_DATA_INVALID                                                       = 0x4A,
    /// Insufficient resources
    WL_GAP_ERR_INSUFF_RESOURCES                                                       = 0x4B,
    /// Unexpected Error
    WL_GAP_ERR_UNEXPECTED                                                             = 0x4C,
    /// Feature mismatch
    WL_GAP_ERR_MISMATCH                                                               = 0x4D,

    // ----------------------------------------------------------------------------------
    // ------------------------- GATT Specific Error ------------------------------------
    // ----------------------------------------------------------------------------------
    /// Problem with ATTC protocol response
    WL_GATT_ERR_INVALID_ATT_LEN                                                       = 0x50,
    /// Error in service search
    WL_GATT_ERR_INVALID_TYPE_IN_SVC_SEARCH                                            = 0x51,
    /// Invalid write data
    WL_GATT_ERR_WRITE                                                                 = 0x52,
    /// Signed write error
    WL_GATT_ERR_SIGNED_WRITE                                                          = 0x53,
    /// No attribute client defined
    WL_GATT_ERR_ATTRIBUTE_CLIENT_MISSING                                              = 0x54,
    /// No attribute server defined
    WL_GATT_ERR_ATTRIBUTE_SERVER_MISSING                                              = 0x55,
    /// Permission set in service/attribute are invalid
    WL_GATT_ERR_INVALID_PERM                                                          = 0x56,

    // ----------------------------------------------------------------------------------
    // ------------------------- SMP Specific Error -------------------------------------
    // ----------------------------------------------------------------------------------
    // SMP Protocol Errors detected on local device
    /// The user input of pass key failed, for example, the user canceled the operation.
    WL_SMP_ERROR_LOC_PASSKEY_ENTRY_FAILED                                             = 0x61,
    /// The OOB Data is not available.
    WL_SMP_ERROR_LOC_OOB_NOT_AVAILABLE                                                = 0x62,
    /// The pairing procedure cannot be performed as authentication requirements cannot be met
    /// due to IO capabilities of one or both devices.
    WL_SMP_ERROR_LOC_AUTH_REQ                                                         = 0x63,
    /// The confirm value does not match the calculated confirm value.
    WL_SMP_ERROR_LOC_CONF_VAL_FAILED                                                  = 0x64,
    /// Pairing is not supported by the device.
    WL_SMP_ERROR_LOC_PAIRING_NOT_SUPP                                                 = 0x65,
    /// The resultant encryption key size is insufficient for the security requirements of
    /// this device.
    WL_SMP_ERROR_LOC_ENC_KEY_SIZE                                                     = 0x66,
    /// The SMP command received is not supported on this device.
    WL_SMP_ERROR_LOC_CMD_NOT_SUPPORTED                                                = 0x67,
    /// Pairing failed due to an unspecified reason.
    WL_SMP_ERROR_LOC_UNSPECIFIED_REASON                                               = 0x68,
    /// Pairing or Authentication procedure is disallowed because too little time has elapsed
    /// since last pairing request or security request.
    WL_SMP_ERROR_LOC_REPEATED_ATTEMPTS                                                = 0x69,
    /// The command length is invalid or a parameter is outside of the specified range.
    WL_SMP_ERROR_LOC_INVALID_PARAM                                                    = 0x6A,
    /// Indicates to the remote device that the DHKey Check value received doesn't
    /// match the one calculated by the local device.
    WL_SMP_ERROR_LOC_DHKEY_CHECK_FAILED                                               = 0x6B,
    /// Indicates that the confirm values in the numeric comparison protocol do not match.
    WL_SMP_ERROR_LOC_NUMERIC_COMPARISON_FAILED                                        = 0x6C,
    /// Indicates that the pairing over the LE transport failed due to a Pairing Request sent
    /// over the BR/EDR transport in process.
    WL_SMP_ERROR_LOC_BREDR_PAIRING_IN_PROGRESS                                        = 0x6D,
    /// Indicates that the BR/EDR Link Key generated on the BR/EDR transport cannot be
    /// used to derive and distribute keys for the LE transport.
    WL_SMP_ERROR_LOC_CROSS_TRANSPORT_KEY_GENERATION_NOT_ALLOWED                       = 0x6E,
    // SMP Protocol Errors detected by remote device
    /// The user input of passkey failed, for example, the user canceled the operation.
    WL_SMP_ERROR_REM_PASSKEY_ENTRY_FAILED                                             = 0x71,
    /// The OOB Data is not available.
    WL_SMP_ERROR_REM_OOB_NOT_AVAILABLE                                                = 0x72,
    /// The pairing procedure cannot be performed as authentication requirements cannot be
    /// met due to IO capabilities of one or both devices.
    WL_SMP_ERROR_REM_AUTH_REQ                                                         = 0x73,
    /// The confirm value does not match the calculated confirm value.
    WL_SMP_ERROR_REM_CONF_VAL_FAILED                                                  = 0x74,
    /// Pairing is not supported by the device.
    WL_SMP_ERROR_REM_PAIRING_NOT_SUPP                                                 = 0x75,
    /// The resultant encryption key size is insufficient for the security requirements of
    /// this device.
    WL_SMP_ERROR_REM_ENC_KEY_SIZE                                                     = 0x76,
    /// The SMP command received is not supported on this device.
    WL_SMP_ERROR_REM_CMD_NOT_SUPPORTED                                                = 0x77,
    /// Pairing failed due to an unspecified reason.
    WL_SMP_ERROR_REM_UNSPECIFIED_REASON                                               = 0x78,
    /// Pairing or Authentication procedure is disallowed because too little time has elapsed
    /// since last pairing request or security request.
    WL_SMP_ERROR_REM_REPEATED_ATTEMPTS                                                = 0x79,
    /// The command length is invalid or a parameter is outside of the specified range.
    WL_SMP_ERROR_REM_INVALID_PARAM                                                    = 0x7A,
    /// Indicates to the remote device that the DHKey Check value received doesn't
    /// match the one calculated by the local device.
    WL_SMP_ERROR_REM_DHKEY_CHECK_FAILED                                               = 0x7B,
    /// Indicates that the confirm values in the numeric comparison protocol do not match.
    WL_SMP_ERROR_REM_NUMERIC_COMPARISON_FAILED                                        = 0x7C,
    /// Indicates that the pairing over the LE transport failed due to a Pairing Request sent
    /// over the BR/EDR transport in process.
    WL_SMP_ERROR_REM_BREDR_PAIRING_IN_PROGRESS                                        = 0x7D,
    /// Indicates that the BR/EDR Link Key generated on the BR/EDR transport cannot be
    /// used to derive and distribute keys for the LE transport.
    WL_SMP_ERROR_REM_CROSS_TRANSPORT_KEY_GENERATION_NOT_ALLOWED                       = 0x7E,
    // SMP Errors triggered by local device
    /// The provided resolvable address has not been resolved.
    WL_SMP_ERROR_ADDR_RESOLV_FAIL                                                     = 0xD0,
    /// The Signature Verification Failed
    WL_SMP_ERROR_SIGN_VERIF_FAIL                                                      = 0xD1,
    /// The encryption procedure failed because the slave device didn't find the LTK
    /// needed to start an encryption session.
    WL_SMP_ERROR_ENC_KEY_MISSING                                                      = 0xD2,
    /// The encryption procedure failed because the slave device doesn't support the
    /// encryption feature.
    WL_SMP_ERROR_ENC_NOT_SUPPORTED                                                    = 0xD3,
    /// A timeout has occurred during the start encryption session.
    WL_SMP_ERROR_ENC_TIMEOUT                                                          = 0xD4,

    // ----------------------------------------------------------------------------------
    //------------------------ Profiles specific error codes ----------------------------
    // ----------------------------------------------------------------------------------
    /// Application Error
    WL_PRF_APP_ERROR                                                                  = 0x80,
    /// Invalid parameter in request
    WL_PRF_ERR_INVALID_PARAM                                                          = 0x81,
    /// Inexistent handle for sending a read/write characteristic request
    WL_PRF_ERR_INEXISTENT_HDL                                                         = 0x82,
    /// Discovery stopped due to missing attribute according to specification
    WL_PRF_ERR_STOP_DISC_CHAR_MISSING                                                 = 0x83,
    /// Too many SVC instances found -> protocol violation
    WL_PRF_ERR_MULTIPLE_SVC                                                           = 0x84,
    /// Discovery stopped due to found attribute with incorrect properties
    WL_PRF_ERR_STOP_DISC_WRONG_CHAR_PROP                                              = 0x85,
    /// Too many Char. instances found-> protocol violation
    WL_PRF_ERR_MULTIPLE_CHAR                                                          = 0x86,
    /// Attribute write not allowed
    WL_PRF_ERR_NOT_WRITABLE                                                           = 0x87,
    /// Attribute read not allowed
    WL_PRF_ERR_NOT_READABLE                                                           = 0x88,
    /// Request not allowed
    WL_PRF_ERR_REQ_DISALLOWED                                                         = 0x89,
    /// Notification Not Enabled
    WL_PRF_ERR_NTF_DISABLED                                                           = 0x8A,
    /// Indication Not Enabled
    WL_PRF_ERR_IND_DISABLED                                                           = 0x8B,
    /// Feature not supported by profile
    WL_PRF_ERR_FEATURE_NOT_SUPPORTED                                                  = 0x8C,
    /// Read value has an unexpected length
    WL_PRF_ERR_UNEXPECTED_LEN                                                         = 0x8D,
    /// Disconnection occurs
    WL_PRF_ERR_DISCONNECTED                                                           = 0x8E,
    /// Procedure Timeout
    WL_PRF_ERR_PROC_TIMEOUT                                                           = 0x8F,
    /// Client characteristic configuration improperly configured
    WL_PRF_CCCD_IMPR_CONFIGURED                                                       = 0xFD,
    /// Procedure already in progress
    WL_PRF_PROC_IN_PROGRESS                                                           = 0xFE,
    /// Out of Range
    WL_PRF_OUT_OF_RANGE                                                               = 0xFF,

    // ----------------------------------------------------------------------------------
    //-------------------- LL Error codes conveyed to upper layer <co_error.h> -------------------
    // ----------------------------------------------------------------------------------
    /// Unknown HCI Command
    WL_LL_ERR_UNKNOWN_HCI_COMMAND                                                     = 0x91,
    /// Unknown Connection Identifier
    WL_LL_ERR_UNKNOWN_CONNECTION_ID                                                   = 0x92,
    /// Hardware Failure
    WL_LL_ERR_HARDWARE_FAILURE                                                        = 0x93,
    /// BT Page Timeout
    WL_LL_ERR_PAGE_TIMEOUT                                                            = 0x94,
    /// Authentication failure
    WL_LL_ERR_AUTH_FAILURE                                                            = 0x95,
    /// Pin code missing
    WL_LL_ERR_PIN_MISSING                                                             = 0x96,
    /// Memory capacity exceed
    WL_LL_ERR_MEMORY_CAPA_EXCEED                                                      = 0x97,
    /// Connection Timeout
    WL_LL_ERR_CON_TIMEOUT                                                             = 0x98,
    /// Connection limit Exceed
    WL_LL_ERR_CON_LIMIT_EXCEED                                                        = 0x99,
    /// Synchronous Connection limit exceed
    WL_LL_ERR_SYNC_CON_LIMIT_DEV_EXCEED                                               = 0x9A,
    /// ACL Connection exits
    WL_LL_ERR_ACL_CON_EXISTS                                                          = 0x9B,
    /// Command Disallowed
    WL_LL_ERR_COMMAND_DISALLOWED                                                      = 0x9C,
    /// Connection rejected due to limited resources
    WL_LL_ERR_CONN_REJ_LIMITED_RESOURCES                                              = 0x9D,
    /// Connection rejected due to security reason
    WL_LL_ERR_CONN_REJ_SECURITY_REASONS                                               = 0x9E,
    /// Connection rejected due to unacceptable BD Addr
    WL_LL_ERR_CONN_REJ_UNACCEPTABLE_BDADDR                                            = 0x9F,
    /// Connection rejected due to Accept connection timeout
    WL_LL_ERR_CONN_ACCEPT_TIMEOUT_EXCEED                                              = 0xA0,
    /// Not Supported
    WL_LL_ERR_UNSUPPORTED                                                             = 0xA1,
    /// invalid parameters
    WL_LL_ERR_INVALID_HCI_PARAM                                                       = 0xA2,
    /// Remote user terminate connection
    WL_LL_ERR_REMOTE_USER_TERM_CON                                                    = 0xA3,
    /// Remote device terminate connection due to low resources
    WL_LL_ERR_REMOTE_DEV_TERM_LOW_RESOURCES                                           = 0xA4,
    /// Remote device terminate connection due to power off
    WL_LL_ERR_REMOTE_DEV_POWER_OFF                                                    = 0xA5,
    /// Connection terminated by local host
    WL_LL_ERR_CON_TERM_BY_LOCAL_HOST                                                  = 0xA6,
    /// Repeated attempts
    WL_LL_ERR_REPEATED_ATTEMPTS                                                       = 0xA7,
    /// Pairing not Allowed
    WL_LL_ERR_PAIRING_NOT_ALLOWED                                                     = 0xA8,
    /// Unknown PDU Error
    WL_LL_ERR_UNKNOWN_LMP_PDU                                                         = 0xA9,
    /// Unsupported remote feature
    WL_LL_ERR_UNSUPPORTED_REMOTE_FEATURE                                              = 0xAA,
    /// Sco Offset rejected
    WL_LL_ERR_SCO_OFFSET_REJECTED                                                     = 0xAB,
    /// SCO Interval Rejected
    WL_LL_ERR_SCO_INTERVAL_REJECTED                                                   = 0xAC,
    /// SCO air mode Rejected
    WL_LL_ERR_SCO_AIR_MODE_REJECTED                                                   = 0xAD,
    /// Invalid LMP parameters
    WL_LL_ERR_INVALID_LMP_PARAM                                                       = 0xAE,
    /// Unspecified error
    WL_LL_ERR_UNSPECIFIED_ERROR                                                       = 0xAF,
    /// Unsupported LMP Parameter value
    WL_LL_ERR_UNSUPPORTED_LMP_PARAM_VALUE                                             = 0xB0,
    /// Role Change Not allowed
    WL_LL_ERR_ROLE_CHANGE_NOT_ALLOWED                                                 = 0xB1,
    /// LMP Response timeout
    WL_LL_ERR_LMP_RSP_TIMEOUT                                                         = 0xB2,
    /// LMP Collision
    WL_LL_ERR_LMP_COLLISION                                                           = 0xB3,
    /// LMP Pdu not allowed
    WL_LL_ERR_LMP_PDU_NOT_ALLOWED                                                     = 0xB4,
    /// Encryption mode not accepted
    WL_LL_ERR_ENC_MODE_NOT_ACCEPT                                                     = 0xB5,
    /// Link Key Cannot be changed
    WL_LL_ERR_LINK_KEY_CANT_CHANGE                                                    = 0xB6,
    /// Quality of Service not supported
    WL_LL_ERR_QOS_NOT_SUPPORTED                                                       = 0xB7,
    /// Error, instant passed
    WL_LL_ERR_INSTANT_PASSED                                                          = 0xB8,
    /// Pairing with unit key not supported
    WL_LL_ERR_PAIRING_WITH_UNIT_KEY_NOT_SUP                                           = 0xB9,
    /// Transaction collision
    WL_LL_ERR_DIFF_TRANSACTION_COLLISION                                              = 0xBA,
    /// Unacceptable parameters
    WL_LL_ERR_QOS_UNACCEPTABLE_PARAM                                                  = 0xBC,
    /// Quality of Service rejected
    WL_LL_ERR_QOS_REJECTED                                                            = 0xBD,
    /// Channel class not supported
    WL_LL_ERR_CHANNEL_CLASS_NOT_SUP                                                   = 0xBE,
    /// Insufficient security
    WL_LL_ERR_INSUFFICIENT_SECURITY                                                   = 0xBF,
    /// Parameters out of mandatory range
    WL_LL_ERR_PARAM_OUT_OF_MAND_RANGE                                                 = 0xC0,
    /// Role switch pending
    WL_LL_ERR_ROLE_SWITCH_PEND                                                        = 0xC2,
    /// Reserved slot violation
    WL_LL_ERR_RESERVED_SLOT_VIOLATION                                                 = 0xC4,
    /// Role Switch fail
    WL_LL_ERR_ROLE_SWITCH_FAIL                                                        = 0xC5,
    /// Error, EIR too large
    WL_LL_ERR_EIR_TOO_LARGE                                                           = 0xC6,
    /// Simple pairing not supported by host
    WL_LL_ERR_SP_NOT_SUPPORTED_HOST                                                   = 0xC7,
    /// Host pairing is busy
    WL_LL_ERR_HOST_BUSY_PAIRING                                                       = 0xC8,
    /// Controller is busy
    WL_LL_ERR_CONTROLLER_BUSY                                                         = 0xCA,
    /// Unacceptable connection initialization
    WL_LL_ERR_UNACCEPTABLE_CONN_INT                                                   = 0xCB,
    /// Direct Advertising Timeout
    WL_LL_ERR_DIRECT_ADV_TO                                                           = 0xCC,
    /// Connection Terminated due to a MIC failure
    WL_LL_ERR_TERMINATED_MIC_FAILURE                                                  = 0xCD,
    /// Connection failed to be established
    WL_LL_ERR_CONN_FAILED_TO_BE_EST                                                   = 0xCE,

	/// Can't allocate memory error
	WL_PLT_ERR_NO_MEM					= 0x100,
	/// Can't allocate semaphore error
	WL_PLT_ERR_OS_SEMAPHORE 	= 0x101,
	/// Can't allocate mutex error
	WL_PLT_ERR_OS_MUTEX 	= 0x102,
	/// Can't allocate task error
	WL_PLT_ERR_OS_TASK 	= 0x103,
	/// Can't allocate message queue
	WL_PLT_ERR_OS_QUE 	= 0x104,
	/// OS wait function failed
	WL_PLT_ERR_OS_WAIT 	= 0x105,
	/// OS timer create failed
	WL_PLT_ERR_OS_TMR_CREATE 	= 0x106,

	/// Invalid parameters
	WL_PLT_ERR_INVALID_PARAM 	= 0x110,
	/// platform not ready
	WL_PLT_ERR_NOT_READY	= 0x111,

	/// Invalid EM configuration
	WL_PLT_ERR_INVALID_CONFIG = 0x120,
	WL_PLT_ERR_EM_NO_MEM = 0x121,

	/// Invalid BLE memory configuration
	WL_PLT_ERR_NOT_ENOUGH_MEM	= 0x122,

	WL_ERR_CMD_EXE		= 		0x1001,
	WL_ERR_CMDID		=		0x100A,
	WL_ERR_CMD_PARAMETER = 		0x100D,
	WL_ERR_BUSY			=		0x100B,
	WL_ERR_CMD_TIMEOUT	=		0x100E,
};

//enum cmd_id {
//	CMD_bcfg=1,
//	CMD_bchn,
//	CMD_bphy,
//	CMD_brf,
//	CMD_bwlst,
//	CMD_scan_start,
//	CMD_tranceive_start,
//	CMD_bmcast,
//	CMD_bfwupd,
//	CMD_tmcast,
//	CMD_tinv,
//	CMD_tchn,
//	CMD_tphy,
//	CMD_trf,
//	CMD_tuuid,
//	CMD_ttx,
//	CMD_trx,
//	CMD_tota,
//	CMD_sys_cmd,

//	EVT_cmp,
//	EVT_data,
//	EVT_net,
//	EVT_sdr_adv,
//	EVT_ble_adv,

//	CMD_statistic,
//	CMD_bconn,
//	CMD_ble_gatt,

//	CMD_max,
//};

//typedef struct {
//	int type;
//	uint32_t scanner_access_addr;
//	uint32_t tranceiver_access_addr;
//}__attribute__ ((packed)) cfg_dev_cfg_t;

//typedef struct {
//	uint8_t uuid[16];
//}__attribute__ ((packed)) cfg_dev_uuid_t;

//typedef struct {
//	uint16_t sleep_intv;
//}__attribute__ ((packed)) cfg_dev_intv_t;

//typedef struct {
//	char ver[32];
//}__attribute__ ((packed)) cfg_dev_ver_t;

//typedef struct {
//	uint8_t chn_map[40];
//}__attribute__ ((packed)) cfg_dev_chn_t;

//typedef struct {
//	uint8_t 	PHY;
//}__attribute__ ((packed)) cfg_dev_phy_t;

//typedef struct {
//	uint16_t rf_power;
//}__attribute__ ((packed)) cfg_dev_rf_t;

//typedef struct {
//	uint16_t dev_addr[10];
//}__attribute__ ((packed)) cfg_dev_wl_t;

//typedef struct {
//	node_id_t node_id;
//	uint8_t 	length;
//	uint8_t 	data[MAX_UCST_PAYLOAD_LEN];
//}__attribute__ ((packed)) cmd_trx_t;

//typedef struct {
//	uint8_t 	cmd;
//	uint8_t 	per_bd_addr[8];
//	uint32_t 	sync_intv;
//}__attribute__ ((packed)) base_ble_adv_cmd_t;

//typedef struct {
//	uint8_t 	start;
//	uint16_t 	scan_intv;
//	uint16_t 	ble_wnd_in_percentage;
//	uint16_t 	filter;//interval;
//	//uint16_t 	window;
//	//uint16_t 	duration;
//	uint8_t		chn;
//}__attribute__ ((packed)) scanner_start_cmd_t;

//typedef struct {
//	uint8_t 	start;
//	uint32_t 	sync_intv;
//}__attribute__ ((packed)) tranceiver_start_cmd_t;

//typedef struct {
//	uint8_t 	cmd;
//}__attribute__ ((packed)) base_multicast_cmd_t;

//typedef struct {
//	bool		start;
//	uint8_t 	group_id;
//	uint8_t 	length;
//	uint8_t 	data[];
//}__attribute__ ((packed)) base_data_mcast_cmd_t;

//typedef struct {
//	int8_t ave_rssi;
//}__attribute__ ((packed)) scanner_statitics_t;

//typedef struct {
//	uint8_t ble_addr[BLE_BDADDR_LEN];
//	uint8_t  conn_or_disconn;
//	uint16_t conn_intv;
//	uint16_t sup_tmo;
//	uint16_t latency;
//	uint16_t conn_tmo;
//}__attribute__ ((packed)) cmd_ble_conn_t;

//typedef struct {
//	uint8_t svc_uuid[16];
//	uint16_t len;
//}__attribute__ ((packed)) att_op_disc_t;

//typedef struct {
//	uint16_t hdl;
//	uint16_t offset;
//	uint16_t len;
//}__attribute__ ((packed)) att_op_read_t;

//typedef struct {
//	uint16_t hdl;
//	uint16_t offset;
//	uint16_t len;
//	uint8_t data[];
//}__attribute__ ((packed)) att_op_write_t;

//typedef struct {
//	uint8_t att_op;//0-read, 1-write, 2-discover
//	union {
//		att_op_disc_t op_disc;
//		att_op_read_t op_read;
//		att_op_write_t op_write;
//	}__attribute__ ((packed)) u;
//}__attribute__ ((packed)) cmd_ble_gatt_t;

//typedef struct
//{
//	uint8_t cmd_id;
//	uint16_t result;
//	union
//	{
//		cfg_dev_cfg_t 			dev_cfg;
//		scanner_start_cmd_t 	scan_cmd;
//		tranceiver_start_cmd_t 	tranceive_cmd;
//		base_data_mcast_cmd_t	mcast_cmd;
//		cmd_trx_t				sdr_trx_cmd;
//		scanner_statitics_t		scanner_stat;
//		cmd_ble_conn_t			ble_conn_cmd;
//		cmd_ble_gatt_t			ble_gatt_cmd;
//	}__attribute__ ((packed)) u;
//}__attribute__ ((packed)) cmd_t;

//typedef struct
//{
//	node_id_t node_id;
//	uint16_t len;
//	uint8_t frame[];
//}__attribute__ ((packed)) evt_data_t;

//typedef struct
//{
//	uint8_t ble_addr[6];
//	int8_t rssi;
//	uint8_t payload[];
//}__attribute__ ((packed)) ble_adv_t;
//typedef struct
//{
//	node_id_t node_id;
//	int8_t rssi;
//	uint16_t seq_no;
//	uint8_t len;
//	uint8_t payload[];
//}__attribute__ ((packed)) sdr_adv_t;

////typedef struct
////{
////	node_id_t node_id;
////	int8_t rssi;
////	uint16_t seq_no;
////	uint8_t len;
////	uint8_t payload[];
////}__attribute__ ((packed)) evt_adv_t;

//typedef struct 
//{
//	node_id_t node_id;
//	int	 state;
//}__attribute__ ((packed)) evt_net_t;

//typedef struct 
//{
//	uint16_t len;
//	uint8_t frame[];
//}__attribute__ ((packed)) evt_cmp_t;

//typedef struct
//{
//	uint8_t evt_id;
//	uint8_t evt[];
//}__attribute__ ((packed)) evt_t;

//typedef struct
//{
//	//uint8_t dir;//0: base=>term; 1: term==>base
//	//uint8_t cmd;
//	uint8_t len;
//	uint8_t param[];//Pointer to cfg_dev_XXX_t
//}__attribute__ ((packed)) term_cmd_frame_t;

//typedef struct 
//{
//	//uint8_t dir;//0: base=>term; 1: term==>base
//	//uint8_t cmd;
//	uint8_t len;
////	uint16_t result;
//	uint8_t param[];//Pointer to cfg_dev_XXX_t
//}__attribute__ ((packed)) term_rsp_frame_t;

//typedef struct 
//{
//	uint8_t initiator; //
//	uint8_t cmdid;
//	union {
//		term_cmd_frame_t cmd;
//		term_rsp_frame_t rsp;
//	}__attribute__ ((packed)) u;
//}__attribute__ ((packed)) term_frame_t;

///*************************************************************************************************
//These are packages download from Transceiver
//*************************************************************************************************/
//#define PKT_NODE_LOC_ID			0x0A01
//#define PKT_NODE_PRI_ID			0x0A02
//#define PKT_NODE_MCAST_ID		0x0A04
//#define PKT_NODE_DUMMY			0xFFFF

//#define PKT_NODE_IBEACON		0x0B01
//#define PKT_NODE_EDDYSTONE		0x0B02

typedef struct {
	uint16_t pkt_id;						// packet identification
	uint16_t magic_word;
	uint16_t sync_intv;				// base periodic advertising interval in millisecond
	uint8_t per_adv_bda[8];			// periodic advertiser BD address
} bcst_data_t;

typedef struct {
	uint16_t pkt_id;			// packet identification
	uint32_t sdr_aa;	// sdr network access address
} per_bcst_data_t;

typedef struct {

	uint16_t sdr_na;
	sdr_node_id_t node_id;
}__attribute__ ((packed)) target_info_t;

//Maximum 78 bytes
typedef struct {
	uint16_t pkt_id;			// packet identification
	uint16_t sdr_ba;			// sdr base station address
	uint32_t sdr_aa;			// sdr network access address

//	uint16_t sdr_na[8];			// node sdr address as sdr master, maximum 8
	uint8_t channel;			// channel to use
//	sdr_node_id_t node_id[8];
	uint32_t seq_no;

	uint8_t target_nb;
	target_info_t target[];
}__attribute__ ((packed)) ucst_data_t;

///*************************************************************************************************
//These are packages upload from node
//*************************************************************************************************/
typedef struct {
	uint16_t pkt_id;
	sdr_node_id_t node_id;
	uint16_t length;
}__attribute__ ((packed)) sdr_node_data_head_t;

typedef struct {
	sdr_node_data_head_t head;
	uint8_t node_req_tran;
	uint16_t rsvd;
	//uint16_t seq_no;
	uint16_t sleep_intv;
	uint8_t payload[];
}__attribute__ ((packed)) sdr_node_data_loc_t;

typedef struct {
	sdr_node_data_head_t head;
	uint32_t seq_no;
	uint16_t tmo;
	uint8_t  data[];
}__attribute__ ((packed)) sdr_node_data_priv_t;

//typedef struct {
//	node_data_head_t head;
//	uint8_t data[];
//} node_data_mcast_t;

////typedef struct {
////	node_data_head_t head;
////	uint32_t seq_no;
////} base_data_priv_t;

///*typedef struct {
//	uint32_t virt_addr;
//	int state;
//	int rssi;
//	uint16_t len;
//	uint8_t payload[];
//	//uint32_t time_stamp;
//	//int node_req_tran;
//	//int data;
//} node_info_t;*/

//typedef struct {
//	uint16_t share_id;
//	uint16_t node_addr;
//	uint32_t access_addr;
//} base_node_adv_data_t;

//#define MSG_CFG_SAVE		0x00010001
//#define MSG_NODE_INFO		0x00010002
//typedef struct {
//	uint32_t msg;
//	uint32_t time_stamp;
//	uint32_t node_addr;
//	int rssi;
//} msg_bcst_t;

///*typedef struct {
//	enum wl_data_type type;
//	uint8_t data[];
//}__attribute__ ((packed)) wl_data_t;*/

//typedef struct {
//	int wl_msg_id;
//	uint8_t data[];
//} wl_msg_t;

////Application payload definition
//typedef enum{
//	PAYLOAD_LED_STATE,
//	PAYLOAD_TEMP,
//}app_type_t;

//typedef struct {
//	app_type_t app_type;
//	uint8_t payload[];
//}app_payload_t;

//typedef struct {
//	uint8_t led;
//	uint8_t state;
//} app_led_state_t;

////	union{
////		struct _struct_scanner_ {
////			uint32_t access_address;
////		} scanner;
////		struct _struct_syncer_ {
////			uint32_t access_address;
////		} syncer;
////	} u;

///*
// * Wireless base APIs
// ****************************************************************************************
// */
////typedef struct {
////	int type;
//////	uint32_t adv_intv;
//////	uint32_t sync_intv;
////	uint8_t bda[8];
//////	uint8_t bc_bda[8];
//////	int scan_ch;
////	uint32_t sdr_aa;
//////	uint32_t sdr_ba;
////	uint32_t sdr_scan_intv;
////} base_start_t;

typedef void (*sdr_scanner_rx_cb_t)(void *arg, uint16_t pkt_id, sdr_node_id_t node_id, uint16_t node_req_tran, uint8_t *payload, uint16_t payload_len, int rssi);
typedef void (*ble_scanner_rx_cb_t)(void *arg, ble_node_id_t node_id, uint8_t *payload, uint16_t payload_len, int rssi);

//typedef void (*scanner_rx_cb_t)(void *arg, uint16_t pkt_id, sdr_node_id_t node_id, uint16_t node_req_tran, uint8_t *payload, uint16_t payload_len, int rssi);
typedef void (*sdr_rx_cb_t)(void *arg, uint16_t pkt_id, sdr_node_id_t node_id, uint16_t node_req_tran, uint8_t *payload, uint16_t payload_len, int rssi);
typedef void (*ble_rx_cb_t)(ble_node_id_t *p_node, short handler, uint8_t *payload, uint16_t payload_len);

int wl_transceiver_init(uint8_t ble_addr[6], uint32_t sdr_bcst_aa, uint32_t sdr_ucst_aa, uint16_t sdr_ucst_addr, void *arg, sdr_rx_cb_t sdr_rx_cb, ble_rx_cb_t ble_rx_cb);
int wl_transceiver_start(uint32_t sync_intv);
int wl_transceiver_sdr_tran_data(sdr_node_id_t node_id, uint16_t tx_len, uint8_t *tx_buf, void *arg, void (*tran_done_cb)(void *arg, int status));

int wl_transceiver_ble_adv(BLE_Adv_param_t *p_param);
int wl_transceiver_ble_conn(uint8_t bd_addr[6], BLE_Connect_param_t *p_param, uint32_t tmo);
int wl_transceiver_ble_disconnect(uint8_t bd_addr[6], uint32_t tmo);
int wl_transceiver_gatt_service(GATT_Svc_param_t *p_param, uint16_t *p_hdl);
int wl_transceiver_gatt_sdp(uint8_t bd_addr[6], GATT_SDP_param_t *p_param, /*uint32_t tmo,*/ uint8_t **p, uint16_t *p_len);
int wl_transceiver_gatt_read(uint8_t bd_addr[6], GATT_Read_param_t *p_param, /*uint32_t tmo,*/ uint8_t **p, uint16_t *p_len);
int wl_transceiver_gatt_write(uint8_t bd_addr[6], GATT_Write_param_t *p_param/*, uint32_t tmo*/);
int wl_transceiver_gatt_notify(uint8_t bd_addr[6], GATT_Notify_param_t *p_param/*, uint32_t tmo*/);

int wl_scanner_init(uint32_t sdr_bcst_aa, void *arg, sdr_scanner_rx_cb_t sdr_rx_cb, ble_scanner_rx_cb_t ble_rx_cb);
int wl_scanner_start(uint16_t sdr_wnd, uint8_t sdr_chn, uint16_t ble_wnd, uint8_t ble_chn);
//int wl_scanner_get_stat(scanner_statitics_t *p_sts);

///*
// * Wireless Node APIs
// ****************************************************************************************
// */
typedef enum{
	WL_STATE_SYNCED,
	WL_STATE_LOST,
}wl_node_state_t;

typedef struct {
	uint16_t bcst_intv;	
	uint16_t sleep_intv;		// node sleep interval, so that some sync will be skipped
	int16_t ho_thrd;
	void (*wl_state_cb)(wl_node_state_t new_state);
	void (*rx_data_cb)(uint8_t *data, uint16_t data_len);
	void* (*sdr_bcst_cb)(uint16_t *p_len);
}__attribute__ ((packed)) sdr_node_param_t;

int wl_node_init(uint8_t bd_addr[6]);
int wl_node_run(sdr_node_id_t *p_node_id, sdr_node_param_t *p_param);

///**
// ****************************************************************************************
// * @brief  Start node operation  
// *
// * @param[in] p_node				Node info
// * @param[in] rx_data_cb			For node type of NODE_TYPE_NODE_WITH_UNICAST, private 
// *									communication's RX callback function
// * @param[in] arg					argument whenever RX callback function is called
// * @param[in] sdr_bcst_cb			Everytime before node starts SDR broadcasting, it is  
// *									called to get application specific data
// * @return  int
// ****************************************************************************************
//*/
//int wl_node_run(node_start_t *p_node);
////int wl_node_start(node_start_t *p_node, void *arg, void (*rx_data_cb)(void *arg, uint8_t *data, uint16_t data_len));
//int wl_node_msg_handler(void* p_evt);
//int wl_node_tran_data(uint16_t tx_len, uint8_t *tx_buf, void *tx_arg, void (*tx_done_cb)(void *arg, int status));
//int wl_node_update_sleep_intv(uint32_t sleep_intv);

#endif
