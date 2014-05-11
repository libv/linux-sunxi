#include "pm_i.h"

//#define CHECK_RESTORE_STATUS
static __ccmu_reg_list_t   *CmuReg;


/*
*********************************************************************************************************
*                           mem_clk_init
*
*Description: ccu init for platform mem
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__ccmu_reg_list_t * mem_clk_init(__u32 mmu_flag )
{
	if(1 == mmu_flag){
		CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	}else{
		CmuReg = (__ccmu_reg_list_t *)(AW_CCM_BASE);
	}

	return CmuReg;
}

#ifdef CONFIG_ARCH_SUN8I
/*
*********************************************************************************************************
*                           mem_clk_save
*
*Description: save ccu for platform mem
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 mem_clk_save(struct clk_state *pclk_state)
{
	pclk_state->CmuReg = CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);

	/*backup clk src and ldo*/
	pclk_state->ccu_reg_back[0] = *(volatile __u32 *)&CmuReg->SysClkDiv;
	pclk_state->ccu_reg_back[1] = *(volatile __u32 *)&CmuReg->Apb2Div;
	pclk_state->ccu_reg_back[2] = *(volatile __u32 *)&CmuReg->Ahb1Div;
	
	/*backup axi, ahb, apb gating*/
	pclk_state->ccu_reg_back[3] = *(volatile __u32 *)&CmuReg->AhbGate0;
	pclk_state->ccu_reg_back[4] = *(volatile __u32 *)&CmuReg->AhbGate1;
	pclk_state->ccu_reg_back[5] = *(volatile __u32 *)&CmuReg->Apb1Gate;
	pclk_state->ccu_reg_back[6] = *(volatile __u32 *)&CmuReg->Apb2Gate;	
	
	/* backup pll registers and tuning?*/
	pclk_state->ccu_reg_back[7] = *(volatile __u32 *)&CmuReg->Pll2Ctl;
	pclk_state->ccu_reg_back[8] = *(volatile __u32 *)&CmuReg->Pll3Ctl;
	pclk_state->ccu_reg_back[9] = *(volatile __u32 *)&CmuReg->Pll4Ctl;
	pclk_state->ccu_reg_back[10] = *(volatile __u32 *)&CmuReg->Pll6Ctl;


	
	return 0;
}


/*
*********************************************************************************************************
*                           mem_clk_exit
*
*Description: restore ccu for platform mem
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 mem_clk_restore(struct clk_state *pclk_state)
{
	/* initialise the CCU io base */
	CmuReg = pclk_state->CmuReg;


	/*restore clk src and ldo*/
	*(volatile __u32 *)&CmuReg->SysClkDiv   = pclk_state->ccu_reg_back[0];  
	*(volatile __u32 *)&CmuReg->Apb2Div     = pclk_state->ccu_reg_back[1];  
	*(volatile __u32 *)&CmuReg->Ahb1Div     = pclk_state->ccu_reg_back[2];

	/*restore axi, ahb, apb gating*/	
	*(volatile __u32 *)&CmuReg->AhbGate0   = pclk_state->ccu_reg_back[3]; 
	*(volatile __u32 *)&CmuReg->AhbGate1   = pclk_state->ccu_reg_back[4]; 
	*(volatile __u32 *)&CmuReg->Apb1Gate   = pclk_state->ccu_reg_back[5]; 
	*(volatile __u32 *)&CmuReg->Apb2Gate   = pclk_state->ccu_reg_back[6]; 

	/* restore pll registers and tuning? latency?*/
	//notice: do not touch pll1 and pll5
	*(volatile __u32 *)&CmuReg->Pll2Ctl    = pclk_state->ccu_reg_back[7];
	*(volatile __u32 *)&CmuReg->Pll3Ctl    = pclk_state->ccu_reg_back[8]; 
	*(volatile __u32 *)&CmuReg->Pll4Ctl    = pclk_state->ccu_reg_back[9]; 
	*(volatile __u32 *)&CmuReg->Pll6Ctl    = pclk_state->ccu_reg_back[10]; 
	 
	/* config the CCU to default status */
	//needed?

	return 0;
}


