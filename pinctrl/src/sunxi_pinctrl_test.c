/*
 * Allwinner A1X SoCs pinctrl driver.
 *
 * Copyright (C) 2012 Maxime Ripard
 *
 * Shaorui Huang  <huangshr@allwinnertech.com>
 * 
 * 2013-06-10  add sunxi pinctrl testing case.
 * 
 * This file is licensed under the terms of the GNU General Public
 * License version 1.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/io.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/pinconf-sunxi.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/device.h> 
#include <linux/string.h>
#include <mach/sys_config.h>
#include <mach/platform.h>
#include <mach/gpio.h>

#include "core.h"

#define SUNXI_PINCTRL_TEST_NUMBER 	10
#define SUNXI_DEV_NAME_MAX_LEN		20

#define CASE_TEST_SUCCESSED			0
#define CASE_TEST_FAILED			1
#define CASE_HAVE_NOT_TEST			2

struct result_class{
	char 		*name;
	int  		result;

};

struct sunxi_pinctrl_test_class{
	unsigned		int exec;
	unsigned		int gpio_index;
	unsigned		int funcs;
	unsigned 		int dat;
	unsigned		int dlevel;
	unsigned 		int pul;
	unsigned 		int trigger;
	unsigned 		int test_result;
	char 				dev_name[SUNXI_DEV_NAME_MAX_LEN];
	struct	device	*dev;
};

static struct result_class  sunxi_pinctrl_result[SUNXI_PINCTRL_TEST_NUMBER];
static struct class *sunxi_pinctrl_test_init_class;
static unsigned int test_case_number;


static int test_request_all_resource_api(struct device *dev,
				struct sunxi_pinctrl_test_class *sunxi_pinctrl_test)
{
	
	struct pinctrl			*pinctrl;
	struct gpio_config		 pin_cfg;
	script_item_u 			*pin_list;
	u16 		 			 pin_count;
	u16 		 			 pin_index;
	long unsigned int  		 config;
	char 		 			 pin_name[SUNXI_PIN_NAME_MAX_LEN];
	int						 ret=0;
	
	sunxi_pinctrl_result[0].name = "test_request_all_resource_api";
	test_case_number = 0;
	dev_set_name(dev, sunxi_pinctrl_test->dev_name);
	pr_warn("\n[%s: %d]start...\n",__func__,__LINE__);
	pr_warn("[%s: %d]request device name :[%s]\n",
			__func__,__LINE__,dev_name(dev));
	dev_set_name(dev, sunxi_pinctrl_test->dev_name);
	pinctrl = devm_pinctrl_get_select_default(dev);	
	if (IS_ERR_OR_NULL(pinctrl)) {
		pr_warn("\nrequest pinctrl handle for device [%s] failed...\n"
				,dev_name(dev));
		pr_warn("check if main_key in the sys_config is same as device name?\n");
		return -EINVAL;
	}
	pr_warn("\n[%s: %d]check pin count...\n",__func__,__LINE__);
	pin_count = script_get_pio_list(sunxi_pinctrl_test->dev_name,&pin_list);
	if (pin_count == 0) {
		pr_warn(" devices own 0 pin resource or look for main key failed!\n");
		return -EINVAL;
	}
	pr_warn("\n[%s: %d]check pin configure...\n",__func__,__LINE__);
	for(pin_index = 0; pin_index < pin_count; pin_index++)
	{
		pin_cfg = pin_list[pin_index].gpio;
		sunxi_gpio_to_name(pin_cfg.gpio, pin_name);
		
		/*check function config */
		config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0xFFFF);
		pin_config_get(SUNXI_PINCTRL,pin_name,&config);
		if (pin_cfg.mul_sel != SUNXI_PINCFG_UNPACK_VALUE(config)){
			pr_warn("[%s: %d]pin mul value isn't equal as sys_config's.failed !"
					,__func__,__LINE__);
			return -EINVAL;
		}
		if (pin_cfg.pull != GPIO_PULL_DEFAULT){
			config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_PUD, 0xFFFF);
			pin_config_get(SUNXI_PINCTRL, pin_name, &config);
			if (pin_cfg.pull != SUNXI_PINCFG_UNPACK_VALUE(config)){
				pr_warn("[%s: %d]pin pull value isn't equal as sys_config's.failed !"
						,__func__,__LINE__);
				return -EINVAL;
			}
		}
		if (pin_cfg.drv_level != GPIO_DRVLVL_DEFAULT){
			config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DRV,0XFFFF);
			pin_config_get(SUNXI_PINCTRL,pin_name,&config);
			if(pin_cfg.drv_level != SUNXI_PINCFG_UNPACK_VALUE(config)){
				pr_warn("[%s: %d]pin dlevel value isn't equal as sys_config's.failed !"
						,__func__,__LINE__);
				return -EINVAL;
			}
		}
		if (pin_cfg.data != GPIO_DATA_DEFAULT){
			config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT,0XFFFF);
			pin_config_get(SUNXI_PINCTRL,pin_name,&config);
			if(pin_cfg.data != SUNXI_PINCFG_UNPACK_VALUE(config)){
				pr_warn("[%s: %d]pin data value isn't equal as sys_config's.failed !"
						,__func__,__LINE__);
				return -EINVAL;
			}
		}
					
	}
	pr_warn("test pinctrl request all resource api success !\n");
	return ret=0;
}

