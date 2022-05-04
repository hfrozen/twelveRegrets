// WizConf.h
#pragma once
#include <stdint.h>

#define	W5500						5500

#ifndef _WIZCHIP_
#define _WIZCHIP_					W5500
#endif

#define _WIZCHIP_IO_MODE_NONE_		0x0000
#define _WIZCHIP_IO_MODE_BUS_		0x0100 /**< Bus interface mode */
#define _WIZCHIP_IO_MODE_SPI_		0x0200 /**< SPI interface mode */

#define _WIZCHIP_IO_MODE_BUS_DIR_	(_WIZCHIP_IO_MODE_BUS_ + 1) /**< BUS interface mode for direct  */
#define _WIZCHIP_IO_MODE_BUS_INDIR_	(_WIZCHIP_IO_MODE_BUS_ + 2) /**< BUS interface mode for indirect */

#define _WIZCHIP_IO_MODE_SPI_VDM_	(_WIZCHIP_IO_MODE_SPI_ + 1) /**< SPI interface mode for variable length data*/
#define _WIZCHIP_IO_MODE_SPI_FDM_	(_WIZCHIP_IO_MODE_SPI_ + 2) /**< SPI interface mode for fixed length data mode*/
#define _WIZCHIP_IO_MODE_SPI_5500_	(_WIZCHIP_IO_MODE_SPI_ + 3) /**< SPI interface mode for fixed length data mode*/

#define _WIZCHIP_ID_				"W5500\0"
  
#ifndef _WIZCHIP_IO_MODE_
//#define _WIZCHIP_IO_MODE_			_WIZCHIP_IO_MODE_SPI_FDM_
#define _WIZCHIP_IO_MODE_			_WIZCHIP_IO_MODE_SPI_VDM_
#endif
//A20150601 : Define the unit of IO DATA.
typedef uint8_t		iodata_t;

#include "Ethernet/W5500/W5500c.h"

//#define _WIZCHIP_IO_BASE_			0x60000000	// for 5100S IND
#define _WIZCHIP_IO_BASE_			0x00000000	// for 5100S SPI

#define _WIZCHIP_SOCK_NUM_			8	// The count of independant socket of @b WIZCHIP

typedef struct _tagWIZCHIP {
	uint16_t	if_mode;			// host interface mode
	uint8_t		id[6];				// WIZCHIP ID such as @b 5100, @b 5200, @b 5500, and so on.

	// The set of critical section callback func.
	struct _tagCRIS {
		void (*_enter)(void);		// crtical section enter
		void (*_exit)(void);		// critial section exit
	} CRIS;

	// The set of @ref \_WIZCHIP_ select control callback func.
	struct _tagCS {
		void (*_select) (void);		// _WIZCHIP_ selected
		void (*_deselect)(void);	// _WIZCHIP_ deselected
	} CS;

	// The set of interface IO callback func.
	union _tagIF {
		struct {
			uint8_t (*_read_byte)(void);
			void (*_write_byte)(uint8_t wb);
			void (*_read_burst)(uint8_t* pBuf, uint16_t len);
			void (*_write_burst)(uint8_t* pBuf, uint16_t len);
		} SPI;
		struct {
			iodata_t (*_read_data)(uint32_t AddrSel);
			void (*_write_data)(uint32_t AddrSel, iodata_t wb);
		} BUS;
	} IF;
} _WIZCHIP;

extern _WIZCHIP  WIZCHIP;

