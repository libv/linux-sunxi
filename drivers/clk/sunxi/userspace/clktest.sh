#!/bin/sh

index=0
print_dir()
{
	if [ -f $1/clk_rate ];then
		clk_val=`cat $1/clk_rate`
	else
		clk_val=
	fi
	if [ x$index = x0 ];then
	echo "|--`basename $1`[$clk_val]"
	fi
        if [ x$index = x1 ];then
        echo "|----`basename $1`[$clk_val]"
        fi
        if [ x$index = x2 ];then
        echo "|------`basename $1`[$clk_val]"
        fi
        if [ x$index = x3 ];then
        echo "|--------`basename $1`[$clk_val]"
        fi
        if [ x$index = x4 ];then
        echo "|----------`basename $1`[$clk_val]"
        fi
        if [ x$index = x5 ];then
        echo "|------------`basename $1`[$clk_val]"
        fi
        if [ x$index = x6 ];then
        echo "|--------------`basename $1`[$clk_val]"
        fi
        if [ x$index = x7 ];then
        echo "|----------------`basename $1`[$clk_val]"
        fi
        if [ x$index = x8 ];then
        echo "|------------------`basename $1`[$clk_val]"
        fi
}
show_dir()
{
	local predir
	if [ ! "x$1" = "x." ];then
	print_dir $1
	predir=$PWD
	cd $1
	index=`expr $index + 1`
	for dir in `find . ! -name "." -type d -prune -o -type d`
	do
        	show_dir $dir
	done
	cd $predir
	index=`expr $index - 1`
	fi
}
if ! mount|grep debugfs >/dev/null;then
mount -t debugfs none /sys/kernel/debug
fi
if [ x$1 = xshow ];then
	cd /sys/kernel/debug/clk/
	for dir in `find . ! -name "." -type d -prune -o -type d`
	do 
		show_dir $dir
	done 
	cd -
else
	if [ x$1 = xgetparents ];then
		echo $1 >/sys/kernel/debug/ccudbg/command
		echo $2 >/sys/kernel/debug/ccudbg/name
		echo 1 >/sys/kernel/debug/ccudbg/start
		echo `cat /sys/kernel/debug/ccudbg/info`
	elif [ x$1 = xgetparent ];then
		echo $1 >/sys/kernel/debug/ccudbg/command
		echo $2 >/sys/kernel/debug/ccudbg/name
		echo 1 >/sys/kernel/debug/ccudbg/start
		echo `cat /sys/kernel/debug/ccudbg/info`
	elif [ x$1 = xsetparent ];then
		echo $1 >/sys/kernel/debug/ccudbg/command
		echo $2 >/sys/kernel/debug/ccudbg/name
		echo $3 >/sys/kernel/debug/ccudbg/param	
		echo 1 >/sys/kernel/debug/ccudbg/start
		echo `cat /sys/kernel/debug/ccudbg/info`
	elif [ x$1 = xgetrate ];then
		echo $1 >/sys/kernel/debug/ccudbg/command
		echo $2 >/sys/kernel/debug/ccudbg/name
		echo 1 >/sys/kernel/debug/ccudbg/start
		echo `cat /sys/kernel/debug/ccudbg/info`
	elif [ x$1 = xsetrate ];then
		echo $1 >/sys/kernel/debug/ccudbg/command
		echo $2 >/sys/kernel/debug/ccudbg/name
		echo $3 >/sys/kernel/debug/ccudbg/param	
		echo 1 >/sys/kernel/debug/ccudbg/start
		echo `cat /sys/kernel/debug/ccudbg/info`
	elif [ x$1 = xis_enabled ];then
		echo $1 >/sys/kernel/debug/ccudbg/command
		echo $2 >/sys/kernel/debug/ccudbg/name
		echo 1 >/sys/kernel/debug/ccudbg/start
		echo `cat /sys/kernel/debug/ccudbg/info`
	elif [ x$1 = xdisable ];then
		echo $1 >/sys/kernel/debug/ccudbg/command
		echo $2 >/sys/kernel/debug/ccudbg/name
		echo 1 >/sys/kernel/debug/ccudbg/start
		echo `cat /sys/kernel/debug/ccudbg/info`		
	elif [ x$1 = xenable ];then
		echo $1 >/sys/kernel/debug/ccudbg/command
		echo $2 >/sys/kernel/debug/ccudbg/name
		echo 1 >/sys/kernel/debug/ccudbg/start
		echo `cat /sys/kernel/debug/ccudbg/info`								
	elif [ x$1 = xdumpreg ];then
		echo $1 >/sys/kernel/debug/ccudbg/command
		echo null >/sys/kernel/debug/ccudbg/name
		echo 1 >/sys/kernel/debug/ccudbg/start
		echo `cat /sys/kernel/debug/ccudbg/info`	
	else
		echo "support command: show getparents getparent setparent getrate setrate is_enabled enable disable dumpreg"
	fi
fi

