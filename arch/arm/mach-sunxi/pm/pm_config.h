#ifndef _PM_CONFIG_H
#define _PM_CONFIG_H


/*
* Copyright (c) 2011-2015 yanggq.young@allwinnertech.com
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License version 2 as published by
* the Free Software Foundation.
*/

#include "pm_def_i.h"
#include "mach/platform.h"
#include "mach/memory.h"
#include "asm-generic/sizes.h"
//#include <generated/autoconf.h>
#include "mach/irqs.h"

#ifdef CONFIG_ARCH_SUN4I 
	#define PERMANENT_REG 		(0xf1c20d20)
	#define PERMANENT_REG_PA 	(0x01c20d20)
	#define STATUS_REG 		(0xf1c20d20)
	#define STATUS_REG_PA 		(0x01c20d20)
#elif defined(CONFIG_ARCH_SUN5I)
	#define PERMANENT_REG 		(0xF1c0123c)
	#define PERMANENT_REG_PA 	(0x01c0123c)
	#define STATUS_REG 		(0xf0000740)
	#define STATUS_REG_PA 		(0x00000740)
	//notice: the address is located in the last word of (DRAM_BACKUP_BASE_ADDR + DRAM_BACKUP_SIZE)
	#define SUN5I_STATUS_REG 	(DRAM_BACKUP_BASE_ADDR + (DRAM_BACKUP_SIZE<<2) -0x4)
	#define SUN5I_STATUS_REG_PA 	(DRAM_BACKUP_BASE_ADDR_PA + (DRAM_BACKUP_SIZE<<2) -0x4)
#elif defined(CONFIG_ARCH_SUN8IW1P1)
	#define STATUS_REG 	(0xf1f00100)
	#define STATUS_REG_PA 	(0x01f00100)
	#define STATUS_REG_NUM 	(6)
#elif defined(CONFIG_ARCH_SUN8IW3P1)
	#define STATUS_REG 	(0xf1f00100)
	#define STATUS_REG_PA 	(0x01f00100)
	#define STATUS_REG_NUM 	(4)
#elif defined(CONFIG_ARCH_SUN9IW1P1)
	#define STATUS_REG	(0xf1f00100)
	#define STATUS_REG_PA	(0x01f00100)
	#define STATUS_REG_NUM	(4)	
#endif

#ifdef CONFIG_ARCH_SUN4I
#define INT_REG_LENGTH		((0x90+0x4)>>2)
#define GPIO_REG_LENGTH		((0x218+0x4)>>2)
#define SRAM_REG_LENGTH		((0x94+0x4)>>2)
#elif defined CONFIG_ARCH_SUN5I
#define INT_REG_LENGTH		((0x94+0x4)>>2)
#define GPIO_REG_LENGTH		((0x218+0x4)>>2)
#define SRAM_REG_LENGTH		((0x94+0x4)>>2)
#elif defined CONFIG_ARCH_SUN8IW1P1
#define GPIO_REG_LENGTH		((0x278+0x4)>>2)
#define CPUS_GPIO_REG_LENGTH	((0x238+0x4)>>2)
#define SRAM_REG_LENGTH		((0x94+0x4)>>2)
#define CCU_REG_LENGTH		((0x308+0x4)>>2)
#elif defined CONFIG_ARCH_SUN8IW3P1
#define GPIO_REG_LENGTH		((0x258+0x4)>>2)
#define CPUS_GPIO_REG_LENGTH	((0x218+0x4)>>2)
#define SRAM_REG_LENGTH		((0x94+0x4)>>2)
#define CCU_REG_LENGTH		((0x2d8+0x4)>>2)
#elif defined CONFIG_ARCH_SUN9IW1P1
#define GPIO_REG_LENGTH		((0x324+0x4)>>2)
#define CPUS_GPIO_REG_LENGTH	((0x304+0x4)>>2)
#define SRAM_REG_LENGTH		((0xF0+0x4)>>2)
#define CCU_REG_LENGTH		((0x184+0x4)>>2)
#define CCU_MOD_REG_LENGTH	((0x1B4+0x4)>>2)
#endif

#ifdef CONFIG_ARCH_SUN8I
#define CORTEX_A7
#endif

//#define CHECK_IC_VERSION

