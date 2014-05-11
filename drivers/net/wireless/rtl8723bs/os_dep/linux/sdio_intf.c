/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#define _HCI_INTF_C_

#include <drv_types.h>

#ifndef CONFIG_SDIO_HCI
#error "CONFIG_SDIO_HCI shall be on!\n"
#endif


#if defined(CONFIG_PLATFORM_ARM_SUN6I) || defined(CONFIG_PLATFORM_ARM_SUN8I)
#include <mach/sys_config.h>
#ifdef CONFIG_GPIO_WAKEUP
#include <linux/gpio.h>
#endif
#endif

#ifdef CONFIG_PLATFORM_ARM_SUNxI
#if defined(CONFIG_MMC_SUNXI_POWER_CONTROL)

#ifdef CONFIG_WITS_EVB_V13
#define SDIOID	0
#else
#define SDIOID (CONFIG_CHIP_ID==1123 ? 3 : 1)
#endif

#define SUNXI_SDIO_WIFI_NUM_RTL8189ES  10
extern void sunximmc_rescan_card(unsigned id, unsigned insert);
extern int mmc_pm_get_mod_type(void);
extern int mmc_pm_gpio_ctrl(char* name, int level);
/*
*	rtl8189es_shdn	= port:PH09<1><default><default><0>
*	rtl8189es_wakeup	= port:PH10<1><default><default><1>
*	rtl8189es_vdd_en  = port:PH11<1><default><default><0>
*	rtl8189es_vcc_en  = port:PH12<1><default><default><0>
*/

int rtl8189es_sdio_powerup(void)
{
	mmc_pm_gpio_ctrl("rtl8189es_vdd_en", 1);
	udelay(100);
	mmc_pm_gpio_ctrl("rtl8189es_vcc_en", 1);
	udelay(50);
	mmc_pm_gpio_ctrl("rtl8189es_shdn", 1);
	return 0;
}
int rtl8189es_sdio_poweroff(void)
{
	mmc_pm_gpio_ctrl("rtl8189es_shdn", 0);
	mmc_pm_gpio_ctrl("rtl8189es_vcc_en", 0);
	mmc_pm_gpio_ctrl("rtl8189es_vdd_en", 0);
	return 0;
}
#endif //defined(CONFIG_MMC_SUNXI_POWER_CONTROL)
#endif //CONFIG_PLATFORM_ARM_SUNxI

#ifdef CONFIG_PLATFORM_ARM_SUN6I
#ifdef CONFIG_MMC
#define SUNXI_SDIO_WIFI_NUM_RTL8723AS  3
#define SUNXI_SDIO_WIFI_NUM_RTL8723BS  10
static int sdio_id = -1;
static signed int gpio_eint_wlan = -1;
static u32 eint_wlan_handle = 0;
extern void sw_mci_rescan_card(unsigned id, unsigned insert);
extern int wifi_pm_get_mod_type(void);
extern void wifi_pm_power(int on);
#endif //CONFIG_MMC
#endif //CONFIG_PLATFORM_ARM_SUN6I

#ifdef CONFIG_PLATFORM_ARM_SUN8I
#ifdef CONFIG_MMC
#define SUNXI_SDIO_WIFI_NUM_RTL8723BS  5
static int sdio_id = -1;
static signed int gpio_eint_wlan = -1;
static u32 eint_wlan_handle = 0;
extern void sunxi_mci_rescan_card(unsigned id, unsigned insert);
extern int wifi_pm_get_mod_type(void);
extern void wifi_pm_power(int on);
#endif //CONFIG_MMC
#endif //CONFIG_PLATFORM_ARM_SUN8I


#ifndef dev_to_sdio_func
#define dev_to_sdio_func(d)     container_of(d, struct sdio_func, dev)
#endif

#ifdef CONFIG_WOWLAN
static struct mmc_host *mmc_host = NULL;
#endif

static const struct sdio_device_id sdio_ids[] =
{
#ifdef CONFIG_RTL8723A
	{ SDIO_DEVICE(0x024c, 0x8723),.driver_data = RTL8723A},
#endif //CONFIG_RTL8723A
#ifdef CONFIG_RTL8723B
	{ SDIO_DEVICE(0x024c, 0xB723),.driver_data = RTL8723B},
#endif
#ifdef CONFIG_RTL8188E
	{ SDIO_DEVICE(0x024c, 0x8179),.driver_data = RTL8188E},
#endif //CONFIG_RTL8188E
#ifdef CONFIG_RTL8821A
	{ SDIO_DEVICE(0x024c, 0x8821),.driver_data = RTL8821},
#endif //CONFIG_RTL8188E

#if defined(RTW_ENABLE_WIFI_CONTROL_FUNC) /* temporarily add this to accept all sdio wlan id */
	{ SDIO_DEVICE_CLASS(SDIO_CLASS_WLAN) },
#endif
//	{ /* end: all zeroes */				},
};

static int rtw_drv_init(struct sdio_func *func, const struct sdio_device_id *id);
static void rtw_dev_remove(struct sdio_func *func);
static int rtw_sdio_resume(struct device *dev);
static int rtw_sdio_suspend(struct device *dev);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)) 
static const struct dev_pm_ops rtw_sdio_pm_ops = {
	.suspend	= rtw_sdio_suspend,
	.resume	= rtw_sdio_resume,
};
#endif
	
struct sdio_drv_priv {
	struct sdio_driver r871xs_drv;
	int drv_registered;
};

static struct sdio_drv_priv sdio_drvpriv = {
	.r871xs_drv.probe = rtw_drv_init,
	.r871xs_drv.remove = rtw_dev_remove,
	.r871xs_drv.name = (char*)DRV_NAME,
	.r871xs_drv.id_table = sdio_ids,
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)) 
	.r871xs_drv.drv = {
		.pm = &rtw_sdio_pm_ops,
	}
	#endif
};

static void sd_sync_int_hdl(struct sdio_func *func)
{
	struct dvobj_priv *psdpriv;


	psdpriv = sdio_get_drvdata(func);

	if (!psdpriv->if1) {
		DBG_871X("%s if1 == NULL\n", __func__);
		return;
	}

	rtw_sdio_set_irq_thd(psdpriv, current);
	sd_int_hdl(psdpriv->if1);
	rtw_sdio_set_irq_thd(psdpriv, NULL);
}

int sdio_alloc_irq(struct dvobj_priv *dvobj)
{
	PSDIO_DATA psdio_data;
	struct sdio_func *func;
	int err;

	psdio_data = &dvobj->intf_data;
	func = psdio_data->func;

	sdio_claim_host(func);

	err = sdio_claim_irq(func, &sd_sync_int_hdl);
	if (err)
	{
		dvobj->drv_dbg.dbg_sdio_alloc_irq_error_cnt++;
		printk(KERN_CRIT "%s: sdio_claim_irq FAIL(%d)!\n", __func__, err);
	}
	else
	{
		dvobj->drv_dbg.dbg_sdio_alloc_irq_cnt++;
		dvobj->irq_alloc = 1;
	}

	sdio_release_host(func);

	return err?_FAIL:_SUCCESS;
}

void sdio_free_irq(struct dvobj_priv *dvobj)
{
    PSDIO_DATA psdio_data;
    struct sdio_func *func;
    int err; 

    if (dvobj->irq_alloc) {
        psdio_data = &dvobj->intf_data;
        func = psdio_data->func;

        if (func) {
            sdio_claim_host(func);
            err = sdio_release_irq(func);
            if (err)
            {
				dvobj->drv_dbg.dbg_sdio_free_irq_error_cnt++;
                DBG_871X_LEVEL(_drv_err_,"%s: sdio_release_irq FAIL(%d)!\n", __func__, err);
            }
            else
            	dvobj->drv_dbg.dbg_sdio_free_irq_cnt++;
            sdio_release_host(func);
        }    
        dvobj->irq_alloc = 0; 
    }    
}