static int test_re_request_all_resource_api(struct device *dev,
				struct sunxi_pinctrl_test_class *sunxi_pinctrl_test)
{
	struct pinctrl 			*pinctrl_1;
	struct pinctrl 			*pinctrl_2;
	int				 		 ret=0;
	sunxi_pinctrl_result[1].name="test_re_request_all_resource_api";
	test_case_number = 1;
	dev_set_name(dev, sunxi_pinctrl_test->dev_name);
	
	/*request all resource */
	pr_warn("\n[%s: %d]start...\n",__func__,__LINE__);
	pr_warn("[%s: %d]request device name :[%s]\n",
			__func__,__LINE__,dev_name(dev));
	pr_warn("\n[%s: %d]first time request pin resource...\n",__func__,__LINE__);
	
	pinctrl_1 = devm_pinctrl_get_select_default(dev);
	if (IS_ERR_OR_NULL(pinctrl_1)) {
		pr_warn("\nrequest pinctrl handle for device [%s] failed...\n"
				,dev_name(dev));
		pr_warn("check if main_key in the sys_config is same as device name?\n");
		return -EINVAL;
	}
	
	/*repeat request */
	pr_warn("\n[%s: %d]repeat request pin resource...\n",__func__,__LINE__);
	pinctrl_2 = devm_pinctrl_get_select_default(dev);
	if (IS_ERR_OR_NULL(pinctrl_2)) {
		pr_warn("repeat request pinctrl handle for device [%s] failed\n", 
			   dev_name(dev));
		pr_warn("test pinctrl repeat request all resource success!\n");
		return ret=0;
	}else{
		pr_warn("test pinctrl repeat request all resource failed!\n");
		return ret=1;
	}
			
}

static int test_config_get_api(struct device *dev,
			struct sunxi_pinctrl_test_class *sunxi_pinctrl_test)
{
	int ret=0;
	struct gpio_config		 pin_cfg;
	script_item_u 			*pin_list;
	u16 		 			 pin_count;
	u16 		 			 pin_index;
	long unsigned int  		 config;
	char 		 			 pin_name[SUNXI_PIN_NAME_MAX_LEN];

	sunxi_pinctrl_result[2].name = "test_config_get_api";
	test_case_number = 2;

	pr_warn("\n[%s: %d]start...\n",__func__,__LINE__);
	pin_count = script_get_pio_list(sunxi_pinctrl_test->dev_name,&pin_list);
	pr_warn("device name : %s.		pin count : %d.\n"
				,sunxi_pinctrl_test->dev_name,pin_count);
	if (pin_count == 0) {
		pr_warn(" devices own 0 pin resource or look for main key failed!\n");
		return -EINVAL;
	}
	pr_warn("\n[%s: %d]begin check pin configure...\n",__func__,__LINE__);
	for(pin_index = 0; pin_index < pin_count; pin_index++)
	{
		pin_cfg = pin_list[pin_index].gpio;
		sunxi_gpio_to_name(pin_cfg.gpio, pin_name);
		/*check function config */
		config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0xFFFF);
		pin_config_get(SUNXI_PINCTRL,pin_name,&config);
		if (pin_cfg.mul_sel != SUNXI_PINCFG_UNPACK_VALUE(config)){
			pr_warn("test pin config for mul getting failed !\n");
			return -EINVAL;
		}
		if (pin_cfg.pull != GPIO_PULL_DEFAULT){
			config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_PUD, 0xFFFF);
			pin_config_get(SUNXI_PINCTRL, pin_name, &config);
			if (pin_cfg.pull != SUNXI_PINCFG_UNPACK_VALUE(config)){
				pr_warn("test pin config for pull getting failed ! \n");
				return -EINVAL;
			}
		}
		if (pin_cfg.drv_level != GPIO_DRVLVL_DEFAULT){
			config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DRV,0XFFFF);
			pin_config_get(SUNXI_PINCTRL,pin_name,&config);
			if(pin_cfg.drv_level != SUNXI_PINCFG_UNPACK_VALUE(config)){
				pr_warn("test pin config for level getting failed !\n");
				return -EINVAL;
			}
		}
		if (pin_cfg.data != GPIO_DATA_DEFAULT){
			config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT,0XFFFF);
			pin_config_get(SUNXI_PINCTRL,pin_name,&config);
			if(pin_cfg.data != SUNXI_PINCFG_UNPACK_VALUE(config)){
				pr_warn("test pin config for data getting failed !\n");
				return -EINVAL;
			}
		}
					
	}
	
	pr_warn("test pin configure get success ! \n");	
	return ret;
}