// WIZCHIP control type enumration used in @ref ctlwizchip().
typedef enum {
	CW_RESET_WIZCHIP,	// Resets WIZCHIP by softly
	CW_INIT_WIZCHIP,	// Initializes to WIZCHIP with SOCKET buffer size 2 or 1 dimension array typed uint8_t.
	CW_GET_INTERRUPT,	// Get Interrupt status of WIZCHIP
	CW_CLR_INTERRUPT,	// Clears interrupt
	CW_SET_INTRMASK,	// Masks interrupt
	CW_GET_INTRMASK,	// Get interrupt mask
	CW_SET_INTRTIME,	// Set interval time between the current and next interrupt.
	CW_GET_INTRTIME,	// Set interval time between the current and next interrupt.
	CW_GET_ID,			// Gets WIZCHIP name.
	CW_RESET_PHY,		// Resets internal PHY. Valid Only W5500
	CW_SET_PHYCONF,		// When PHY configured by internal register, PHY operation mode (Manual/Auto, 10/100, Half/Full). Valid Only W5000
	CW_GET_PHYCONF,		// Get PHY operation mode in internal register. Valid Only W5500
	CW_GET_PHYSTATUS,	// Get real PHY status on operating. Valid Only W5500
	CW_SET_PHYPOWMODE,  // Set PHY power mode as normal and down when PHYSTATUS.OPMD == 1. Valid Only W5500
	CW_GET_PHYPOWMODE,  // Get PHY Power mode as down or normal, Valid Only W5100, W5200
	CW_GET_PHYLINK		// Get PHY Link status, Valid Only W5100, W5200
//#endif
} ctlwizchip_type;

// Network control type enumration used in @ref ctlnetwork().
typedef enum {
	CN_SET_NETINFO,		// Set Network with @ref wiz_NetInfo
	CN_GET_NETINFO,		// Get Network with @ref wiz_NetInfo
	CN_SET_NETMODE,		// Set network mode as WOL, PPPoE, Ping Block, and Force ARP mode
	CN_GET_NETMODE,		// Get network mode as WOL, PPPoE, Ping Block, and Force ARP mode
	CN_SET_TIMEOUT,		// Set network timeout as retry count and time.
	CN_GET_TIMEOUT,		// Get network timeout as retry count and time.
} ctlnetwork_type;

typedef enum {
	IK_WOL				= (1 << 4),		// Wake On Lan by receiving the magic packet. Valid in W500.
	IK_PPPOE_TERMINATED	= (1 << 5),		// PPPoE Disconnected
	IK_DEST_UNREACH		= (1 << 6),		// Destination IP & Port Unreachable, No use in W5200
	IK_IP_CONFLICT		= (1 << 7),		// IP conflict occurred
	IK_SOCK_0			= (1 << 8),		// Socket 0 interrupt
	IK_SOCK_1			= (1 << 9),		// Socket 1 interrupt
	IK_SOCK_2			= (1 << 10),	// Socket 2 interrupt
	IK_SOCK_3			= (1 << 11),	// Socket 3 interrupt
	IK_SOCK_4			= (1 << 12),	// Socket 4 interrupt, No use in 5100
	IK_SOCK_5			= (1 << 13),	// Socket 5 interrupt, No use in 5100
	IK_SOCK_6			= (1 << 14),	// Socket 6 interrupt, No use in 5100
	IK_SOCK_7			= (1 << 15),	// Socket 7 interrupt, No use in 5100
	IK_SOCK_ALL			= (0xFF << 8)	// All Socket interrupt
} intr_kind;

#define PHY_CONFBY_HW			0	// Configured PHY operation mode by HW pin
#define PHY_CONFBY_SW			1	// Configured PHY operation mode by SW register
#define PHY_MODE_MANUAL			0	// Configured PHY operation mode with user setting.
#define PHY_MODE_AUTONEGO		1	// Configured PHY operation mode with auto-negotiation
#define PHY_SPEED_10			0	// Link Speed 10
#define PHY_SPEED_100			1	// Link Speed 100
#define PHY_DUPLEX_HALF			0	// Link Half-Duplex
#define PHY_DUPLEX_FULL			1	// Link Full-Duplex
#define PHY_LINK_OFF			0	// Link Off
#define PHY_LINK_ON				1	// Link On
#define PHY_POWER_NORM			0	// PHY power normal mode
#define PHY_POWER_DOWN			1	// PHY power down mode