#ifdef CONFIG_GPIO_WAKEUP
extern unsigned int oob_irq;
static irqreturn_t gpio_hostwakeup_irq_thread(int irq, void *data)
{
	PADAPTER padapter = (PADAPTER)data;
	DBG_871X_LEVEL(_drv_always_, "gpio_hostwakeup_irq_thread\n");
	/* Disable interrupt before calling handler */
	//disable_irq_nosync(oob_irq);
	rtw_lock_suspend_timeout(HZ/2);
#ifdef CONFIG_PLATFORM_ARM_SUN6I
	return 0;
#else
        return IRQ_HANDLED;
#endif
}

static u8 gpio_hostwakeup_alloc_irq(PADAPTER padapter)
{
	int err;
	if (oob_irq == 0)
		return _FAIL;
	/* dont set it IRQF_TRIGGER_LOW, or wowlan */
	/* power is high after suspend */
	/* and failing can prevent can not sleep issue if */
	/* wifi gpio12 pin is not linked with CPU */
	err = request_threaded_irq(oob_irq, gpio_hostwakeup_irq_thread, NULL,
		//IRQF_TRIGGER_LOW | IRQF_ONESHOT,
		IRQF_TRIGGER_FALLING,
		"rtw_wifi_gpio_wakeup", padapter);
	if (err < 0) {
		DBG_871X("Oops: can't allocate gpio irq %d err:%d\n", oob_irq, err);
		return _FALSE;
	} else {
		DBG_871X("allocate gpio irq %d ok\n", oob_irq);
	}
	
#ifndef CONFIG_PLATFORM_ARM_SUN8I	
	enable_irq_wake(oob_irq);
#endif

	return _SUCCESS;
}

static void gpio_hostwakeup_free_irq(PADAPTER padapter)
{
	if (oob_irq == 0)
		return;
		
#ifndef CONFIG_PLATFORM_ARM_SUN8I
	disable_irq_wake(oob_irq);
#endif

	free_irq(oob_irq, padapter);
}
#endif

static u32 sdio_init(struct dvobj_priv *dvobj)
{
	PSDIO_DATA psdio_data;
	struct sdio_func *func;
	int err;

_func_enter_;

	psdio_data = &dvobj->intf_data;
	func = psdio_data->func;

	//3 1. init SDIO bus
	sdio_claim_host(func);

	err = sdio_enable_func(func);
	if (err) {
		dvobj->drv_dbg.dbg_sdio_init_error_cnt++;
		DBG_8192C(KERN_CRIT "%s: sdio_enable_func FAIL(%d)!\n", __func__, err);
		goto release;
	}

	err = sdio_set_block_size(func, 512);
	if (err) {
		dvobj->drv_dbg.dbg_sdio_init_error_cnt++;
		DBG_8192C(KERN_CRIT "%s: sdio_set_block_size FAIL(%d)!\n", __func__, err);
		goto release;
	}
	psdio_data->block_transfer_len = 512;
	psdio_data->tx_block_mode = 1;
	psdio_data->rx_block_mode = 1;

release:
	sdio_release_host(func);

exit:
_func_exit_;

	if (err) return _FAIL;
	return _SUCCESS;
}

static void sdio_deinit(struct dvobj_priv *dvobj)
{
	struct sdio_func *func;
	int err;


	RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("+sdio_deinit\n"));

	func = dvobj->intf_data.func;

	if (func) {
		sdio_claim_host(func);
		err = sdio_disable_func(func);
		if (err)
		{
			dvobj->drv_dbg.dbg_sdio_deinit_error_cnt++;
			DBG_8192C(KERN_ERR "%s: sdio_disable_func(%d)\n", __func__, err);
		}

		if (dvobj->irq_alloc) {
			err = sdio_release_irq(func);
			if (err)
			{
				dvobj->drv_dbg.dbg_sdio_free_irq_error_cnt++;
				DBG_8192C(KERN_ERR "%s: sdio_release_irq(%d)\n", __func__, err);
		}
			else
				dvobj->drv_dbg.dbg_sdio_free_irq_cnt++;
		}

		sdio_release_host(func);
	}
}
static struct dvobj_priv *sdio_dvobj_init(struct sdio_func *func)
{
	int status = _FAIL;
	struct dvobj_priv *dvobj = NULL;
	PSDIO_DATA psdio;
_func_enter_;

	if((dvobj = devobj_init()) == NULL) {
		goto exit;
	}


#ifdef CONFIG_WOWLAN
	sdio_claim_host(func);
	sdio_set_drvdata(func, dvobj);
	mmc_host = func->card->host;
   	sdio_set_host_pm_flags(func, MMC_PM_KEEP_POWER);
	sdio_release_host(func);
#else
	sdio_set_drvdata(func, dvobj);
#endif

	psdio = &dvobj->intf_data;
	psdio->func = func;

	if (sdio_init(dvobj) != _SUCCESS) {
		RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("%s: initialize SDIO Failed!\n", __FUNCTION__));
		goto free_dvobj;
	}
	rtw_reset_continual_io_error(dvobj);
	status = _SUCCESS;

free_dvobj:
	if (status != _SUCCESS && dvobj) {
		sdio_set_drvdata(func, NULL);
		
		devobj_deinit(dvobj);
		
		dvobj = NULL;
	}
exit:
_func_exit_;
	return dvobj;
}

static void sdio_dvobj_deinit(struct sdio_func *func)
{
	struct dvobj_priv *dvobj = sdio_get_drvdata(func);
_func_enter_;

	sdio_set_drvdata(func, NULL);
	if (dvobj) {
		sdio_deinit(dvobj);
		devobj_deinit(dvobj);
	}

_func_exit_;
	return;
}
static void rtw_decide_chip_type_by_device_id(PADAPTER padapter, const struct sdio_device_id  *pdid)
{
	padapter->chip_type = pdid->driver_data;

#if defined(CONFIG_RTL8723A)
	if( padapter->chip_type == RTL8723A){
		padapter->HardwareType = HARDWARE_TYPE_RTL8723AS;
		DBG_871X("CHIP TYPE: RTL8723A\n");
	}
#endif

#if defined(CONFIG_RTL8188E)
	if(padapter->chip_type == RTL8188E){
		padapter->HardwareType = HARDWARE_TYPE_RTL8188ES;
		DBG_871X("CHIP TYPE: RTL8188E\n");
	}
#endif

#if defined(CONFIG_RTL8723B)
	padapter->chip_type = RTL8723B;
	padapter->HardwareType = HARDWARE_TYPE_RTL8723BS;
#endif

#if defined(CONFIG_RTL8821A)
	if (padapter->chip_type == RTL8821) {
		padapter->HardwareType = HARDWARE_TYPE_RTL8821S;
		DBG_871X("CHIP TYPE: RTL8821A\n");
	}
#endif
}

void rtw_set_hal_ops(PADAPTER padapter)
{
#if defined(CONFIG_RTL8723A)
	if( padapter->chip_type == RTL8723A){
		rtl8723as_set_hal_ops(padapter);
	}
#endif
#if defined(CONFIG_RTL8188E)
	if(padapter->chip_type == RTL8188E){
		rtl8188es_set_hal_ops(padapter);
	}
#endif
#if defined(CONFIG_RTL8723B)
	if(padapter->chip_type == RTL8723B){
		rtl8723bs_set_hal_ops(padapter);
	}
#endif
#if defined(CONFIG_RTL8821A)
	if(padapter->chip_type == RTL8821){
		rtl8821as_set_hal_ops(padapter);
	}
#endif
}

static void sd_intf_start(PADAPTER padapter)
{
	if (padapter == NULL) {
		DBG_8192C(KERN_ERR "%s: padapter is NULL!\n", __func__);
		return;
	}

	// hal dep
	rtw_hal_enable_interrupt(padapter);
}