static int test_config_set_api(struct device *dev,
			struct sunxi_pinctrl_test_class *sunxi_pinctrl_test)
{
	int ret=0;
	long unsigned int 		 config_set;
	long unsigned int 		 config_get;
	char 					 pin_name[SUNXI_PIN_NAME_MAX_LEN];
	int 					 dat=sunxi_pinctrl_test->dat;
	int 					 dlevel=sunxi_pinctrl_test->dlevel;
	int 					 func=sunxi_pinctrl_test->funcs;
	int 					 pul=sunxi_pinctrl_test->pul;
	sunxi_pinctrl_result[3].name = "test_config_set_api";
	test_case_number = 3;
	sunxi_gpio_to_name(sunxi_pinctrl_test->gpio_index, pin_name);
	pr_warn("pin name :%s",pin_name);
	pr_warn("\n[%s: %d]start...\n",__func__,__LINE__);
	pr_warn("\n[%s: %d]check pin mul set api\n",__func__,__LINE__);
	/*check if pin mul setting right */
	config_set = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,func);
	pin_config_set(SUNXI_PINCTRL,pin_name,config_set);
	config_get = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0XFFFF);
	pin_config_get(SUNXI_PINCTRL,pin_name,&config_get);
	if (func != SUNXI_PINCFG_UNPACK_VALUE(config_get)){
		pr_warn("test pin config for mul setting failed !\n");
		return -EINVAL;
	}

	/*check if pin pull setting right */
	pr_warn("\n[%s: %d]check pin pull set api\n",__func__,__LINE__);
	config_set = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_PUD,pul);
	pin_config_set(SUNXI_PINCTRL,pin_name,config_set);
	config_get = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_PUD,0XFFFF);
	pin_config_get(SUNXI_PINCTRL,pin_name,&config_get);
	if (pul != SUNXI_PINCFG_UNPACK_VALUE(config_get)){
		pr_warn("test pin config for pull setting failed !\n");
		return -EINVAL;
	}

	/*check if pin dlevel setting right */
	pr_warn("\n[%s: %d]check pin dlevel set api\n",__func__,__LINE__);
	config_set = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DRV,dlevel);
	pin_config_set(SUNXI_PINCTRL,pin_name,config_set);
	config_get = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DRV,0XFFFF);
	pin_config_get(SUNXI_PINCTRL,pin_name,&config_get);
	if (dlevel != SUNXI_PINCFG_UNPACK_VALUE(config_get)){
		pr_warn("test pin config for dlevel setting failed !\n");
		return -EINVAL;
	}

	/*check if pin dat setting right */
	pr_warn("\n[%s: %d]check pin data set api\n",__func__,__LINE__);
	config_set = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT,dat);
	pin_config_set(SUNXI_PINCTRL,pin_name,config_set);
	config_get = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT,0XFFFF);
	pin_config_get(SUNXI_PINCTRL,pin_name,&config_get);
	if (dat != SUNXI_PINCFG_UNPACK_VALUE(config_get)){
		pr_warn("test pin config for dat setting failed !\n");
		return -EINVAL;
	}

	pr_warn("test pin configure set success ! \n");
	return ret;
}



static int test_gpio_request_free_api(struct sunxi_pinctrl_test_class *sunxi_pinctrl_test)
{
	int 			ret=0;
	int 			req_status;
	int 			gpio_index=sunxi_pinctrl_test->gpio_index;
	sunxi_pinctrl_result[4].name="test_gpio_request_free_api";
	test_case_number = 4;

	/* request gpio*/
	pr_warn("\n[%s: %d]start...\n",__func__,__LINE__);
	gpio_free(gpio_index);
	req_status = gpio_request(gpio_index,NULL);
	if(0 != req_status){
		pr_warn("gpio request failed !\n");
		return -EINVAL;
	} 
	
	gpio_free(gpio_index);
	pr_warn("test gpio request free api success!\n");
	return ret;


}


