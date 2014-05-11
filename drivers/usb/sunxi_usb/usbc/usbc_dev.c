/*
********************************************************************************************************************
*                                              usb controller
*
*                              (c) Copyright 2007-2009,
*										All	Rights Reserved
*
* File Name 	: usbc_dev.c
*
* Author 		: daniel
*
* Version 		: 1.0
*
* Date 			: 2009.09.01
*
* Description 	: ☠☠☠sunii平台☠USB锟侥达拷☠原锟接诧拷☠
*
* History 		:
*
********************************************************************************************************************
*/

#include  "usbc_i.h"

/*
 ***************************************************************************
 *
 * 选☠ usb device 锟侥达拷☠☠锟酵★拷☠☠拼☠洹⑼拷☠☠洹拷卸洗☠洹拷☠☠☠洹
 *
 ***************************************************************************
 */
static void __USBC_Dev_TsType_default(__u32 usbc_base_addr)
{
    //disable☠锟叫达拷☠☠☠
    USBC_REG_clear_bit_b(USBC_BP_POWER_D_ISO_UPDATE_EN, USBC_REG_PCTL(usbc_base_addr));
}

static void __USBC_Dev_TsType_Ctrl(__u32 usbc_base_addr)
{
    //--<1>--disable☠☠☠☠☠☠
    USBC_REG_clear_bit_b(USBC_BP_POWER_D_ISO_UPDATE_EN, USBC_REG_PCTL(usbc_base_addr));

	//--<2>--选☠ Ctrl ☠☠
	/* ☠☠要☠☠ */
}

static void __USBC_Dev_TsType_Iso(__u32 usbc_base_addr)
{
    //--<1>--disable☠☠☠☠☠☠
    /* ☠☠要☠☠ */

    //--<2>--选☠ Ctrl ☠☠
    USBC_REG_set_bit_b(USBC_BP_POWER_D_ISO_UPDATE_EN, USBC_REG_PCTL(usbc_base_addr));
}

static void __USBC_Dev_TsType_Int(__u32 usbc_base_addr)
{
    //--<1>--disable☠☠☠☠☠☠
    USBC_REG_clear_bit_b(USBC_BP_POWER_D_ISO_UPDATE_EN, USBC_REG_PCTL(usbc_base_addr));

	//--<2>--选☠ Ctrl ☠☠
	/* ☠☠要☠☠ */
}

static void __USBC_Dev_TsType_Bulk(__u32 usbc_base_addr)
{
    //--<1>--disable☠☠☠☠☠☠
    USBC_REG_clear_bit_b(USBC_BP_POWER_D_ISO_UPDATE_EN, USBC_REG_PCTL(usbc_base_addr));

	//--<2>--选☠ Ctrl ☠☠
	/* ☠☠要☠☠ */
}

/*
 ***************************************************************************
 *
 * 选☠ usb device ☠锟劫讹拷☠锟酵★拷☠ ☠锟劫★拷全锟劫★拷☠锟劫★拷
 *
 ***************************************************************************
 */

static void __USBC_Dev_TsMode_default(__u32 usbc_base_addr)
{
    USBC_REG_clear_bit_b(USBC_BP_POWER_D_HIGH_SPEED_EN, USBC_REG_PCTL(usbc_base_addr));
}

static void __USBC_Dev_TsMode_Hs(__u32 usbc_base_addr)
{
    USBC_REG_set_bit_b(USBC_BP_POWER_D_HIGH_SPEED_EN, USBC_REG_PCTL(usbc_base_addr));
}

static void __USBC_Dev_TsMode_Fs(__u32 usbc_base_addr)
{
    USBC_REG_clear_bit_b(USBC_BP_POWER_D_HIGH_SPEED_EN, USBC_REG_PCTL(usbc_base_addr));
}

static void __USBC_Dev_TsMode_Ls(__u32 usbc_base_addr)
{
    //硬☠☠支☠ls, ☠☠默☠选☠fs
    __USBC_Dev_TsMode_Fs(usbc_base_addr);
}

static void __USBC_Dev_ep0_ConfigEp0_Default(__u32 usbc_base_addr)
{
    USBC_Writew(1<<USBC_BP_CSR0_D_FLUSH_FIFO, USBC_REG_CSR0(usbc_base_addr));
}

static void __USBC_Dev_ep0_ConfigEp0(__u32 usbc_base_addr)
{
    USBC_Writew(1<<USBC_BP_CSR0_D_FLUSH_FIFO, USBC_REG_CSR0(usbc_base_addr));
}

static __u32 __USBC_Dev_ep0_IsReadDataReady(__u32 usbc_base_addr)
{
    return USBC_REG_test_bit_w(USBC_BP_CSR0_D_RX_PKT_READY, USBC_REG_CSR0(usbc_base_addr));
}

static __u32 __USBC_Dev_ep0_IsWriteDataReady(__u32 usbc_base_addr)
{
    return USBC_REG_test_bit_w(USBC_BP_CSR0_D_TX_PKT_READY, USBC_REG_CSR0(usbc_base_addr));
}

static void __USBC_Dev_ep0_ReadDataHalf(__u32 usbc_base_addr)
{
	USBC_Writew(1<<USBC_BP_CSR0_D_SERVICED_RX_PKT_READY, USBC_REG_CSR0(usbc_base_addr));
}