static void sd_intf_stop(PADAPTER padapter)
{
	if (padapter == NULL) {
		DBG_8192C(KERN_ERR "%s: padapter is NULL!\n", __func__);
		return;
	}

	// hal dep
	rtw_hal_disable_interrupt(padapter);
}

/*
 * Do deinit job corresponding to netdev_open()
 */
static void rtw_dev_unload(PADAPTER padapter)
{
	struct net_device *pnetdev = (struct net_device*)padapter->pnetdev;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct pwrctrl_priv *pwrctl = adapter_to_pwrctl(padapter);
	struct dvobj_priv *psdpriv = padapter->dvobj;

	RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("+rtw_dev_unload\n"));

	padapter->bDriverStopped = _TRUE;
	#ifdef CONFIG_XMIT_ACK
	if (padapter->xmitpriv.ack_tx)
		rtw_ack_tx_done(&padapter->xmitpriv, RTW_SCTX_DONE_DRV_STOP);
	#endif

	if (padapter->bup == _TRUE)
	{
		// stop TX
//		val8 = 0xFF;
//		rtw_hal_set_hwreg(padapter, HW_VAR_TXPAUSE,&val8);

#if 0
		if (padapter->intf_stop)
			padapter->intf_stop(padapter);
#else
		sd_intf_stop(padapter);
#endif
		RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("@ rtw_dev_unload: stop intf complete!\n"));

		if (!pwrctl->bInternalAutoSuspend)
			rtw_stop_drv_threads(padapter);
		
		RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("@ rtw_dev_unload: stop thread complete!\n"));

		//check the status of IPS
		if(rtw_hal_check_ips_status(padapter) == _TRUE || pwrctl->rf_pwrstate == rf_off) { //check HW status and SW state
			DBG_871X_LEVEL(_drv_always_, "%s: driver in IPS-FWLPS\n", __func__);
			LeaveAllPowerSaveMode(padapter);
		} else {
			DBG_871X_LEVEL(_drv_always_, "%s: driver not in IPS\n", __func__);
		}

		if (padapter->bSurpriseRemoved == _FALSE)
		{
#ifdef CONFIG_WOWLAN
			if (pwrctl->bSupportRemoteWakeup == _TRUE && 
				pwrctl->wowlan_mode ==_TRUE) {
				DBG_871X_LEVEL(_drv_always_, "%s bSupportRemoteWakeup==_TRUE  do not run rtw_hal_deinit()\n",__FUNCTION__);
			}
			else
#endif
			{
				//amy modify 20120221 for power seq is different between driver open and ips
				rtw_hal_deinit(padapter);
			}
			padapter->bSurpriseRemoved = _TRUE;
		}
		RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("@ rtw_dev_unload: deinit hal complelt!\n"));

		padapter->bup = _FALSE;
	}
	else {
		RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("rtw_dev_unload: bup==_FALSE\n"));
	}

	RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("-rtw_dev_unload\n"));
}

#ifdef RTW_SUPPORT_PLATFORM_SHUTDOWN
PADAPTER g_test_adapter = NULL;
#endif // RTW_SUPPORT_PLATFORM_SHUTDOWN

_adapter *rtw_sdio_if1_init(struct dvobj_priv *dvobj, const struct sdio_device_id  *pdid)
{
	int status = _FAIL;
	struct net_device *pnetdev;
	PADAPTER padapter = NULL;
	
	if ((padapter = (_adapter *)rtw_zvmalloc(sizeof(*padapter))) == NULL) {
		goto exit;
	}

#ifdef RTW_SUPPORT_PLATFORM_SHUTDOWN
	g_test_adapter = padapter;
#endif // RTW_SUPPORT_PLATFORM_SHUTDOWN
	padapter->dvobj = dvobj;
	dvobj->if1 = padapter;

	padapter->bDriverStopped=_TRUE;

	dvobj->padapters[dvobj->iface_nums++] = padapter;
	padapter->iface_id = IFACE_ID0;

#if defined(CONFIG_CONCURRENT_MODE) || defined(CONFIG_DUALMAC_CONCURRENT)
	//set adapter_type/iface type for primary padapter
	padapter->isprimary = _TRUE;
	padapter->adapter_type = PRIMARY_ADAPTER;	
	#ifndef CONFIG_HWPORT_SWAP
	padapter->iface_type = IFACE_PORT0;
	#else
	padapter->iface_type = IFACE_PORT1;
	#endif
#endif

	padapter->interface_type = RTW_SDIO;
	rtw_decide_chip_type_by_device_id(padapter, pdid);
	
	//3 1. init network device data
	pnetdev = rtw_init_netdev(padapter);
	if (!pnetdev)
		goto free_adapter;
	
	SET_NETDEV_DEV(pnetdev, dvobj_to_dev(dvobj));

	padapter = rtw_netdev_priv(pnetdev);

#ifdef CONFIG_IOCTL_CFG80211
	rtw_wdev_alloc(padapter, dvobj_to_dev(dvobj));
#endif

	//3 3. init driver special setting, interface, OS and hardware relative

	//4 3.1 set hardware operation functions
	rtw_set_hal_ops(padapter);


	//3 5. initialize Chip version
	padapter->intf_start = &sd_intf_start;
	padapter->intf_stop = &sd_intf_stop;

	if (rtw_init_io_priv(padapter, sdio_set_intf_ops) == _FAIL)
	{
		RT_TRACE(_module_hci_intfs_c_, _drv_err_,
			("rtw_drv_init: Can't init io_priv\n"));
		goto free_hal_data;
	}

	rtw_hal_read_chip_version(padapter);

	rtw_hal_chip_configure(padapter);

#ifdef CONFIG_BT_COEXIST
	rtw_btcoex_Initialize(padapter);
#endif // CONFIG_BT_COEXIST

	//3 6. read efuse/eeprom data
	rtw_hal_read_chip_info(padapter);

	//3 7. init driver common data
	if (rtw_init_drv_sw(padapter) == _FAIL) {
		RT_TRACE(_module_hci_intfs_c_, _drv_err_,
			 ("rtw_drv_init: Initialize driver software resource Failed!\n"));
		goto free_hal_data;
	}

	//3 8. get WLan MAC address
	// set mac addr
	rtw_macaddr_cfg(padapter->eeprompriv.mac_addr);
	rtw_init_wifidirect_addrs(padapter, padapter->eeprompriv.mac_addr, padapter->eeprompriv.mac_addr);

	rtw_hal_disable_interrupt(padapter);

	DBG_871X("bDriverStopped:%d, bSurpriseRemoved:%d, bup:%d, hw_init_completed:%d\n"
		,padapter->bDriverStopped
		,padapter->bSurpriseRemoved
		,padapter->bup
		,padapter->hw_init_completed
	);
	
	status = _SUCCESS;
	
free_hal_data:
	if(status != _SUCCESS && padapter->HalData)
		rtw_mfree(padapter->HalData, sizeof(*(padapter->HalData)));

free_wdev:
	if(status != _SUCCESS) {
		#ifdef CONFIG_IOCTL_CFG80211
		rtw_wdev_unregister(padapter->rtw_wdev);
		rtw_wdev_free(padapter->rtw_wdev);
		#endif
	}

free_adapter:
	if (status != _SUCCESS) {
		if (pnetdev)
			rtw_free_netdev(pnetdev);
		else if (padapter)
			rtw_vmfree((u8*)padapter, sizeof(*padapter));
		padapter = NULL;
	}
exit:
	return padapter;
}