static int test_gpio_re_request_free_api(struct sunxi_pinctrl_test_class *sunxi_pinctrl_test)
{
	int 			ret=0;
	int 			req_status;
	int 			gpio_index=sunxi_pinctrl_test->gpio_index;
	sunxi_pinctrl_result[5].name="test_gpio_re_request_free_api";
	test_case_number = 5;

	gpio_free(gpio_index);
	/* request gpio*/
	pr_warn("\n[%s: %d]start...\n",__func__,__LINE__);
	pr_warn("\n[%s: %d]first time request gpio \n",__func__,__LINE__);
	req_status = gpio_request(gpio_index,NULL);
	if(0 != req_status){
		pr_warn("gpio request failed !\n");
		return -EINVAL;
	}
	/* repeat request gpio */
	pr_warn("\n[%s: %d] repeat request gpio \n",__func__,__LINE__);
	req_status = gpio_request(gpio_index,NULL);
	if(0 != req_status){
		pr_warn("test gpio repeat request success!\n");
		gpio_free(gpio_index);	
		return ret=0;
	}else{
		pr_warn("a gpio can repeat request! test failed!\n");
		gpio_free(gpio_index);	
		return ret=1;

	}
			
}


static int test_gpiolib_api(struct sunxi_pinctrl_test_class *sunxi_pinctrl_test)
{
	int 					 ret=0;
	int 					 val;
	u16 		 			 pin_index;
	char					 pin_name[SUNXI_PIN_NAME_MAX_LEN];
	int 					 req_status;
	int						 set_direct_status;
	long unsigned int 		 config;
	sunxi_pinctrl_result[6].name = "test_gpiolib_api";
	test_case_number = 6;
	pin_index = sunxi_pinctrl_test->gpio_index;
	sunxi_gpio_to_name(pin_index, pin_name);
	
	pr_warn("\n[%s: %d]start...\n",__func__,__LINE__);
	pr_warn("gpio name is : %s.  pin index is : %d. \n",pin_name,pin_index);

	/* test gpio set direction input */
	pr_warn("\n[ %s: %d ] --> test gpio direction input api : \n",__func__,__LINE__);
	req_status = gpio_request(pin_index,NULL);
	if(0 != req_status){
		pr_warn("gpio request failed for test gpio set direction input!\n");
		return -EINVAL;
	}
	set_direct_status = gpio_direction_input(pin_index);
	if (IS_ERR_VALUE(set_direct_status)) {
		pr_warn("set gpio direction input failed, errno %d\n", set_direct_status);
		goto test_gpiolib_api_failed;
	}
	config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0xFFFF);
	pin_config_get(SUNXI_PINCTRL,pin_name,&config);
	if (0 != SUNXI_PINCFG_UNPACK_VALUE(config)){
		pr_warn("test gpio set direction input failed !\n");
		goto test_gpiolib_api_failed;
	}	
	gpio_free(pin_index);


	/* test gpio set direction output  */
	pr_warn("\n[ %s: %d ] --> test gpio direction output api : \n",__func__,__LINE__);
	req_status = gpio_request(pin_index,NULL);
	if(0 != req_status){
		pr_warn("gpio request failed for test gpio set direction output!\n");
		return -EINVAL;
	}
	set_direct_status = gpio_direction_output(pin_index,1);
	if (IS_ERR_VALUE(set_direct_status)) {
		pr_warn("set gpio direction output failed, errno %d\n", set_direct_status);
		goto test_gpiolib_api_failed;
	}
	config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_FUNC,0xFFFF);
	pin_config_get(SUNXI_PINCTRL,pin_name,&config);
	if (1 != SUNXI_PINCFG_UNPACK_VALUE(config)){
		pr_warn("test gpio set direction output failed !\n");
		goto test_gpiolib_api_failed;
	}
	gpio_free(pin_index);


	/* check gpio get value */
	pr_warn("\n[ %s: %d ] --> test gpio get value api : \n",__func__,__LINE__);
	req_status = gpio_request(pin_index,NULL);
	if(0 != req_status){
		pr_warn("gpio request failed for check gpio get value!\n");
		return -EINVAL;
	}
	set_direct_status = gpio_direction_output(pin_index,0);
	if (IS_ERR_VALUE(set_direct_status)) {
		pr_warn("set gpio direction output failed for check gpio get value %d\n"
				, set_direct_status);
		goto test_gpiolib_api_failed;
	}
	val=__gpio_get_value(pin_index);
	pr_warn("gpio value : %d \n",val);
	
	config = SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT,0xFFFF);
	pin_config_get(SUNXI_PINCTRL,pin_name,&config);
	if (val != SUNXI_PINCFG_UNPACK_VALUE(config)){
		pr_warn("check gpio get value failed !\n");
		goto test_gpiolib_api_failed;
	}
	gpio_free(pin_index);

	/* check gpio set value */
	pr_warn("\n[ %s: %d ] --> test gpio set value api : \n",__func__,__LINE__);
	req_status = gpio_request(pin_index,NULL);
	if(0 != req_status){
		pr_warn("gpio request failed for check gpio get value!\n");
		return -EINVAL;
	}
	set_direct_status = gpio_direction_output(pin_index,0);
	if (IS_ERR_VALUE(set_direct_status)) {
		pr_warn("set gpio direction output failed for check gpio get value %d\n"
				, set_direct_status);
		goto test_gpiolib_api_failed;
	}
	__gpio_set_value(pin_index,1);
	config=SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT,0xFFFF);
	pin_config_get(SUNXI_PINCTRL,pin_name,&config);
	if (1 != SUNXI_PINCFG_UNPACK_VALUE(config)){
		pr_warn("test gpio set dat value 1 failed ! \n");
		goto test_gpiolib_api_failed;
	}
	__gpio_set_value(pin_index,0);
	config=SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT,0xFFFF);
	pin_config_get(SUNXI_PINCTRL,pin_name,&config);
	if (0 != SUNXI_PINCFG_UNPACK_VALUE(config)){
		pr_warn("test gpio set dat value 0 failed ! \n");
		goto test_gpiolib_api_failed;
	}
	
	gpio_free(pin_index);
	pr_warn("test gpiolib api success!\n\n");
	return ret;
	