/*
*********************************************************************************************************
*                                     mem_clk_setdiv
*
* Description: set div ratio
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 mem_clk_setdiv(struct clk_div_t *clk_div)
{
	if(!clk_div){
		return -1;
	}

	CmuReg = (__ccmu_reg_list_t *)(AW_CCM_BASE);
	
	//set axi ratio
	CmuReg->SysClkDiv.AXIClkDiv = clk_div->axi_div;
	//set ahb1/apb1 clock divide ratio
	//first, config ratio; 
	*(volatile __u32 *)(&CmuReg->Ahb1Div) = (((clk_div->ahb_apb_div)&(~0x3000)) | (0x1000));
	//sec, config src.
	*(volatile __u32 *)(&CmuReg->Ahb1Div) = (clk_div->ahb_apb_div);

	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_getdiv
*
* Description: 
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 mem_clk_getdiv(struct clk_div_t  *clk_div)
{
	if(!clk_div){
		return -1;
	}
	CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	
	clk_div->axi_div = CmuReg->SysClkDiv.AXIClkDiv;
	clk_div->ahb_apb_div = *(volatile __u32 *)(&CmuReg->Ahb1Div);
	
	return 0;
}


/*
*********************************************************************************************************
*                                     mem_clk_set_pll_factor
*
* Description: set pll factor, target cpu freq is ?M hz
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/

__s32 mem_clk_set_pll_factor(struct pll_factor_t *pll_factor)
{
	CmuReg = (__ccmu_reg_list_t *)(AW_CCM_BASE);
	//set pll factor: notice: when raise freq, N must be the last to set
#if defined(CONFIG_ARCH_SUN8IW3P1)
	CmuReg->Pll1Ctl.FactorP = pll_factor->FactorP;
#endif
	CmuReg->Pll1Ctl.FactorM = pll_factor->FactorM;
	CmuReg->Pll1Ctl.FactorK = pll_factor->FactorK;
	CmuReg->Pll1Ctl.FactorN = pll_factor->FactorN;
	//need delay?
	//busy_waiting();
	
	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_get_pll_factor
*
* Description: get pll factor
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/

__s32 mem_clk_get_pll_factor(struct pll_factor_t *pll_factor)
{
	CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	pll_factor->FactorN = CmuReg->Pll1Ctl.FactorN;
	pll_factor->FactorK = CmuReg->Pll1Ctl.FactorK;
	pll_factor->FactorM = CmuReg->Pll1Ctl.FactorM;
#if defined(CONFIG_ARCH_SUN8IW3P1)
	pll_factor->FactorP = CmuReg->Pll1Ctl.FactorP;
#endif
	//busy_waiting();
	
	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_set_misc
*
* Description: set clk_misc
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 mem_clk_set_misc(struct clk_misc_t *clk_misc)
{
	CmuReg = (__ccmu_reg_list_t *)(AW_CCM_BASE);
	//
	CmuReg->PllxBias[0]	= clk_misc->pll1_bias;
	CmuReg->PllxBias[5]	= clk_misc->pll6_bias;
	CmuReg->Pll1Tun		= clk_misc->pll1_tun;
	//config axi ratio to 1+1 = 2;
	//axi can not exceed 300M;
	CmuReg->SysClkDiv.AXIClkDiv = 1;
	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_get_misc
*
* Description: get clk_misc
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/

__s32 mem_clk_get_misc(struct clk_misc_t *clk_misc)
{
	CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	clk_misc->pll1_bias	=	CmuReg->PllxBias[0];	 
	clk_misc->pll6_bias	=	CmuReg->PllxBias[5];	
	clk_misc->pll1_tun	=	CmuReg->Pll1Tun;	
	
	//busy_waiting();
	return 0;
}
#endif

#ifdef CONFIG_ARCH_SUN9IW1P1
/*
*********************************************************************************************************
*                           mem_clk_save
*
*Description: save ccu for platform mem
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 mem_clk_save(struct clk_state *pclk_state)
{
	pclk_state->CmuReg = CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);

	/*backup clk src*/
	pclk_state->ccu_reg_back[0] = *(volatile __u32 *)&CmuReg->Cpu_Clk_Src;
	pclk_state->ccu_reg_back[1] = *(volatile __u32 *)&CmuReg->Axi0_Cfg;
	pclk_state->ccu_reg_back[2] = *(volatile __u32 *)&CmuReg->Cci400_Cfg;	
	pclk_state->ccu_reg_back[3] = *(volatile __u32 *)&CmuReg->Gtclk_Cfg;
	pclk_state->ccu_reg_back[4] = *(volatile __u32 *)&CmuReg->Ahb0_Cfg;
	pclk_state->ccu_reg_back[5] = *(volatile __u32 *)&CmuReg->Ahb1_Cfg;
	pclk_state->ccu_reg_back[6] = *(volatile __u32 *)&CmuReg->Apb0_Cfg;	
	pclk_state->ccu_reg_back[7] = *(volatile __u32 *)&CmuReg->Apb1_Cfg;

	return 0;
}