static void __USBC_Dev_ep0_ReadDataComplete(__u32 usbc_base_addr)
{
    USBC_Writew((1<<USBC_BP_CSR0_D_SERVICED_RX_PKT_READY) | (1<<USBC_BP_CSR0_D_DATA_END),
	            USBC_REG_CSR0(usbc_base_addr));
}

static void __USBC_Dev_ep0_WriteDataHalf(__u32 usbc_base_addr)
{
	USBC_Writew(1<<USBC_BP_CSR0_D_TX_PKT_READY, USBC_REG_CSR0(usbc_base_addr));
}

static void __USBC_Dev_ep0_WriteDataComplete(__u32 usbc_base_addr)
{
    USBC_Writew((1<<USBC_BP_CSR0_D_TX_PKT_READY) | (1<<USBC_BP_CSR0_D_DATA_END),
	            USBC_REG_CSR0(usbc_base_addr));
}

static __u32 __USBC_Dev_ep0_IsEpStall(__u32 usbc_base_addr)
{
    return USBC_REG_test_bit_w(USBC_BP_CSR0_D_SENT_STALL, USBC_REG_CSR0(usbc_base_addr));
}

static void __USBC_Dev_ep0_SendStall(__u32 usbc_base_addr)
{
    USBC_REG_set_bit_w(USBC_BP_CSR0_D_SEND_STALL,
		               USBC_REG_CSR0(usbc_base_addr));
}

static void __USBC_Dev_ep0_ClearStall(__u32 usbc_base_addr)
{
    USBC_REG_clear_bit_w(USBC_BP_CSR0_D_SEND_STALL, USBC_REG_CSR0(usbc_base_addr));
    USBC_REG_clear_bit_w(USBC_BP_CSR0_D_SENT_STALL, USBC_REG_CSR0(usbc_base_addr));
}

static __u32 __USBC_Dev_ep0_IsSetupEnd(__u32 usbc_base_addr)
{
    return USBC_REG_test_bit_w(USBC_BP_CSR0_D_SETUP_END, USBC_REG_CSR0(usbc_base_addr));
}

static void __USBC_Dev_ep0_ClearSetupEnd(__u32 usbc_base_addr)
{
    USBC_REG_set_bit_w(USBC_BP_CSR0_D_SERVICED_SETUP_END, USBC_REG_CSR0(usbc_base_addr));
}

static void __USBC_Dev_Tx_EnableIsoEp(__u32 usbc_base_addr)
{
	USBC_REG_set_bit_w(USBC_BP_TXCSR_D_ISO, USBC_REG_TXCSR(usbc_base_addr));
}

static void __USBC_Dev_Tx_EnableIntEp(__u32 usbc_base_addr)
{
	USBC_REG_clear_bit_w(USBC_BP_TXCSR_D_ISO, USBC_REG_TXCSR(usbc_base_addr));
}

static void __USBC_Dev_Tx_EnableBulkEp(__u32 usbc_base_addr)
{
	USBC_REG_clear_bit_w(USBC_BP_TXCSR_D_ISO, USBC_REG_TXCSR(usbc_base_addr));
}

static void __USBC_Dev_Tx_ConfigEp_Default(__u32 usbc_base_addr)
{
    //--<1>--clear tx csr
	USBC_Writew(0x00, USBC_REG_TXCSR(usbc_base_addr));

	//--<2>--clear tx ep max packet
	USBC_Writew(0x00, USBC_REG_TXMAXP(usbc_base_addr));

	//--<3>--config ep transfer type
}

static void __USBC_Dev_Tx_ConfigEp(__u32 usbc_base_addr, __u32 ts_type, __u32 is_double_fifo, __u32 ep_MaxPkt)
{
    __u16 reg_val = 0;
	__u16 temp = 0;

    //--<1>--config tx csr
	reg_val = (1 << USBC_BP_TXCSR_D_MODE);
	reg_val |= (1 << USBC_BP_TXCSR_D_CLEAR_DATA_TOGGLE);
	reg_val |= (1 << USBC_BP_TXCSR_D_FLUSH_FIFO);
	USBC_Writew(reg_val, USBC_REG_TXCSR(usbc_base_addr));

	if(is_double_fifo){
		USBC_Writew(reg_val, USBC_REG_TXCSR(usbc_base_addr));
	}

	//--<2>--config tx ep max packet
	reg_val = USBC_Readw(USBC_REG_TXMAXP(usbc_base_addr));
	temp    = ep_MaxPkt & ((1 << USBC_BP_TXMAXP_PACKET_COUNT) - 1);
	reg_val |= temp;
	USBC_Writew(reg_val, USBC_REG_TXMAXP(usbc_base_addr));

	//--<3>--config ep transfer type
	switch(ts_type){
		case USBC_TS_TYPE_ISO:
			__USBC_Dev_Tx_EnableIsoEp(usbc_base_addr);
		break;

		case USBC_TS_TYPE_INT:
			__USBC_Dev_Tx_EnableIntEp(usbc_base_addr);
		break;

		case USBC_TS_TYPE_BULK:
			__USBC_Dev_Tx_EnableBulkEp(usbc_base_addr);
		break;

		default:
			__USBC_Dev_Tx_EnableBulkEp(usbc_base_addr);
	}
}

