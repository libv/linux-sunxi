################################################################################
# Author: huangshaorui
# Date  : 2013-06-10
#
# Goals: test sunxi pinctrl pin configure setting api
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
#test PA05
	echo 6 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/gpio_index
	echo 1 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/dat
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/dlevel
	echo 1 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/pul
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/funcs
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/exec
	
#test PB02
	echo 34 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/gpio_index
	echo 0 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/dat
	echo 1 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/dlevel
	echo 1 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/pul
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/funcs
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/exec
	
#test PG07
	echo 199 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/gpio_index
	echo 0 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/dat
	echo 1 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/dlevel
	echo 1 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/pul
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/funcs
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/exec

fi

if [ ${TEST_HOST_NAME} == "sun9i" ]; then
#test PA05
	echo 6 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/gpio_index
	echo 1 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/dat
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/dlevel
	echo 1 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/pul
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/funcs
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/exec
#test PG15
	echo 207 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/gpio_index
	echo 1 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/dat
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/dlevel
	echo 1 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/pul
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/funcs
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/exec
fi

if [ ${TEST_HOST_NAME} == "SUNXI_MACHINE" ]; then
#test PA05
	echo 6 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/gpio_index
	echo 1 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/dat
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/dlevel
	echo 1 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/pul
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/funcs
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/exec
	
#test PB02
	echo 34 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/gpio_index
	echo 0 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/dat
	echo 1 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/dlevel
	echo 1 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/pul
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/funcs
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/exec
	
#test PG07
	echo 199 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/gpio_index
	echo 0 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/dat
	echo 1 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/dlevel
	echo 1 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/pul
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/funcs
	echo 2 > /sys/class/sunxi_pinctrl_test_init_class/sunxi_pinctrl_test/exec
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

