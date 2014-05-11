/*
 * header file
 * get platform config info
 * Author: raymonxiu
 * 
 */

#ifndef __PLATFORM_CFG__H__
#define __PLATFORM_CFG__H__

#if defined CONFIG_FPGA_V4_PLATFORM
#define FPGA_VER
#elif defined CONFIG_FPGA_V7_PLATFORM
#define FPGA_VER
#endif

//#define FPGA_VER

#define VFE_CLK 
#define VFE_GPIO
//#define VFE_PMU
#define VFE_SYS_CONFIG
#define SUNXI_MEM
#ifdef FPGA_VER
#define FPGA_PIN
#else
#define VFE_PMU
#endif

#include <mach/irqs.h>
#include <mach/platform.h>
#include <linux/gpio.h>

#ifdef VFE_CLK 
//#include <mach/clock.h>
#include <linux/clk.h>
#include <linux/clk/sunxi_name.h>
#endif

#ifdef VFE_GPIO
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/pinconf-sunxi.h>
#include <mach/gpio.h>
#endif

#ifdef VFE_PMU
#include <linux/regulator/consumer.h>
#endif

#ifdef VFE_SYS_CONFIG
#include <mach/sys_config.h>
#endif


#ifdef FPGA_VER
#define DPHY_CLK (48*1000*1000)
#else
#define DPHY_CLK (150*1000*1000)
#endif

#define VFE_CORE_CLK_RATE (300*1000*1000)


#if defined CONFIG_ARCH_SUN8IW1P1
#define CSI0_REGS_BASE          	0x01cb0000
#define MIPI_CSI0_REGS_BASE      	0x01cb1000
#define MIPI_DPHY0_REGS_BASE     	0x01cb2000
#define CSI1_REGS_BASE          	0x01cb3000
#define MIPI_CSI1_REGS_BASE      	0x01cb4000
#define MIPI_DPHY1_REGS_BASE     	0x01cb5000
#define ISP_REGS_BASE           	0x01cb8000
#define CPU_DRAM_PADDR_ORG 				0x40000000
#define HW_DMA_OFFSET							0x00000000
#define MAX_VFE_INPUT   					2     //the maximum number of input source of video front end
#define VFE_CORE_CLK							CSI0_S_CLK
#define VFE_CORE_CLK_SRC					PLL10_CLK
#define VFE_MASTER_CLK0						CSI0_M_CLK
#define VFE_MASTER_CLK1						CSI1_M_CLK
#define VFE_MASTER_CLK_24M_SRC		HOSC_CLK
#define VFE_MASTER_CLK_PLL_SRC		PLL7_CLK
#define VFE_MIPI_DPHY_CLK					MIPICSI_CLK
#define VFE_MIPI_DPHY_CLK_SRC			PLL7_CLK
#define VFE_MIPI_CSI_CLK
#define VFE_MIPI_CSI_CLK_SRC	
#elif defined CONFIG_ARCH_SUN8IW3P1
#define CSI0_REGS_BASE          	0x01cb0000
#define ISP_REGS_BASE           	0x01cb8000
#define CPU_DRAM_PADDR_ORG 				0x40000000
#define HW_DMA_OFFSET							0x00000000
#define MAX_VFE_INPUT   					1     //the maximum number of input source of video front end
#define VFE_CORE_CLK							CSI_S_CLK
#define VFE_CORE_CLK_SRC					PLL3_CLK
#define VFE_MASTER_CLK0						CSI_M_CLK
#define VFE_MASTER_CLK1						
#define VFE_MASTER_CLK_24M_SRC		HOSC_CLK
#define VFE_MASTER_CLK_PLL_SRC		PLL3_CLK
#define VFE_MIPI_DPHY_CLK					
#define VFE_MIPI_DPHY_CLK_SRC			
#define VFE_MIPI_CSI_CLK
#define VFE_MIPI_CSI_CLK_SRC	
#elif defined CONFIG_ARCH_SUN9IW1P1
#define CSI0_REGS_BASE          	0x03800000
#define MIPI_CSI0_REGS_BASE      	0x03801000
#define MIPI_DPHY0_REGS_BASE     	0x03802000
#define CSI1_REGS_BASE          	0x03900000
#define MIPI_CSI1_REGS_BASE      	0x03901000
#define MIPI_DPHY1_REGS_BASE     	0x03902000
#define ISP_REGS_BASE           	0x03808000
#define CPU_DRAM_PADDR_ORG 				0x20000000
#define HW_DMA_OFFSET							0x20000000
#define MAX_VFE_INPUT   					2     //the maximum number of input source of video front end
#define VFE_CORE_CLK							CSI_ISP_CLK
#define VFE_CORE_CLK_SRC					CSI_ISP_CLK
#define VFE_MASTER_CLK0						CSI0_MCLK_CLK
#define VFE_MASTER_CLK1						CSI1_MCLK_CLK
#define VFE_MASTER_CLK_24M_SRC		HOSC_CLK
#define VFE_MASTER_CLK_PLL_SRC		CSI_ISP_CLK
#define VFE_MIPI_DPHY_CLK					MIPI_CSI_CLK
#define VFE_MIPI_DPHY_CLK_SRC			
#define VFE_MIPI_CSI_CLK
#define VFE_MIPI_CSI_CLK_SRC	
#endif
   