test_gpiolib_api_failed:
	pr_warn("test gpiolib api failed!\n\n");
	gpio_free(pin_index);
	return -EINVAL;

}


static int test_pinctrl_scripts_api(struct sunxi_pinctrl_test_class *sunxi_pinctrl_test)
{
	int ret=0;
	sunxi_pinctrl_result[7].name="test_pinctrl_eint_api";
	test_case_number = 7;
	pr_warn("api be remove !\n");
	return ret;
}

static irqreturn_t test_sunxi_pinctrl_irq_handler(int irq, void *dev_id)
{
	
	pr_warn("[ %s: %d ] : test sunxi pinctrl pin irq!\n",__func__,__LINE__);
	disable_irq_nosync(irq);
	return IRQ_HANDLED;
}
static irqreturn_t test_sunxi_pinctrl_irq_handler_demo1(int irq, void *dev_id)
{
	
	pr_warn("[ %s: %d ] : test sunxi pinctrl pin  irq!\n",__func__,__LINE__);
	return IRQ_HANDLED;
}

static irqreturn_t test_sunxi_pinctrl_irq_handler_demo2(int irq, void *dev_id)
{
	
	pr_warn("[ %s: %d ] : test sunxi pinctrl pin repeat irq!\n",__func__,__LINE__);
	return IRQ_HANDLED;
}


static int test_pinctrl_eint_api(struct device *dev,
			struct sunxi_pinctrl_test_class *sunxi_pinctrl_test)
{
	int 					ret=0;
	int 					virq;
	int						req_status;
	int						set_direct_status;
	long unsigned int 		config;
	int 					req_IRQ_status;
	int						pin_index=sunxi_pinctrl_test->gpio_index;
	char					 pin_name[SUNXI_PIN_NAME_MAX_LEN];
	sunxi_pinctrl_result[8].name="test_pinctrl_eint_api";
	test_case_number = 8;
	sunxi_gpio_to_name(pin_index, pin_name);
	
	pr_warn("\n[%s: %d]start...\n",__func__,__LINE__);
	pr_warn("\n[%s: %d]set pin data to 0...\n",__func__,__LINE__);
	req_status = gpio_request(pin_index,NULL);
	if(0 != req_status){
		pr_warn("gpio request failed \n");
		return -EINVAL;
	}
	set_direct_status = gpio_direction_output(pin_index,0);
	if (IS_ERR_VALUE(set_direct_status)) {
		pr_warn("set gpio direction output failed for check gpio get value %d\n"
				, set_direct_status);
		return -EINVAL;
	}
	__gpio_set_value(pin_index,0);
	config=SUNXI_PINCFG_PACK(SUNXI_PINCFG_TYPE_DAT,0xFFFF);
	pin_config_get(SUNXI_PINCTRL,pin_name,&config);
	gpio_free(pin_index);
	

	pr_warn("[ %s: %d ]generate vitural irq ...\n",__func__,__LINE__);
	virq = gpio_to_irq(pin_index);
	if (IS_ERR_VALUE(virq)){
		pr_warn("map gpio [%d] to virq [%d] failed !\n ",pin_index,virq);
		return -EINVAL;
	}
	pr_warn("[ %s: %d ] request irq...\n",__func__,__LINE__);
	req_IRQ_status=devm_request_irq(dev, virq, test_sunxi_pinctrl_irq_handler, 
			       IRQF_TRIGGER_LOW, "PIN_EINT", NULL);
	if (IS_ERR_VALUE(req_IRQ_status)){
		pr_warn("test pin request irq failed !\n");
		return -EINVAL;
	} else {
		pr_warn("[ %s: %d ] free irq...\n",__func__,__LINE__);
		devm_free_irq(dev,virq,NULL);
		pr_warn("test pin eint sunccess !\n\n");
		return ret;
	}




	
}