static void __USBC_Dev_Tx_ConfigEpDma(__u32 usbc_base_addr)
{
    __u16 ep_csr = 0;

	//auto_set, tx_mode, dma_tx_en, mode1
	ep_csr = USBC_Readb(USBC_REG_TXCSR(usbc_base_addr) + 1);
	ep_csr |= (1 << USBC_BP_TXCSR_D_AUTOSET) >> 8;
	ep_csr |= (1 << USBC_BP_TXCSR_D_MODE) >> 8;
	ep_csr |= (1 << USBC_BP_TXCSR_D_DMA_REQ_EN) >> 8;
	ep_csr |= (1 << USBC_BP_TXCSR_D_DMA_REQ_MODE) >> 8;
	USBC_Writeb(ep_csr, (USBC_REG_TXCSR(usbc_base_addr) + 1));
}

static void __USBC_Dev_Tx_ClearEpDma(__u32 usbc_base_addr)
{
    __u16 ep_csr = 0;

	//auto_set, dma_tx_en, mode1
	ep_csr = USBC_Readb(USBC_REG_TXCSR(usbc_base_addr) + 1);
	ep_csr &= ~((1 << USBC_BP_TXCSR_D_AUTOSET) >> 8);
	ep_csr &= ~((1 << USBC_BP_TXCSR_D_DMA_REQ_EN) >> 8);
	USBC_Writeb(ep_csr, (USBC_REG_TXCSR(usbc_base_addr) + 1));

	//DMA_REQ_EN☠DMA_REQ_MODE☠☠☠同一☠cycle☠☠锟
	ep_csr = USBC_Readb(USBC_REG_TXCSR(usbc_base_addr) + 1);
	ep_csr &= ~((1 << USBC_BP_TXCSR_D_DMA_REQ_MODE) >> 8);
	USBC_Writeb(ep_csr, (USBC_REG_TXCSR(usbc_base_addr) + 1));
}

static __u32 __USBC_Dev_Tx_IsWriteDataReady(__u32 usbc_base_addr)
{
    __u32 temp = 0;

    temp = USBC_Readw(USBC_REG_TXCSR(usbc_base_addr));
	temp &= (1 << USBC_BP_TXCSR_D_TX_READY) | (1 << USBC_BP_TXCSR_D_FIFO_NOT_EMPTY);

	return temp;
}

static void __USBC_Dev_Tx_WriteDataHalf(__u32 usbc_base_addr)
{
    __u16 ep_csr = 0;

	ep_csr = USBC_Readw(USBC_REG_TXCSR(usbc_base_addr));
	ep_csr |= 1 << USBC_BP_TXCSR_D_TX_READY;
	ep_csr &= ~(1 << USBC_BP_TXCSR_D_UNDER_RUN);
	USBC_Writew(ep_csr, USBC_REG_TXCSR(usbc_base_addr));
}

static void __USBC_Dev_Tx_WriteDataComplete(__u32 usbc_base_addr)
{
    __u16 ep_csr = 0;

	ep_csr = USBC_Readw(USBC_REG_TXCSR(usbc_base_addr));
	ep_csr |= 1 << USBC_BP_TXCSR_D_TX_READY;
	ep_csr &= ~(1 << USBC_BP_TXCSR_D_UNDER_RUN);
	USBC_Writew(ep_csr, USBC_REG_TXCSR(usbc_base_addr));
}

static void __USBC_Dev_Tx_SendStall(__u32 usbc_base_addr)
{
    //send stall, and fifo is flushed automaticly
	USBC_REG_set_bit_w(USBC_BP_TXCSR_D_SEND_STALL, USBC_REG_TXCSR(usbc_base_addr));
}

static __u32 __USBC_Dev_Tx_IsEpStall(__u32 usbc_base_addr)
{
	return USBC_REG_test_bit_w(USBC_BP_TXCSR_D_SENT_STALL, USBC_REG_TXCSR(usbc_base_addr));
}


static void __USBC_Dev_Tx_ClearStall(__u32 usbc_base_addr)
{
	__u32 reg_val;

	reg_val = USBC_Readw(USBC_REG_TXCSR(usbc_base_addr));
	reg_val &= ~((1 << USBC_BP_TXCSR_D_SENT_STALL)|(1 << USBC_BP_TXCSR_D_SEND_STALL));
	USBC_Writew(reg_val, USBC_REG_TXCSR(usbc_base_addr));
}

//☠☠为☠去☠☠锟绞憋拷锟warning, ☠☠☠☠☠锟轿碉拷锟
/*
static __u32 __USBC_Dev_Tx_IsEpIncomp(__u32 usbc_base_addr)
{
	return USBC_REG_test_bit_w(USBC_BP_TXCSR_D_INCOMPLETE, USBC_REG_TXCSR(usbc_base_addr));
}

static void __USBC_Dev_Tx_ClearIncomp(__u32 usbc_base_addr)
{
	USBC_REG_clear_bit_w(USBC_BP_TXCSR_D_INCOMPLETE, USBC_REG_TXCSR(usbc_base_addr));
}
*/
static void __USBC_Dev_Rx_EnableIsoEp(__u32 usbc_base_addr)
{
	USBC_REG_set_bit_w(USBC_BP_RXCSR_D_ISO, USBC_REG_RXCSR(usbc_base_addr));
}

static void __USBC_Dev_Rx_EnableIntEp(__u32 usbc_base_addr)
{
	USBC_REG_clear_bit_w(USBC_BP_RXCSR_D_ISO, USBC_REG_RXCSR(usbc_base_addr));
}

static void __USBC_Dev_Rx_EnableBulkEp(__u32 usbc_base_addr)
{
	USBC_REG_clear_bit_w(USBC_BP_RXCSR_D_ISO, USBC_REG_RXCSR(usbc_base_addr));
}

