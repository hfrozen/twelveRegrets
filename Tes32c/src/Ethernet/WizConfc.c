// WizConfc.c
#include <Ethernet/WizConfc.h>
#include <stddef.h>
#include "stm32f4xx_hal.h"

void wizchip_cris_enter(void)	{}
void wizchip_cris_exit(void)	{}
void wizchip_cs_select(void)	{}
void wizchip_cs_deselect(void)	{}
iodata_t wizchip_bus_readdata(uint32_t AddrSel)	{ return * ((volatile iodata_t *)((ptrdiff_t) AddrSel)); }
void wizchip_bus_writedata(uint32_t AddrSel, iodata_t wb)	{ *((volatile iodata_t*)((ptrdiff_t)AddrSel)) = wb; }
uint8_t wizchip_spi_readbyte(void)	{ return 0; }
void wizchip_spi_writebyte(uint8_t wb)	{ wb = 0; -- wb; }
void wizchip_spi_readburst(uint8_t* pBuf, uint16_t len)	{ while (len > 0) { *pBuf ++ = 0; len --; } }
void wizchip_spi_writeburst(uint8_t* pBuf, uint16_t len)	{ while (len > 0) { *pBuf ++ = 0; len --; } }

_WIZCHIP WIZCHIP = {
	//_WIZCHIP_IO_MODE_,
	//_WIZCHIP_ID_ ,
	_WIZCHIP_IO_MODE_SPI_VDM_,
	"W5500\n",
	{
		wizchip_cris_enter,
		wizchip_cris_exit
	},
	{
		wizchip_cs_select,
		wizchip_cs_deselect
	},
	{
		wizchip_spi_readbyte,
		wizchip_spi_writebyte,
		wizchip_spi_readburst,
		wizchip_spi_writeburst
	}
	//wizchip_bus_readdata,
	//wizchip_bus_writedata,
};

static uint8_t		_DNS_[4];	// DNS server ip address
static dhcp_mode	_DHCP_;		// DHCP mode

void reg_wizchip_cris_cbfunc(void(*cris_en)(void), void(*cris_ex)(void))
{
	if (!cris_en || !cris_ex) {
		WIZCHIP.CRIS._enter = wizchip_cris_enter;
		WIZCHIP.CRIS._exit = wizchip_cris_exit;
	}
	else {
		WIZCHIP.CRIS._enter = cris_en;
		WIZCHIP.CRIS._exit = cris_ex;
	}
}

void reg_wizchip_cs_cbfunc(void(*cs_sel)(void), void(*cs_desel)(void))
{
	if (!cs_sel || !cs_desel) {
		WIZCHIP.CS._select = wizchip_cs_select;
		WIZCHIP.CS._deselect = wizchip_cs_deselect;
	}
	else {
		WIZCHIP.CS._select = cs_sel;
		WIZCHIP.CS._deselect = cs_desel;
	}
}

void reg_wizchip_bus_cbfunc(iodata_t(*bus_rb)(uint32_t addr), void (*bus_wb)(uint32_t addr, iodata_t wb))
{
	while (!(WIZCHIP.if_mode & _WIZCHIP_IO_MODE_BUS_));

	if(!bus_rb || !bus_wb) {
		WIZCHIP.IF.BUS._read_data = wizchip_bus_readdata;
		WIZCHIP.IF.BUS._write_data = wizchip_bus_writedata;
	}
	else {
		WIZCHIP.IF.BUS._read_data = bus_rb;
		WIZCHIP.IF.BUS._write_data = bus_wb;
	}
}

void reg_wizchip_spi_cbfunc(uint8_t (*spi_rb)(void), void (*spi_wb)(uint8_t wb))
{
	while (!(WIZCHIP.if_mode & _WIZCHIP_IO_MODE_SPI_));

	if (!spi_rb || !spi_wb) {
		WIZCHIP.IF.SPI._read_byte = wizchip_spi_readbyte;
		WIZCHIP.IF.SPI._write_byte = wizchip_spi_writebyte;
	}
	else {
		WIZCHIP.IF.SPI._read_byte = spi_rb;
		WIZCHIP.IF.SPI._write_byte = spi_wb;
	}
}

void reg_wizchip_spiburst_cbfunc(void (*spi_rb)(uint8_t* pBuf, uint16_t len), void (*spi_wb)(uint8_t* pBuf, uint16_t len))
{
	while (!(WIZCHIP.if_mode & _WIZCHIP_IO_MODE_SPI_));

	if (!spi_rb || !spi_wb) {
		WIZCHIP.IF.SPI._read_burst = wizchip_spi_readburst;
		WIZCHIP.IF.SPI._write_burst = wizchip_spi_writeburst;
	}
	else {
		WIZCHIP.IF.SPI._read_burst = spi_rb;
		WIZCHIP.IF.SPI._write_burst = spi_wb;
	}
}