static void rtw_sdio_if1_deinit(_adapter *if1)
{
	struct net_device *pnetdev = if1->pnetdev;
	struct mlme_priv *pmlmepriv= &if1->mlmepriv;

	if(check_fwstate(pmlmepriv, _FW_LINKED))
		rtw_disassoc_cmd(if1, 0, _FALSE);

#ifdef CONFIG_AP_MODE
	free_mlme_ap_info(if1);
	#ifdef CONFIG_HOSTAPD_MLME
	hostapd_mode_unload(if1);
	#endif
#endif

#ifdef CONFIG_GPIO_WAKEUP
#ifdef CONFIG_PLATFORM_ARM_SUN6I 
  sw_gpio_eint_set_enable(gpio_eint_wlan, 0);
  sw_gpio_irq_free(eint_wlan_handle);
#else  
	gpio_hostwakeup_free_irq(if1);
#endif
#endif

/*
	if(if1->DriverState != DRIVER_DISAPPEAR) {
		if(pnetdev) {
			unregister_netdev(pnetdev); //will call netdev_close()
			rtw_proc_remove_one(pnetdev);
		}
	}
*/

	rtw_cancel_all_timer(if1);

#ifdef CONFIG_WOWLAN
	adapter_to_pwrctl(if1)->wowlan_mode=_FALSE;
	DBG_871X_LEVEL(_drv_always_, "%s wowlan_mode:%d\n", __func__, adapter_to_pwrctl(if1)->wowlan_mode);
#endif //CONFIG_WOWLAN

	rtw_dev_unload(if1);
	DBG_871X("+r871xu_dev_remove, hw_init_completed=%d\n", if1->hw_init_completed);
	
	rtw_handle_dualmac(if1, 0);

#ifdef CONFIG_IOCTL_CFG80211
	if (if1->rtw_wdev)
	{
		//rtw_wdev_unregister(if1->rtw_wdev);
		rtw_wdev_free(if1->rtw_wdev);
	}
#endif

	rtw_free_drv_sw(if1);

	if(pnetdev)
		rtw_free_netdev(pnetdev);

#ifdef CONFIG_PLATFORM_RTD2880B
	DBG_871X("wlan link down\n");
	rtd2885_wlan_netlink_sendMsg("linkdown", "8712");
#endif

#ifdef RTW_SUPPORT_PLATFORM_SHUTDOWN
	g_test_adapter = NULL;
#endif // RTW_SUPPORT_PLATFORM_SHUTDOWN
}

/*
 * drv_init() - a device potentially for us
 *
 * notes: drv_init() is called when the bus driver has located a card for us to support.
 *        We accept the new device by returning 0.
 */
static int rtw_drv_init(
	struct sdio_func *func,
	const struct sdio_device_id *id)
{
	int status = _FAIL;
	struct net_device *pnetdev;
	PADAPTER if1 = NULL, if2 = NULL;
	struct dvobj_priv *dvobj;

	RT_TRACE(_module_hci_intfs_c_, _drv_info_,
		("+rtw_drv_init: vendor=0x%04x device=0x%04x class=0x%02x\n",
		func->vendor, func->device, func->class));

	if ((dvobj = sdio_dvobj_init(func)) == NULL) {
		RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("initialize device object priv Failed!\n"));
		goto exit;
	}

	if ((if1 = rtw_sdio_if1_init(dvobj, id)) == NULL) {
		DBG_871X("rtw_init_primary_adapter Failed!\n");
		goto free_dvobj;
	}

#ifdef CONFIG_CONCURRENT_MODE
	if ((if2 = rtw_drv_if2_init(if1, sdio_set_intf_ops)) == NULL) {
		goto free_if1;
	}
#endif

	//dev_alloc_name && register_netdev
	if((status = rtw_drv_register_netdev(if1)) != _SUCCESS) {
		goto free_if2;
	}

#ifdef CONFIG_HOSTAPD_MLME
	hostapd_mode_init(if1);
#endif

#ifdef CONFIG_PLATFORM_RTD2880B
	DBG_871X("wlan link up\n");
	rtd2885_wlan_netlink_sendMsg("linkup", "8712");
#endif

#ifdef RTK_DMP_PLATFORM
	rtw_proc_init_one(if1->pnetdev);
#endif

	if (sdio_alloc_irq(dvobj) != _SUCCESS)
		goto free_if2;

#ifdef	CONFIG_GPIO_WAKEUP
#ifdef CONFIG_PLATFORM_ARM_SUN6I
        eint_wlan_handle = sw_gpio_irq_request(gpio_eint_wlan, TRIG_EDGE_NEGATIVE,(peint_handle)gpio_hostwakeup_irq_thread, NULL);
        if (!eint_wlan_handle) {
               DBG_871X( "%s: request irq failed\n",__func__);
               return -1;
  }
#else
	gpio_hostwakeup_alloc_irq(if1);
#endif
#endif

#ifdef CONFIG_GLOBAL_UI_PID
	if(ui_pid[1]!=0) {
		DBG_871X("ui_pid[1]:%d\n",ui_pid[1]);
		rtw_signal_process(ui_pid[1], SIGUSR2);
	}
#endif

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-871x_drv - drv_init, success!\n"));

	status = _SUCCESS;

free_if2:
	if(status != _SUCCESS && if2) {
		#ifdef CONFIG_CONCURRENT_MODE
		rtw_drv_if2_stop(if2);
		rtw_drv_if2_free(if2);
		#endif
	}
free_if1:
	if (status != _SUCCESS && if1) {
		rtw_sdio_if1_deinit(if1);
	}
free_dvobj:
	if (status != _SUCCESS)
		sdio_dvobj_deinit(func);
exit:
	return status == _SUCCESS?0:-ENODEV;
}

static void rtw_dev_remove(struct sdio_func *func)
{
	struct dvobj_priv *dvobj = sdio_get_drvdata(func);
	struct pwrctrl_priv *pwrctl = dvobj_to_pwrctl(dvobj);
	PADAPTER padapter = dvobj->if1;

_func_enter_;

	RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("+rtw_dev_remove\n"));

	dvobj->processing_dev_remove = _TRUE;

	rtw_unregister_netdevs(dvobj);

	if (padapter->bSurpriseRemoved == _FALSE) {
		int err;

		/* test surprise remove */
		sdio_claim_host(func);
		sdio_readb(func, 0, &err);
		sdio_release_host(func);
		if (err == -ENOMEDIUM) {
			padapter->bSurpriseRemoved = _TRUE;
			DBG_871X(KERN_NOTICE "%s: device had been removed!\n", __func__);
		}
	}

#if defined(CONFIG_HAS_EARLYSUSPEND) || defined(CONFIG_ANDROID_POWER)
	rtw_unregister_early_suspend(pwrctl);
#endif

	rtw_pm_set_ips(padapter, IPS_NONE);
	rtw_pm_set_lps(padapter, PS_MODE_ACTIVE);

	LeaveAllPowerSaveMode(padapter);

#ifdef CONFIG_CONCURRENT_MODE
	rtw_drv_if2_stop(dvobj->if2);
#endif

#ifdef CONFIG_BT_COEXIST
	rtw_btcoex_HaltNotify(padapter);
#endif

	rtw_sdio_if1_deinit(padapter);

#ifdef CONFIG_CONCURRENT_MODE
	rtw_drv_if2_free(dvobj->if2);
#endif

	sdio_dvobj_deinit(func);

	RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("-rtw_dev_remove\n"));

_func_exit_;
}