static void __USBC_Dev_Rx_ConfigEp_Default(__u32 usbc_base_addr)
{
    //--<1>--clear tx csr
	USBC_Writew(0x00, USBC_REG_RXCSR(usbc_base_addr));

	//--<2>--clear tx ep max packet
	USBC_Writew(0x00, USBC_REG_RXMAXP(usbc_base_addr));

	//--<3>--config ep transfer type
}

static void __USBC_Dev_Rx_ConfigEp(__u32 usbc_base_addr, __u32 ts_type, __u32 is_double_fifo, __u32 ep_MaxPkt)
{
    __u16 reg_val = 0;
	__u16 temp = 0;

    //--<1>--config tx csr
    USBC_Writew((1 << USBC_BP_RXCSR_D_CLEAR_DATA_TOGGLE) | (1 << USBC_BP_RXCSR_D_FLUSH_FIFO),
                USBC_REG_RXCSR(usbc_base_addr));

	if(is_double_fifo){
		USBC_Writew((1 << USBC_BP_RXCSR_D_CLEAR_DATA_TOGGLE) | (1 << USBC_BP_RXCSR_D_FLUSH_FIFO),
                    USBC_REG_RXCSR(usbc_base_addr));
	}

	//--<2>--config tx ep max packet
	reg_val = USBC_Readw(USBC_REG_RXMAXP(usbc_base_addr));
	temp    = ep_MaxPkt & ((1 << USBC_BP_RXMAXP_PACKET_COUNT) - 1);
	reg_val |= temp;
	USBC_Writew(reg_val, USBC_REG_RXMAXP(usbc_base_addr));

	//--<3>--config ep transfer type
	switch(ts_type){
		case USBC_TS_TYPE_ISO:
			__USBC_Dev_Rx_EnableIsoEp(usbc_base_addr);
		break;

		case USBC_TS_TYPE_INT:
			__USBC_Dev_Rx_EnableIntEp(usbc_base_addr);
		break;

		case USBC_TS_TYPE_BULK:
			__USBC_Dev_Rx_EnableBulkEp(usbc_base_addr);
		break;

		default:
			__USBC_Dev_Rx_EnableBulkEp(usbc_base_addr);
	}
}

static void __USBC_Dev_Rx_ConfigEpDma(__u32 usbc_base_addr)
{
    __u16 ep_csr = 0;

    //auto_clear, dma_rx_en, mode0
	ep_csr = USBC_Readb(USBC_REG_RXCSR(usbc_base_addr) + 1);
	ep_csr |= ((1 << USBC_BP_RXCSR_D_AUTO_CLEAR) >> 8);
	ep_csr &= ~((1 << USBC_BP_RXCSR_D_DMA_REQ_MODE) >> 8);
	ep_csr |= ((1 << USBC_BP_RXCSR_D_DMA_REQ_EN) >> 8);
	USBC_Writeb(ep_csr, (USBC_REG_RXCSR(usbc_base_addr) + 1));
}

static void __USBC_Dev_Rx_ClearEpDma(__u32 usbc_base_addr)
{
    __u16 ep_csr = 0;

    //auto_clear, dma_rx_en, mode0
	ep_csr = USBC_Readb(USBC_REG_RXCSR(usbc_base_addr) + 1);

	ep_csr &= ~((1 << USBC_BP_RXCSR_D_AUTO_CLEAR) >> 8);
	ep_csr &= ~((1 << USBC_BP_RXCSR_D_DMA_REQ_MODE) >> 8);
	ep_csr &= ~((1 << USBC_BP_RXCSR_D_DMA_REQ_EN) >> 8);
	USBC_Writeb(ep_csr, (USBC_REG_RXCSR(usbc_base_addr) + 1));
}

static __u32 __USBC_Dev_Rx_IsReadDataReady(__u32 usbc_base_addr)
{
    return USBC_REG_test_bit_w(USBC_BP_RXCSR_D_RX_PKT_READY, USBC_REG_RXCSR(usbc_base_addr));
}

static void __USBC_Dev_Rx_ReadDataHalf(__u32 usbc_base_addr)
{
	__u32 reg_val = 0;

	//overrun, dataerr is used in iso transfer
    reg_val = USBC_Readw(USBC_REG_RXCSR(usbc_base_addr));
	reg_val &= ~(1 << USBC_BP_RXCSR_D_RX_PKT_READY);
	reg_val &= ~(1 << USBC_BP_RXCSR_D_OVERRUN);
	reg_val &= ~(1 << USBC_BP_RXCSR_D_DATA_ERROR);
	USBC_Writew(reg_val, USBC_REG_RXCSR(usbc_base_addr));
}

static void __USBC_Dev_Rx_ReadDataComplete(__u32 usbc_base_addr)
{
    __u32 reg_val = 0;

	//overrun, dataerr is used in iso transfer
	reg_val = USBC_Readw(USBC_REG_RXCSR(usbc_base_addr));
	reg_val &= ~(1 << USBC_BP_RXCSR_D_RX_PKT_READY);
	reg_val &= ~(1 << USBC_BP_RXCSR_D_OVERRUN);
	reg_val &= ~(1 << USBC_BP_RXCSR_D_DATA_ERROR);
	USBC_Writew(reg_val, USBC_REG_RXCSR(usbc_base_addr));
}

static void __USBC_Dev_Rx_SendStall(__u32 usbc_base_addr)
{
	USBC_REG_set_bit_w(USBC_BP_RXCSR_D_SEND_STALL, USBC_REG_RXCSR(usbc_base_addr));
}

