################################################################################
# Author: huangshaorui
# Date  : 2013-06-10
#
# Goals: test sunxi pinctrl repeat request gpio resource api.
# usage:
#      
# parameter:
#
# example:
#     
#
################################################################################

#===============================================================================
#  Variables setting
#===============================================================================
#!bin/bash
THIS_MODULE=pinctrl
TOP_COMMON=${TESH_PATH}/common
THIS_CASE=`basename $0 | sed 's/\.sh//'`
RESULT=0
THIS_RESULT="pass"
insmod ${TEST_MODULES_DIR}/sunxi_pinctrl_test.ko

#===============================================================================
# start test
#===============================================================================
cd ../common/
sh start_test.sh $THIS_MODULE $THIS_CASE
cd ../pinctrl

#
# test host environment
#
if [ ${TEST_HOST_NAME} == "sun8i" ]; then
#test PA04
	echo 5 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/gpio_index
	echo 4 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/exec
#test PL01
	echo 353 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/gpio_index
	echo 4 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/exec

fi

if [ ${TEST_HOST_NAME} == "sun9i" ]; then
#test PA04
	echo 5 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/gpio_index
	echo 4 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/exec
#test PM01
	echo 385 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/gpio_index
	echo 4 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/exec
fi

if [ ${TEST_HOST_NAME} == "SUNXI_MACHINE" ]; then
#test PA04
	echo 5 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/gpio_index
	echo 4 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/exec
#test PL01
	echo 353 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/gpio_index
	echo 4 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/exec
fi

#
#get result
#
RESULT=`cat /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/test_result`
if [ ${RESULT} != 0 ]; then
	THIS_RESULT="fail"
fi


#===============================================================================
# end test
#===============================================================================
cd ../common/
sh end_test.sh $THIS_MODULE $THIS_CASE $THIS_RESULT
cd ../pinctrl/
rmmod sunxi_pinctrl_test.ko