/*
*********************************************************************************************************
*                           mem_clk_exit
*
*Description: restore ccu for platform mem
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 mem_clk_restore(struct clk_state *pclk_state)
{
	/* initialise the CCU io base */
	CmuReg = pclk_state->CmuReg;

	/*restore clk src and ldo*/
	*(volatile __u32 *)&CmuReg->Cpu_Clk_Src = pclk_state->ccu_reg_back[0];  
	*(volatile __u32 *)&CmuReg->Axi0_Cfg    = pclk_state->ccu_reg_back[1];  
	*(volatile __u32 *)&CmuReg->Cci400_Cfg	= pclk_state->ccu_reg_back[2];	
	*(volatile __u32 *)&CmuReg->Gtclk_Cfg   = pclk_state->ccu_reg_back[3]; 
	*(volatile __u32 *)&CmuReg->Ahb0_Cfg    = pclk_state->ccu_reg_back[4]; 
	*(volatile __u32 *)&CmuReg->Ahb1_Cfg    = pclk_state->ccu_reg_back[5]; 
	*(volatile __u32 *)&CmuReg->Apb0_Cfg	= pclk_state->ccu_reg_back[6]; 
	*(volatile __u32 *)&CmuReg->Apb1_Cfg    = pclk_state->ccu_reg_back[7];	 

	return 0;
}