static __u32 __USBC_Dev_Rx_IsEpStall(__u32 usbc_base_addr)
{
	return USBC_REG_test_bit_w(USBC_BP_RXCSR_D_SENT_STALL, USBC_REG_RXCSR(usbc_base_addr));
}

static void __USBC_Dev_Rx_ClearStall(__u32 usbc_base_addr)
{
    USBC_REG_clear_bit_w(USBC_BP_RXCSR_D_SEND_STALL, USBC_REG_RXCSR(usbc_base_addr));
	USBC_REG_clear_bit_w(USBC_BP_RXCSR_D_SENT_STALL, USBC_REG_RXCSR(usbc_base_addr));
}

static void __USBC_Dev_ClearDma_Trans(__u32 usbc_base_addr)
{
	__u32 reg_val;

	reg_val  = readl(usbc_base_addr + USBC_REG_o_PCTL);
	reg_val &= ~(1 << 24);
	writel(reg_val, usbc_base_addr + USBC_REG_o_PCTL);
}

static void __USBC_Dev_ConfigDma_Trans(__u32 usbc_base_addr)
{
	__u32 reg_val;

	reg_val  = readl(usbc_base_addr + USBC_REG_o_PCTL);
	reg_val |= (1 << 24);
	writel(reg_val, usbc_base_addr + USBC_REG_o_PCTL);
}
/*
***********************************************************************************
*                     USBC_Dev_SetAddress_default
*
* Description:
*    ☠锟host☠device☠☠牡锟街
*
* Arguments:
*    hUSB       :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*
* Returns:
*
*
* note:
*    ☠
*
***********************************************************************************
*/
void USBC_Dev_SetAddress_default(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return;
	}

    USBC_Writeb(0x00, USBC_REG_FADDR(usbc_otg->base_addr));
}

/*
***********************************************************************************
*                     USBC_Dev_SetAddress
*
* Description:
*    ☠锟矫碉拷址
*
* Arguments:
*    hUSB       :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*    address    :  input.  host☠☠牡锟街
*
* Returns:
*
*
* note:
*    ☠
*
***********************************************************************************
*/
void USBC_Dev_SetAddress(__hdle hUSB, __u8 address)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return;
	}

    USBC_Writeb(address, USBC_REG_FADDR(usbc_otg->base_addr));
}

__u32 USBC_Dev_QueryTransferMode(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return USBC_TS_MODE_UNKOWN;
	}

	if(USBC_REG_test_bit_b(USBC_BP_POWER_D_HIGH_SPEED_FLAG, USBC_REG_PCTL(usbc_otg->base_addr))){
		return USBC_TS_MODE_HS;
	}else{
	    return USBC_TS_MODE_FS;
	}
}

/*
***********************************************************************************
*                     USBC_Dev_ConfigTransferMode
*
* Description:
*    ☠☠device锟侥达拷☠☠锟酵猴拷锟劫讹拷模式
*
* Arguments:
*    hUSB       :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*    type       :  input.  ☠☠☠☠
*    speed_mode :  input.  锟劫讹拷模式
*
* Returns:
*
*
* note:
*    ☠
*
***********************************************************************************
*/
void USBC_Dev_ConfigTransferMode(__hdle hUSB, __u8 ts_type, __u8 speed_mode)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return;
	}

    //--<1>--选☠☠☠☠
    //默☠☠bulk☠☠
    switch(ts_type){
		case USBC_TS_TYPE_CTRL:
			__USBC_Dev_TsType_Ctrl(usbc_otg->base_addr);
		break;

		case USBC_TS_TYPE_ISO:
			__USBC_Dev_TsType_Iso(usbc_otg->base_addr);
		break;

		case USBC_TS_TYPE_INT:
			__USBC_Dev_TsType_Int(usbc_otg->base_addr);
		break;

		case USBC_TS_TYPE_BULK:
			__USBC_Dev_TsType_Bulk(usbc_otg->base_addr);
		break;

		default:
			__USBC_Dev_TsType_default(usbc_otg->base_addr);
	}

    //--<2>--选☠☠锟劫讹拷
    switch(speed_mode){
		case USBC_TS_MODE_HS:
			__USBC_Dev_TsMode_Hs(usbc_otg->base_addr);
		break;

		case USBC_TS_MODE_FS:
			__USBC_Dev_TsMode_Fs(usbc_otg->base_addr);
		break;

		case USBC_TS_MODE_LS:
			__USBC_Dev_TsMode_Ls(usbc_otg->base_addr);
		break;

		default:
			__USBC_Dev_TsMode_default(usbc_otg->base_addr);
	}
}

/*
***********************************************************************************
*                     USBC_Dev_ConectSwitch
*
* Description:
*    ☠PC通锟脚的匡拷☠
*
* Arguments:
*    hUSB   :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*    is_on  :  input.  1: ☠☠☠PC通锟脚匡拷☠. 0: 锟截闭猴拷PC通☠通☠
*
* Returns:
*
*
* note:
*    ☠
*
***********************************************************************************
*/
void USBC_Dev_ConectSwitch(__hdle hUSB, __u32 is_on)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	if(is_on == USBC_DEVICE_SWITCH_ON){
		USBC_REG_set_bit_b(USBC_BP_POWER_D_SOFT_CONNECT, USBC_REG_PCTL(usbc_otg->base_addr));
	}else{
		USBC_REG_clear_bit_b(USBC_BP_POWER_D_SOFT_CONNECT, USBC_REG_PCTL(usbc_otg->base_addr));
	}
}

