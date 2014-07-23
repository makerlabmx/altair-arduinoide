#ifndef MAC_H
#define MAC_H

/*
 *	MAC Frame definitions.
 *	Author: Rodrigo Méndez Gamboa, rmendez@makerlab.mx
 */

#include "config.h"

#define TX_TIMEOUT  50 //max waiting time in ms before sending a new packet when another is pending.

#define MAX_PHY_PACKET_SIZE 	127

//Frame format, in order.
#define FRAME_CONTROL_SIZE 		2
#define SECUENCE_NUMBER_SIZE 	1
#define DEST_PAN_SIZE 			2
#define DEST_ADDRESS_SIZE 		8
#define SRC_ADDRESS_SIZE 		8

#if PHY_SECURITY_ENABLED == 1
	#define SECURITY_HEADER_SIZE	5
	#define MUI_SIZE				4
#else
	#define SECURITY_HEADER_SIZE 	0
	#define MUI_SIZE				0
#endif

#define MAC_HEADER_SIZE		(FRAME_CONTROL_SIZE + SECUENCE_NUMBER_SIZE + DEST_PAN_SIZE + DEST_ADDRESS_SIZE + SRC_ADDRESS_SIZE + SECURITY_HEADER_SIZE)
#define MAC_BROADCAST_HEADER_SIZE MAC_HEADER_SIZE - 6	//on broadcast, dest address is 16 bits 0xFFFF
#define FCS_SIZE			2

#define MAX_DATA_SIZE		(MAX_PHY_PACKET_SIZE - MAC_HEADER_SIZE - MUI_SIZE - FCS_SIZE)




#define MAC_FRAME_CONTROL	0
#define MAC_SEQ_NUMBER		2
#define MAC_PAN				3
#define MAC_DEST_ADDRESS	5
#define MAC_SRC_ADDRESS		13
#define MAC_SECURITY_HEADER	21

//Frame Control Field bit definitions:
#define FRAME_TYPE_B		0
#define SECURITY_EN_B		3
#define FRAME_PEND_B		4
#define ACK_REQ_B			5
#define PANID_COMP_B		6
#define RESERVED_FC_B		7
#define DEST_ADDR_MODE_B	(10 - 8)
#define FRAME_VERSION_B		(12 - 8)
#define SRC_ADDR_MODE_B		(14 - 8)

//Frame Control Field default bit values:
#define FRAME_TYPE 		0x01

#if PHY_SECURITY_ENABLED == 1
	#define SECURITY_EN		0x01
#else
	#define SECURITY_EN		0x00
#endif

#define FRAME_PEND 		0x00
#define ACK_REQ 		0x01
#define PANID_COMP 		0x01
#define FC_RESERVED		0x00
#define DEST_ADDR_MODE 	0x03
#define DEST_ADDR_MODE_BROADCAST 0x02
#define FRAME_VERSION 	0x01
#define SRC_ADDR_MODE 	0x03

//Frame Control field Byte 1
#define FRAME_CONTROL_B1	( (FRAME_TYPE << FRAME_TYPE_B) | (SECURITY_EN << SECURITY_EN_B) | (FRAME_PEND << FRAME_PEND_B) | (ACK_REQ << ACK_REQ_B) | (PANID_COMP << PANID_COMP_B) )
#define FRAME_CONTROL_B2	( (DEST_ADDR_MODE << DEST_ADDR_MODE_B) | (FRAME_VERSION << FRAME_VERSION_B) | (SRC_ADDR_MODE << SRC_ADDR_MODE_B) )

#define FRAME_CONTROL_BROADCAST_B1	( (FRAME_TYPE << FRAME_TYPE_B) | (SECURITY_EN << SECURITY_EN_B) | (FRAME_PEND << FRAME_PEND_B) | (0 << ACK_REQ_B) | (PANID_COMP << PANID_COMP_B) )
#define FRAME_CONTROL_BROADCAST_B2	( (DEST_ADDR_MODE_BROADCAST << DEST_ADDR_MODE_B) | (FRAME_VERSION << FRAME_VERSION_B) | (SRC_ADDR_MODE << SRC_ADDR_MODE_B) )


//Security Header bytes:
#define SECURITY_CTRL_SIZE			1
#define SECURITY_FRAME_COUNT_SIZE	4

//Security Control bit definitions:
#define SECURITY_LEVEL_B	0
#define KEY_ID_MODE_B		3
#define SC_RESERVED_B		5

//Security Control default bit values:
#define SECURITY_LEVEL	 	0x05
#define KEY_ID_MODE		0x00
#define SC_RESERVED		0x00

#define SECURITY_CONTROL_B1	( (SECURITY_LEVEL << SECURITY_LEVEL_B) | (KEY_ID_MODE << KEY_ID_MODE_B) | (SC_RESERVED << SC_RESERVED_B) )

#endif /* MAC_H */