/*
*********************************************************************************************************
*                                     mem_clk_setdiv
*
* Description: set div ratio
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 mem_clk_setdiv(struct clk_div_t *clk_div)
{
	if(!clk_div){
		return -1;
	}

	CmuReg = (__ccmu_reg_list_t *)(AW_CCM_BASE);

	*(volatile __u32 *)&CmuReg->Cpu_Clk_Src = clk_div->Cpu_Clk_Src	;  
	*(volatile __u32 *)&CmuReg->Axi0_Cfg    = clk_div->Axi0_Cfg	;  
	*(volatile __u32 *)&CmuReg->Cci400_Cfg	= clk_div->Cci400_Cfg	;	
	*(volatile __u32 *)&CmuReg->Gtclk_Cfg   = clk_div->Gtclk_Cfg	; 
	*(volatile __u32 *)&CmuReg->Ahb0_Cfg    = clk_div->Ahb0_Cfg	; 
	*(volatile __u32 *)&CmuReg->Ahb1_Cfg    = clk_div->Ahb1_Cfg	; 
	*(volatile __u32 *)&CmuReg->Apb0_Cfg	= clk_div->Apb0_Cfg	; 
	*(volatile __u32 *)&CmuReg->Apb1_Cfg    = clk_div->Apb1_Cfg	;	 

	
	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_getdiv
*
* Description: 
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 mem_clk_getdiv(struct clk_div_t  *clk_div)
{
	if(!clk_div){
		return -1;
	}
	CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	
	clk_div->Cpu_Clk_Src	=	*(volatile __u32 *)&CmuReg->Cpu_Clk_Src;        
	clk_div->Axi0_Cfg	=	*(volatile __u32 *)&CmuReg->Axi0_Cfg;           
	clk_div->Cci400_Cfg	=	*(volatile __u32 *)&CmuReg->Cci400_Cfg;	
	clk_div->Gtclk_Cfg	=	*(volatile __u32 *)&CmuReg->Gtclk_Cfg;          
	clk_div->Ahb0_Cfg	=	*(volatile __u32 *)&CmuReg->Ahb0_Cfg;           
	clk_div->Ahb1_Cfg	=	*(volatile __u32 *)&CmuReg->Ahb1_Cfg;           
	clk_div->Apb0_Cfg	=	*(volatile __u32 *)&CmuReg->Apb0_Cfg;	        
	clk_div->Apb1_Cfg	=	*(volatile __u32 *)&CmuReg->Apb1_Cfg;           
	
	return 0;
}


/*
*********************************************************************************************************
*                                     mem_clk_set_pll_factor
*
* Description: set pll factor, target cpu freq is ?M hz
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/

__s32 mem_clk_set_pll_factor(struct pll_factor_t *pll_factor)
{
	CmuReg = (__ccmu_reg_list_t *)(AW_CCM_BASE);
	//set pll factor: notice: when raise freq, N must be the last to set
	CmuReg->Pll_C0_Cfg.bits.pll_postdiv_m		= pll_factor->FactorM;
	CmuReg->Pll_C0_Cfg.bits.pll_out_ext_divp	= pll_factor->FactorP;
	CmuReg->Pll_C0_Cfg.bits.pll_factor_n		= pll_factor->FactorN;
	
	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_get_pll_factor
*
* Description: get pll factor
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/

__s32 mem_clk_get_pll_factor(struct pll_factor_t *pll_factor)
{
	CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	pll_factor->FactorN = CmuReg->Pll_C0_Cfg.bits.pll_factor_n;
	pll_factor->FactorM = CmuReg->Pll_C0_Cfg.bits.pll_postdiv_m;
	pll_factor->FactorP = CmuReg->Pll_C0_Cfg.bits.pll_out_ext_divp;
	
	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_set_misc
*
* Description: set clk_misc
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 mem_clk_set_misc(struct clk_misc_t *clk_misc)
{
	CmuReg = (__ccmu_reg_list_t *)(AW_CCM_BASE);
	//
	CmuReg->Pll_C0_Bias.dwval	= clk_misc->Pll_C0_Bias     ;
	CmuReg->Pll_C0_Tun.dwval      	= clk_misc->Pll_C0_Tun      ;
	CmuReg->Pll_Periph1_Bias.dwval	= clk_misc->Pll_Periph1_Bias;
	CmuReg->Pll_Periph2_Bias.dwval	= clk_misc->Pll_Periph2_Bias;
	
	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_get_misc
*
* Description: get clk_misc
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/

__s32 mem_clk_get_misc(struct clk_misc_t *clk_misc)
{
	CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	clk_misc->Pll_C0_Bias     	=	CmuReg->Pll_C0_Bias.dwval     ;	 
	clk_misc->Pll_C0_Tun      	=       CmuReg->Pll_C0_Tun.dwval      ;	
	clk_misc->Pll_Periph1_Bias	=	CmuReg->Pll_Periph1_Bias.dwval;	
	clk_misc->Pll_Periph2_Bias	=	CmuReg->Pll_Periph2_Bias.dwval;	

	return 0;
}

#endif
