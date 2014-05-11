obj-y += axp-mfd.o
obj-y += axp-rw.o
obj-y += axp-script.o

#Make this built-in, so that it will be loaded before I2C driver
#AXP22x
obj-$(CONFIG_AW_AXP22) += axp22-board.o
obj-$(CONFIG_AW_AXP22) += axp22-regu.o
obj-$(CONFIG_AW_AXP22) += axp22-sply.o
obj-$(CONFIG_AW_AXP22) += virtual22.o
obj-$(CONFIG_AW_AXP22) += virtual22_dev.o
#AXP15x
obj-$(CONFIG_AW_AXP15) += axp15-board.o
obj-$(CONFIG_AW_AXP15) += axp15-regu.o
obj-$(CONFIG_AW_AXP15) += virtual15.o
obj-$(CONFIG_AW_AXP15) += virtual15_dev.o