/*
***********************************************************************************
*                     USBC_Dev_PowerStatus
*
* Description:
*    ☠询☠前device☠☠☠状态, ☠reset☠resume☠suspend☠状态☠
*
* Arguments:
*    hUSB   :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*
* Returns:
*
*
* note:
*    ☠
*
***********************************************************************************
*/
__u32 USBC_Dev_QueryPowerStatus(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

    return (USBC_Readb(USBC_REG_PCTL(usbc_otg->base_addr)) & 0x0f);
}

/*
***********************************************************************************
*                     USBC_Dev_ConfigEp
*
* Description:
*    ☠☠ep, ☠☠双FIFO☠☠☠☠☠
*
* Arguments:
*    hUSB           :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*    ep_type        :  input.  ☠☠☠☠
*    is_double_fifo :  input.  锟劫讹拷模式
*    ep_MaxPkt      :  input.  ☠☠
*
* Returns:
*    0  :  锟缴癸拷
*   !0  :  失☠
*
* note:
*    ☠
*
***********************************************************************************
*/
__s32 USBC_Dev_ConfigEp(__hdle hUSB, __u32 ts_type, __u32 ep_type, __u32 is_double_fifo, __u32 ep_MaxPkt)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return -1;
	}

	switch(ep_type){
		case USBC_EP_TYPE_EP0:
			__USBC_Dev_ep0_ConfigEp0(usbc_otg->base_addr);
		break;

		case USBC_EP_TYPE_TX:
			__USBC_Dev_Tx_ConfigEp(usbc_otg->base_addr, ts_type, is_double_fifo,  ep_MaxPkt);
		break;

		case USBC_EP_TYPE_RX:
			__USBC_Dev_Rx_ConfigEp(usbc_otg->base_addr, ts_type, is_double_fifo, ep_MaxPkt);
		break;

		default:
			return -1;
	}

    return 0;
}

/*
***********************************************************************************
*                     USBC_Dev_ConfigEp
*
* Description:
*    锟酵凤拷ep☠锟叫碉拷☠源, 锟叫断筹拷☠
*
* Arguments:
*    hUSB           :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*
* Returns:
*    0  :  锟缴癸拷
*   !0  :  失☠
*
* note:
*    ☠
*
***********************************************************************************
*/
__s32 USBC_Dev_ConfigEp_Default(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return -1;
	}

	switch(ep_type){
		case USBC_EP_TYPE_EP0:
			__USBC_Dev_ep0_ConfigEp0_Default(usbc_otg->base_addr);
		break;

		case USBC_EP_TYPE_TX:
			__USBC_Dev_Tx_ConfigEp_Default(usbc_otg->base_addr);
		break;

		case USBC_EP_TYPE_RX:
			__USBC_Dev_Rx_ConfigEp_Default(usbc_otg->base_addr);
		break;

		default:
			return -1;
	}

    return 0;
}

/*
***********************************************************************************
*                     USBC_Dev_ConfigEpDma
*
* Description:
*    ☠☠ep☠dma☠☠
*
* Arguments:
*    hUSB           :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*    ep_type        :  input.  ☠☠☠☠
* Returns:
*    0  :  锟缴癸拷
*   !0  :  失☠
*
* note:
*    ☠
*
***********************************************************************************
*/
__s32 USBC_Dev_ConfigEpDma(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return -1;
	}

	switch(ep_type){
		case USBC_EP_TYPE_EP0:
			//not support

			return -1;
		//break;

		case USBC_EP_TYPE_TX:
			__USBC_Dev_Tx_ConfigEpDma(usbc_otg->base_addr);
		//	__USBC_Dev_ConfigDma_Trans(usbc_otg->base_addr);
		break;

		case USBC_EP_TYPE_RX:
			__USBC_Dev_Rx_ConfigEpDma(usbc_otg->base_addr);
		//	__USBC_Dev_ConfigDma_Trans(usbc_otg->base_addr);
		break;

		default:
			return -1;
	}

    return 0;
}

/*
***********************************************************************************
*                     USBC_Dev_ClearEpDma
*
* Description:
*    ☠锟ep☠dma☠☠
*
* Arguments:
*    hUSB           :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*    ep_type        :  input.  ☠☠☠☠
* Returns:
*    0  :  锟缴癸拷
*   !0  :  失☠
*
* note:
*    ☠
*
***********************************************************************************
*/
__s32 USBC_Dev_ClearEpDma(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return -1;
	}

	switch(ep_type){
		case USBC_EP_TYPE_EP0:
			//not support

			return -1;
		//break;

		case USBC_EP_TYPE_TX:
			__USBC_Dev_Tx_ClearEpDma(usbc_otg->base_addr);
			__USBC_Dev_ClearDma_Trans(usbc_otg->base_addr);
		break;

		case USBC_EP_TYPE_RX:
			__USBC_Dev_Rx_ClearEpDma(usbc_otg->base_addr);
			__USBC_Dev_ClearDma_Trans(usbc_otg->base_addr);
		break;

		default:
			return -1;
	}

    return 0;
}