typedef struct wiz_PhyConf_t {
	uint8_t by;			// set by @ref PHY_CONFBY_HW or @ref PHY_CONFBY_SW
	uint8_t mode;		// set by @ref PHY_MODE_MANUAL or @ref PHY_MODE_AUTONEGO
	uint8_t speed;		// set by @ref PHY_SPEED_10 or @ref PHY_SPEED_100
	uint8_t duplex;		// set by @ref PHY_DUPLEX_HALF @ref PHY_DUPLEX_FULL
	//uint8_t power;	// set by @ref PHY_POWER_NORM or @ref PHY_POWER_DOWN
	//uint8_t link;		// Valid only in CW_GET_PHYSTATUS. set by @ref PHY_LINK_ON or PHY_DUPLEX_OFF
} wiz_PhyConf;

typedef enum {
	NETINFO_STATIC = 1,	///< Static IP configuration by manually.
	NETINFO_DHCP		///< Dynamic IP configruation from a DHCP sever
} dhcp_mode;

typedef struct wiz_NetInfo_t {
	uint8_t mac[6];		///< Source Mac Address
	uint8_t ip[4];		///< Source IP Address
	uint8_t sn[4];		///< Subnet Mask
	uint8_t gw[4];		///< Gateway IP Address
	uint8_t dns[4];		///< DNS server IP Address
	dhcp_mode dhcp;		///< 1 - Static, 2 - DHCP
} wiz_NetInfo;

typedef enum {
	NM_FORCEARP		= (1<<1),	// Force to APP send whenever udp data is sent. Valid only in W5500
	NM_WAKEONLAN	= (1<<5),	// Wake On Lan
	NM_PINGBLOCK	= (1<<4),	// Block ping-request
	NM_PPPOE		= (1<<3),	// PPPoE mode
} netmode_type;

typedef struct wiz_NetTimeout_t {
	uint8_t  retry_cnt;		///< retry count
	uint16_t time_100us;	///< time unit 100us
} wiz_NetTimeout;

void wizchip_cs_select(void);
void wizchip_cs_deselect(void);
uint8_t wizchip_spi_readbyte(void);
void wizchip_spi_writebyte(uint8_t wb);
void reg_wizchip_cris_cbfunc(void(*cris_en)(void), void(*cris_ex)(void));
void reg_wizchip_cs_cbfunc(void(*cs_sel)(void), void(*cs_desel)(void));
void reg_wizchip_bus_cbfunc(iodata_t (*bus_rb)(uint32_t addr), void (*bus_wb)(uint32_t addr, iodata_t wb));
void reg_wizchip_spi_cbfunc(uint8_t (*spi_rb)(void), void (*spi_wb)(uint8_t wb));
void reg_wizchip_spiburst_cbfunc(void (*spi_rb)(uint8_t* pBuf, uint16_t len), void (*spi_wb)(uint8_t* pBuf, uint16_t len));
int8_t ctlwizchip(ctlwizchip_type cwtype, void* arg);
int8_t ctlnetwork(ctlnetwork_type cntype, void* arg);
void wizchip_sw_reset(void);
int8_t wizchip_init(uint8_t* txsize, uint8_t* rxsize);
void wizchip_clrinterrupt(intr_kind intr);
intr_kind wizchip_getinterrupt(void);
void wizchip_setinterruptmask(intr_kind intr);
intr_kind wizchip_getinterruptmask(void);
int8_t wizphy_getphylink(void);
int8_t wizphy_getphypmode(void);
void wizphy_reset(void);
void wizphy_setphyconf(wiz_PhyConf* phyconf);
void wizphy_getphyconf(wiz_PhyConf* phyconf);
void wizphy_getphystat(wiz_PhyConf* phyconf);
int8_t wizphy_setphypmode(uint8_t pmode);
void wizchip_setnetinfo(wiz_NetInfo* pnetinfo);
void wizchip_getnetinfo(wiz_NetInfo* pnetinfo);
int8_t wizchip_setnetmode(netmode_type netmode);
netmode_type wizchip_getnetmode(void);
void wizchip_settimeout(wiz_NetTimeout* nettime);
void wizchip_gettimeout(wiz_NetTimeout* nettime);