int8_t ctlwizchip(ctlwizchip_type cwtype, void* arg)
{
	uint8_t tmp = 0;
	uint8_t* ptmp[2] = {0,0};
	switch (cwtype) {
		case CW_RESET_WIZCHIP:
			wizchip_sw_reset();
			break;
		case CW_INIT_WIZCHIP:
			if (arg != 0) {
				ptmp[0] = (uint8_t*)arg;
				ptmp[1] = ptmp[0] + _WIZCHIP_SOCK_NUM_;
			}
			return wizchip_init(ptmp[0], ptmp[1]);
		case CW_CLR_INTERRUPT:
			wizchip_clrinterrupt(*((intr_kind*)arg));
			break;
		case CW_GET_INTERRUPT:
			*((intr_kind*)arg) = wizchip_getinterrupt();
			break;
		case CW_SET_INTRMASK:
			wizchip_setinterruptmask(*((intr_kind*)arg));
			break;
		case CW_GET_INTRMASK:
			*((intr_kind*)arg) = wizchip_getinterruptmask();
			break;
		case CW_SET_INTRTIME:
			setINTLEVEL(*(uint16_t*)arg);
			break;
		case CW_GET_INTRTIME:
			*(uint16_t*)arg = getINTLEVEL();
			break;
		case CW_GET_ID:
			((uint8_t*)arg)[0] = WIZCHIP.id[0];
			((uint8_t*)arg)[1] = WIZCHIP.id[1];
			((uint8_t*)arg)[2] = WIZCHIP.id[2];
			((uint8_t*)arg)[3] = WIZCHIP.id[3];
			((uint8_t*)arg)[4] = WIZCHIP.id[4];
			((uint8_t*)arg)[5] = 0;
			break;
		case CW_RESET_PHY:
			wizphy_reset();
			break;
		case CW_SET_PHYCONF:
			wizphy_setphyconf((wiz_PhyConf*)arg);
			break;
		case CW_GET_PHYCONF:
			wizphy_getphyconf((wiz_PhyConf*)arg);
			break;
		case CW_GET_PHYSTATUS:
			break;
		case CW_SET_PHYPOWMODE:
			return wizphy_setphypmode(*(uint8_t*)arg);
		case CW_GET_PHYPOWMODE:
			tmp = wizphy_getphypmode();
			if((int8_t)tmp == -1) return -1;
			*(uint8_t*)arg = tmp;
			break;
		case CW_GET_PHYLINK:
			tmp = wizphy_getphylink();
			if((int8_t)tmp == -1) return -1;
			*(uint8_t*)arg = tmp;
			break;
		default:
			return -1;
	}
	return 0;
}

int8_t ctlnetwork(ctlnetwork_type cntype, void* arg)
{
	switch (cntype) {
		case CN_SET_NETINFO:
			wizchip_setnetinfo((wiz_NetInfo*)arg);
			break;
		case CN_GET_NETINFO:
			wizchip_getnetinfo((wiz_NetInfo*)arg);
			break;
		case CN_SET_NETMODE:
			return wizchip_setnetmode(*(netmode_type*)arg);
		case CN_GET_NETMODE:
			*(netmode_type*)arg = wizchip_getnetmode();
			break;
		case CN_SET_TIMEOUT:
			wizchip_settimeout((wiz_NetTimeout*)arg);
			break;
		case CN_GET_TIMEOUT:
			wizchip_gettimeout((wiz_NetTimeout*)arg);
			break;
		default:
			return -1;
	}
	return 0;
}

void wizchip_sw_reset(void)
{
	uint8_t gw[4], sn[4], sip[4];
	uint8_t mac[6];
	//uint8_t tmp;
	//tmp = getPHYCFGR();
	//printf("Reset:phy 0x%02x\r\n", tmp);
	getSHAR(mac);
	//printf("Reset:mac %02x %02x %02x %02x %02x %02x\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	getGAR(gw);
	//printf("Reset:gw %d %d %d %d\r\n", gw[0], gw[1], gw[2], gw[3]);
	getSUBR(sn);
	//printf("Reset:sn %d %d %d %d\r\n", sn[0], sn[1], sn[2], sn[3]);
	getSIPR(sip);
	//printf("Reset:sip %d %d %d %d\r\n", sip[0], sip[1], sip[2], sip[3]);
	setMR(MR_RST);
	setMR(MR_RST);
	setMR(MR_RST);
	getMR(); // for delay
	//tmp = getPHYCFGR();
	//printf("Reset:phy 0x%02x\r\n", tmp);
	//printf("Reset:mr %02x\r\n", tmp);
	setSHAR(mac);
	setGAR(gw);
	setSUBR(sn);
	setSIPR(sip);
	//tmp = getPHYCFGR();
	//printf("Reset:phy 0x%02x\r\n", tmp);
}