//#define RETURN_FROM_RESUME0_WITH_MMU    //suspend: 0xf000, resume0: 0xc010, resume1: 0xf000
//#define RETURN_FROM_RESUME0_WITH_NOMMU // suspend: 0x0000, resume0: 0x4010, resume1: 0x0000
//#define DIRECT_RETURN_FROM_SUSPEND //not support yet
#define ENTER_SUPER_STANDBY    //suspend: 0xf000, resume0: 0x4010, resume1: 0x0000
//#define ENTER_SUPER_STANDBY_WITH_NOMMU //not support yet, suspend: 0x0000, resume0: 0x4010, resume1: 0x0000
//#define WATCH_DOG_RESET

//NOTICE: only need one definiton
#define RESUME_FROM_RESUME1

/**start address for function run in sram*/
#define SRAM_FUNC_START    	(0xf0000000)
#define SRAM_FUNC_START_PA 	(0x00000000)

#define DRAM_BASE_ADDR      	(0xc0000000)
#define DRAM_BASE_ADDR_PA	(0x40000000)
#define DRAM_TRANING_SIZE   	(16)

#define DRAM_MEM_PARA_INFO_PA			(SUPER_STANDBY_MEM_BASE)	//0x43010000, 0x43010000+2k;
#define DRAM_MEM_PARA_INFO_SIZE			((SUPER_STANDBY_MEM_SIZE)>>1) 	//DRAM_MEM_PARA_INFO_SIZE = 1K bytes. 

#define DRAM_EXTENDED_STANDBY_INFO_PA		(DRAM_MEM_PARA_INFO_PA + DRAM_MEM_PARA_INFO_SIZE)
#define DRAM_EXTENDED_STANDBY_INFO_SIZE		((SUPER_STANDBY_MEM_SIZE)>>1)

#define RUNTIME_CONTEXT_SIZE 		(14 * sizeof(__u32)) //r0-r13

#define DRAM_COMPARE_DATA_ADDR 		(0xc0100000) //1Mbytes offset
#define DRAM_COMPARE_SIZE 		(0x10000) //?

//for mem mapping
#define MEM_SW_VA_SRAM_BASE (0x00000000)
#define MEM_SW_PA_SRAM_BASE (0x00000000)

#define __AC(X,Y)		(X##Y)
#define _AC(X,Y)		__AC(X,Y)
#define _AT(T,X)		((T)(X))
#define UL(x) 			_AC(x, UL)

#define SUSPEND_FREQ 		(720000)	//720M
#define SUSPEND_DELAY_MS 	(10)

#define AW_SRAM_A1_BASE		(SUNXI_SRAM_A1_PBASE)
#define AW_SRAM_A2_BASE		(SUNXI_SRAM_A2_PBASE)
#define AW_MSGBOX_BASE		(SUNXI_MSGBOX_PBASE)
#define AW_SPINLOCK_BASE	(SUNXI_SPINLOCK_PBASE)
#define AW_PIO_BASE		(SUNXI_PIO_PBASE)
#define AW_R_PRCM_BASE		(SUNXI_R_PRCM_PBASE)
#define AW_R_CPUCFG_BASE	(SUNXI_R_CPUCFG_PBASE)
#define AW_UART0_BASE		(SUNXI_UART0_PBASE)
#define AW_R_PIO_BASE		(SUNXI_R_PIO_PBASE)

#ifdef CONFIG_ARCH_SUN8IW1P1
#define AW_LRADC01_BASE		(SUNXI_LRADC01_PBASE)
#define AW_CCM_BASE		(SUNXI_CCM_PBASE)
#elif defined CONFIG_ARCH_SUN8IW3P1
#define AW_LRADC01_BASE		(SUNXI_LRADC_PBASE)
#define AW_CCM_BASE		(SUNXI_CCM_PBASE)
#elif defined CONFIG_ARCH_SUN9IW1P1
#define AW_LRADC01_BASE		(SUNXI_LRADC01_PBASE)
#define AW_CCM_BASE		(SUNXI_CCM_PLL_PBASE)
#define AW_CCM_MOD_BASE		(SUNXI_CCM_MOD_PBASE)
#endif