#define CSI0_REG_SIZE               0x1000
#define MIPI_CSI_REG_SIZE           0x1000
#define MIPI_DPHY_REG_SIZE          0x1000
#define CSI1_REG_SIZE               0x1000
#define ISP_REG_SIZE                0x1000
#define ISP_LOAD_REG_SIZE           0x1000
#define ISP_SAVED_REG_SIZE          0x1000

#define ISP_LUT_LENS_GAMMA_MEM_SIZE 0x1000
#define ISP_LUT_MEM_SIZE            0x0400
#define ISP_LENS_MEM_SIZE           0x0600
#define ISP_GAMMA_MEM_SIZE          0x0200
#define ISP_DRC_MEM_SIZE            0x0200

#define ISP_LUT_MEM_OFS             0x0
#define ISP_LENS_MEM_OFS            (ISP_LUT_MEM_OFS + ISP_LUT_MEM_SIZE)
#define ISP_GAMMA_MEM_OFS           (ISP_LENS_MEM_OFS + ISP_LENS_MEM_SIZE)


#if defined CONFIG_ARCH_SUN9IW1P1
#define ISP_STAT_TOTAL_SIZE         0x1500

#define ISP_STAT_HIST_MEM_SIZE      0x0200
#define ISP_STAT_AE_MEM_SIZE        0x0600
#define ISP_STAT_AWB_MEM_SIZE       0x0200
#define ISP_STAT_AF_MEM_SIZE        0x0500
#define ISP_STAT_AFS_MEM_SIZE       0x0200
#define ISP_STAT_AWB_WIN_MEM_SIZE   0x0400

#define ISP_STAT_HIST_MEM_OFS       0x0
#define ISP_STAT_AE_MEM_OFS         (ISP_STAT_HIST_MEM_OFS + ISP_STAT_HIST_MEM_SIZE)
#define ISP_STAT_AWB_MEM_OFS        (ISP_STAT_AE_MEM_OFS   + ISP_STAT_AE_MEM_SIZE)
#define ISP_STAT_AF_MEM_OFS         (ISP_STAT_AWB_MEM_OFS  + ISP_STAT_AWB_MEM_SIZE)
#define ISP_STAT_AFS_MEM_OFS        (ISP_STAT_AF_MEM_OFS   + ISP_STAT_AF_MEM_SIZE)
#define ISP_STAT_AWB_WIN_MEM_OFS    (ISP_STAT_AFS_MEM_OFS   + ISP_STAT_AFS_MEM_SIZE)

#else
#define ISP_STAT_TOTAL_SIZE         0x1700

#define ISP_STAT_HIST_MEM_SIZE      0x0200
#define ISP_STAT_AE_MEM_SIZE        0x0c00
#define ISP_STAT_AWB_MEM_SIZE       0x0500
#define ISP_STAT_AF_MEM_SIZE        0x0200
#define ISP_STAT_AFS_MEM_SIZE       0x0200
#define ISP_STAT_AWB_WIN_MEM_SIZE   0x0000

#define ISP_STAT_HIST_MEM_OFS       0x0
#define ISP_STAT_AE_MEM_OFS         (ISP_STAT_HIST_MEM_OFS + ISP_STAT_HIST_MEM_SIZE)
#define ISP_STAT_AWB_MEM_OFS        (ISP_STAT_AE_MEM_OFS   + ISP_STAT_AE_MEM_SIZE)
#define ISP_STAT_AF_MEM_OFS         (ISP_STAT_AWB_MEM_OFS  + ISP_STAT_AWB_MEM_SIZE)
#define ISP_STAT_AFS_MEM_OFS        (ISP_STAT_AF_MEM_OFS   + ISP_STAT_AF_MEM_SIZE)
#define ISP_STAT_AWB_WIN_MEM_OFS    (ISP_STAT_AFS_MEM_OFS   + ISP_STAT_AFS_MEM_SIZE)
#endif


#define MAX_CH_NUM      4
#define MAX_INPUT_NUM   2     //the maximum number of device connected to the same bus
#define MAX_ISP_STAT_BUF  5   //the maximum number of isp statistic buffer

#define MAX_AF_WIN_NUM 1
#define MAX_AE_WIN_NUM 1

#endif //__PLATFORM_CFG__H__