static int rtw_sdio_suspend(struct device *dev)
{
	struct sdio_func *func =dev_to_sdio_func(dev);
	struct dvobj_priv *psdpriv = sdio_get_drvdata(func);
	struct pwrctrl_priv *pwrpriv = dvobj_to_pwrctl(psdpriv);
	_adapter *padapter = psdpriv->if1;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct net_device *pnetdev = padapter->pnetdev;
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;
		
	int ret = 0;
#ifdef CONFIG_PLATFORM_SPRD
	u32 value;
#endif // CONFIG_PLATFORM_SPRD

#ifdef CONFIG_WOWLAN
	struct wowlan_ioctl_param poidparam;
	u8 ch, bw, offset;
	u8 ps_mode;
#endif //CONFIG_WOWLAN

	u32 start_time = rtw_get_current_time();

	_func_enter_;

	if (pwrpriv->bInSuspend == _TRUE)
        {
                DBG_871X("[WIFIDBG] bInSuspend = _TRUE in %s!!!!!!!!!!\n", __FUNCTION__);
                goto exit;
        }

	DBG_871X_LEVEL(_drv_always_, "sdio suspend start\n");
	DBG_871X("==> %s (%s:%d)\n",__FUNCTION__, current->comm, current->pid);
	pdbgpriv->dbg_suspend_cnt++;
	pwrpriv->bInSuspend = _TRUE;

#ifdef CONFIG_WOWLAN
	if (check_fwstate(pmlmepriv, _FW_LINKED))
		pwrpriv->wowlan_mode = _TRUE;
	else
		pwrpriv->wowlan_mode = _FALSE;
#endif

	while (pwrpriv->bips_processing == _TRUE)
		rtw_msleep_os(1);

	if((!padapter->bup) || (padapter->bDriverStopped)||(padapter->bSurpriseRemoved))
	{
		DBG_871X("%s bup=%d bDriverStopped=%d bSurpriseRemoved = %d\n", __FUNCTION__
			,padapter->bup, padapter->bDriverStopped,padapter->bSurpriseRemoved);
		pdbgpriv->dbg_suspend_error_cnt++;
		goto exit;
	}

	rtw_cancel_all_timer(padapter);

#ifdef CONFIG_WOWLAN
	if (pwrpriv->wowlan_mode == _TRUE)
	{
		// 1. stop thread
		padapter->bDriverStopped = _TRUE;	//for stop thread
		rtw_stop_drv_threads(padapter);
		padapter->bDriverStopped = _FALSE;	//for 32k command

#ifdef CONFIG_CONCURRENT_MODE
		if (padapter->pbuddy_adapter)
		{
			padapter->pbuddy_adapter->bDriverStopped = _TRUE;	//for stop thread
			rtw_stop_drv_threads(padapter->pbuddy_adapter);
			padapter->pbuddy_adapter->bDriverStopped = _FALSE;	//for 32k command
		}
#endif // CONFIG_CONCURRENT_MODE

#ifdef CONFIG_POWER_SAVING
		rtw_set_ps_mode(padapter, PS_MODE_ACTIVE, 0, 0, "WOWLAN");
#endif
		// 2. disable interrupt
		rtw_hal_disable_interrupt(padapter); // It need wait for leaving 32K.

		// 2.1 clean interupt
		if (padapter->HalFunc.clear_interrupt)
			padapter->HalFunc.clear_interrupt(padapter);

		// 2.2 free irq
		sdio_free_irq(adapter_to_dvobj(padapter));
	} else {
		LeaveAllPowerSaveMode(padapter);
	}

#ifdef CONFIG_BT_COEXIST
	rtw_btcoex_SuspendNotify(padapter, 1);
#endif // CONFIG_BT_COEXIST

	if(pnetdev){
		if(pwrpriv->wowlan_mode == _TRUE) {
			rtw_netif_stop_queue(pnetdev);
		} else {
			netif_carrier_off(pnetdev);
			rtw_netif_stop_queue(pnetdev);
		}
	}

	DBG_871X("wowlan_mode: %d\n", pwrpriv->wowlan_mode);

 	if ((pwrpriv->bSupportRemoteWakeup == _TRUE) &&
		(pwrpriv->wowlan_mode == _TRUE)) {
		poidparam.subcode = WOWLAN_ENABLE;
		padapter->HalFunc.SetHwRegHandler(padapter,HW_VAR_WOWLAN,(u8 *)&poidparam);
	} else {
	//s2-1.  issue rtw_disassoc_cmd to fw
		rtw_disassoc_cmd(padapter, 0, _FALSE);
	}
#else // !CONFIG_WOWLAN
	LeaveAllPowerSaveMode(padapter);

#ifdef CONFIG_BT_COEXIST
	rtw_btcoex_SuspendNotify(padapter, 1);
#endif // CONFIG_BT_COEXIST

	if(pnetdev)
	{
		netif_carrier_off(pnetdev);
		rtw_netif_stop_queue(pnetdev);
	}

	rtw_disassoc_cmd(padapter, 0, _FALSE);
#endif // !CONFIG_WOWLAN

	//for power down during suspend, need leave ips mode before entering power down.
	//pwrpriv->bInSuspend = _TRUE;

	//padapter->net_closed = _TRUE;
	//s1.

#ifdef CONFIG_LAYER2_ROAMING_RESUME
	if(check_fwstate(pmlmepriv, WIFI_STATION_STATE) && check_fwstate(pmlmepriv, _FW_LINKED) )
	{
		DBG_871X("%s %s(" MAC_FMT "), length:%d assoc_ssid.length:%d\n",__FUNCTION__,
				pmlmepriv->cur_network.network.Ssid.Ssid,
				MAC_ARG(pmlmepriv->cur_network.network.MacAddress),
				pmlmepriv->cur_network.network.Ssid.SsidLength,
				pmlmepriv->assoc_ssid.SsidLength);
#ifdef CONFIG_WOWLAN
		if (pwrpriv->wowlan_mode != _TRUE)
			rtw_set_roaming(padapter, 1);
		else
			rtw_set_roaming(padapter, 0);
#else // !CONFIG_WOWLAN
		rtw_set_roaming(padapter, 1);
#endif // !CONFIG_WOWLAN
	}
#endif //CONFIG_LAYER2_ROAMING_RESUME

#ifdef CONFIG_WOWLAN
	if (pwrpriv->wowlan_mode == _FALSE)
	{
		//s2-2.  indicate disconnect to os
		rtw_indicate_disconnect(padapter);
		//s2-3.
		rtw_free_assoc_resources(padapter, 1);

		//s2-4.
		rtw_free_network_queue(padapter, _TRUE);
		//s2-5 dev unload and stop thread
		rtw_dev_unload(padapter);

		if(check_fwstate(pmlmepriv, _FW_UNDER_SURVEY)){
			DBG_871X_LEVEL(_drv_always_, "%s: fw_under_survey\n", __func__);
			rtw_indicate_scan_done(padapter, 1);
		}

		if(check_fwstate(pmlmepriv, _FW_UNDER_LINKING)){
			DBG_871X_LEVEL(_drv_always_, "%s: fw_under_linking\n", __func__);
			rtw_indicate_disconnect(padapter);
		}
		
		sdio_deinit(adapter_to_dvobj(padapter));
	}
	else
	{
		DBG_871X_LEVEL(_drv_always_, "%s: wowmode suspending\n", __func__);

		if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY) == _TRUE)
		{
			DBG_871X_LEVEL(_drv_always_, "%s: fw_under_survey\n", __func__);
			rtw_indicate_scan_done(padapter, 1);
			clr_fwstate(pmlmepriv, _FW_UNDER_SURVEY);
		}
		
		if (rtw_get_ch_setting_union(padapter, &ch, &bw, &offset) != 0) {
			DBG_871X(FUNC_ADPT_FMT" back to linked union - ch:%u, bw:%u, offset:%u\n",
				FUNC_ADPT_ARG(padapter), ch, bw, offset);
			set_channel_bwmode(padapter, ch, offset, bw);
		}

#ifdef CONFIG_POWER_SAVING
		rtw_set_ps_mode(padapter, PS_MODE_MIN, 0, 0, "WOWLAN");
#endif
	}