int8_t wizchip_init(uint8_t* txsize, uint8_t* rxsize)
{
	int8_t i;
	int8_t tmp = 0;
	wizchip_sw_reset();
	if (txsize) {
		tmp = 0;
		for (i = 0 ; i < _WIZCHIP_SOCK_NUM_; i ++) {
			tmp += txsize[i];
			if (tmp > 16)	return -1;
		}
		//for (i = 0 ; i < _WIZCHIP_SOCK_NUM_; i ++) {
		//	setSn_TXBUF_SIZE(i, txsize[i]);
		//	printf("INIT:set txsize to (%d-%d)\r\n", i, txsize[i]);
		//}
		for (i = 0; i < _WIZCHIP_SOCK_NUM_; i ++) {
			uint8_t s = getSn_TXBUF_SIZE(i);
			if (s != txsize[i])	printf("INIT:tx size failed! (%d-%d)\r\n", i, s);
		}
	}
	if (rxsize) {
		tmp = 0;
		for (i = 0 ; i < _WIZCHIP_SOCK_NUM_; i ++) {
			tmp += rxsize[i];
			if (tmp > 16)	return -1;
		}
		//for (i = 0 ; i < _WIZCHIP_SOCK_NUM_; i ++) {
		//	setSn_RXBUF_SIZE(i, rxsize[i]);
		//	printf("INIT:set rxsize to (%d-%d)\r\n", i, rxsize[i]);
		//}
		for (i = 0; i < _WIZCHIP_SOCK_NUM_; i ++) {
			uint8_t s = getSn_RXBUF_SIZE(i);
			if (s != rxsize[i])	printf("INIT:rx size failed! (%d-%d)\r\n", i, s);
		}
	}
	return 0;
}

void wizchip_clrinterrupt(intr_kind intr)
{
	uint8_t ir  = (uint8_t)intr;
	uint8_t sir = (uint8_t)((uint16_t)intr >> 8);
	setIR(ir);
	setSIR(sir);
}

intr_kind wizchip_getinterrupt(void)
{
	uint8_t ir  = 0;
	uint8_t sir = 0;
	uint16_t ret = 0;
	ir  = getIR();
	sir = getSIR();
	ret = sir;
	ret = (ret << 8) + ir;
	return (intr_kind)ret;
}

void wizchip_setinterruptmask(intr_kind intr)
{
	uint8_t imr  = (uint8_t)intr;
	uint8_t simr = (uint8_t)((uint16_t)intr >> 8);
	setIMR(imr);
	setSIMR(simr);
}

intr_kind wizchip_getinterruptmask(void)
{
	uint8_t imr  = 0;
	uint8_t simr = 0;
	uint16_t ret = 0;
	imr  = getIMR();
	simr = getSIMR();
	ret = simr;
	ret = (ret << 8) + imr;
	return (intr_kind)ret;
}

int8_t wizphy_getphylink(void)
{
	int8_t tmp = PHY_LINK_OFF;
	if (getPHYCFGR() & PHYCFGR_LNK_ON)	tmp = PHY_LINK_ON;
	//tmp = -1;
	return tmp;
}

int8_t wizphy_getphypmode(void)
{
	int8_t tmp = 0;
	if ((getPHYCFGR() & PHYCFGR_OPMDC_ALLA) == PHYCFGR_OPMDC_PDOWN)
		tmp = PHY_POWER_DOWN;
	else	tmp = PHY_POWER_NORM;
	return tmp;
}

void wizphy_reset(void)
{
	uint8_t tmp = getPHYCFGR();
	tmp &= PHYCFGR_RST;
	setPHYCFGR(tmp);
	tmp = getPHYCFGR();
	tmp |= ~PHYCFGR_RST;
	setPHYCFGR(tmp);
}

void wizphy_setphyconf(wiz_PhyConf* phyconf)
{
	uint8_t tmp = 0;
	if (phyconf->by == PHY_CONFBY_SW)
		tmp |= PHYCFGR_OPMD;
	else	tmp &= ~PHYCFGR_OPMD;
	if (phyconf->mode == PHY_MODE_AUTONEGO)
		tmp |= PHYCFGR_OPMDC_ALLA;
	else {
		if (phyconf->duplex == PHY_DUPLEX_FULL) {
			if (phyconf->speed == PHY_SPEED_100)
				tmp |= PHYCFGR_OPMDC_100F;
			else	tmp |= PHYCFGR_OPMDC_10F;
		}
		else {
			if (phyconf->speed == PHY_SPEED_100)
				tmp |= PHYCFGR_OPMDC_100H;
			else	tmp |= PHYCFGR_OPMDC_10H;
		}
	}
	setPHYCFGR(tmp);
	wizphy_reset();
}