static int test_pinctrl_re_eint_api(struct device *dev,
			struct sunxi_pinctrl_test_class *sunxi_pinctrl_test)
{
	int 					ret=0;
	int 					virq;
	int 					req_status;
	int 					re_req_status;
	int						pin_index=sunxi_pinctrl_test->gpio_index;
	sunxi_pinctrl_result[9].name="test_pinctrl_re_eint_api";
	test_case_number = 9;

	pr_warn("\n[%s: %d]start...\n",__func__,__LINE__);
	pr_warn("[ %s: %d ]generate vitural irq ...\n",__func__,__LINE__);
	
	
	virq = gpio_to_irq(pin_index);
	if (IS_ERR_VALUE(virq)){
		pr_warn("map gpio [%d] to virq [%d] failed !\n ",pin_index,virq);
		return -EINVAL;
	}

	pr_warn("[ %s: %d ] first time request irq...\n",__func__,__LINE__);
	req_status=devm_request_irq(dev, virq, test_sunxi_pinctrl_irq_handler_demo1, 
			       IRQF_TRIGGER_HIGH, "PIN_EINT", NULL);
	if (IS_ERR_VALUE(req_status)){
		devm_free_irq(dev,virq,NULL);
		pr_warn("test pin request irq failed !\n");
		return -EINVAL;
	}
	pr_warn("[ %s: %d ] repeat request irq...\n",__func__,__LINE__);
	re_req_status=devm_request_irq(dev, virq, test_sunxi_pinctrl_irq_handler_demo2, 
			       IRQF_TRIGGER_HIGH, "PIN_EINT", NULL);
	if (IS_ERR_VALUE(re_req_status)){		
		devm_free_irq(dev,virq,NULL);
		pr_warn("test pin repeat eint sunccess !\n");	
		return ret=0;
	}else{
		devm_free_irq(dev,virq,NULL);
		pr_warn("test pin repeat eint failed !\n");
		return ret=1;
	}
		


}



static ssize_t exec_show(struct device *dev,struct device_attribute *attr,
							char *buf)
{
	struct sunxi_pinctrl_test_class *sunxi_pinctrl_test = dev_get_drvdata(dev);
	return sprintf(buf,"%u\n",sunxi_pinctrl_test->exec);
}
static ssize_t exec_store(struct device *dev,struct device_attribute *attr,
							const char *buf,size_t size)
{
	struct sunxi_pinctrl_test_class *sunxi_pinctrl_test = dev_get_drvdata(dev);
	char 			*after;
	int 			final =0;
	int 			exec_number =simple_strtoul(buf,&after,10);
	switch(exec_number){
		case 0:
			final=test_request_all_resource_api(dev,sunxi_pinctrl_test);
			break;
		case 1:
			final=test_re_request_all_resource_api(dev,sunxi_pinctrl_test);
			break;
		case 2: 
			final=test_config_get_api(dev,sunxi_pinctrl_test);
			break;
		case 3: 
			final=test_config_set_api(dev,sunxi_pinctrl_test);
			break;
		case 4:
			final=test_gpio_request_free_api(sunxi_pinctrl_test);
			break;
		case 5:
			final=test_gpio_re_request_free_api(sunxi_pinctrl_test);
			break;
		case 6: 
			final=test_gpiolib_api(sunxi_pinctrl_test);
			break;
		case 7:
			final=test_pinctrl_scripts_api(sunxi_pinctrl_test);
			break;
		case 8:
			final=test_pinctrl_eint_api(dev,sunxi_pinctrl_test);
			break;
		case 9:
			final=test_pinctrl_re_eint_api(dev,sunxi_pinctrl_test);
			break;
		default:
			pr_warn(" your input number should less than case number.\n");
			final=1;
			break;			
	}

	sunxi_pinctrl_test->exec = exec_number;
	if (final)
		sunxi_pinctrl_result[exec_number].result=CASE_TEST_FAILED;
	else{
		
		if (sunxi_pinctrl_result[exec_number].result==CASE_HAVE_NOT_TEST)
			sunxi_pinctrl_result[exec_number].result = CASE_TEST_SUCCESSED;
	}
	return size;
		
}


