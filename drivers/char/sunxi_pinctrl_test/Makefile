#
# Makefile for pinctrl sunxi  test driver
#
PWD=$(shell pwd)

obj-$(CONFIG_SUNXI_PINCTRL_TEST)+= sunxi_pinctrl_test.o

install: build
	cp sunxi_pinctrl_test.ko $(LICHEE_MOD_DIR)/

build:
	@echo $(LICHEE_KDIR)
	$(MAKE) -C $(LICHEE_KDIR) M=$(PWD)

clean:
	@rm -rf *.o *.ko .*.cmd *.mod.c *.order *.symvers .tmp_versions *~