#else // !CONFIG_WOWLAN
	//s2-2.  indicate disconnect to os
	rtw_indicate_disconnect(padapter);
	//s2-3.
	rtw_free_assoc_resources(padapter, 1);

	//s2-4.
	rtw_free_network_queue(padapter, _TRUE);

	rtw_dev_unload(padapter);

	if(rtw_hal_check_ips_status(padapter) == _TRUE || pwrpriv->rf_pwrstate == rf_off) {
		DBG_871X_LEVEL(_drv_always_, "%s: driver in IPS\n", __func__);
		LeaveAllPowerSaveMode(padapter);
		DBG_871X_LEVEL(_drv_always_, "%s: driver not in IPS\n", __func__);
	}

	if(check_fwstate(pmlmepriv, _FW_UNDER_SURVEY))
		rtw_indicate_scan_done(padapter, 1);

	if(check_fwstate(pmlmepriv, _FW_UNDER_LINKING))
		rtw_indicate_disconnect(padapter);

	// interface deinit
	sdio_deinit(adapter_to_dvobj(padapter));
#endif // !CONFIG_WOWLAN

	DBG_871X_LEVEL(_drv_always_, "sdio suspend success in %d ms\n",
			rtw_get_passing_time_ms(start_time));

exit:

//#if (defined CONFIG_WOWLAN) || (!(defined ANDROID_2X) && (defined CONFIG_PLATFORM_SPRD))
#if (defined CONFIG_WOWLAN) || (!(defined ANDROID_2X))
	//Android 4.0 don't support WIFI close power
	//or power down or clock will close after wifi resume,
	//this is sprd's bug in Android 4.0, but sprd don't
	//want to fix it.
	//we have test power under 8723as, power consumption is ok
	if (func) {
		mmc_pm_flag_t pm_flag = 0;
		pm_flag = sdio_get_host_pm_caps(func);
		DBG_871X("cmd: %s: suspend: PM flag = 0x%x\n", sdio_func_id(func), pm_flag);
		if (!(pm_flag & MMC_PM_KEEP_POWER)) {
			DBG_871X("%s: cannot remain alive while host is suspended\n", sdio_func_id(func));
			pdbgpriv->dbg_suspend_error_cnt++;
			return -ENOSYS;
		} else {
			DBG_871X("cmd: suspend with MMC_PM_KEEP_POWER\n");
			sdio_set_host_pm_flags(func, MMC_PM_KEEP_POWER);
		}
	}
#endif

#ifdef CONFIG_PLATFORM_SPRD
#ifndef CONFIG_WOWLAN
#ifdef CONFIG_RTL8188E
	/*
	 * Pull down wifi power pin here
	 * Pull up wifi power pin before sdio resume.
	 */
	rtw_wifi_gpio_wlan_ctrl(WLAN_POWER_OFF);
#endif // CONFIG_RTL8188E
#endif // CONFIG_WOWLAN
#endif // CONFIG_PLATFORM_SPRD

	DBG_871X("<===  %s return %d.............. in %dms\n", __FUNCTION__
		, ret, rtw_get_passing_time_ms(start_time));

	_func_exit_;
	return ret;
}

extern int pm_netdev_open(struct net_device *pnetdev,u8 bnormal);
extern int pm_netdev_close(struct net_device *pnetdev,u8 bnormal);

int rtw_resume_process(_adapter *padapter)
{
	struct net_device *pnetdev;
	struct pwrctrl_priv *pwrpriv = NULL;
	u8 is_pwrlock_hold_by_caller;
	u8 is_directly_called_by_auto_resume;
	int ret = 0;
	u32 start_time = rtw_get_current_time();
	struct dvobj_priv *psdpriv = padapter->dvobj;
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;
#ifdef CONFIG_WOWLAN
	u32 value = 0;
	struct wowlan_ioctl_param poidparam;
	struct sta_info	*psta = NULL;
#endif // CONFIG_WOWLAN

	_func_enter_;

	DBG_871X_LEVEL(_drv_always_, "sdio resume start\n");
	DBG_871X("==> %s (%s:%d)\n",__FUNCTION__, current->comm, current->pid);

	if (padapter) {
		pnetdev = padapter->pnetdev;
		pwrpriv = adapter_to_pwrctl(padapter);
	} else {
		pdbgpriv->dbg_resume_error_cnt++;
		ret = -1;
		goto exit;
	}

	if (pwrpriv->bInSuspend == _FALSE)
        {
                DBG_871X("[WIFIDBG] bInSuspend = _FALSE in %s!!!!!!!!!!\n", __FUNCTION__);
                goto exit;
        }

#ifdef CONFIG_WOWLAN
	if (pwrpriv->wowlan_mode == _FALSE){

	// interface init
	if (sdio_init(adapter_to_dvobj(padapter)) != _SUCCESS)
	{
		ret = -1;
		RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("%s: initialize SDIO Failed!!\n", __FUNCTION__));
		goto exit;
	}

	rtw_hal_disable_interrupt(padapter);

		if (padapter->HalFunc.clear_interrupt)
			padapter->HalFunc.clear_interrupt(padapter);

	if (sdio_alloc_irq(adapter_to_dvobj(padapter)) != _SUCCESS)
	{
		ret = -1;
		RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("%s: sdio_alloc_irq Failed!!\n", __FUNCTION__));
		goto exit;
	}

	rtw_reset_drv_sw(padapter);
	pwrpriv->bkeepfwalive = _FALSE;

	DBG_871X("bkeepfwalive(%x)\n",pwrpriv->bkeepfwalive);

	if(pm_netdev_close(pnetdev, _TRUE) == 0) {
		DBG_871X("netdev_close success\n");
	}

	if(pm_netdev_open(pnetdev,_TRUE) != 0) {
		ret = -1;
		pdbgpriv->dbg_resume_error_cnt++;
		goto exit;
	}

	netif_device_attach(pnetdev);	
	netif_carrier_on(pnetdev);
	} else {

#ifdef CONFIG_POWER_SAVING
#ifdef CONFIG_LPS
		rtw_set_ps_mode(padapter, PS_MODE_ACTIVE, 0, 0, "WOWLAN");
#endif //CONFIG_LPS
#endif

		pwrpriv->bFwCurrentInPSMode = _FALSE;

		rtw_hal_disable_interrupt(padapter);

		if (padapter->HalFunc.clear_interrupt)
			padapter->HalFunc.clear_interrupt(padapter);

		if (sdio_alloc_irq(adapter_to_dvobj(padapter)) != _SUCCESS) {
			ret = -1;
			RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("%s: sdio_alloc_irq Failed!!\n", __FUNCTION__));
			goto exit;
		}

		//Disable WOW, set H2C command
		poidparam.subcode=WOWLAN_DISABLE;
		padapter->HalFunc.SetHwRegHandler(padapter,HW_VAR_WOWLAN,(u8 *)&poidparam);

		psta = rtw_get_stainfo(&padapter->stapriv, get_bssid(&padapter->mlmepriv));
		if (psta) {
			set_sta_rate(padapter, psta);
		}

		padapter->bDriverStopped = _FALSE;
		DBG_871X("%s: wowmode resuming, DriverStopped:%d\n", __func__, padapter->bDriverStopped);
		rtw_start_drv_threads(padapter);

#ifdef CONFIG_CONCURRENT_MODE
		if (padapter->pbuddy_adapter)
		{
			padapter->pbuddy_adapter->bDriverStopped = _FALSE;
			DBG_871X("%s: wowmode resuming, pbuddy_adapter->DriverStopped:%d\n",
				__FUNCTION__, padapter->pbuddy_adapter->bDriverStopped);
			rtw_start_drv_threads(padapter->pbuddy_adapter);
		}
#endif // CONFIG_CONCURRENT_MODE

		rtw_hal_enable_interrupt(padapter);

		// start netif queue
		if(pnetdev) {
			if(!rtw_netif_queue_stopped(pnetdev))
				rtw_netif_start_queue(pnetdev);
			else 
				rtw_netif_wake_queue(pnetdev);
		}
	}