/*
***********************************************************************************
*                     USBC_Dev_IsEpStall
*
* Description:
*    ☠询ep锟角凤拷stall
*
* Arguments:
*    hUSB           :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*    ep_type        :  input.  ☠☠☠☠
* Returns:
*    0  :  锟缴癸拷
*   !0  :  失☠
*
* note:
*    ☠
*
***********************************************************************************
*/
__s32 USBC_Dev_IsEpStall(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return -1;
	}

    switch(ep_type){
		case USBC_EP_TYPE_EP0:
			__USBC_Dev_ep0_IsEpStall(usbc_otg->base_addr);
		break;

		case USBC_EP_TYPE_TX:
			__USBC_Dev_Tx_IsEpStall(usbc_otg->base_addr);
		break;

		case USBC_EP_TYPE_RX:
			__USBC_Dev_Rx_IsEpStall(usbc_otg->base_addr);
		break;

		default:
			return -1;
	}

    return 0;
}

/*
***********************************************************************************
*                     USBC_Dev_EpEnterStall
*
* Description:
*    使ep☠☠stall状态
*
* Arguments:
*    hUSB           :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*    ep_type        :  input.  ☠☠☠☠
* Returns:
*    0  :  锟缴癸拷
*   !0  :  失☠
*
* note:
*    ☠
*
***********************************************************************************
*/
__s32 USBC_Dev_EpSendStall(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return -1;
	}

    switch(ep_type){
		case USBC_EP_TYPE_EP0:
			__USBC_Dev_ep0_SendStall(usbc_otg->base_addr);
		break;

		case USBC_EP_TYPE_TX:
			__USBC_Dev_Tx_SendStall(usbc_otg->base_addr);
		break;

		case USBC_EP_TYPE_RX:
			__USBC_Dev_Rx_SendStall(usbc_otg->base_addr);
		break;

		default:
			return -1;
	}

    return 0;
}

/*
***********************************************************************************
*                     USBC_Dev_EpClearStall
*
* Description:
*    ☠锟ep☠stall状态
*
* Arguments:
*    hUSB           :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*    ep_type        :  input.  ☠☠☠☠
* Returns:
*    0  :  锟缴癸拷
*   !0  :  失☠
*
* note:
*    ☠
*
***********************************************************************************
*/
__s32 USBC_Dev_EpClearStall(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return -1;
	}

    switch(ep_type){
		case USBC_EP_TYPE_EP0:
			__USBC_Dev_ep0_ClearStall(usbc_otg->base_addr);
		break;

		case USBC_EP_TYPE_TX:
			__USBC_Dev_Tx_ClearStall(usbc_otg->base_addr);
		break;

		case USBC_EP_TYPE_RX:
			__USBC_Dev_Rx_ClearStall(usbc_otg->base_addr);
		break;

		default:
			return -1;
	}

    return 0;
}

/*
***********************************************************************************
*                     USBC_Dev_Ctrl_IsSetupEnd
*
* Description:
*    ☠询ep0锟角凤拷SetupEnd
*
* Arguments:
*    hUSB  :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*
* Returns:
*
* note:
*    ☠
*
***********************************************************************************
*/
__u32 USBC_Dev_Ctrl_IsSetupEnd(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

    return __USBC_Dev_ep0_IsSetupEnd(usbc_otg->base_addr);
}

/*
***********************************************************************************
*                     USBC_Dev_Ctrl_ClearSetupEnd
*
* Description:
*    ☠锟ep0☠SetupEnd状态
*
* Arguments:
*    hUSB  :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*
* Returns:
*
* note:
*    ☠
*
***********************************************************************************
*/
void USBC_Dev_Ctrl_ClearSetupEnd(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	__USBC_Dev_ep0_ClearSetupEnd(usbc_otg->base_addr);
}


static __s32 __USBC_Dev_WriteDataHalf(__u32 usbc_base_addr, __u32 ep_type)
{
    switch(ep_type){
		case USBC_EP_TYPE_EP0:
			__USBC_Dev_ep0_WriteDataHalf(usbc_base_addr);
		break;

		case USBC_EP_TYPE_TX:
			__USBC_Dev_Tx_WriteDataHalf(usbc_base_addr);
		break;

		case USBC_EP_TYPE_RX:
			//not support
			return -1;
		//break;

		default:
			return -1;
	}

    return 0;
}

static __s32 __USBC_Dev_WriteDataComplete(__u32 usbc_base_addr, __u32 ep_type)
{
    switch(ep_type){
		case USBC_EP_TYPE_EP0:
			__USBC_Dev_ep0_WriteDataComplete(usbc_base_addr);
		break;

		case USBC_EP_TYPE_TX:
			__USBC_Dev_Tx_WriteDataComplete(usbc_base_addr);
		break;

		case USBC_EP_TYPE_RX:
			//not support
			return -1;
		//break;

		default:
			return -1;
	}

    return 0;
}

static __s32 __USBC_Dev_ReadDataHalf(__u32 usbc_base_addr, __u32 ep_type)
{
    switch(ep_type){
		case USBC_EP_TYPE_EP0:
			__USBC_Dev_ep0_ReadDataHalf(usbc_base_addr);
		break;

		case USBC_EP_TYPE_TX:
			//not support
			return -1;
		//break;

		case USBC_EP_TYPE_RX:
			__USBC_Dev_Rx_ReadDataHalf(usbc_base_addr);
		break;

		default:
			return -1;
	}

    return 0;
}

