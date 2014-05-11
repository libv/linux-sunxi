#include <mach/sys_config.h>
#include <linux/module.h>
#include "axp-cfg.h"

struct axp15_config_info axp15_config;
struct axp22_config_info axp22_config;

int axp_script_parser_fetch(char *main, char *sub, u32 *val, u32 size)
{
	script_item_u script_val;
	script_item_value_type_e type;

	type = script_get_item(main, sub, &script_val);
	if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
		printk("type err!");
	}
	*val = script_val.val;
	return 0;
}

int axp22_fetch_sysconfig_para(void)
{
	int ret;

	ret = axp_script_parser_fetch("pmu1_para", "pmu_used", &axp22_config.pmu_used, sizeof(int));
	if (ret)
		return -1;
	if (axp22_config.pmu_used) {
		ret = axp_script_parser_fetch("pmu1_para", "pmu_twi_id", &axp22_config.pmu_twi_id, sizeof(int));
		if (ret)
			axp22_config.pmu_twi_id = 0;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_twi_addr", &axp22_config.pmu_twi_addr, sizeof(int));
		if (ret)
			axp22_config.pmu_twi_addr = 34;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_irq_id", &axp22_config.pmu_irq_id, sizeof(int));
		if (ret)
			axp22_config.pmu_irq_id = 0;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_battery_rdc", &axp22_config.pmu_battery_rdc, sizeof(int));
		if (ret)
			axp22_config.pmu_battery_rdc = BATRDC;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_battery_cap", &axp22_config.pmu_battery_cap, sizeof(int));
		if (ret)
			axp22_config.pmu_battery_cap = 4000;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_batdeten", &axp22_config.pmu_batdeten, sizeof(int));
		if (ret)
			axp22_config.pmu_batdeten = 1;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_runtime_chgcur", &axp22_config.pmu_runtime_chgcur, sizeof(int));
		if (ret)
			axp22_config.pmu_runtime_chgcur = INTCHGCUR / 1000;
		axp22_config.pmu_runtime_chgcur = axp22_config.pmu_runtime_chgcur * 1000;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_earlysuspend_chgcur", &axp22_config.pmu_earlysuspend_chgcur, sizeof(int));
		if (ret)
			axp22_config.pmu_earlysuspend_chgcur = 500;
		axp22_config.pmu_earlysuspend_chgcur = axp22_config.pmu_earlysuspend_chgcur * 1000,

		ret = axp_script_parser_fetch("pmu1_para", "pmu_suspend_chgcur", &axp22_config.pmu_suspend_chgcur, sizeof(int));
		if (ret)
			axp22_config.pmu_suspend_chgcur = 1200;
		axp22_config.pmu_suspend_chgcur = axp22_config.pmu_suspend_chgcur * 1000;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_shutdown_chgcur", &axp22_config.pmu_shutdown_chgcur, sizeof(int));
		if (ret)
			axp22_config.pmu_shutdown_chgcur = 1200;
		axp22_config.pmu_shutdown_chgcur = axp22_config.pmu_shutdown_chgcur *1000;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_init_chgvol", &axp22_config.pmu_init_chgvol, sizeof(int));
		if (ret)
			axp22_config.pmu_init_chgvol = INTCHGVOL / 1000;
		axp22_config.pmu_init_chgvol = axp22_config.pmu_init_chgvol * 1000;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_init_chgend_rate", &axp22_config.pmu_init_chgend_rate, sizeof(int));
		if (ret)
			axp22_config.pmu_init_chgend_rate = INTCHGENDRATE;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_init_chg_enabled", &axp22_config.pmu_init_chg_enabled, sizeof(int));
		if (ret)
			axp22_config.pmu_init_chg_enabled = 1;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_init_adc_freq", &axp22_config.pmu_init_adc_freq, sizeof(int));
		if (ret)
			axp22_config.pmu_init_adc_freq = INTADCFREQ;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_init_adcts_freq", &axp22_config.pmu_init_adcts_freq, sizeof(int));
		if (ret)
			axp22_config.pmu_init_adcts_freq = INTADCFREQC;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_init_chg_pretime", &axp22_config.pmu_init_chg_pretime, sizeof(int));
		if (ret)
			axp22_config.pmu_init_chg_pretime = INTCHGPRETIME;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_init_chg_csttime", &axp22_config.pmu_init_chg_csttime, sizeof(int));
		if (ret)
			axp22_config.pmu_init_chg_csttime = INTCHGCSTTIME;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_batt_cap_correct", &axp22_config.pmu_batt_cap_correct, sizeof(int));
		if (ret)
			axp22_config.pmu_batt_cap_correct = 1;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_regu_en", &axp22_config.pmu_bat_regu_en, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_regu_en = 0;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para1", &axp22_config.pmu_bat_para1, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para1 = OCVREG0;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para2", &axp22_config.pmu_bat_para2, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para2 = OCVREG1;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para3", &axp22_config.pmu_bat_para3, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para3 = OCVREG2;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para4", &axp22_config.pmu_bat_para4, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para4 = OCVREG3;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para5", &axp22_config.pmu_bat_para5, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para5 = OCVREG4;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para6", &axp22_config.pmu_bat_para6, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para6 = OCVREG5;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para7", &axp22_config.pmu_bat_para7, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para7 = OCVREG6;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para8", &axp22_config.pmu_bat_para8, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para8 = OCVREG7;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para9", &axp22_config.pmu_bat_para9, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para9 = OCVREG8;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para10", &axp22_config.pmu_bat_para10, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para10 = OCVREG9;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para11", &axp22_config.pmu_bat_para11, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para11 = OCVREGA;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para12", &axp22_config.pmu_bat_para12, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para12 = OCVREGB;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para13", &axp22_config.pmu_bat_para13, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para13 = OCVREGC;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para14", &axp22_config.pmu_bat_para14, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para14 = OCVREGD;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para15", &axp22_config.pmu_bat_para15, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para15 = OCVREGE;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para16", &axp22_config.pmu_bat_para16, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para16 = OCVREGF;

		//Add 32 Level OCV para 20121128 by evan
		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para17", &axp22_config.pmu_bat_para17, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para17 = OCVREG10;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para18", &axp22_config.pmu_bat_para18, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para18 = OCVREG11;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para19", &axp22_config.pmu_bat_para19, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para19 = OCVREG12;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para20", &axp22_config.pmu_bat_para20, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para20 = OCVREG13;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para21", &axp22_config.pmu_bat_para21, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para21 = OCVREG14;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para22", &axp22_config.pmu_bat_para22, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para22 = OCVREG15;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para23", &axp22_config.pmu_bat_para23, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para23 = OCVREG16;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para24", &axp22_config.pmu_bat_para24, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para24 = OCVREG17;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para25", &axp22_config.pmu_bat_para25, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para25 = OCVREG18;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para26", &axp22_config.pmu_bat_para26, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para26 = OCVREG19;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para27", &axp22_config.pmu_bat_para27, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para27 = OCVREG1A;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para28", &axp22_config.pmu_bat_para28, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para28 = OCVREG1B;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para29", &axp22_config.pmu_bat_para29, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para29 = OCVREG1C;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para30", &axp22_config.pmu_bat_para30, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para30 = OCVREG1D;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para31", &axp22_config.pmu_bat_para31, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para31 = OCVREG1E;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_bat_para32", &axp22_config.pmu_bat_para32, sizeof(int));
		if (ret)
			axp22_config.pmu_bat_para32 = OCVREG1F;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_usbvol_limit", &axp22_config.pmu_usbvol_limit, sizeof(int));
		if (ret)
			axp22_config.pmu_usbvol_limit = 1;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_usbvol", &axp22_config.pmu_usbvol, sizeof(int));
		if (ret)
			axp22_config.pmu_usbvol = 4400;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_usbvol_pc", &axp22_config.pmu_usbvol_pc, sizeof(int));
		if (ret)
			axp22_config.pmu_usbvol_pc = 4400;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_usbcur_limit", &axp22_config.pmu_usbcur_limit, sizeof(int));
		if (ret)
			axp22_config.pmu_usbcur_limit = 1;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_usbcur", &axp22_config.pmu_usbcur, sizeof(int));
		if (ret)
			axp22_config.pmu_usbcur = 0;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_usbcur_pc", &axp22_config.pmu_usbcur_pc, sizeof(int));
		if (ret)
			axp22_config.pmu_usbcur_pc = 0;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_pwroff_vol", &axp22_config.pmu_pwroff_vol, sizeof(int));
		if (ret)
			axp22_config.pmu_pwroff_vol = 3300;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_pwron_vol", &axp22_config.pmu_pwron_vol, sizeof(int));
		if (ret)
			axp22_config.pmu_pwron_vol = 2900;

		ret = axp_script_parser_fetch("power_sply", "dcdc1_vol", &axp22_config.dcdc1_vol, sizeof(int));
		if (ret)
			axp22_config.dcdc1_vol = 3000;

		ret = axp_script_parser_fetch("power_sply", "dcdc2_vol", &axp22_config.dcdc2_vol, sizeof(int));
		if (ret)
			axp22_config.dcdc2_vol = 1100;

		ret = axp_script_parser_fetch("power_sply", "dcdc3_vol", &axp22_config.dcdc3_vol, sizeof(int));
		if (ret)
			axp22_config.dcdc3_vol = 1200;

		ret = axp_script_parser_fetch("power_sply", "dcdc4_vol", &axp22_config.dcdc4_vol, sizeof(int));
		if (ret)
			axp22_config.dcdc4_vol = 1200;

		ret = axp_script_parser_fetch("power_sply", "dcdc5_vol", &axp22_config.dcdc5_vol, sizeof(int));
		if (ret)
			axp22_config.dcdc5_vol = 1500;

		ret = axp_script_parser_fetch("power_sply", "aldo2_vol", &axp22_config.aldo2_vol, sizeof(int));
		if (ret)
			axp22_config.aldo2_vol = 1800;

		ret = axp_script_parser_fetch("power_sply", "aldo3_vol", &axp22_config.aldo3_vol, sizeof(int));
		if (ret)
			axp22_config.aldo3_vol = 3000;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_pekoff_time", &axp22_config.pmu_pekoff_time, sizeof(int));
		if (ret)
			axp22_config.pmu_pekoff_time = 6000;

		//offlevel restart or not 0:not restart 1:restart
		ret = axp_script_parser_fetch("pmu1_para", "pmu_pekoff_func", &axp22_config.pmu_pekoff_func, sizeof(int));
		if (ret)
			axp22_config.pmu_pekoff_func   = 0;

		//16's power restart or not 0:not restart 1:restart
		ret = axp_script_parser_fetch("pmu1_para", "pmu_pekoff_en", &axp22_config.pmu_pekoff_en, sizeof(int));
		if (ret)
			axp22_config.pmu_pekoff_en   = 1;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_peklong_time", &axp22_config.pmu_peklong_time, sizeof(int));
		if (ret)
			axp22_config.pmu_peklong_time = 1500;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_pwrok_time", &axp22_config.pmu_pwrok_time, sizeof(int));
		if (ret)
			axp22_config.pmu_pwrok_time    = 64;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_pekon_time", &axp22_config.pmu_pekon_time, sizeof(int));
		if (ret)
			axp22_config.pmu_pekon_time = 1000;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_battery_warning_level1", &axp22_config.pmu_battery_warning_level1, sizeof(int));
		if (ret)
			axp22_config.pmu_battery_warning_level1 = 15;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_battery_warning_level2", &axp22_config.pmu_battery_warning_level2, sizeof(int));
		if (ret)
			axp22_config.pmu_battery_warning_level2 = 0;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_restvol_adjust_time", &axp22_config.pmu_restvol_adjust_time, sizeof(int));
		if (ret)
			axp22_config.pmu_restvol_adjust_time = 30;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_ocv_cou_adjust_time", &axp22_config.pmu_ocv_cou_adjust_time, sizeof(int));
		if (ret)
			axp22_config.pmu_ocv_cou_adjust_time = 60;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_chgled_func", &axp22_config.pmu_chgled_func, sizeof(int));
		if (ret)
			axp22_config.pmu_chgled_func = 0;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_chgled_type", &axp22_config.pmu_chgled_type, sizeof(int));
		if (ret)
			axp22_config.pmu_chgled_type = 0;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_vbusen_func", &axp22_config.pmu_vbusen_func, sizeof(int));
		if (ret)
			axp22_config.pmu_vbusen_func = 1;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_reset", &axp22_config.pmu_reset, sizeof(int));
		if (ret)
			axp22_config.pmu_reset = 0;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_IRQ_wakeup", &axp22_config.pmu_IRQ_wakeup, sizeof(int));
		if (ret)
			axp22_config.pmu_IRQ_wakeup = 0;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_hot_shutdowm", &axp22_config.pmu_hot_shutdowm, sizeof(int));
		if (ret)
			axp22_config.pmu_hot_shutdowm = 1;

		ret = axp_script_parser_fetch("pmu1_para", "pmu_inshort", &axp22_config.pmu_inshort, sizeof(int));
		if (ret)
			axp22_config.pmu_inshort = 0;

		ret = axp_script_parser_fetch("pmu1_para", "power_start", &axp22_config.power_start, sizeof(int));
		if (ret)
			axp22_config.power_start = 0;
	} else {
		return -1;
	}

	return 0;
}

int axp15_fetch_sysconfig_para(void)
{
	int ret;

	ret = axp_script_parser_fetch("pmu2_para", "pmu_used", &axp15_config.pmu_used, sizeof(int));
	if (ret)
		return -1;
	if (axp15_config.pmu_used) {
		ret = axp_script_parser_fetch("pmu2_para", "pmu_twi_id", &axp15_config.pmu_twi_id, sizeof(int));
		if (ret)
			axp15_config.pmu_twi_id = 0;

		ret = axp_script_parser_fetch("pmu2_para", "pmu_twi_addr", &axp15_config.pmu_twi_addr, sizeof(int));
		if (ret)
			axp15_config.pmu_twi_addr = 34;

		ret = axp_script_parser_fetch("pmu2_para", "pmu_irq_id", &axp15_config.pmu_irq_id, sizeof(int));
		if (ret)
			axp15_config.pmu_irq_id = 0;
	} else {
		return -1;
	}
	return 0;
}

MODULE_DESCRIPTION("X-POWERS axp script");
MODULE_AUTHOR("Li Ming");
MODULE_LICENSE("GPL");