static ssize_t gpio_index_show(struct device *dev,struct device_attribute *attr,
							char *buf)
{
	/*chang gpio index to pin name */
	char pin_name[SUNXI_PIN_NAME_MAX_LEN];
	struct sunxi_pinctrl_test_class *sunxi_pinctrl_test=dev_get_drvdata(dev);
	sunxi_gpio_to_name(sunxi_pinctrl_test->gpio_index, pin_name);
	
	return sprintf(buf,"%s\n",pin_name);
}
static ssize_t gpio_index_store(struct device *dev,struct device_attribute *attr,
							const char *buf,size_t size)
{
	struct sunxi_pinctrl_test_class *sunxi_pinctrl_test = dev_get_drvdata(dev);

	char 			*after;
	int 			gpio_index= simple_strtoul(buf,&after,10);
	size_t			count=after-buf;
	ssize_t 		ret= -EINVAL;
	if (isspace(*after))
		count++;
	if(count==size){
		ret = count;
		sunxi_pinctrl_test->gpio_index	= gpio_index;
	}
	return ret;
}


static ssize_t funcs_show(struct device *dev,struct device_attribute *attr,
							char *buf)
{
	struct sunxi_pinctrl_test_class *sunxi_pinctrl_test=dev_get_drvdata(dev);
	return sprintf(buf,"%u\n",sunxi_pinctrl_test->funcs);
}
static ssize_t funcs_store(struct device *dev,struct device_attribute *attr,
							const char *buf,size_t size)
{
	struct sunxi_pinctrl_test_class *sunxi_pinctrl_test=dev_get_drvdata(dev);
	
	char 			*after;
	int 			funcs =simple_strtoul(buf,&after,10);
	size_t 			count =after-buf;
	ssize_t 		ret =-EINVAL;
	
	if (isspace(*after))
		count++;
	if(count==size){
		ret=count;
		sunxi_pinctrl_test->funcs=funcs;
	}
	return ret;
}


static ssize_t dat_show(struct device *dev,struct device_attribute *attr,
							char *buf)
{
	struct sunxi_pinctrl_test_class *sunxi_pinctrl_test=dev_get_drvdata(dev);
	return sprintf(buf,"%u\n",sunxi_pinctrl_test->dat);

}
static ssize_t dat_store(struct device *dev,struct device_attribute *attr,
							const char *buf,size_t size)
{
	struct sunxi_pinctrl_test_class *sunxi_pinctrl_test=dev_get_drvdata(dev);
	
	char 			*after;
	int 			dat =simple_strtoul(buf,&after,10);
	size_t 			count =after-buf;
	ssize_t 		ret =-EINVAL;
	
	if (isspace(*after))
		count++;
	if(count==size){
		ret=count;
		sunxi_pinctrl_test->dat=dat;
	}
	return ret;
}

static ssize_t dlevel_show(struct device *dev,struct device_attribute *attr,
							char *buf)
{
	struct sunxi_pinctrl_test_class *sunxi_pinctrl_test=dev_get_drvdata(dev);
	return sprintf(buf,"%u\n",sunxi_pinctrl_test->dlevel);
}
static ssize_t dlevel_store(struct device *dev,struct device_attribute *attr,
							const char *buf,size_t size)
{
	struct sunxi_pinctrl_test_class *sunxi_pinctrl_test=dev_get_drvdata(dev);
	
	char 			*after;
	int 			dlevel =simple_strtoul(buf,&after,10);
	size_t 			count =after-buf;
	ssize_t 		ret =-EINVAL;
	
	if (isspace(*after))
		count++;
	if(count==size){
		ret=count;
		sunxi_pinctrl_test->dlevel=dlevel;
	}
	return ret;
}


static ssize_t pul_show(struct device *dev,struct device_attribute *attr,
							char *buf)
{
	struct sunxi_pinctrl_test_class *sunxi_pinctrl_test=dev_get_drvdata(dev);
	return sprintf(buf,"%u\n",sunxi_pinctrl_test->pul);
}
static ssize_t pul_store(struct device *dev,struct device_attribute *attr,
							const char *buf,size_t size)
{
	struct sunxi_pinctrl_test_class *sunxi_pinctrl_test = dev_get_drvdata(dev);

	char 			*after;
	int 			pul	= simple_strtoul(buf,&after,10);
	size_t			count =after-buf;
	ssize_t 		ret = -EINVAL;

	if (isspace(*after))
		count++;
	if(count==size){
		ret = count;
		sunxi_pinctrl_test->pul	= pul;
	}
	return ret;
}

static ssize_t trigger_show(struct device *dev,struct device_attribute *attr,
							char *buf)
{
	struct sunxi_pinctrl_test_class *sunxi_pinctrl_test=dev_get_drvdata(dev);
	return sprintf(buf,"%u\n",sunxi_pinctrl_test->trigger);
}

static ssize_t trigger_store(struct device *dev,
		struct device_attribute *attr,const char *buf,size_t size)
{
	struct sunxi_pinctrl_test_class *sunxi_pinctrl_test = dev_get_drvdata(dev);

	char 			*after;
	int 			trigger	= simple_strtoul(buf,&after,10);
	size_t			count =after-buf;
	ssize_t 		ret = -EINVAL;

