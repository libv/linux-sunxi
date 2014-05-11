#!bin/bash
chmod 777 sunxi_pinctrl_test.ko
insmod ./sunxi_pinctrl_test.ko

	echo twi1 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/dev_name
	echo 0 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/exec

rmmod sunxi_pinctrl_test.ko

exit 0