#else //!CONFIG_WOWLAN

		// interface init
		if (sdio_init(adapter_to_dvobj(padapter)) != _SUCCESS)
		{
			ret = -1;
			RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("%s: initialize SDIO Failed!!\n", __FUNCTION__));
			goto exit;
		}
		rtw_hal_disable_interrupt(padapter);
		if (sdio_alloc_irq(adapter_to_dvobj(padapter)) != _SUCCESS)
		{
			ret = -1;
			RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("%s: sdio_alloc_irq Failed!!\n", __FUNCTION__));
			goto exit;
		}

		rtw_reset_drv_sw(padapter);
		pwrpriv->bkeepfwalive = _FALSE;

		DBG_871X("bkeepfwalive(%x)\n",pwrpriv->bkeepfwalive);
		if(pm_netdev_open(pnetdev,_TRUE) != 0) {
			ret = -1;
			pdbgpriv->dbg_resume_error_cnt++;
			goto exit;
		}

		netif_device_attach(pnetdev);
		netif_carrier_on(pnetdev);
#endif
	if( padapter->pid[1]!=0) {
		DBG_871X("pid[1]:%d\n",padapter->pid[1]);
		rtw_signal_process(padapter->pid[1], SIGUSR2);
	}	

#ifdef CONFIG_LAYER2_ROAMING_RESUME
#ifdef CONFIG_WOWLAN
	if (pwrpriv->wowlan_wake_reason == FWDecisionDisconnect ||
		pwrpriv->wowlan_wake_reason == Rx_DisAssoc ||
		pwrpriv->wowlan_wake_reason == Rx_DeAuth) {

		DBG_871X("%s: disconnect reason: %02x\n", __func__,
						pwrpriv->wowlan_wake_reason);
		rtw_indicate_disconnect(padapter);
		rtw_sta_media_status_rpt(padapter, rtw_get_stainfo(&padapter->stapriv, get_bssid(&padapter->mlmepriv)), 0);
		rtw_free_assoc_resources(padapter, 1);
	} else {
		DBG_871X("%s: do roaming\n", __func__);
		rtw_roaming(padapter, NULL);
	}
#else
	rtw_roaming(padapter, NULL);
#endif //CONFOG_WOWLAN
#endif //CONFIG_LAYER2_ROAMING_RESUME

	#ifdef CONFIG_RESUME_IN_WORKQUEUE
	rtw_unlock_suspend();
	#endif //CONFIG_RESUME_IN_WORKQUEUE

#ifdef CONFIG_WOWLAN
	if (pwrpriv->wowlan_wake_reason == Rx_GTK ||
		pwrpriv->wowlan_wake_reason == Rx_DisAssoc ||
		pwrpriv->wowlan_wake_reason == Rx_DeAuth) {
		DBG_871X("%s: set ext wake lock\n", __func__);
		rtw_lock_ext_suspend_timeout(1500);
	}

	if (pwrpriv->wowlan_mode == _TRUE) {
		pwrpriv->bips_processing = _FALSE;
		_set_timer(&padapter->mlmepriv.dynamic_chk_timer, 2000);
#ifndef CONFIG_IPS_CHECK_IN_WD
		rtw_set_pwr_state_check_timer(pwrpriv);
#endif
		rtw_unlock_suspend();
	} else {
		DBG_871X_LEVEL(_drv_always_, "do not reset timer\n");
	}

	pwrpriv->wowlan_mode =_FALSE;

	//clean driver side wake up reason.
	pwrpriv->wowlan_wake_reason = 0;
#endif //CONFIG_WOWLAN

#ifdef CONFIG_BT_COEXIST
	rtw_btcoex_SuspendNotify(padapter, 0);
#endif // CONFIG_BT_COEXIST

exit:
	if (pwrpriv)
		pwrpriv->bInSuspend = _FALSE;
	DBG_871X_LEVEL(_drv_always_, "sdio resume ret:%d in %d ms\n", ret,
		rtw_get_passing_time_ms(start_time));

	_func_exit_;
	
	return ret;
}

static int rtw_sdio_resume(struct device *dev)
{
	struct sdio_func *func =dev_to_sdio_func(dev);
	struct dvobj_priv *psdpriv = sdio_get_drvdata(func);
	struct pwrctrl_priv *pwrpriv = dvobj_to_pwrctl(psdpriv);
	_adapter *padapter = psdpriv->if1;
	 int ret = 0;
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;

	DBG_871X("==> %s (%s:%d)\n",__FUNCTION__, current->comm, current->pid);
	pdbgpriv->dbg_resume_cnt++;

	if(pwrpriv->bInternalAutoSuspend ){
 		ret = rtw_resume_process(padapter);
	} else {
#ifdef CONFIG_RESUME_IN_WORKQUEUE
		rtw_resume_in_workqueue(pwrpriv);
#else
		if (rtw_is_earlysuspend_registered(pwrpriv)
			#ifdef CONFIG_WOWLAN
			&& !pwrpriv->wowlan_mode
			#endif /* CONFIG_WOWLAN */
		) {
			/* jeff: bypass resume here, do in late_resume */
			rtw_set_do_late_resume(pwrpriv, _TRUE);
		} else {
			#ifdef CONFIG_WOWLAN
			//rtw_lock_suspend_timeout(4000);
			rtw_lock_suspend();
			#endif
			ret = rtw_resume_process(padapter);
		}
#endif /* CONFIG_RESUME_IN_WORKQUEUE */
	}
	
	DBG_871X("<========  %s return %d\n", __FUNCTION__, ret);
	return ret;

}




#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)) 
extern int console_suspend_enabled;
#endif


#ifdef CONFIG_PLATFORM_SPRD
extern void sdhci_bus_scan(void);
#ifndef ANDROID_2X
extern int sdhci_device_attached(void);
#endif
#endif // CONFIG_PLATFORM_SPRD