	if (isspace(*after))
		count++;
	if(count==size){
		ret = count;
		sunxi_pinctrl_test->trigger	= trigger;
	}
	return ret;
		
}

static ssize_t test_result_show(struct device *dev,struct device_attribute *attr,
							char *buf)
{
	return sprintf(buf,"%d\n",sunxi_pinctrl_result[test_case_number].result);
}


static ssize_t test_result_shore(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	return size;
}

static ssize_t dev_name_show(struct device *dev,struct device_attribute *attr,
							char *buf)
{
	struct sunxi_pinctrl_test_class *sunxi_pinctrl_test=dev_get_drvdata(dev);
	return sprintf(buf,"%s\n",sunxi_pinctrl_test->dev_name);
}

static ssize_t dev_name_shore(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int 		ret;
	struct sunxi_pinctrl_test_class *sunxi_pinctrl_test = dev_get_drvdata(dev);
	if(size>SUNXI_DEV_NAME_MAX_LEN){
		pr_warn("sunxi dev name max len less than 20 char.\n");
		return -EINVAL;
	}
	ret=strlcpy(sunxi_pinctrl_test->dev_name,buf, size);
	return ret;

		

}


static struct device_attribute sunxi_pinctrl_class_attrs[] = {
	__ATTR(exec, 0644, exec_show, exec_store),
	__ATTR(gpio_index, 0644, gpio_index_show, gpio_index_store),
	__ATTR(funcs, 0644, funcs_show, funcs_store),
	__ATTR(dat, 0644, dat_show, dat_store),
	__ATTR(dlevel, 0644, dlevel_show, dlevel_store),
	__ATTR(pul, 0644, pul_show, pul_store),
	__ATTR(trigger, 0644, trigger_show, trigger_store),
	__ATTR(test_result,0644,test_result_show,test_result_shore),
	__ATTR(dev_name,0644,dev_name_show,dev_name_shore),
	__ATTR_NULL,
};


static int sunxi_pinctrl_test_probe(struct platform_device *pdev)
{
	struct sunxi_pinctrl_test_class *sunxi_pinctrl_test;
	sunxi_pinctrl_test=devm_kzalloc(&pdev->dev,sizeof(*sunxi_pinctrl_test),GFP_KERNEL);
	if(!sunxi_pinctrl_test){
		dev_err(&pdev->dev,"No enougt memory for device \n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev,sunxi_pinctrl_test);
	
	sunxi_pinctrl_test_init_class=class_create(THIS_MODULE,"sunxi_pinctrl_test_init_class");
	if(IS_ERR(sunxi_pinctrl_test_init_class))
		return PTR_ERR(sunxi_pinctrl_test_init_class);
	sunxi_pinctrl_test_init_class->dev_attrs = sunxi_pinctrl_class_attrs;

	sunxi_pinctrl_test->dev=device_create(sunxi_pinctrl_test_init_class,&pdev->dev,0,
										sunxi_pinctrl_test,"sunxi_pinctrl_test");

	if(IS_ERR(sunxi_pinctrl_test->dev))
		return PTR_ERR(sunxi_pinctrl_test->dev);
	return 0;
	
}

static struct platform_driver sunxi_pinctrl_test_driver={
	.probe		=sunxi_pinctrl_test_probe,
	.driver		={
		.name		="sunxi_pinctrl_test",
		.owner		=THIS_MODULE,
	},
};
static struct platform_device sunxi_pinctrl_test_devices={
	.name		="sunxi_pinctrl_test",
	.id			=PLATFORM_DEVID_NONE,

};


static int __init sunxi_pinctrl_test_init(void)
{	
	int ret,i;
	for(i=0;i<SUNXI_PINCTRL_TEST_NUMBER;i++){
		sunxi_pinctrl_result[i].result=CASE_HAVE_NOT_TEST;
	}
	ret=platform_device_register(&sunxi_pinctrl_test_devices);
	if(IS_ERR_VALUE(ret)){
		pr_warn("register sunxi pinctrl test device failed\n");
		return -EINVAL;
	}
	ret=platform_driver_register(&sunxi_pinctrl_test_driver);
	if(IS_ERR_VALUE(ret)){
		pr_warn("register sunxi pinctrl test driver failed\n");
		return -EINVAL;
	}
	return ret;

}


static void __exit sunxi_pinctrl_test_exit(void)
{
	platform_device_unregister(&sunxi_pinctrl_test_devices);
	platform_driver_unregister(&sunxi_pinctrl_test_driver);
	class_destroy(sunxi_pinctrl_test_init_class);
}
module_init(sunxi_pinctrl_test_init);
module_exit(sunxi_pinctrl_test_exit);
MODULE_AUTHOR("Huang shaorui");
MODULE_LICENSE("GPL");


