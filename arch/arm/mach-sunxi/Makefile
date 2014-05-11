ifneq ($(strip $(CONFIG_ARCH_SUN4I) $(CONFIG_ARCH_SUN5I)),)
obj-y += sun4i.o
endif

ifneq ($(strip $(CONFIG_ARCH_SUN6I) $(CONFIG_ARCH_SUN7I)),)
obj-y += sun6i.o
endif

ifneq ($(strip $(CONFIG_ARCH_SUN8I)),)
obj-y += sun8i.o
obj-$(CONFIG_SW_POWERNOW) += powernow.o
obj-$(CONFIG_PM) += pm/
obj-$(CONFIG_NET) += rf/
endif

ifneq ($(strip $(CONFIG_ARCH_SUN9I)),)
obj-y += sun9i.o
obj-$(CONFIG_PM) += pm/
endif

ifeq ($(CONFIG_SMP), y)
obj-y += sunxi-headsmp.o sunxi-platsmp.o
obj-$(CONFIG_HOTPLUG_CPU) += sunxi-hotplug.o
endif

obj-y += sys_config.o sunxi_dump_reg.o