static int __init rtw_drv_entry(void)
{
	int ret = 0;
  script_item_u val ;
  script_item_value_type_e type;

#ifdef CONFIG_PLATFORM_ARM_SUNxI
/*depends on sunxi power control */
#if defined CONFIG_MMC_SUNXI_POWER_CONTROL
	unsigned int mod_sel = mmc_pm_get_mod_type();
#endif
#endif

#if defined(CONFIG_PLATFORM_ARM_SUN6I) || defined(CONFIG_PLATFORM_ARM_SUN8I)
#ifdef CONFIG_MMC
	unsigned int mod_sel = wifi_pm_get_mod_type();
#endif //CONFIG_MMC
#endif //CONFIG_PLATFORM_ARM_SUN6I

	DBG_871X_LEVEL(_drv_always_, "module init start version:"DRIVERVERSION"\n");

//	DBG_871X(KERN_INFO "+%s", __func__);
	RT_TRACE(_module_hci_intfs_c_, _drv_notice_, ("+rtw_drv_entry\n"));
	DBG_871X(DRV_NAME " driver version= %s\n", DRIVERVERSION);
#ifdef BTCOEXVERSION
	DBG_871X(DRV_NAME " BT-Coex version= %s\n", BTCOEXVERSION);
#endif // BTCOEXVERSION
	DBG_871X("build time: %s %s\n", __DATE__, __TIME__);

#ifdef CONFIG_PLATFORM_ARM_SUNxI
/*depends on sunxi power control */
#if defined CONFIG_MMC_SUNXI_POWER_CONTROL

	if(mod_sel == SUNXI_SDIO_WIFI_NUM_RTL8189ES)
	{
		rtl8189es_sdio_powerup();
		sunximmc_rescan_card(SDIOID, 1);
		DBG_8192C("[rtl8189es] %s: power up, rescan card.\n", __FUNCTION__);  			
	}
	else
	{
		ret = -1;
		DBG_8192C("[rtl8189es] %s: mod_sel = %d is incorrect.\n", __FUNCTION__, mod_sel);	
	}
#endif	// defined CONFIG_MMC_SUNXI_POWER_CONTROL
	if(ret != 0)
		goto exit;
	
#endif //CONFIG_PLATFORM_ARM_SUNxI

#ifdef CONFIG_PLATFORM_ARM_SUN6I
#ifdef CONFIG_MMC
{
        script_item_u val ;
        script_item_value_type_e type;

	type = script_get_item("wifi_para", "wifi_sdc_id", &val);
	if (SCIRPT_ITEM_VALUE_TYPE_INT!=type) {
		DBG_871X("get wifi_sdc_id failed\n");
		return -1;
	} else {
		sdio_id = val.val;
        }
	
	DBG_871X("----- %s mod_sel: %d, sdio num: %d\n", __FUNCTION__, mod_sel, sdio_id);
	if((mod_sel == SUNXI_SDIO_WIFI_NUM_RTL8723AS) || (mod_sel == SUNXI_SDIO_WIFI_NUM_RTL8723BS))
	{
		wifi_pm_power(1);
		mdelay(10);
		sw_mci_rescan_card(sdio_id, 1);
		DBG_871X("[rtl8723as] %s: power up, rescan card.\n", __FUNCTION__);  			
	}
	else
	{
		ret = -1;
		DBG_871X("[rtl8723as] %s: mod_sel = %d is incorrect.\n", __FUNCTION__, mod_sel);	
	}
        if(ret != 0)
		goto exit;

#ifdef CONFIG_GPIO_WAKEUP
  	type = script_get_item("wifi_para","rtl8723bs_wl_host_wake",&val);
  	if (SCIRPT_ITEM_VALUE_TYPE_PIO!=type){
                DBG_871X("has no rtl8723bs_wl_wake_host\n");
                return -1;
        }else{
                gpio_eint_wlan = val.gpio.gpio;
  	}
#endif  
}
#endif	//CONFIG_MMC		
#endif //CONFIG_PLATFORM_ARM_SUN6I

#ifdef CONFIG_PLATFORM_ARM_SUN8I
#ifdef CONFIG_MMC
	type = script_get_item("wifi_para", "wifi_sdc_id", &val);
	if (SCIRPT_ITEM_VALUE_TYPE_INT!=type) {
		DBG_871X("get wifi_sdc_id failed\n");
		return -1;
	} else {
		sdio_id = val.val;
  }
	
	DBG_871X("----- %s mod_sel: %d, sdio num: %d\n", __FUNCTION__, mod_sel, sdio_id);
  wifi_pm_power(1);
  mdelay(10);
  sunxi_mci_rescan_card(sdio_id, 1);
  printk("[rtl8723as] %s: power up, rescan card.\n", __FUNCTION__);
#endif	//CONFIG_MMC
	if(ret != 0)
		goto exit;		
#endif //CONFIG_PLATFORM_ARM_SUN8I


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)) 
	//console_suspend_enabled=0;
#endif

#if defined(CONFIG_PLATFORM_ARM_SUN6I) || defined(CONFIG_PLATFORM_ARM_SUN8I)
  type = script_get_item("wifi_para","rtl8723bs_wl_host_wake",&val);
  if (SCIRPT_ITEM_VALUE_TYPE_PIO!=type){ 
		printk("has no rtl8723bs_wl_wake_host\n");
		return -1;
	}else{
		gpio_eint_wlan = val.gpio.gpio;
  }
#endif

#if defined(CONFIG_PLATFORM_ARM_SUN8I)
  oob_irq = gpio_to_irq(gpio_eint_wlan);
#endif

#ifdef CONFIG_PLATFORM_SPRD

#ifdef CONFIG_RTL8188E
	rtw_wifi_gpio_wlan_ctrl(WLAN_POWER_ON);

	DBG_8192C("%s: turn on VDD-WIFI0 3.3V\n", __func__);
	//---VDD-WIFI0  3.3V
	LDO_TurnOnLDO(LDO_LDO_WIF0);
	LDO_SetVoltLevel(LDO_LDO_WIF0,LDO_VOLT_LEVEL1); 
#endif //CONFIG_RTL8188E 

	/* Pull up pwd pin, make wifi leave power down mode. */
	rtw_wifi_gpio_init();
	rtw_wifi_gpio_wlan_ctrl(WLAN_PWDN_ON);

#if (MP_DRIVER == 1) && (defined(CONFIG_RTL8723A) ||defined(CONFIG_RTL8723B))
	// Pull up BT reset pin.
	rtw_wifi_gpio_wlan_ctrl(WLAN_BT_PWDN_ON);
#endif
	rtw_mdelay_os(5);

	sdhci_bus_scan();
#ifdef CONFIG_RTL8723B
	//YJ,test,130305
	rtw_mdelay_os(1000);
#endif
#if (defined ANDROID_2X)
	rtw_mdelay_os(200);
#else
	if (1) {
		int i = 0;

		for (i = 0; i <= 50; i++) {
			msleep(10);
			if (sdhci_device_attached())
				break;
			printk("%s delay times:%d\n", __func__, i);
		}
	}
#endif
#endif // CONFIG_PLATFORM_SPRD

	rtw_suspend_lock_init();

	
	sdio_drvpriv.drv_registered = _TRUE;

	ret = sdio_register_driver(&sdio_drvpriv.r871xs_drv);

exit:
	DBG_871X_LEVEL(_drv_always_, "module init ret=%d\n", ret);

	rtw_android_wifictrl_func_add();

	return ret;
}

static void __exit rtw_drv_halt(void)
{
	DBG_871X_LEVEL(_drv_always_, "module exit start\n");

	sdio_drvpriv.drv_registered = _FALSE;

	sdio_unregister_driver(&sdio_drvpriv.r871xs_drv);
	rtw_android_wifictrl_func_del();

#ifdef CONFIG_PLATFORM_SPRD
	/* Pull down pwd pin, make wifi enter power down mode. */
	rtw_wifi_gpio_wlan_ctrl(WLAN_PWDN_OFF);
	rtw_mdelay_os(5);
	rtw_wifi_gpio_deinit();

#ifdef CONFIG_RTL8188E
	DBG_8192C("%s: turn off VDD-WIFI0 3.3V\n", __func__);
	LDO_TurnOffLDO(LDO_LDO_WIF0);
	rtw_wifi_gpio_wlan_ctrl(WLAN_POWER_OFF);
#endif // CONFIG_RTL8188E

#ifdef CONFIG_WOWLAN
	if(mmc_host){
		mmc_host->pm_flags &= ~MMC_PM_KEEP_POWER;
	}
#endif  //CONFIG_WOWLAN

#endif // CONFIG_PLATFORM_SPRD

#ifdef CONFIG_PLATFORM_ARM_SUNxI
#if defined(CONFIG_MMC_SUNXI_POWER_CONTROL)	
	sunximmc_rescan_card(SDIOID, 0);
#ifdef CONFIG_RTL8188E
	rtl8189es_sdio_poweroff();
	DBG_8192C("[rtl8189es] %s: remove card, power off.\n", __FUNCTION__);
#endif //CONFIG_RTL8188E
#endif //defined(CONFIG_MMC_SUNXI_POWER_CONTROL)
#endif //CONFIG_PLATFORM_ARM_SUNxI

#ifdef CONFIG_PLATFORM_ARM_SUN6I
#ifdef CONFIG_MMC
	wifi_pm_power(0);
	sw_mci_rescan_card(sdio_id, 0);
	DBG_871X("[rtl8723as] %s: remove card, power off.\n", __FUNCTION__);
#endif //CONFIG_MMC
#endif //CONFIG_PLATFORM_ARM_SUN6I

#ifdef CONFIG_PLATFORM_ARM_SUN8I
#ifdef CONFIG_MMC
	wifi_pm_power(0);
	sunxi_mci_rescan_card(sdio_id, 0);
	printk("[rtl8723as] %s: remove card, power off.\n", __FUNCTION__);
#endif //CONFIG_MMC
#endif //CONFIG_PLATFORM_ARM_SUN8I

	rtw_suspend_lock_uninit();
	DBG_871X_LEVEL(_drv_always_, "module exit success\n");

	rtw_mstat_dump();
}


module_init(rtw_drv_entry);
module_exit(rtw_drv_halt);