static __s32 __USBC_Dev_ReadDataComplete(__u32 usbc_base_addr, __u32 ep_type)
{
    switch(ep_type){
		case USBC_EP_TYPE_EP0:
			__USBC_Dev_ep0_ReadDataComplete(usbc_base_addr);
		break;

		case USBC_EP_TYPE_TX:
			//not support
			return -1;
		//break;

		case USBC_EP_TYPE_RX:
			__USBC_Dev_Rx_ReadDataComplete(usbc_base_addr);
		break;

		default:
			return -1;
	}

    return 0;
}

/*
***********************************************************************************
*                     USBC_Dev_WriteDataStatus
*
* Description:
*    写☠锟捷碉拷状☠, ☠写☠一☠☠, ☠☠☠全写☠☠
*
* Arguments:
*    hUSB      :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*    ep_type   :  input.  ☠☠☠☠
*    complete  :  input.  锟角凤拷☠锟叫碉拷☠锟捷讹拷写☠☠
* Returns:
*    0  :  锟缴癸拷
*   !0  :  失☠
*
* note:
*    ☠
*
***********************************************************************************
*/
__s32 USBC_Dev_WriteDataStatus(__hdle hUSB, __u32 ep_type, __u32 complete)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return -1;
	}

	if(complete){
	    return __USBC_Dev_WriteDataComplete(usbc_otg->base_addr, ep_type);
	}else{
 		return __USBC_Dev_WriteDataHalf(usbc_otg->base_addr, ep_type);
   }
}

/*
***********************************************************************************
*                     USBC_Dev_ReadDataStatus
*
* Description:
*    写☠锟捷碉拷状☠, ☠写☠一☠☠, ☠☠☠全写☠☠
*
* Arguments:
*    hUSB      :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*    ep_type   :  input.  ☠☠☠☠
*    complete  :  input.  锟角凤拷☠锟叫碉拷☠锟捷讹拷写☠☠
* Returns:
*    0  :  锟缴癸拷
*   !0  :  失☠
*
* note:
*    ☠
*
***********************************************************************************
*/
__s32 USBC_Dev_ReadDataStatus(__hdle hUSB, __u32 ep_type, __u32 complete)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return -1;
	}

	if(complete){
	    return __USBC_Dev_ReadDataComplete(usbc_otg->base_addr, ep_type);
	}else{
		return __USBC_Dev_ReadDataHalf(usbc_otg->base_addr, ep_type);
    }
}

/*
***********************************************************************************
*                     USBC_Dev_IsReadDataReady
*
* Description:
*    ☠询usb准☠☠取☠☠☠锟角凤拷准☠☠☠
*
* Arguments:
*    hUSB     :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*    ep_type  :  input.  ☠☠☠☠
*
* Returns:
*
* note:
*    ☠
*
***********************************************************************************
*/
__u32 USBC_Dev_IsReadDataReady(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

    switch(ep_type){
		case USBC_EP_TYPE_EP0:
			return __USBC_Dev_ep0_IsReadDataReady(usbc_otg->base_addr);

		case USBC_EP_TYPE_TX:
			//not support
		break;

		case USBC_EP_TYPE_RX:
			return __USBC_Dev_Rx_IsReadDataReady(usbc_otg->base_addr);

		default:
		break;
	}

	return 0;
}

/*
***********************************************************************************
*                     USBC_Dev_IsWriteDataReady
*
* Description:
*    ☠询fifo锟角凤拷为☠
*
* Arguments:
*    hUSB    :  input.  USBC_open_otg☠玫木☠, ☠录☠USBC☠☠要☠一些锟截硷拷☠☠
*    ep_type :  input.  ☠☠☠☠
*
* Returns:
*
* note:
*    ☠
*
***********************************************************************************
*/
__u32 USBC_Dev_IsWriteDataReady(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

    switch(ep_type){
		case USBC_EP_TYPE_EP0:
			return __USBC_Dev_ep0_IsWriteDataReady(usbc_otg->base_addr);

		case USBC_EP_TYPE_TX:
			return __USBC_Dev_Tx_IsWriteDataReady(usbc_otg->base_addr);

		case USBC_EP_TYPE_RX:
			//not support
		break;

		default:
		break;
	}

	return 0;
}


EXPORT_SYMBOL(USBC_Dev_SetAddress_default);
EXPORT_SYMBOL(USBC_Dev_SetAddress);

EXPORT_SYMBOL(USBC_Dev_QueryTransferMode);
EXPORT_SYMBOL(USBC_Dev_ConfigTransferMode);
EXPORT_SYMBOL(USBC_Dev_ConectSwitch);
EXPORT_SYMBOL(USBC_Dev_QueryPowerStatus);

EXPORT_SYMBOL(USBC_Dev_ConfigEp);
EXPORT_SYMBOL(USBC_Dev_ConfigEp_Default);
EXPORT_SYMBOL(USBC_Dev_ConfigEpDma);
EXPORT_SYMBOL(USBC_Dev_ClearEpDma);

EXPORT_SYMBOL(USBC_Dev_IsEpStall);
EXPORT_SYMBOL(USBC_Dev_EpSendStall);
EXPORT_SYMBOL(USBC_Dev_EpClearStall);

EXPORT_SYMBOL(USBC_Dev_Ctrl_IsSetupEnd);
EXPORT_SYMBOL(USBC_Dev_Ctrl_ClearSetupEnd);

EXPORT_SYMBOL(USBC_Dev_IsReadDataReady);
EXPORT_SYMBOL(USBC_Dev_IsWriteDataReady);
EXPORT_SYMBOL(USBC_Dev_WriteDataStatus);
EXPORT_SYMBOL(USBC_Dev_ReadDataStatus);