void wizphy_getphyconf(wiz_PhyConf* phyconf)
{
	uint8_t tmp = 0;
	tmp = getPHYCFGR();
	phyconf->by	= (tmp & PHYCFGR_OPMD) ? PHY_CONFBY_SW : PHY_CONFBY_HW;
	switch (tmp & PHYCFGR_OPMDC_ALLA) {
		case PHYCFGR_OPMDC_ALLA:
		case PHYCFGR_OPMDC_100FA:
			phyconf->mode = PHY_MODE_AUTONEGO;
			break;
		default:
			phyconf->mode = PHY_MODE_MANUAL;
			break;
	}
	switch (tmp & PHYCFGR_OPMDC_ALLA) {
		case PHYCFGR_OPMDC_100FA:
		case PHYCFGR_OPMDC_100F:
		case PHYCFGR_OPMDC_100H:
			phyconf->speed = PHY_SPEED_100;
			break;
		default:
			phyconf->speed = PHY_SPEED_10;
			break;
	}
	switch(tmp & PHYCFGR_OPMDC_ALLA) {
		case PHYCFGR_OPMDC_100FA:
		case PHYCFGR_OPMDC_100F:
		case PHYCFGR_OPMDC_10F:
			phyconf->duplex = PHY_DUPLEX_FULL;
			break;
		default:
			phyconf->duplex = PHY_DUPLEX_HALF;
			break;
	}
}

void wizphy_getphystat(wiz_PhyConf* phyconf)
{
	uint8_t tmp = getPHYCFGR();
	phyconf->duplex = (tmp & PHYCFGR_DPX_FULL) ? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF;
	phyconf->speed = (tmp & PHYCFGR_SPD_100) ? PHY_SPEED_100 : PHY_SPEED_10;
}

int8_t wizphy_setphypmode(uint8_t pmode)
{
	uint8_t tmp = 0;
	tmp = getPHYCFGR();
	if ((tmp & PHYCFGR_OPMD)== 0)	return -1;
	tmp &= ~PHYCFGR_OPMDC_ALLA;
	if (pmode == PHY_POWER_DOWN)
		tmp |= PHYCFGR_OPMDC_PDOWN;
	else	tmp |= PHYCFGR_OPMDC_ALLA;
	setPHYCFGR(tmp);
	wizphy_reset();
	tmp = getPHYCFGR();
	if (pmode == PHY_POWER_DOWN) {
		if (tmp & PHYCFGR_OPMDC_PDOWN)	return 0;
	}
	else {
		if (tmp & PHYCFGR_OPMDC_ALLA)	return 0;
	}
	return -1;
}

void wizchip_setnetinfo(wiz_NetInfo* pnetinfo)
{
	setSHAR(pnetinfo->mac);
	setGAR(pnetinfo->gw);
	setSUBR(pnetinfo->sn);
	setSIPR(pnetinfo->ip);
	_DNS_[0] = pnetinfo->dns[0];
	_DNS_[1] = pnetinfo->dns[1];
	_DNS_[2] = pnetinfo->dns[2];
	_DNS_[3] = pnetinfo->dns[3];
	_DHCP_	= pnetinfo->dhcp;
}

void wizchip_getnetinfo(wiz_NetInfo* pnetinfo)
{
	getSHAR(pnetinfo->mac);
	getGAR(pnetinfo->gw);
	getSUBR(pnetinfo->sn);
	getSIPR(pnetinfo->ip);
	pnetinfo->dns[0]= _DNS_[0];
	pnetinfo->dns[1]= _DNS_[1];
	pnetinfo->dns[2]= _DNS_[2];
	pnetinfo->dns[3]= _DNS_[3];
	pnetinfo->dhcp  = _DHCP_;
}

int8_t wizchip_setnetmode(netmode_type netmode)
{
	uint8_t tmp = 0;
	if (netmode & ~(NM_WAKEONLAN | NM_PPPOE | NM_PINGBLOCK | NM_FORCEARP))	return -1;
	tmp = getMR();
	tmp |= (uint8_t)netmode;
	setMR(tmp);
	return 0;
}

netmode_type wizchip_getnetmode(void)
{
	return (netmode_type) getMR();
}

void wizchip_settimeout(wiz_NetTimeout* nettime)
{
	setRCR(nettime->retry_cnt);
	setRTR(nettime->time_100us);
}

void wizchip_gettimeout(wiz_NetTimeout* nettime)
{
	nettime->retry_cnt = getRCR();
	nettime->time_100us = getRTR();
}