#ifdef CONFIG_ARCH_SUN8IW1P1
#define AW_CCU_UART_PA		(0x01c2006C)
#define AW_CCU_UART_RESET_PA	(0x01c202D8)
#define AW_JTAG_GPIO_PA		(0x01c20800 + 0x100)
#define AW_JTAG_CONFIG_VAL	(0x00033330)
#define AW_UART_GPIO_PA		(0x01c20800 + 0x104)		//UART0:sun8iw1
#define AW_RTC_BASE		(SUNXI_RTC_PBASE)
#define AW_SRAMCTRL_BASE	(SUNXI_SRAMCTRL_PBASE)
#elif defined CONFIG_ARCH_SUN8IW3P1
#define AW_CCU_UART_PA		(0x01c2006C)
#define AW_CCU_UART_RESET_PA	(0x01c202D8)
#define AW_JTAG_GPIO_PA		(0x01c20800 + 0xB4)		//jtag1:pf, 0xb4
#define AW_JTAG_CONFIG_VAL	(0x00303033)
#define AW_UART_GPIO_PA		(0x01c20800 + 0xB4)		//UART0:sun8iw3
#define AW_RTC_BASE		(SUNXI_RTC_PBASE)
#define AW_SRAMCTRL_BASE	(SUNXI_SRAMCTRL_PBASE)
#elif defined CONFIG_ARCH_SUN9IW1P1
#define AW_CCU_UART_PA		(0x06000400 + 0x194)		//bit16: uart0;
#define AW_CCU_UART_RESET_PA	(0x06000400 + 0x1b4)		//bit16: uart0;
//Notice: jtag&uart_ph use the same addr, on sun9i platform.
#define AW_JTAG_GPIO_PA		(0x06000800 + 0x100)		//uart0: PH12, PH13
#define AW_UART_GPIO_PA		(0x06000800 + 0x100)		//jtag0: PH8-PH11
#define AW_UART_PF_GPIO_PA	(0x06000800 + 0xB4)		//uart0: PF2,PF4
#define AW_RTC_BASE		(AW_SRAM_A1_BASE)		//notice: fake addr.
#define AW_SRAMCTRL_BASE	(SUNXI_SYS_CTRL_PBASE)
#endif

//interrupt src definition.
#define AW_IRQ_TIMER0		(SUNXI_IRQ_TIMER0	)
#define AW_IRQ_MBOX		(SUNXI_IRQ_MBOX         )

#if defined(CONFIG_ARCH_SUN9IW1P1) && defined(CONFIG_FPGA_V4_PLATFORM)
#define AW_IRQ_TIMER1		(0)
#define AW_IRQ_TOUCHPANEL	(0)
#define AW_IRQ_LRADC		(0)
#define AW_IRQ_NMI			(0)
#elif defined(CONFIG_ARCH_SUN8IW1P1) || defined(CONFIG_ARCH_SUN9IW1P1)
#define AW_IRQ_TIMER1		(SUNXI_IRQ_TIMER1	)
#define AW_IRQ_TOUCHPANEL	(SUNXI_IRQ_TOUCHPANEL   )
#define AW_IRQ_LRADC		(SUNXI_IRQ_LRADC        )
#define AW_IRQ_NMI			(SUNXI_IRQ_NMI          )
#elif defined(CONFIG_ARCH_SUN8IW3P1)
#define AW_IRQ_TIMER1		(SUNXI_IRQ_TIMER1	)
#define AW_IRQ_TOUCHPANEL	(0)
#define AW_IRQ_LRADC		(SUNXI_IRQ_LRADC        )
#define AW_IRQ_NMI			(SUNXI_IRQ_NMI          )
#endif

#define AW_TWI0_BASE		(SUNXI_TWI0_PBASE)
#define AW_TWI1_BASE		(SUNXI_TWI1_PBASE)
#define AW_TWI2_BASE		(SUNXI_TWI2_PBASE)
#define AW_CPUCFG_P_REG0	(SUNXI_CPUCFG_P_REG0)
#define AW_CPUCFG_GENCTL	(SUNXI_CPUCFG_GENCTL)
#define AW_CPUX_PWR_CLAMP(x)	(SUNXI_CPUX_PWR_CLAMP(x))
#define AW_CPUX_PWR_CLAMP_STATUS(x)	(SUNXI_CPUX_PWR_CLAMP_STATUS(x))
#define AW_CPU_PWROFF_REG	(SUNXI_CPU_PWROFF_REG)

#define SRAM_CTRL_REG1_ADDR_PA 0x01c00004
#define SRAM_CTRL_REG1_ADDR_VA IO_ADDRESS(SRAM_CTRL_REG1_ADDR_PA)

/**-----------------------------stack point address in sram-----------------------------------------*/
#define SP_IN_SRAM	0xf0003ffc //16k
#define SP_IN_SRAM_PA   0x00003ffc //16k

#endif /*_PM_CONFIG_H*/
