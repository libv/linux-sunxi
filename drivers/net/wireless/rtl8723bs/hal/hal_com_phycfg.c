/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#define _HAL_COM_PHYCFG_C_

#include <drv_types.h>
#include <hal_data.h>


//
//	Description:
//		Map Tx power index into dBm according to 
//		current HW model, for example, RF and PA, and
//		current wireless mode.
//	By Bruce, 2008-01-29.
//
s32
phy_TxPwrIdxToDbm(
	IN	PADAPTER		Adapter,
	IN	WIRELESS_MODE	WirelessMode,
	IN	u8				TxPwrIdx
	)
{
	s32				Offset = 0;
	s32				PwrOutDbm = 0;
	
	//
	// Tested by MP, we found that CCK Index 0 equals to -7dbm, OFDM legacy equals to -8dbm.
	// Note:
	//	The mapping may be different by different NICs. Do not use this formula for what needs accurate result.  
	// By Bruce, 2008-01-29.
	// 
	switch(WirelessMode)
	{
	case WIRELESS_MODE_B:
		Offset = -7;		
		break;

	case WIRELESS_MODE_G:
	case WIRELESS_MODE_N_24G:
		Offset = -8;
		break;
		
	default: //for MacOSX compiler warning
		break;		
	}

	PwrOutDbm = TxPwrIdx / 2 + Offset; // Discard the decimal part.

	return PwrOutDbm;
}

VOID 
PHY_ConvertTxPowerByRateInDbmToRelativeValues(
	IN	u32		*pData,
	IN	u8		Start,
	IN	u8		End,
	IN	u8		BaseValue
	)
{
	s8	i = 0;
	u8	TempValue = 0;
	u32	TempData = 0;
	
	for ( i = 3; i >= 0; --i )
	{
		if ( i >= Start && i <= End )
		{
			// Get the exact value
			TempValue = ( u8 ) ( *pData >> ( i * 8 ) ) & 0xF; 
			TempValue += ( ( u8 ) ( ( *pData >> ( i * 8 + 4 ) ) & 0xF ) ) * 10; 
			
			// Change the value to a relative value
			TempValue = ( TempValue > BaseValue ) ? TempValue - BaseValue : BaseValue - TempValue;
		}
		else
		{
			TempValue = ( u8 ) ( *pData >> ( i * 8 ) ) & 0xFF;
		}
		
		TempData <<= 8;
		TempData |= TempValue;
	}

	*pData = TempData;
}

u8
PHY_GetTxPowerByRateBase(
	IN	PADAPTER		Adapter,
	IN	u8				Band,
	IN	u8				RfPath,
	IN	u8				TxNum,
	IN	RATE_SECTION	RateSection
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u8 			value = 0;

	if ( RfPath > ODM_RF_PATH_D )
	{
		DBG_871X("Invalid Rf Path %d in PHY_GetTxPowerByRateBase()\n", RfPath );
		return 0;
	}
	
	if ( Band == BAND_ON_2_4G )
	{
		switch ( RateSection ) {
			case CCK:
				value = pHalData->TxPwrByRateBase2_4G[RfPath][TxNum][0];
				break;
			case OFDM:
				value = pHalData->TxPwrByRateBase2_4G[RfPath][TxNum][1];
				break;
			case HT_MCS0_MCS7:
				value = pHalData->TxPwrByRateBase2_4G[RfPath][TxNum][2];
				break;
			case HT_MCS8_MCS15:
				value = pHalData->TxPwrByRateBase2_4G[RfPath][TxNum][3];
				break;
			case VHT_1SSMCS0_1SSMCS9:
				value = pHalData->TxPwrByRateBase2_4G[RfPath][TxNum][4];
				break;
			case VHT_2SSMCS0_2SSMCS9:
				value = pHalData->TxPwrByRateBase2_4G[RfPath][TxNum][5];
				break;
			default:
				DBG_871X("Invalid RateSection %d in Band 2.4G, Rf Path %d, %dTx in PHY_GetTxPowerByRateBase()\n", 
						 RateSection, RfPath, TxNum );
				break;
				
		};
	}
	else if ( Band == BAND_ON_5G )
	{
		switch ( RateSection ) {
			case OFDM:
				value = pHalData->TxPwrByRateBase5G[RfPath][TxNum][0];
				break;
			case HT_MCS0_MCS7:
				value = pHalData->TxPwrByRateBase5G[RfPath][TxNum][1];
				break;
			case HT_MCS8_MCS15:
				value = pHalData->TxPwrByRateBase5G[RfPath][TxNum][2];
				break;
			case VHT_1SSMCS0_1SSMCS9:
				value = pHalData->TxPwrByRateBase5G[RfPath][TxNum][3];
				break;
			case VHT_2SSMCS0_2SSMCS9:
				value = pHalData->TxPwrByRateBase5G[RfPath][TxNum][4];
				break;
			default:
				DBG_871X("Invalid RateSection %d in Band 5G, Rf Path %d, %dTx in PHY_GetTxPowerByRateBase()\n", 
						 RateSection, RfPath, TxNum );
				break;
		};
	}
	else
	{
		DBG_871X("Invalid Band %d in PHY_GetTxPowerByRateBase()\n", Band );
	}

	return value;
}

VOID
PHY_SetTxPowerByRateBase(
	IN	PADAPTER		Adapter,
	IN	u8				Band,
	IN	u8				RfPath,
	IN	RATE_SECTION	RateSection,
	IN	u8				TxNum,
	IN	u8				Value
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	
	if ( RfPath > ODM_RF_PATH_D )
	{
		DBG_871X("Invalid Rf Path %d in phy_SetTxPowerByRatBase()\n", RfPath );
		return;
	}
	
	if ( Band == BAND_ON_2_4G )
	{
		switch ( RateSection ) {
			case CCK:
				pHalData->TxPwrByRateBase2_4G[RfPath][TxNum][0] = Value;
				break;
			case OFDM:
				pHalData->TxPwrByRateBase2_4G[RfPath][TxNum][1] = Value;
				break;
			case HT_MCS0_MCS7:
				pHalData->TxPwrByRateBase2_4G[RfPath][TxNum][2] = Value;
				break;
			case HT_MCS8_MCS15:
				pHalData->TxPwrByRateBase2_4G[RfPath][TxNum][3] = Value;
				break;
			case VHT_1SSMCS0_1SSMCS9:
				pHalData->TxPwrByRateBase2_4G[RfPath][TxNum][4] = Value;
				break;
			case VHT_2SSMCS0_2SSMCS9:
				pHalData->TxPwrByRateBase2_4G[RfPath][TxNum][5] = Value;
				break;
			default:
				DBG_871X("Invalid RateSection %d in Band 2.4G, Rf Path %d, %dTx in PHY_SetTxPowerByRateBase()\n", 
						 RateSection, RfPath, TxNum );
				break;
		};
	}
	else if ( Band == BAND_ON_5G )
	{
		switch ( RateSection ) {
			case OFDM:
				pHalData->TxPwrByRateBase5G[RfPath][TxNum][0] = Value;
				break;
			case HT_MCS0_MCS7:
				pHalData->TxPwrByRateBase5G[RfPath][TxNum][1] = Value;
				break;
			case HT_MCS8_MCS15:
				pHalData->TxPwrByRateBase5G[RfPath][TxNum][2] = Value;
				break;
			case VHT_1SSMCS0_1SSMCS9:
				pHalData->TxPwrByRateBase5G[RfPath][TxNum][3] = Value;
				break;
			case VHT_2SSMCS0_2SSMCS9:
				pHalData->TxPwrByRateBase5G[RfPath][TxNum][4] = Value;
				break;
			default:
				DBG_871X("Invalid RateSection %d in Band 5G, Rf Path %d, %dTx in PHY_SetTxPowerByRateBase()\n", 
						 RateSection, RfPath, TxNum );
				break;
		};
	}
	else
	{
		DBG_871X("Invalid Band %d in PHY_SetTxPowerByRateBase()\n", Band );
	}
}

VOID
phy_StoreTxPowerByRateBaseOld(	
	IN	PADAPTER	pAdapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA( pAdapter );
	u16			rawValue = 0;
	u8			base = 0;
	u8			path = 0;

	rawValue = ( u16 ) ( pHalData->MCSTxPowerLevelOriginalOffset[0][7] >> 8 ) & 0xFF; 
	base = ( rawValue >> 4 ) * 10 + ( rawValue & 0xF );
	PHY_SetTxPowerByRateBase( pAdapter, BAND_ON_2_4G, ODM_RF_PATH_A, CCK, RF_1TX, base );

	rawValue = ( u16 ) ( pHalData->MCSTxPowerLevelOriginalOffset[0][1] >> 24 ) & 0xFF; 
	base = ( rawValue >> 4 ) * 10 + ( rawValue & 0xF );
	PHY_SetTxPowerByRateBase( pAdapter, BAND_ON_2_4G, ODM_RF_PATH_A, OFDM, RF_1TX, base );

	rawValue = ( u16 ) ( pHalData->MCSTxPowerLevelOriginalOffset[0][3] >> 24 ) & 0xFF; 
	base = ( rawValue >> 4 ) * 10 + ( rawValue & 0xF );
	PHY_SetTxPowerByRateBase( pAdapter, BAND_ON_2_4G, ODM_RF_PATH_A, HT_MCS0_MCS7, RF_1TX, base );

	rawValue = ( u16 ) ( pHalData->MCSTxPowerLevelOriginalOffset[0][5] >> 24 ) & 0xFF; 
	base = ( rawValue >> 4) * 10 + ( rawValue & 0xF );
	PHY_SetTxPowerByRateBase( pAdapter, BAND_ON_2_4G, ODM_RF_PATH_A, HT_MCS8_MCS15, RF_2TX, base );

	rawValue = ( u16 ) ( pHalData->MCSTxPowerLevelOriginalOffset[0][7] & 0xFF ); 
	base = ( rawValue >> 4 ) * 10 + ( rawValue & 0xF );
	PHY_SetTxPowerByRateBase( pAdapter, BAND_ON_2_4G, ODM_RF_PATH_B, CCK, RF_1TX, base );

	rawValue = ( u16 ) ( pHalData->MCSTxPowerLevelOriginalOffset[0][9] >> 24 ) & 0xFF; 
	base = ( rawValue >> 4 ) * 10 + ( rawValue & 0xF );
	PHY_SetTxPowerByRateBase( pAdapter, BAND_ON_2_4G, ODM_RF_PATH_B, OFDM, RF_1TX, base );

	rawValue = ( u16 ) ( pHalData->MCSTxPowerLevelOriginalOffset[0][11] >> 24 ) & 0xFF; 
	base = ( rawValue >> 4 ) * 10 + ( rawValue & 0xF );
	PHY_SetTxPowerByRateBase( pAdapter, BAND_ON_2_4G, ODM_RF_PATH_B, HT_MCS0_MCS7, RF_1TX, base );

	rawValue = ( u16 ) ( pHalData->MCSTxPowerLevelOriginalOffset[0][13] >> 24 ) & 0xFF; 
	base = ( rawValue >> 4 ) * 10 + ( rawValue & 0xF );
	PHY_SetTxPowerByRateBase( pAdapter, BAND_ON_2_4G, ODM_RF_PATH_B, HT_MCS8_MCS15, RF_2TX, base );
}

VOID
PHY_StoreTxPowerByRateBase(	
	IN	PADAPTER	pAdapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA( pAdapter );
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;

	if ( pDM_Odm->PhyRegPgVersion == 0 )
	{
		phy_StoreTxPowerByRateBaseOld( pAdapter );
	}
	else 
	{
		if ( IS_HARDWARE_TYPE_8723B( pAdapter ) ) {
#if (RTL8723B_SUPPORT == 1)
			PHY_StoreTxPowerByRateBase_8723B( pAdapter );
#endif
		} else if ( IS_HARDWARE_TYPE_8192E( pAdapter ) ) {
#if (RTL8192E_SUPPORT == 1)
			PHY_StoreTxPowerByRateBase_8192E( pAdapter );
#endif
		} else if ( IS_HARDWARE_TYPE_JAGUAR( pAdapter ) ) {
#if ((RTL8812A_SUPPORT==1) || (RTL8821A_SUPPORT == 1))
			PHY_StoreTxPowerByRateBase_8812A( pAdapter );
#endif
		}
	}
}

u8
PHY_GetRateSectionIndexOfTxPowerByRate(
	IN	PADAPTER	pAdapter,
	IN	u32			RegAddr,
	IN	u32			BitMask
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA( pAdapter );
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	u8 			index = 0;
	
	if ( pDM_Odm->PhyRegPgVersion == 0 )
	{
		switch ( RegAddr )
		{
			case rTxAGC_A_Rate18_06:	 index = 0;		break;
			case rTxAGC_A_Rate54_24:	 index = 1;		break;
			case rTxAGC_A_CCK1_Mcs32:	 index = 6;		break;
			case rTxAGC_B_CCK11_A_CCK2_11:
				if ( BitMask == 0xffffff00 )
					index = 7;
				else if ( BitMask == 0x000000ff )
					index = 15;
				break;
				
			case rTxAGC_A_Mcs03_Mcs00:	 index = 2;		break;
			case rTxAGC_A_Mcs07_Mcs04:	 index = 3;		break;
			case rTxAGC_A_Mcs11_Mcs08:	 index = 4;		break;
			case rTxAGC_A_Mcs15_Mcs12:	 index = 5;		break;
			case rTxAGC_B_Rate18_06:	 index = 8;		break;
			case rTxAGC_B_Rate54_24:	 index = 9;		break;
			case rTxAGC_B_CCK1_55_Mcs32: index = 14;	break;
			case rTxAGC_B_Mcs03_Mcs00:	 index = 10;	break;
			case rTxAGC_B_Mcs07_Mcs04:	 index = 11;	break;
			case rTxAGC_B_Mcs11_Mcs08:	 index = 12;	break;
			case rTxAGC_B_Mcs15_Mcs12:	 index = 13;	break;
			default:
				DBG_871X("Invalid RegAddr 0x3%x in PHY_GetRateSectionIndexOfTxPowerByRate()", RegAddr );
				break;
		};
	}
	else if ( pDM_Odm->PhyRegPgVersion > 0 )
	{
		switch ( RegAddr )
		{
			case rTxAGC_A_Rate18_06:	   index = 0;		break;
			case rTxAGC_A_Rate54_24:	   index = 1;		break;
			case rTxAGC_A_CCK1_Mcs32:	   index = 2;		break;
			case rTxAGC_B_CCK11_A_CCK2_11: index = 3;		break;			
			case rTxAGC_A_Mcs03_Mcs00:	   index = 4;		break;
			case rTxAGC_A_Mcs07_Mcs04:	   index = 5;		break;
			case rTxAGC_A_Mcs11_Mcs08:	   index = 6;		break;
			case rTxAGC_A_Mcs15_Mcs12:	   index = 7;		break;
			case rTxAGC_B_Rate18_06:	   index = 0;		break;
			case rTxAGC_B_Rate54_24:	   index = 1;		break;
			case rTxAGC_B_CCK1_55_Mcs32:   index = 2;		break;
			case rTxAGC_B_Mcs03_Mcs00:	   index = 4;		break;
			case rTxAGC_B_Mcs07_Mcs04:	   index = 5;		break;
			case rTxAGC_B_Mcs11_Mcs08:	   index = 6;		break;
			case rTxAGC_B_Mcs15_Mcs12:	   index = 7;		break;
			default:
				RegAddr =  ( RegAddr & 0xFFF );
				if ( RegAddr >= 0xC20 && RegAddr <= 0xC4C )
					index = ( u8 ) ( ( RegAddr - 0xC20 ) / 4 );
				else if ( RegAddr >= 0xE20 && RegAddr <= 0xE4C )
					index = ( u8 ) ( ( RegAddr - 0xE20 ) / 4 );
				else
					DBG_871X("Invalid RegAddr 0x%x in PHY_GetRateSectionIndexOfTxPowerByRate()\n", RegAddr);
				break;
		};
	}
	
	return index;
}

u8
PHY_GetTxPowerByRateNew(
	IN	PADAPTER	pAdapter,
	IN	u8			Band,
	IN	u8			RfPath,
	IN	u8			TxNum,
	IN	u8			Rate
	)
{
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(pAdapter);
	u8			shift = 0;
	u8			tx_pwr_diff = 0;
	u8			rate_section = 0;

	if ( Band != BAND_ON_2_4G && Band != BAND_ON_5G )
	{
		DBG_871X("Invalid Band %d\n", Band );
	}

	if ( RfPath > ODM_RF_PATH_MAX )
	{
		DBG_871X("Invalid RfPath %d\n", RfPath );
	}

	if ( TxNum > ODM_RF_PATH_MAX )
	{
		DBG_871X("Invalid TxNum %d\n", TxNum );
	}

	switch	(Rate)
	{
		case	MGN_1M:
		case	MGN_2M:
		case	MGN_5_5M:	
		case	MGN_11M:	
			rate_section = 0;
			break;

		case	MGN_6M:		
		case	MGN_9M:		
		case	MGN_12M:	
		case	MGN_18M:	
			rate_section = 1;
			break;

		case	MGN_24M:	
		case	MGN_36M:    
		case	MGN_48M:    
		case	MGN_54M:    
			rate_section = 2;
			break;
		
		case	MGN_MCS0:	
		case	MGN_MCS1:   
		case	MGN_MCS2:   
		case	MGN_MCS3:   
			rate_section = 3;
			break;

		case	MGN_MCS4:	
		case	MGN_MCS5:   
		case	MGN_MCS6:   
		case	MGN_MCS7:   
			rate_section = 4;
			break;

		case	MGN_MCS8:	
		case	MGN_MCS9:   
		case	MGN_MCS10:  
		case	MGN_MCS11:  
			rate_section = 5;
			break;

		case	MGN_MCS12:	
		case	MGN_MCS13:  
		case	MGN_MCS14:  
		case	MGN_MCS15:  
			rate_section = 6;
			break;
		
		case	MGN_VHT1SS_MCS0:	
		case	MGN_VHT1SS_MCS1:    
		case	MGN_VHT1SS_MCS2:    
		case	MGN_VHT1SS_MCS3:    
			rate_section = 7;
			break;

		case	MGN_VHT1SS_MCS4:	
		case	MGN_VHT1SS_MCS5:    
		case	MGN_VHT1SS_MCS6:    
		case	MGN_VHT1SS_MCS7:    
			rate_section = 8;
			break;

		case	MGN_VHT1SS_MCS8:	
		case	MGN_VHT1SS_MCS9:    
		case	MGN_VHT2SS_MCS0:    
		case	MGN_VHT2SS_MCS1:    
			rate_section = 9;
			break;

		case	MGN_VHT2SS_MCS2:	
		case	MGN_VHT2SS_MCS3:    
		case	MGN_VHT2SS_MCS4:    
		case	MGN_VHT2SS_MCS5:    
			rate_section = 10;
			break;

		case	MGN_VHT2SS_MCS6:	
		case	MGN_VHT2SS_MCS7:    
		case	MGN_VHT2SS_MCS8:    
		case	MGN_VHT2SS_MCS9:    
			rate_section = 11;
			break;
			
		default:
			DBG_871X("Rate %dis Illegal\n", Rate);
			break;
	}
	
	switch	(Rate)
	{
		case	MGN_1M:		shift = 0;		break;
		case	MGN_2M:		shift = 8;		break;
		case	MGN_5_5M:	shift = 16;		break;
		case	MGN_11M:	shift = 24;		break;			

		case	MGN_6M:		shift = 0;		break;
		case	MGN_9M:		shift = 8;      break;
		case	MGN_12M:	shift = 16;     break;
		case	MGN_18M:	shift = 24;     break;
			
		case	MGN_24M:	shift = 0;    	break;
		case	MGN_36M:    shift = 8;      break;
		case	MGN_48M:    shift = 16;     break;
		case	MGN_54M:    shift = 24;     break;
			
		case	MGN_MCS0:	shift = 0; 		break;
		case	MGN_MCS1:   shift = 8;      break;
		case	MGN_MCS2:   shift = 16;     break;
		case	MGN_MCS3:   shift = 24;     break;
			
		case	MGN_MCS4:	shift = 0; 		break;
		case	MGN_MCS5:   shift = 8;      break;
		case	MGN_MCS6:   shift = 16;     break;
		case	MGN_MCS7:   shift = 24;     break;
			
		case	MGN_MCS8:	shift = 0; 		break;
		case	MGN_MCS9:   shift = 8;      break;
		case	MGN_MCS10:  shift = 16;     break;
		case	MGN_MCS11:  shift = 24;     break;
			
		case	MGN_MCS12:	shift = 0; 		break;
		case	MGN_MCS13:  shift = 8;      break;
		case	MGN_MCS14:  shift = 16;     break;
		case	MGN_MCS15:  shift = 24;     break;
			
		case	MGN_VHT1SS_MCS0:	shift = 0; 		break;
		case	MGN_VHT1SS_MCS1:    shift = 8;      break;
		case	MGN_VHT1SS_MCS2:    shift = 16;     break;
		case	MGN_VHT1SS_MCS3:    shift = 24;     break;
			
		case	MGN_VHT1SS_MCS4:	shift = 0; 		break;
		case	MGN_VHT1SS_MCS5:    shift = 8;      break;
		case	MGN_VHT1SS_MCS6:    shift = 16;     break;
		case	MGN_VHT1SS_MCS7:    shift = 24;     break;
			
		case	MGN_VHT1SS_MCS8:	shift = 0; 		break;
		case	MGN_VHT1SS_MCS9:    shift = 8;      break;
		case	MGN_VHT2SS_MCS0:    shift = 16;     break;
		case	MGN_VHT2SS_MCS1:    shift = 24;     break;
			
		case	MGN_VHT2SS_MCS2:	shift = 0; 		break;
		case	MGN_VHT2SS_MCS3:    shift = 8;      break;
		case	MGN_VHT2SS_MCS4:    shift = 16;     break;
		case	MGN_VHT2SS_MCS5:    shift = 24;     break;
			
		case	MGN_VHT2SS_MCS6:	shift = 0; 		break;
		case	MGN_VHT2SS_MCS7:    shift = 8;      break;
		case	MGN_VHT2SS_MCS8:    shift = 16;     break;
		case	MGN_VHT2SS_MCS9:    shift = 24;     break;
			
		default:
			DBG_871X("Rate_Section is Illegal\n");
			break;
	}

	tx_pwr_diff = ( u8 ) (pHalData->TxPwrByRateOffset[Band][RfPath][TxNum][rate_section] >> shift) & 0xff;

	//DBG_871X("TxPwrByRateOffset-BAND(%d)-RF(%d)-RAS(%d)=%x tx_pwr_diff=%d shift=%d\n", 
	//Band, RfPath, TxNum, rate_section, pHalData->TxPwrByRateOffset[Band][RfPath][TxNum][rate_section], tx_pwr_diff, shift);

	return	tx_pwr_diff;
}

void
PHY_SetTxPowerByRateNew(
	IN	PADAPTER	pAdapter,
	IN	u32			Band,
	IN	u32			RfPath,
	IN	u32			TxNum,
	IN	u32			RegAddr,
	IN	u32			BitMask,
	IN	u32			Data
	)
{
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(pAdapter);
	u8			rate_section = PHY_GetRateSectionIndexOfTxPowerByRate( pAdapter, RegAddr, BitMask );

	if ( Band != BAND_ON_2_4G && Band != BAND_ON_5G )
	{
		DBG_871X("Invalid Band %d\n", Band );
	}

	if ( RfPath > ODM_RF_PATH_MAX )
	{
		DBG_871X("Invalid RfPath %d\n", RfPath );
	}

	if ( TxNum > ODM_RF_PATH_MAX )
	{
		DBG_871X("Invalid TxNum %d\n", TxNum );
	}
	
	pHalData->TxPwrByRateOffset[Band][RfPath][TxNum][rate_section] = Data;
	//DBG_871X("pHalData->TxPwrByRateOffset[Band %d][RfPath %d][TxNum %d][RateSection %d] = 0x%x\n", 
	//		Band, RfPath, TxNum, rate_section, pHalData->TxPwrByRateOffset[Band][RfPath][TxNum][rate_section]);
}

void 
PHY_SetTxPowerByRateOld(
	IN	PADAPTER		pAdapter,
	IN	u32				RegAddr,
	IN	u32				BitMask,
	IN	u32				Data
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	u8			index = PHY_GetRateSectionIndexOfTxPowerByRate( pAdapter, RegAddr, BitMask );

	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][index] = Data;
	//DBG_871X("MCSTxPowerLevelOriginalOffset[%d][0] = 0x%x\n", pHalData->pwrGroupCnt,
	//	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][0]);
}

VOID
PHY_InitTxPowerByRate(
	IN	PADAPTER	pAdapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	u1Byte band = 0, rfPath = 0, TxNum = 0, rateSection = 0, i = 0, j = 0;

	if ( IS_HARDWARE_TYPE_8188E( pAdapter ) || IS_HARDWARE_TYPE_8723A( pAdapter ) )
	{
		for ( i = 0; i < MAX_PG_GROUP; ++i )
			for ( j = 0; j < 16; ++j )
				pHalData->MCSTxPowerLevelOriginalOffset[i][j] = 0;
	}
	else
	{
		for ( band = BAND_ON_2_4G; band <= BAND_ON_5G; ++band )
				for ( rfPath = 0; rfPath < TX_PWR_BY_RATE_NUM_RF; ++rfPath )
					for ( TxNum = 0; TxNum < TX_PWR_BY_RATE_NUM_RF; ++TxNum )
						for ( rateSection = 0; rateSection < TX_PWR_BY_RATE_NUM_SECTION; ++rateSection )
							pHalData->TxPwrByRateOffset[band][rfPath][TxNum][rateSection] = 0;
	}
}

VOID
PHY_StoreTxPowerByRate(
	IN	PADAPTER	pAdapter,
	IN	u32			Band,
	IN	u32			RfPath,
	IN	u32			TxNum,
	IN	u32			RegAddr,
	IN	u32			BitMask,
	IN	u32			Data
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	PDM_ODM_T  		pDM_Odm = &pHalData->odmpriv;
	
	if ( pDM_Odm->PhyRegPgVersion > 0 )
	{
		PHY_SetTxPowerByRateNew( pAdapter, Band, RfPath, TxNum, RegAddr, BitMask, Data );
	}
	else if ( pDM_Odm->PhyRegPgVersion == 0 )
	{
		PHY_SetTxPowerByRateOld( pAdapter, RegAddr, BitMask, Data );
	
		if ( RegAddr == rTxAGC_A_Mcs15_Mcs12 && pHalData->rf_type == RF_1T1R )
			pHalData->pwrGroupCnt++;
		else if ( RegAddr == rTxAGC_B_Mcs15_Mcs12 && pHalData->rf_type != RF_1T1R )
			pHalData->pwrGroupCnt++;
	}
	else
		DBG_871X("Invalid PHY_REG_PG.txt version %d\n",  pDM_Odm->PhyRegPgVersion );
	
}


VOID
PHY_ConvertTxPowerByRateInDbmToRelativeValuesOld(
	IN	PADAPTER	pAdapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA( pAdapter );
	u8			base = 0;
	
	//DBG_871X("===>PHY_ConvertTxPowerByRateInDbmToRelativeValuesOld()\n" );
	
	// CCK
	base = PHY_GetTxPowerByRateBase( pAdapter, BAND_ON_2_4G, ODM_RF_PATH_A, RF_1TX, CCK );
	PHY_ConvertTxPowerByRateInDbmToRelativeValues( 
			&( pHalData->MCSTxPowerLevelOriginalOffset[0][6] ), 1, 1, base );
	PHY_ConvertTxPowerByRateInDbmToRelativeValues( 
			&( pHalData->MCSTxPowerLevelOriginalOffset[0][7] ), 1, 3, base );

	// OFDM
	base = PHY_GetTxPowerByRateBase( pAdapter, BAND_ON_2_4G, ODM_RF_PATH_A, RF_1TX, OFDM );
	PHY_ConvertTxPowerByRateInDbmToRelativeValues( 
			&( pHalData->MCSTxPowerLevelOriginalOffset[0][0] ), 0, 3, base );
	PHY_ConvertTxPowerByRateInDbmToRelativeValues( 
			&( pHalData->MCSTxPowerLevelOriginalOffset[0][1] ),	0, 3, base );

	// HT MCS0~7
	base = PHY_GetTxPowerByRateBase( pAdapter, BAND_ON_2_4G, ODM_RF_PATH_A, RF_1TX, HT_MCS0_MCS7 );
	PHY_ConvertTxPowerByRateInDbmToRelativeValues( 
			&( pHalData->MCSTxPowerLevelOriginalOffset[0][2] ),	0, 3, base );
	PHY_ConvertTxPowerByRateInDbmToRelativeValues( 
			&( pHalData->MCSTxPowerLevelOriginalOffset[0][3] ),	0, 3, base );

	// HT MCS8~15
	base = PHY_GetTxPowerByRateBase( pAdapter, BAND_ON_2_4G, ODM_RF_PATH_A, RF_2TX, HT_MCS8_MCS15 );
	PHY_ConvertTxPowerByRateInDbmToRelativeValues( 
			&( pHalData->MCSTxPowerLevelOriginalOffset[0][4] ), 0, 3, base );
	PHY_ConvertTxPowerByRateInDbmToRelativeValues( 
			&( pHalData->MCSTxPowerLevelOriginalOffset[0][5] ), 0, 3, base );

	// CCK
	base = PHY_GetTxPowerByRateBase( pAdapter, BAND_ON_2_4G, ODM_RF_PATH_B, RF_1TX, CCK );
	PHY_ConvertTxPowerByRateInDbmToRelativeValues( 
			&( pHalData->MCSTxPowerLevelOriginalOffset[0][14] ), 1, 3, base );
	PHY_ConvertTxPowerByRateInDbmToRelativeValues( 
			&( pHalData->MCSTxPowerLevelOriginalOffset[0][15] ), 0, 0, base );

	// OFDM
	base = PHY_GetTxPowerByRateBase( pAdapter, BAND_ON_2_4G, ODM_RF_PATH_B, RF_1TX, OFDM );
	PHY_ConvertTxPowerByRateInDbmToRelativeValues( 
			&( pHalData->MCSTxPowerLevelOriginalOffset[0][8] ), 0, 3, base );
	PHY_ConvertTxPowerByRateInDbmToRelativeValues( 
			&( pHalData->MCSTxPowerLevelOriginalOffset[0][9] ),	0, 3, base );

	// HT MCS0~7
	base = PHY_GetTxPowerByRateBase( pAdapter, BAND_ON_2_4G, ODM_RF_PATH_B, RF_1TX, HT_MCS0_MCS7 );
	PHY_ConvertTxPowerByRateInDbmToRelativeValues( 
			&( pHalData->MCSTxPowerLevelOriginalOffset[0][10] ), 0, 3, base );
	PHY_ConvertTxPowerByRateInDbmToRelativeValues( 
			&( pHalData->MCSTxPowerLevelOriginalOffset[0][11] ), 0, 3, base );

	// HT MCS8~15
	base = PHY_GetTxPowerByRateBase( pAdapter, BAND_ON_2_4G, ODM_RF_PATH_B, RF_2TX, HT_MCS8_MCS15 );
	PHY_ConvertTxPowerByRateInDbmToRelativeValues( 
			&( pHalData->MCSTxPowerLevelOriginalOffset[0][12] ), 0, 3, base );
	PHY_ConvertTxPowerByRateInDbmToRelativeValues( 
			&( pHalData->MCSTxPowerLevelOriginalOffset[0][13] ), 0, 3, base );

	//DBG_871X("<===PHY_ConvertTxPowerByRateInDbmToRelativeValuesOld()\n" );
}

/*
  * This function must be called if the value in the PHY_REG_PG.txt(or header)
  * is exact dBm values
  */
VOID
PHY_TxPowerByRateConfiguration(
	IN  PADAPTER			pAdapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA( pAdapter);

	PHY_StoreTxPowerByRateBase( pAdapter );
	if ( IS_HARDWARE_TYPE_8723B( pAdapter ) ) {
#if (RTL8723B_SUPPORT == 1)
		PHY_ConvertTxPowerByRateInDbmToRelativeValues_8723B( pAdapter );
#endif
	} else if ( IS_HARDWARE_TYPE_8192E( pAdapter ) ) {
#if (RTL8192E_SUPPORT == 1)
		PHY_ConvertTxPowerByRateInDbmToRelativeValues_8192E( pAdapter );
#endif
	} else if ( IS_HARDWARE_TYPE_JAGUAR( pAdapter ) ) {
#if ((RTL8812A_SUPPORT==1) || (RTL8821A_SUPPORT == 1))
		PHY_ConvertTxPowerByRateInDbmToRelativeValues_8812A( pAdapter );
#endif
	} else if ( pHalData->odmpriv.PhyRegPgVersion == 0 ) {
		PHY_ConvertTxPowerByRateInDbmToRelativeValuesOld( pAdapter );
	}
	
}

static VOID
PHY_SetTxPowerIndexByRateArray(
	IN	PADAPTER			pAdapter,
	IN 	u8					RFPath,
	IN	CHANNEL_WIDTH		BandWidth,	
	IN	u8					Channel,
	IN	u8*					Rates,
	IN	u8					RateArraySize
	)
{
	if (IS_HARDWARE_TYPE_JAGUAR(pAdapter)) {
#if ((RTL8812A_SUPPORT==1) || (RTL8821A_SUPPORT == 1))
		PHY_SetTxPowerIndexByRateArray_8812A(pAdapter, RFPath, BandWidth, Channel, Rates, RateArraySize );
#endif
	}
	else if (IS_HARDWARE_TYPE_8723B(pAdapter)) {
#if (RTL8723B_SUPPORT==1)
		PHY_SetTxPowerIndexByRateArray_8723B(pAdapter, RFPath, BandWidth, Channel, Rates, RateArraySize );
#endif
	}
	else if (IS_HARDWARE_TYPE_8192E(pAdapter)) {
#if (RTL8192E_SUPPORT==1)
		PHY_SetTxPowerIndexByRateArray_8192E(pAdapter, RFPath, BandWidth, Channel, Rates, RateArraySize );
#endif
	}
}

VOID
PHY_SetTxPowerIndexByRateSection(
	IN	PADAPTER			pAdapter,
	IN	u8					RFPath,	
	IN	u8					Channel,
	IN	u8					RateSection
	)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(pAdapter);

	if ( RateSection == CCK )
	{
		u8	cckRates[]   = {MGN_1M, MGN_2M, MGN_5_5M, MGN_11M};
		if ( pHalData->CurrentBandType == BAND_ON_2_4G )
			PHY_SetTxPowerIndexByRateArray( pAdapter, RFPath, pHalData->CurrentChannelBW, Channel,
									  cckRates, sizeof(cckRates)/sizeof(u8) );
			
	}
	else if ( RateSection == OFDM )
	{
		u8	ofdmRates[]  = {MGN_6M, MGN_9M, MGN_12M, MGN_18M, MGN_24M, MGN_36M, MGN_48M, MGN_54M};
		PHY_SetTxPowerIndexByRateArray( pAdapter, RFPath, pHalData->CurrentChannelBW, Channel,
									 ofdmRates, sizeof(ofdmRates)/sizeof(u8));
		
	}
	else if ( RateSection == HT_MCS0_MCS7 )
	{
		u8	htRates1T[]  = {MGN_MCS0, MGN_MCS1, MGN_MCS2, MGN_MCS3, MGN_MCS4, MGN_MCS5, MGN_MCS6, MGN_MCS7};
		PHY_SetTxPowerIndexByRateArray( pAdapter, RFPath, pHalData->CurrentChannelBW, Channel,
									 htRates1T, sizeof(htRates1T)/sizeof(u8));

	}
	else if ( RateSection == HT_MCS8_MCS15 )
	{
		u8	htRates2T[]  = {MGN_MCS8, MGN_MCS9, MGN_MCS10, MGN_MCS11, MGN_MCS12, MGN_MCS13, MGN_MCS14, MGN_MCS15};
		PHY_SetTxPowerIndexByRateArray( pAdapter, RFPath, pHalData->CurrentChannelBW, Channel,
								  	 htRates2T, sizeof(htRates2T)/sizeof(u8));
		
	}
	else if ( RateSection == VHT_1SSMCS0_1SSMCS9 )
	{	
		u8	vhtRates1T[] = {MGN_VHT1SS_MCS0, MGN_VHT1SS_MCS1, MGN_VHT1SS_MCS2, MGN_VHT1SS_MCS3, MGN_VHT1SS_MCS4, 
                            	MGN_VHT1SS_MCS5, MGN_VHT1SS_MCS6, MGN_VHT1SS_MCS7, MGN_VHT1SS_MCS8, MGN_VHT1SS_MCS9};
		PHY_SetTxPowerIndexByRateArray( pAdapter, RFPath, pHalData->CurrentChannelBW, Channel,
									vhtRates1T, sizeof(vhtRates1T)/sizeof(u8));

	}
	else if ( RateSection == VHT_2SSMCS0_2SSMCS9 )
	{
		u8	vhtRates2T[] = {MGN_VHT2SS_MCS0, MGN_VHT2SS_MCS1, MGN_VHT2SS_MCS2, MGN_VHT2SS_MCS3, MGN_VHT2SS_MCS4, 
                            	MGN_VHT2SS_MCS5, MGN_VHT2SS_MCS6, MGN_VHT2SS_MCS7, MGN_VHT2SS_MCS8, MGN_VHT2SS_MCS9};

		PHY_SetTxPowerIndexByRateArray( pAdapter, RFPath, pHalData->CurrentChannelBW, Channel,
								  vhtRates2T, sizeof(vhtRates2T)/sizeof(u8));
	}
	else
	{
		DBG_871X("Invalid RateSection %d in %s", RateSection, __FUNCTION__ );
	}
}

s8
phy_GetChannelIndexOfTxPowerLimit(
	IN		u8			Band,
	IN		u8			Channel
	)
{
	s8	channelIndex = -1;
	u8	channel5G[CHANNEL_MAX_NUMBER_5G] = 
				 {36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,100,102,104,106,108,110,112,
				114,116,118,120,122,124,126,128,130,132,134,136,138,140,142,144,149,151,
				153,155,157,159,161,163,165,167,168,169,171,173,175,177};
	u8  i = 0;
	if ( Band == BAND_ON_2_4G )
	{
		channelIndex = Channel - 1;
	}
	else if ( Band == BAND_ON_5G )
	{
		for ( i = 0; i < sizeof(channel5G)/sizeof(u8); ++i )
		{
			if ( channel5G[i] == Channel )
				channelIndex = i;
		}
	}
	else {
		DBG_871X("Invalid Band %d in %s", Band, __FUNCTION__ );
	}

	if ( channelIndex == -1 ) {
		DBG_871X("Invalid Channel %d of Band %d in %s", Channel, Band, __FUNCTION__ );
	}

	return channelIndex;
}

VOID
PHY_SetTxPowerLimit(
	IN	PADAPTER		Adapter,
	IN	u8				*Regulation,
	IN	u8				*Band,
	IN	u8				*Bandwidth,
	IN	u8				*RateSection,
	IN	u8				*RfPath,
	IN	u8				*Channel,
	IN	u8				*PowerLimit
	)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA( Adapter );
	u8				regulation=0, bandwidth=0, rateSection=0, 
					channel;
	s8 				powerLimit = 0, prevPowerLimit, channelIndex;

	//DBG_871X( "Index of power limit table [band %s][regulation %s][bw %s][rate section %s][rf path %s][chnl %s][val %s]\n", 
	//	  Band, Regulation, Bandwidth, RateSection, RfPath, Channel, PowerLimit );

	if ( !GetU1ByteIntegerFromStringInDecimal( (ps1Byte)Channel, &channel ) ||
		 !GetU1ByteIntegerFromStringInDecimal( (ps1Byte)PowerLimit, &powerLimit ) )
	{
		DBG_871X("Illegal index of power limit table [chnl %s][val %s]\n", Channel, PowerLimit );
	}

	powerLimit = powerLimit > MAX_POWER_INDEX ? MAX_POWER_INDEX : powerLimit;

	if ( eqNByte( Regulation, (u8 *)("FCC"), 3 ) ) regulation = 0;
	else if ( eqNByte( Regulation, (u8 *)("MKK"), 3 ) ) regulation = 1;
	else if ( eqNByte( Regulation, (u8 *)("ETSI"), 4 ) ) regulation = 2;
	else if ( eqNByte( Regulation, (pu1Byte)("WW13"), 4 ) ) regulation = 3;

	if ( eqNByte( RateSection, (u8 *)("CCK"), 3 ) )
		rateSection = 0;
	else if ( eqNByte( RateSection, (u8 *)("OFDM"), 4 ) )
		rateSection = 1;
	else if ( eqNByte( RateSection, (u8 *)("HT"), 2 ) && eqNByte( RfPath, (u8 *)("1T"), 2 ) )
		rateSection = 2;
	else if ( eqNByte( RateSection, (u8 *)("HT"), 2 ) && eqNByte( RfPath, (u8 *)("2T"), 2 ) )
		rateSection = 3;
	else if ( eqNByte( RateSection, (u8 *)("VHT"), 3 ) && eqNByte( RfPath, (u8 *)("1T"), 2 ) )
		rateSection = 4;
	else if ( eqNByte( RateSection, (u8 *)("VHT"), 3 ) && eqNByte( RfPath, (u8 *)("2T"), 2 ) )
		rateSection = 5;
			

	if ( eqNByte( Bandwidth, (u8 *)("20M"), 3 ) ) bandwidth = 0;
	else if ( eqNByte( Bandwidth, (u8 *)("40M"), 3 ) ) bandwidth = 1;
	else if ( eqNByte( Bandwidth, (u8 *)("80M"), 3 ) ) bandwidth = 2;
	else if ( eqNByte( Bandwidth, (u8 *)("160M"), 4 ) ) bandwidth = 3;

	if ( eqNByte( Band, (u8 *)("2.4G"), 4 ) )
	{
		channelIndex = phy_GetChannelIndexOfTxPowerLimit( BAND_ON_2_4G, channel );

		if ( channelIndex == -1 )
			return;

		prevPowerLimit = pHalData->TxPwrLimit_2_4G[regulation][bandwidth][rateSection][channelIndex][ODM_RF_PATH_A];

		if ( powerLimit < prevPowerLimit )
			pHalData->TxPwrLimit_2_4G[regulation][bandwidth][rateSection][channelIndex][ODM_RF_PATH_A] = powerLimit;
		
		//DBG_871X( "2.4G Band value : [regulation %d][bw %d][rate_section %d][chnl %d][val %d]\n", 
		//	  regulation, bandwidth, rateSection, channelIndex, pHalData->TxPwrLimit_2_4G[regulation][bandwidth][rateSection][channelIndex][ODM_RF_PATH_A] );
	}
	else if ( eqNByte( Band, (u8 *)("5G"), 2 ) )
	{
		channelIndex = phy_GetChannelIndexOfTxPowerLimit( BAND_ON_5G, channel );

		if ( channelIndex == -1 )
			return;

		prevPowerLimit = pHalData->TxPwrLimit_5G[regulation][bandwidth][rateSection][channelIndex][ODM_RF_PATH_A];

		if ( powerLimit < prevPowerLimit )
			pHalData->TxPwrLimit_5G[regulation][bandwidth][rateSection][channelIndex][ODM_RF_PATH_A] = powerLimit;

		//DBG_871X( "5G Band value : [regulation %d][bw %d][rate_section %d][chnl %d][val %d]\n", 
		//	  regulation, bandwidth, rateSection, channel, pHalData->TxPwrLimit_5G[regulation][bandwidth][rateSection][channelIndex][ODM_RF_PATH_A] );
	}
	else
	{
		DBG_871X("Cannot recognize the band info in %s\n", Band );
		return;
	}
}

s8
phy_GetWorldWideLimit(
	s8	*LimitTable	
)
{
	s8 min = LimitTable[0];
	u8 i = 0;
	
	for (i = 0; i < MAX_REGULATION_NUM; ++i) {
		if (LimitTable[i] < min)
			min = LimitTable[i];
	}

	return min;
}

u8
PHY_GetTxPowerByRateBaseIndex(
	IN	BAND_TYPE			Band,
	IN	u8					Rate
	)
{
	u8 index = 0;
	if ( Band == BAND_ON_2_4G )
	{
		switch ( Rate )
		{
			case MGN_1M: case MGN_2M: case MGN_5_5M: case MGN_11M:
				index = 0;
				break;

			case MGN_6M: case MGN_9M: case MGN_12M: case MGN_18M:
			case MGN_24M: case MGN_36M: case MGN_48M: case MGN_54M:
				index = 1;
				break;

			case MGN_MCS0: case MGN_MCS1: case MGN_MCS2: case MGN_MCS3: 
			case MGN_MCS4: case MGN_MCS5: case MGN_MCS6: case MGN_MCS7:
				index = 2;
				break;
				
			case MGN_MCS8: case MGN_MCS9: case MGN_MCS10: case MGN_MCS11: 
			case MGN_MCS12: case MGN_MCS13: case MGN_MCS14: case MGN_MCS15:
				index = 3;
				break;

			default:
				DBG_871X("Wrong rate 0x%x to obtain index in 2.4G in PHY_GetTxPowerByRateBaseIndex()\n", Rate );
				break;
		}
	}
	else if ( Band == BAND_ON_5G )
	{
		switch ( Rate )
		{
			case MGN_6M: case MGN_9M: case MGN_12M: case MGN_18M:
			case MGN_24M: case MGN_36M: case MGN_48M: case MGN_54M:
				index = 0;
				break;

			case MGN_MCS0: case MGN_MCS1: case MGN_MCS2: case MGN_MCS3: 
			case MGN_MCS4: case MGN_MCS5: case MGN_MCS6: case MGN_MCS7:
				index = 1;
				break;
				
			case MGN_MCS8: case MGN_MCS9: case MGN_MCS10: case MGN_MCS11: 
			case MGN_MCS12: case MGN_MCS13: case MGN_MCS14: case MGN_MCS15:
				index = 2;
				break;

			case MGN_VHT1SS_MCS0: case MGN_VHT1SS_MCS1: case MGN_VHT1SS_MCS2:
			case MGN_VHT1SS_MCS3: case MGN_VHT1SS_MCS4: case MGN_VHT1SS_MCS5:
			case MGN_VHT1SS_MCS6: case MGN_VHT1SS_MCS7: case MGN_VHT1SS_MCS8:
			case MGN_VHT1SS_MCS9:
				index = 3;
				break;
				
			case MGN_VHT2SS_MCS0: case MGN_VHT2SS_MCS1: case MGN_VHT2SS_MCS2:
			case MGN_VHT2SS_MCS3: case MGN_VHT2SS_MCS4: case MGN_VHT2SS_MCS5:
			case MGN_VHT2SS_MCS6: case MGN_VHT2SS_MCS7: case MGN_VHT2SS_MCS8:
			case MGN_VHT2SS_MCS9:
				index = 4;
				break;

			default:
				DBG_871X("Wrong rate 0x%x to obtain index in 5G in PHY_GetTxPowerByRateBaseIndex()\n", Rate );
				break;
		}
	}

	return index;
}

s8
PHY_GetTxPowerLimit(
	IN	PADAPTER			Adapter,
	IN	u32					RegPwrTblSel,
	IN	BAND_TYPE			Band,
	IN	CHANNEL_WIDTH		Bandwidth,
	IN	u8					RfPath,
	IN	u8					DataRate,
	IN	u8					Channel
	)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	s16				band = -1, regulation = -1, bandwidth = -1,
					rateSection = -1, channel = -1;
	s8				powerLimit = MAX_POWER_INDEX;

	if ( ( Adapter->registrypriv.RegEnableTxPowerLimit == 0 && pHalData->EEPROMRegulatory != 1 ) ||
		  pHalData->EEPROMRegulatory == 2 )
		return MAX_POWER_INDEX;

	switch( Adapter->registrypriv.RegPwrTblSel )
	{
		case 1:
				regulation = TXPWR_LMT_ETSI; 
				break;
		case 2:
				regulation = TXPWR_LMT_MKK;
				break;
		case 3:
				regulation = TXPWR_LMT_FCC;
				break;

		case 4:
				regulation = TXPWR_LMT_WW;
				break;

		default:
				regulation = ( Band == BAND_ON_2_4G ) ? pHalData->Regulation2_4G 
					                                  : pHalData->Regulation5G;
				break;
	}
	
	//DBG_871X("pMgntInfo->RegPwrTblSel %d, final regulation %d\n", Adapter->registrypriv.RegPwrTblSel, regulation );

	
	if ( Band == BAND_ON_2_4G ) band = 0; 
	else if ( Band == BAND_ON_5G ) band = 1; 

	if ( Bandwidth == CHANNEL_WIDTH_20 ) bandwidth = 0;
	else if ( Bandwidth == CHANNEL_WIDTH_40 ) bandwidth = 1;
	else if ( Bandwidth == CHANNEL_WIDTH_80 ) bandwidth = 2;
	else if ( Bandwidth == CHANNEL_WIDTH_160 ) bandwidth = 3;

	switch ( DataRate )
	{
		case MGN_1M: case MGN_2M: case MGN_5_5M: case MGN_11M:
			rateSection = 0;
			break;

		case MGN_6M: case MGN_9M: case MGN_12M: case MGN_18M:
		case MGN_24M: case MGN_36M: case MGN_48M: case MGN_54M:
			rateSection = 1;
			break;

		case MGN_MCS0: case MGN_MCS1: case MGN_MCS2: case MGN_MCS3: 
		case MGN_MCS4: case MGN_MCS5: case MGN_MCS6: case MGN_MCS7:
			rateSection = 2;
			break;
			
		case MGN_MCS8: case MGN_MCS9: case MGN_MCS10: case MGN_MCS11: 
		case MGN_MCS12: case MGN_MCS13: case MGN_MCS14: case MGN_MCS15:
			rateSection = 3;
			break;

		case MGN_VHT1SS_MCS0: case MGN_VHT1SS_MCS1: case MGN_VHT1SS_MCS2:
		case MGN_VHT1SS_MCS3: case MGN_VHT1SS_MCS4: case MGN_VHT1SS_MCS5:
		case MGN_VHT1SS_MCS6: case MGN_VHT1SS_MCS7: case MGN_VHT1SS_MCS8:
		case MGN_VHT1SS_MCS9:
			rateSection = 4;
			break;
			
		case MGN_VHT2SS_MCS0: case MGN_VHT2SS_MCS1: case MGN_VHT2SS_MCS2:
		case MGN_VHT2SS_MCS3: case MGN_VHT2SS_MCS4: case MGN_VHT2SS_MCS5:
		case MGN_VHT2SS_MCS6: case MGN_VHT2SS_MCS7: case MGN_VHT2SS_MCS8:
		case MGN_VHT2SS_MCS9:
			rateSection = 5;
			break;

		default:
			DBG_871X("Wrong rate 0x%x\n", DataRate );
			break;
	}

	if ( Band == BAND_ON_5G  && rateSection == 0 )
			DBG_871X("Wrong rate 0x%x: No CCK in 5G Band\n", DataRate );

	// workaround for wrong index combination to obtain tx power limit, 
	// OFDM only exists in BW 20M
	if ( rateSection == 1 )
		bandwidth = 0;

	// workaround for wrong index combination to obtain tx power limit, 
	// CCK table will only be given in BW 20M
	if ( rateSection == 0 )
		bandwidth = 0;

	// workaround for wrong indxe combination to obtain tx power limit, 
	// HT on 80M will reference to HT on 40M
	if ( ( rateSection == 2 || rateSection == 3 ) && Band == BAND_ON_5G && bandwidth == 2 ) {
		bandwidth = 1;
	}
	
	if ( Band == BAND_ON_2_4G )
		channel = phy_GetChannelIndexOfTxPowerLimit( BAND_ON_2_4G, Channel );
	else if ( Band == BAND_ON_5G )
		channel = phy_GetChannelIndexOfTxPowerLimit( BAND_ON_5G, Channel );
	else if ( Band == BAND_ON_BOTH )
	{
		// BAND_ON_BOTH don't care temporarily 
	}
	
	if ( band == -1 || regulation == -1 || bandwidth == -1 || 
	     rateSection == -1 || channel == -1 )
	{
		//DBG_871X("Wrong index value to access power limit table [band %d][regulation %d][bandwidth %d][rf_path %d][rate_section %d][chnlGroup %d]\n",
		//	  band, regulation, bandwidth, RfPath, rateSection, channelGroup );

		return MAX_POWER_INDEX;
	}

	if ( Band == BAND_ON_2_4G ) {
		s8 limits[10] = {0}; u8 i = 0;
		for (i = 0; i < MAX_REGULATION_NUM; ++i)
			limits[i] = pHalData->TxPwrLimit_2_4G[i][bandwidth][rateSection][channel][RfPath]; 

		powerLimit = (regulation == TXPWR_LMT_WW) ? phy_GetWorldWideLimit(limits) :
			          pHalData->TxPwrLimit_2_4G[regulation][bandwidth][rateSection][channel][RfPath];

	} else if ( Band == BAND_ON_5G ) {
		s8 limits[10] = {0}; u8 i = 0;
		for (i = 0; i < MAX_REGULATION_NUM; ++i)
			limits[i] = pHalData->TxPwrLimit_5G[i][bandwidth][rateSection][channel][RfPath];
		
		powerLimit = (regulation == TXPWR_LMT_WW) ? phy_GetWorldWideLimit(limits) : 
					  pHalData->TxPwrLimit_5G[regulation][bandwidth][rateSection][channel][RfPath];
	} else 
		DBG_871X("No power limit table of the specified band\n" );

	// combine 5G VHT & HT rate
	// 5G 20M and 40M HT and VHT can cross reference
	/*
	if ( Band == BAND_ON_5G && powerLimit == MAX_POWER_INDEX ) {
		if ( bandwidth == 0 || bandwidth == 1 ) { 
			RT_TRACE( COMP_INIT, DBG_LOUD, ( "No power limit table of the specified band %d, bandwidth %d, ratesection %d, rf path %d\n", 
					  band, bandwidth, rateSection, RfPath ) );
			if ( rateSection == 2 )
				powerLimit = pHalData->TxPwrLimit_5G[regulation]
										[bandwidth][4][channelGroup][RfPath];
			else if ( rateSection == 4 )
				powerLimit = pHalData->TxPwrLimit_5G[regulation]
										[bandwidth][2][channelGroup][RfPath];
			else if ( rateSection == 3 )
				powerLimit = pHalData->TxPwrLimit_5G[regulation]
										[bandwidth][5][channelGroup][RfPath];
			else if ( rateSection == 5 )
				powerLimit = pHalData->TxPwrLimit_5G[regulation]
										[bandwidth][3][channelGroup][RfPath];
		}
	}
	*/
	//DBG_871X("TxPwrLmt[Regulation %d][Band %d][BW %d][RFPath %d][Rate 0x%x][Chnl %d] = %d\n", 
	//		regulation, pHalData->CurrentBandType, Bandwidth, RfPath, DataRate, Channel, powerLimit);
	return powerLimit;
}

VOID
phy_CrossReferenceHTAndVHTTxPowerLimit(
	IN	PADAPTER			pAdapter
	)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(pAdapter);
	u8 				regulation, bw, channel, rateSection;	
	s8 				tempPwrLmt = 0;
	
	for ( regulation = 0; regulation < MAX_REGULATION_NUM; ++regulation )
	{
		for ( bw = 0; bw < MAX_5G_BANDWITH_NUM; ++bw )
		{
			for ( channel = 0; channel < CHANNEL_MAX_NUMBER_5G; ++channel )
			{
				for ( rateSection = 0; rateSection < MAX_RATE_SECTION_NUM; ++rateSection )
				{	
					tempPwrLmt = pHalData->TxPwrLimit_5G[regulation][bw][rateSection][channel][ODM_RF_PATH_A];
					if ( tempPwrLmt == MAX_POWER_INDEX )
					{
						if ( bw == 0 || bw == 1 ) { // 5G 20M 40M VHT and HT can cross reference
							//DBG_871X("No power limit table of the specified band %d, bandwidth %d, ratesection %d, channel %d, rf path %d\n",
							//			1, bw, rateSection, channel, ODM_RF_PATH_A );
							if ( rateSection == 2 ) {
								pHalData->TxPwrLimit_5G[regulation][bw][2][channel][ODM_RF_PATH_A] = 
									pHalData->TxPwrLimit_5G[regulation][bw][4][channel][ODM_RF_PATH_A];
							}
							else if ( rateSection == 4 ) {
								pHalData->TxPwrLimit_5G[regulation][bw][4][channel][ODM_RF_PATH_A] = 
									pHalData->TxPwrLimit_5G[regulation][bw][2][channel][ODM_RF_PATH_A];
							}
							else if ( rateSection == 3 ) {
								pHalData->TxPwrLimit_5G[regulation][bw][3][channel][ODM_RF_PATH_A] = 
									pHalData->TxPwrLimit_5G[regulation][bw][5][channel][ODM_RF_PATH_A];
							}
							else if ( rateSection == 5 ) {
								pHalData->TxPwrLimit_5G[regulation][bw][5][channel][ODM_RF_PATH_A] = 
									pHalData->TxPwrLimit_5G[regulation][bw][3][channel][ODM_RF_PATH_A];
							}

							//DBG_871X("use other value %d", tempPwrLmt );
						}
					}
				}
			}
		}
	}
}

VOID 
PHY_ConvertTxPowerLimitToPowerIndex(
	IN	PADAPTER			Adapter
	)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	u8 				BW40PwrBasedBm2_4G, BW40PwrBasedBm5G;
	u8 				regulation, bw, channel, rateSection;	
	u8 				baseIndex2_4G;
	u8				baseIndex5G;
	s8 				tempValue = 0, tempPwrLmt = 0;
	u8 				rfPath = 0;

	//DBG_871X("=====> PHY_ConvertTxPowerLimitToPowerIndex()\n" );

	phy_CrossReferenceHTAndVHTTxPowerLimit( Adapter );

	for ( regulation = 0; regulation < MAX_REGULATION_NUM; ++regulation )
	{
		for ( bw = 0; bw < MAX_2_4G_BANDWITH_NUM; ++bw )
		{
			for ( channel = 0; channel < CHANNEL_MAX_NUMBER_2G; ++channel )
			{						
				for ( rateSection = 0; rateSection < MAX_RATE_SECTION_NUM; ++rateSection )
				{	
					baseIndex2_4G = 0;
					if ( pHalData->odmpriv.PhyRegPgValueType == PHY_REG_PG_EXACT_VALUE ) {
						// obtain the base dBm values in 2.4G band
						// CCK => 11M, OFDM => 54M, HT 1T => MCS7, HT 2T => MCS15
						if ( rateSection == 0 ) { //CCK
							baseIndex2_4G = PHY_GetTxPowerByRateBaseIndex( BAND_ON_2_4G, MGN_11M );
						}
						else if ( rateSection == 1 ) { //OFDM
							baseIndex2_4G = PHY_GetTxPowerByRateBaseIndex( BAND_ON_2_4G, MGN_54M );
						}
						else if ( rateSection == 2 ) { //HT IT
							baseIndex2_4G = PHY_GetTxPowerByRateBaseIndex( BAND_ON_2_4G, MGN_MCS7 );
						}
						else if ( rateSection == 3 ) { //HT 2T
							baseIndex2_4G = PHY_GetTxPowerByRateBaseIndex( BAND_ON_2_4G, MGN_MCS15 );
						}
					}

					tempPwrLmt = pHalData->TxPwrLimit_2_4G[regulation][bw][rateSection][channel][ODM_RF_PATH_A];

					for ( rfPath = ODM_RF_PATH_A; rfPath < MAX_RF_PATH_NUM; ++rfPath )
					{
						if ( pHalData->odmpriv.PhyRegPgValueType == PHY_REG_PG_EXACT_VALUE )
						{
							if ( rateSection == 3 )
								BW40PwrBasedBm2_4G = pHalData->TxPwrByRateBase2_4G[rfPath][RF_2TX][baseIndex2_4G];
							else
								BW40PwrBasedBm2_4G = pHalData->TxPwrByRateBase2_4G[rfPath][RF_1TX][baseIndex2_4G];
						}
						else
							BW40PwrBasedBm2_4G = Adapter->registrypriv.RegPowerBase * 2;

						if ( tempPwrLmt != MAX_POWER_INDEX ) {
							tempValue = tempPwrLmt - BW40PwrBasedBm2_4G;
							pHalData->TxPwrLimit_2_4G[regulation][bw][rateSection][channel][rfPath] = tempValue;
						}
						
						/*DBG_871X("TxPwrLimit_2_4G[regulation %d][bw %d][rateSection %d][channel %d] %d=\n\
							(TxPwrLimit in dBm %d - BW40PwrLmt2_4G[chnl group %d][rfPath %d] %d) \n",
							regulation, bw, rateSection, group, pHalData->TxPwrLimit_2_4G[regulation][bw][rateSection][channel][rfPath], 
							tempPwrLmt, group, rfPath, BW40PwrBasedBm2_4G );*/
					}
				}
			}
		}
	}
	
	if ( IS_HARDWARE_TYPE_8812( Adapter ) || IS_HARDWARE_TYPE_8821( Adapter ) )
	{
		for ( regulation = 0; regulation < MAX_REGULATION_NUM; ++regulation )
		{
			for ( bw = 0; bw < MAX_5G_BANDWITH_NUM; ++bw )
			{
				for ( channel = 0; channel < CHANNEL_MAX_NUMBER_5G; ++channel )
				{
					for ( rateSection = 0; rateSection < MAX_RATE_SECTION_NUM; ++rateSection )
					{	
						baseIndex5G = 0;
						if ( pHalData->odmpriv.PhyRegPgValueType == PHY_REG_PG_EXACT_VALUE ) {
							// obtain the base dBm values in 5G band
							// OFDM => 54M, HT 1T => MCS7, HT 2T => MCS15, 
							// VHT => 1SSMCS7, VHT 2T => 2SSMCS7
							if ( rateSection == 1 ) { //OFDM
								baseIndex5G = PHY_GetTxPowerByRateBaseIndex( BAND_ON_5G, MGN_54M );
							}
							else if ( rateSection == 2 ) { //HT 1T
								baseIndex5G = PHY_GetTxPowerByRateBaseIndex( BAND_ON_5G, MGN_MCS7 );
							}
							else if ( rateSection == 3 ) { //HT 2T
								baseIndex5G = PHY_GetTxPowerByRateBaseIndex( BAND_ON_5G, MGN_MCS15 );
							}
							else if ( rateSection == 4 ) { //VHT 1T
								baseIndex5G = PHY_GetTxPowerByRateBaseIndex( BAND_ON_5G, MGN_VHT1SS_MCS7 );
							}
							else if ( rateSection == 5 ) { //VHT 2T
								baseIndex5G = PHY_GetTxPowerByRateBaseIndex( BAND_ON_5G, MGN_VHT2SS_MCS7 );
							}
						}

						tempPwrLmt = pHalData->TxPwrLimit_5G[regulation][bw][rateSection][channel][ODM_RF_PATH_A];
	
						for ( rfPath = ODM_RF_PATH_A; rfPath < MAX_RF_PATH_NUM; ++rfPath )
						{
							if ( pHalData->odmpriv.PhyRegPgValueType == PHY_REG_PG_EXACT_VALUE )
							{
								if ( rateSection == 3 || rateSection == 5 )
									BW40PwrBasedBm5G = pHalData->TxPwrByRateBase5G[rfPath][RF_2TX][baseIndex5G];
								else
									BW40PwrBasedBm5G = pHalData->TxPwrByRateBase5G[rfPath][RF_1TX][baseIndex5G];
							}
							else
								BW40PwrBasedBm5G = Adapter->registrypriv.RegPowerBase * 2;

							if ( tempPwrLmt != MAX_POWER_INDEX ) {
								tempValue = tempPwrLmt - BW40PwrBasedBm5G;
								pHalData->TxPwrLimit_5G[regulation][bw][rateSection][channel][rfPath] = tempValue;
							}
							
							/*DBG_871X("TxPwrLimit_5G[regulation %d][bw %d][rateSection %d][channel %d] %d=\n\
								(TxPwrLimit in dBm %d - BW40PwrLmt5G[chnl group %d][rfPath %d] %d) \n",
								regulation, bw, rateSection, group, pHalData->TxPwrLimit_5G[regulation][bw][rateSection][channel][rfPath], 
								tempPwrLmt, group, rfPath, BW40PwrBasedBm5G );*/
						}
					}
				}
			}
		}
	}
	//DBG_871X("<===== PHY_ConvertTxPowerLimitToPowerIndex()\n" );
}

VOID
PHY_InitTxPowerLimit(
	IN	PADAPTER		Adapter
	)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	u8				i, j, k, l, m;

	//DBG_871X("=====> PHY_InitTxPowerLimit()!\n" );
	
	for ( i = 0; i < MAX_REGULATION_NUM; ++i )
	{
		for ( j = 0; j < MAX_2_4G_BANDWITH_NUM; ++j )
			for ( k = 0; k < MAX_RATE_SECTION_NUM; ++k )
				for ( m = 0; m < CHANNEL_MAX_NUMBER_2G; ++m )
					for ( l = 0; l < MAX_RF_PATH_NUM; ++l )
						pHalData->TxPwrLimit_2_4G[i][j][k][m][l] = MAX_POWER_INDEX;
	}

	for ( i = 0; i < MAX_REGULATION_NUM; ++i )
    {
		for ( j = 0; j < MAX_5G_BANDWITH_NUM; ++j )
			for ( k = 0; k < MAX_RATE_SECTION_NUM; ++k )
				for ( m = 0; m < CHANNEL_MAX_NUMBER_5G; ++m )
					for ( l = 0; l < MAX_RF_PATH_NUM; ++l )
						pHalData->TxPwrLimit_5G[i][j][k][m][l] = MAX_POWER_INDEX;
    }
	
	//DBG_871X("<===== PHY_InitTxPowerLimit()!\n" );
}

VOID
Hal_ChannelPlanToRegulation(
	IN	PADAPTER		Adapter,
	IN	u16				ChannelPlan
	)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	pHalData->Regulation2_4G = TXPWR_LMT_FCC;
	pHalData->Regulation5G = TXPWR_LMT_FCC;

	switch(ChannelPlan)
	{	
		case RT_CHANNEL_DOMAIN_WORLD_NULL:
			pHalData->Regulation2_4G = TXPWR_LMT_WW;	
			break;
		case RT_CHANNEL_DOMAIN_ETSI1_NULL:
			pHalData->Regulation2_4G = TXPWR_LMT_ETSI;
			break;
		case RT_CHANNEL_DOMAIN_FCC1_NULL:
			pHalData->Regulation2_4G = TXPWR_LMT_FCC;
			break;
		case RT_CHANNEL_DOMAIN_MKK1_NULL:
			pHalData->Regulation2_4G = TXPWR_LMT_MKK;
			break;
		case RT_CHANNEL_DOMAIN_ETSI2_NULL:
			pHalData->Regulation2_4G = TXPWR_LMT_ETSI;
			break;
		case RT_CHANNEL_DOMAIN_FCC1_FCC1:
			pHalData->Regulation2_4G = TXPWR_LMT_FCC;
			pHalData->Regulation5G = TXPWR_LMT_FCC;
			break;
		case RT_CHANNEL_DOMAIN_WORLD_ETSI1:
			pHalData->Regulation2_4G = TXPWR_LMT_FCC;
			pHalData->Regulation5G = TXPWR_LMT_ETSI;
			break;
		case RT_CHANNEL_DOMAIN_MKK1_MKK1:
			pHalData->Regulation2_4G = TXPWR_LMT_MKK;
			pHalData->Regulation5G = TXPWR_LMT_MKK;
			break;
		case RT_CHANNEL_DOMAIN_WORLD_KCC1:
			pHalData->Regulation2_4G = TXPWR_LMT_FCC;
			pHalData->Regulation5G = TXPWR_LMT_MKK;
			break;
		case RT_CHANNEL_DOMAIN_WORLD_FCC2:
			pHalData->Regulation2_4G = TXPWR_LMT_FCC;
			pHalData->Regulation5G = TXPWR_LMT_FCC;
					break;
		case RT_CHANNEL_DOMAIN_WORLD_FCC3:
			pHalData->Regulation2_4G = TXPWR_LMT_FCC;
			pHalData->Regulation5G = TXPWR_LMT_FCC;
			break;
		case RT_CHANNEL_DOMAIN_WORLD_FCC4:
			pHalData->Regulation2_4G = TXPWR_LMT_FCC;
			pHalData->Regulation5G = TXPWR_LMT_FCC;
			break;
		case RT_CHANNEL_DOMAIN_WORLD_FCC5:
			pHalData->Regulation2_4G = TXPWR_LMT_FCC;
			pHalData->Regulation5G = TXPWR_LMT_FCC;
			break;
		case RT_CHANNEL_DOMAIN_WORLD_FCC6:
			pHalData->Regulation2_4G = TXPWR_LMT_FCC;
			pHalData->Regulation5G = TXPWR_LMT_FCC;
			break;
		case RT_CHANNEL_DOMAIN_FCC1_FCC7:
			pHalData->Regulation2_4G = TXPWR_LMT_FCC;
			pHalData->Regulation5G = TXPWR_LMT_FCC;
			break;
		case RT_CHANNEL_DOMAIN_WORLD_ETSI2:
			pHalData->Regulation2_4G = TXPWR_LMT_FCC;
			pHalData->Regulation5G = TXPWR_LMT_FCC;
			break;
		case RT_CHANNEL_DOMAIN_WORLD_ETSI3:
			pHalData->Regulation2_4G = TXPWR_LMT_FCC;
			pHalData->Regulation5G = TXPWR_LMT_FCC;
			break;
		case RT_CHANNEL_DOMAIN_MKK1_MKK2:
			pHalData->Regulation2_4G = TXPWR_LMT_MKK;
			pHalData->Regulation5G = TXPWR_LMT_FCC;
			break;
		case RT_CHANNEL_DOMAIN_MKK1_MKK3:
			pHalData->Regulation2_4G = TXPWR_LMT_MKK;
			pHalData->Regulation5G = TXPWR_LMT_FCC;
			break;
		case RT_CHANNEL_DOMAIN_FCC1_NCC1:
			pHalData->Regulation2_4G = TXPWR_LMT_FCC;
			pHalData->Regulation5G = TXPWR_LMT_FCC;
			break;
		case RT_CHANNEL_DOMAIN_FCC1_NCC2:
			pHalData->Regulation2_4G = TXPWR_LMT_FCC;
			pHalData->Regulation5G = TXPWR_LMT_FCC;
			break;
		case RT_CHANNEL_DOMAIN_GLOBAL_NULL:
			pHalData->Regulation2_4G = TXPWR_LMT_WW;
			pHalData->Regulation5G = TXPWR_LMT_WW;
			break;
		case RT_CHANNEL_DOMAIN_ETSI1_ETSI4:
			pHalData->Regulation2_4G = TXPWR_LMT_ETSI;
			pHalData->Regulation5G = TXPWR_LMT_ETSI;
			break;
		case RT_CHANNEL_DOMAIN_FCC1_FCC2:
			pHalData->Regulation2_4G = TXPWR_LMT_FCC;
			pHalData->Regulation5G = TXPWR_LMT_FCC;
			break;
		case RT_CHANNEL_DOMAIN_FCC1_NCC3:
			pHalData->Regulation2_4G = TXPWR_LMT_FCC;
			pHalData->Regulation5G = TXPWR_LMT_FCC;
			break;
		case RT_CHANNEL_DOMAIN_WORLD_ETSI5:
			pHalData->Regulation2_4G = TXPWR_LMT_ETSI;
			pHalData->Regulation5G = TXPWR_LMT_ETSI;
			break;
		case RT_CHANNEL_DOMAIN_FCC1_FCC8:
			pHalData->Regulation2_4G = TXPWR_LMT_FCC;
			pHalData->Regulation5G = TXPWR_LMT_FCC;
		case RT_CHANNEL_DOMAIN_REALTEK_DEFINE: //Realtek Reserve
			pHalData->Regulation2_4G = TXPWR_LMT_WW;
			pHalData->Regulation5G = TXPWR_LMT_WW;
			break;
		default:
			break;
	}
}

#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE

extern char *rtw_phy_file_path;
char	file_path[PATH_LENGTH_MAX];

#define GetLineFromBuffer(buffer)	 strsep(&buffer, "\n")

int
phy_ConfigMACWithParaFile(
	IN	PADAPTER	Adapter,
	IN	char* 		pFileName
)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	int	rlen = 0, rtStatus = _FAIL;
	char	*szLine, *ptmp;
	u32	u4bRegOffset, u4bRegValue, u4bMove;

	if(!(Adapter->registrypriv.load_phy_file & LOAD_MAC_PARA_FILE))
		return rtStatus;

	_rtw_memset(pHalData->para_file_buf, 0, MAX_PARA_FILE_BUF_LEN);

	if ((pHalData->mac_reg_len == 0) && (pHalData->mac_reg == NULL))
	{
		rtw_merge_string(file_path, PATH_LENGTH_MAX, rtw_phy_file_path, pFileName);
	
		if (rtw_is_file_readable(file_path) == _TRUE)
		{
			rlen = rtw_retrive_from_file(file_path, pHalData->para_file_buf, MAX_PARA_FILE_BUF_LEN);
			if (rlen > 0)
			{
				rtStatus = _SUCCESS;
				pHalData->mac_reg = rtw_zmalloc(rlen);
				if(pHalData->mac_reg) {
					_rtw_memcpy(pHalData->mac_reg, pHalData->para_file_buf, rlen);
					pHalData->mac_reg_len = rlen;
				}
				else {
					DBG_871X("%s mac_reg alloc fail !\n",__FUNCTION__);
				}
			}
		}
	}
	else
	{
		if ((pHalData->mac_reg_len != 0) && (pHalData->mac_reg != NULL)) {
			_rtw_memcpy(pHalData->para_file_buf, pHalData->mac_reg, pHalData->mac_reg_len);
			rtStatus = _SUCCESS;
		}
		else {
			DBG_871X("%s(): Critical Error !!!\n",__FUNCTION__);
		}
	}

	if (rtStatus == _SUCCESS)
	{
		ptmp = pHalData->para_file_buf;
		for (szLine = GetLineFromBuffer(ptmp); szLine != NULL; szLine = GetLineFromBuffer(ptmp))
		{
			if(!IsCommentString(szLine))
			{
				// Get 1st hex value as register offset
				if(GetHexValueFromString(szLine, &u4bRegOffset, &u4bMove))
				{
					if(u4bRegOffset == 0xffff)
					{ // Ending.
						break;
					}

					// Get 2nd hex value as register value.
					szLine += u4bMove;
					if(GetHexValueFromString(szLine, &u4bRegValue, &u4bMove))
					{
						rtw_write8(Adapter, u4bRegOffset, (u8)u4bRegValue);
					}
				}
			}
		}
	}

	return rtStatus;
}

int
phy_ConfigBBWithParaFile(
	IN	PADAPTER	Adapter,
	IN	char*		pFileName,
	IN	u32			ConfigType
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	int	rlen = 0, rtStatus = _FAIL;
	char	*szLine, *ptmp;
	u32	u4bRegOffset, u4bRegValue, u4bMove;
	char	*pBuf = NULL;
	u32	*pBufLen = NULL;

	if(!(Adapter->registrypriv.load_phy_file & LOAD_BB_PARA_FILE))
		return rtStatus;

	switch(ConfigType)
	{
		case CONFIG_BB_PHY_REG:
			pBuf = pHalData->bb_phy_reg;
			pBufLen = &pHalData->bb_phy_reg_len;
			break;
		case CONFIG_BB_AGC_TAB:
			pBuf = pHalData->bb_agc_tab;
			pBufLen = &pHalData->bb_agc_tab_len;
			break;
		default:
			DBG_871X("Unknown ConfigType!! %d\r\n", ConfigType);
			break;
	}

	_rtw_memset(pHalData->para_file_buf, 0, MAX_PARA_FILE_BUF_LEN);

	if ((*pBufLen == 0) && (pBuf == NULL))
	{
		rtw_merge_string(file_path, PATH_LENGTH_MAX, rtw_phy_file_path, pFileName);
	
		if (rtw_is_file_readable(file_path) == _TRUE)
		{
			rlen = rtw_retrive_from_file(file_path, pHalData->para_file_buf, MAX_PARA_FILE_BUF_LEN);
			if (rlen > 0)
			{
				rtStatus = _SUCCESS;
				pBuf = rtw_zmalloc(rlen);
				if(pBuf) {
					_rtw_memcpy(pBuf, pHalData->para_file_buf, rlen);
					*pBufLen = rlen;

					switch(ConfigType)
					{
						case CONFIG_BB_PHY_REG:
							pHalData->bb_phy_reg = pBuf;
							break;
						case CONFIG_BB_AGC_TAB:
							pHalData->bb_agc_tab = pBuf;
							break;
						default:
							DBG_871X("Unknown ConfigType!! %d\r\n", ConfigType);
							rtw_mfree(pBuf, rlen);
							break;
					}
				}
				else {
					DBG_871X("%s(): ConfigType %d  alloc fail !\n",__FUNCTION__,ConfigType);
				}
			}
		}
	}
	else
	{
		if ((*pBufLen != 0) && (pBuf != NULL)) {
			_rtw_memcpy(pHalData->para_file_buf, pBuf, *pBufLen);
			rtStatus = _SUCCESS;
		}
		else {
			DBG_871X("%s(): Critical Error !!!\n",__FUNCTION__);
		}
	}

	if (rtStatus == _SUCCESS)
	{
		ptmp = pHalData->para_file_buf;
		for (szLine = GetLineFromBuffer(ptmp); szLine != NULL; szLine = GetLineFromBuffer(ptmp))
		{
			if(!IsCommentString(szLine))
			{
				// Get 1st hex value as register offset.
				if(GetHexValueFromString(szLine, &u4bRegOffset, &u4bMove))
				{
					if(u4bRegOffset == 0xffff)
					{ // Ending.
						break;
					}
					else if (u4bRegOffset == 0xfe || u4bRegOffset == 0xffe)
					{
						#ifdef CONFIG_LONG_DELAY_ISSUE
						rtw_msleep_os(50);
						#else
						rtw_mdelay_os(50);
						#endif
					}
					else if (u4bRegOffset == 0xfd)
					{
						rtw_mdelay_os(5);
					}
					else if (u4bRegOffset == 0xfc)
					{
						rtw_mdelay_os(1);
					}
					else if (u4bRegOffset == 0xfb)
					{
						rtw_udelay_os(50);
					}
					else if (u4bRegOffset == 0xfa)
					{
						rtw_udelay_os(5);
					}
					else if (u4bRegOffset == 0xf9)
					{
						rtw_udelay_os(1);
					}
					
					// Get 2nd hex value as register value.
					szLine += u4bMove;
					if(GetHexValueFromString(szLine, &u4bRegValue, &u4bMove))
					{
						//DBG_871X("[BB-ADDR]%03lX=%08lX\n", u4bRegOffset, u4bRegValue);
						PHY_SetBBReg(Adapter, u4bRegOffset, bMaskDWord, u4bRegValue);

						if (u4bRegOffset == 0xa24)
							pHalData->odmpriv.RFCalibrateInfo.RegA24 = u4bRegValue;

						// Add 1us delay between BB/RF register setting.
						rtw_udelay_os(1);
					}
				}
			}
		}
	}
	else
	{
		DBG_871X("phy_ConfigBBWithParaFile(): Failed to read %s!\n", pFileName);
	}

	return rtStatus;
}

VOID
phy_DecryptBBPgParaFile(
	PADAPTER		Adapter,
	char*			buffer
	)
{
	u32	i = 0, j = 0;
	u8	map[95] = {0};
	u8	currentChar;
	char	*BufOfLines, *ptmp;

	//DBG_871X("=====>phy_DecryptBBPgParaFile()\n");
	// 32 the ascii code of the first visable char, 126 the last one
	for ( i = 0; i < 95; ++i )
		map[i] = ( u8 ) ( 94 - i );

	ptmp = buffer;
	i = 0;
	for (BufOfLines = GetLineFromBuffer(ptmp); BufOfLines != NULL; BufOfLines = GetLineFromBuffer(ptmp))
	{
		//DBG_871X("Encrypted Line: %s\n", BufOfLines);

		for ( j = 0; j < strlen(BufOfLines); ++j )
		{
			currentChar = BufOfLines[j];

			if ( currentChar == '\0' )
				break;

			currentChar -=  (u8) ( ( ( ( i + j ) * 3 ) % 128 ) );
			
			BufOfLines[j] = map[currentChar - 32] + 32;
		}
		//DBG_871X("Decrypted Line: %s\n", BufOfLines );
		if (strlen(BufOfLines) != 0)
			i++;
		BufOfLines[strlen(BufOfLines)] = '\n';
	}
}

int
phy_ParseBBPgParaFile(
	PADAPTER		Adapter,
	char*			buffer
	)
{
	int	rtStatus = _SUCCESS;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	char	*szLine, *ptmp;
	u32	u4bRegOffset, u4bRegMask, u4bRegValue;
	u32	u4bMove;
	BOOLEAN firstLine = _TRUE;
	u8	tx_num = 0;
	u8	band = 0, rf_path = 0;

	//DBG_871X("=====>phy_ParseBBPgParaFile()\n");
	
	if ( Adapter->registrypriv.RegDecryptCustomFile == 1 )
		phy_DecryptBBPgParaFile( Adapter, buffer);

	ptmp = buffer;
	for (szLine = GetLineFromBuffer(ptmp); szLine != NULL; szLine = GetLineFromBuffer(ptmp))
	{
		if(!IsCommentString(szLine))
		{
			if( isAllSpaceOrTab( szLine, sizeof( szLine ) ) )
				continue;

			// Get header info (relative value or exact value)
			if ( firstLine )
			{
				if ( eqNByte( szLine, (u8 *)("#[v1]"), 5 ) )
				{
					
					pHalData->odmpriv.PhyRegPgVersion = szLine[3] - '0';
					//DBG_871X("This is a new format PHY_REG_PG.txt \n");
				}
				else if ( eqNByte( szLine, (u8 *)("#[v0]"), 5 ))
				{
					pHalData->odmpriv.PhyRegPgVersion = szLine[3] - '0';
					//DBG_871X("This is a old format PHY_REG_PG.txt ok\n");
				}
				else
				{
					DBG_871X("The format in PHY_REG_PG are invalid %s\n", szLine);
					return _FAIL;
				}
					
				if ( eqNByte( szLine + 5, (u8 *)("[Exact]#"), 8 ) )
				{
					pHalData->odmpriv.PhyRegPgValueType = PHY_REG_PG_EXACT_VALUE;
					//DBG_871X("The values in PHY_REG_PG are exact values ok\n");
					firstLine = _FALSE;
					continue;
				}
				else if ( eqNByte( szLine + 5, (pu1Byte)("[Relative]#"), 11 ) )
				{
					pHalData->odmpriv.PhyRegPgValueType = PHY_REG_PG_RELATIVE_VALUE;
					//DBG_871X("The values in PHY_REG_PG are relative values ok\n");
					firstLine = _FALSE;
					continue;
				}
				else
				{
					DBG_871X("The values in PHY_REG_PG are invalid %s\n", szLine);
					return _FAIL;
				}
			}

			if ( pHalData->odmpriv.PhyRegPgVersion == 0 )
			{
				// Get 1st hex value as register offset.
				if(GetHexValueFromString(szLine, &u4bRegOffset, &u4bMove))
				{
					szLine += u4bMove;
					if(u4bRegOffset == 0xffff)
					{ // Ending.
						break;
					}

					// Get 2nd hex value as register mask.
					if ( GetHexValueFromString(szLine, &u4bRegMask, &u4bMove) )
						szLine += u4bMove;
					else
						return _FAIL;

					if ( pHalData->odmpriv.PhyRegPgValueType == PHY_REG_PG_RELATIVE_VALUE ) 
					{
						// Get 3rd hex value as register value.
						if(GetHexValueFromString(szLine, &u4bRegValue, &u4bMove))
						{
							PHY_StoreTxPowerByRate(Adapter, 0, 0, 1, u4bRegOffset, u4bRegMask, u4bRegValue);
							//DBG_871X("[ADDR] %03X=%08X Mask=%08x\n", u4bRegOffset, u4bRegValue, u4bRegMask);
						}
						else
						{
							return _FAIL;
						}
					}
					else if ( pHalData->odmpriv.PhyRegPgValueType == PHY_REG_PG_EXACT_VALUE ) 
					{
						u32	combineValue = 0;
						u8	integer = 0, fraction = 0;
						
						if ( GetFractionValueFromString( szLine, &integer, &fraction, &u4bMove ) )
							szLine += u4bMove;
						else 
							return _FAIL;
						
						integer *= 2;
						if ( fraction == 5 ) integer += 1;
						combineValue |= ( ( ( integer / 10 ) << 4 ) + ( integer % 10 ) );
						//DBG_871X(" %d", integer );

						if ( GetFractionValueFromString( szLine, &integer, &fraction, &u4bMove ) )
							szLine += u4bMove;
						else 
							return _FAIL;

						integer *= 2;
						if ( fraction == 5 ) integer += 1;
						combineValue <<= 8;
						combineValue |= ( ( ( integer / 10 ) << 4 ) + ( integer % 10 ) );
						//DBG_871X(" %d", integer );

						if ( GetFractionValueFromString( szLine, &integer, &fraction, &u4bMove ) )
							szLine += u4bMove;
						else
							return _FAIL;
						
						integer *= 2;
						if ( fraction == 5 ) integer += 1;
						combineValue <<= 8;
						combineValue |= ( ( ( integer / 10 ) << 4 ) + ( integer % 10 ) );
						//DBG_871X(" %d", integer );

						if ( GetFractionValueFromString( szLine, &integer, &fraction, &u4bMove ) )
							szLine += u4bMove;
						else 
							return _FAIL;

						integer *= 2;
						if ( fraction == 5 ) integer += 1;
						combineValue <<= 8;
						combineValue |= ( ( ( integer / 10 ) << 4 ) + ( integer % 10 ) );
						//DBG_871X(" %d", integer );
						PHY_StoreTxPowerByRate(Adapter, 0, 0, 1, u4bRegOffset, u4bRegMask, combineValue);

						//DBG_871X("[ADDR] 0x%3x = 0x%4x\n", u4bRegOffset, combineValue );
					}
				}
			}
			else if ( pHalData->odmpriv.PhyRegPgVersion > 0 )
			{
				u32	index = 0, cnt = 0;

				if ( eqNByte( szLine, "0xffff", 6 ) )
					break;

				if( !eqNByte( "#[END]#", szLine, 7 ) )
				{
					// load the table label info
					if ( szLine[0] == '#' )
					{
						index = 0;
						if ( eqNByte( szLine, "#[2.4G]" , 7 ) )
						{
							band = BAND_ON_2_4G;
							index += 8;
						}
						else if ( eqNByte( szLine, "#[5G]", 5) )
						{
							band = BAND_ON_5G;
							index += 6;
						}
						else
						{
							DBG_871X("Invalid band %s in PHY_REG_PG.txt \n", szLine );
							return _FAIL;
						}

						rf_path= szLine[index] - 'A';
						//DBG_871X(" Table label Band %d, RfPath %d\n", band, rf_path );
					}
					else // load rows of tables
					{
						if ( szLine[1] == '1' )
							tx_num = RF_1TX;
						else if ( szLine[1] == '2' )
							tx_num = RF_2TX;
						else
						{
							DBG_871X("Invalid row in PHY_REG_PG.txt %c\n", szLine[1] );
							return _FAIL;
						}

						while ( szLine[index] != ']' )
							++index;
						++index;// skip ]

						// Get 2nd hex value as register offset.
						szLine += index;
						if ( GetHexValueFromString(szLine, &u4bRegOffset, &u4bMove) )
							szLine += u4bMove;
						else
							return _FAIL;

						// Get 2nd hex value as register mask.
						if ( GetHexValueFromString(szLine, &u4bRegMask, &u4bMove) )
							szLine += u4bMove;
						else
							return _FAIL;

						if ( pHalData->odmpriv.PhyRegPgValueType == PHY_REG_PG_RELATIVE_VALUE ) 
						{
							// Get 3rd hex value as register value.
							if(GetHexValueFromString(szLine, &u4bRegValue, &u4bMove))
							{
								PHY_StoreTxPowerByRate(Adapter, band, rf_path, tx_num, u4bRegOffset, u4bRegMask, u4bRegValue);
								//DBG_871X("[ADDR] %03X (tx_num %d) =%08X Mask=%08x\n", u4bRegOffset, tx_num, u4bRegValue, u4bRegMask);
							}
							else
							{
								return _FAIL;
							}
						}
						else if ( pHalData->odmpriv.PhyRegPgValueType == PHY_REG_PG_EXACT_VALUE ) 
						{
							u32	combineValue = 0;
							u8	integer = 0, fraction = 0;

							if ( GetFractionValueFromString( szLine, &integer, &fraction, &u4bMove ) )
								szLine += u4bMove;
							else
								return _FAIL;

							integer *= 2;
							if ( fraction == 5 ) integer += 1;
							combineValue |= ( ( ( integer / 10 ) << 4 ) + ( integer % 10 ) );
							//DBG_871X(" %d", integer );

							if ( GetFractionValueFromString( szLine, &integer, &fraction, &u4bMove ) )
								szLine += u4bMove;
							else
								return _FAIL;

							integer *= 2;
							if ( fraction == 5 ) integer += 1;
							combineValue <<= 8;
							combineValue |= ( ( ( integer / 10 ) << 4 ) + ( integer % 10 ) );
							//DBG_871X(" %d", integer );

							if ( GetFractionValueFromString( szLine, &integer, &fraction, &u4bMove ) )
								szLine += u4bMove;
							else
								return _FAIL;

							integer *= 2;
							if ( fraction == 5 ) integer += 1;
							combineValue <<= 8;
							combineValue |= ( ( ( integer / 10 ) << 4 ) + ( integer % 10 ) );
							//DBG_871X(" %d", integer );

							if ( GetFractionValueFromString( szLine, &integer, &fraction, &u4bMove ) )
								szLine += u4bMove;
							else
								return _FAIL;

							integer *= 2;
							if ( fraction == 5 ) integer += 1;
							combineValue <<= 8;
							combineValue |= ( ( ( integer / 10 ) << 4 ) + ( integer % 10 ) );
							//DBG_871X(" %d", integer );
							PHY_StoreTxPowerByRate(Adapter, band, rf_path, tx_num, u4bRegOffset, u4bRegMask, combineValue);

							//DBG_871X("[ADDR] 0x%3x (tx_num %d) = 0x%4x\n", u4bRegOffset, tx_num, combineValue );
						}
					}
				}
			}
		}
	}
	//DBG_871X("<=====phy_ParseBBPgParaFile()\n");
	return rtStatus;
}

int
phy_ConfigBBWithPgParaFile(
	IN	PADAPTER	Adapter,
	IN	char* 		pFileName)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	int	rlen = 0, rtStatus = _FAIL;

	if(!(Adapter->registrypriv.load_phy_file & LOAD_BB_PG_PARA_FILE))
		return rtStatus;

	_rtw_memset(pHalData->para_file_buf, 0, MAX_PARA_FILE_BUF_LEN);

	if ((pHalData->bb_phy_reg_pg_len == 0) && (pHalData->bb_phy_reg_pg == NULL))
	{
		rtw_merge_string(file_path, PATH_LENGTH_MAX, rtw_phy_file_path, pFileName);
	
		if (rtw_is_file_readable(file_path) == _TRUE)
		{
			rlen = rtw_retrive_from_file(file_path, pHalData->para_file_buf, MAX_PARA_FILE_BUF_LEN);
			if (rlen > 0)
			{
				rtStatus = _SUCCESS;
				pHalData->bb_phy_reg_pg = rtw_zmalloc(rlen);
				if(pHalData->bb_phy_reg_pg) {
					_rtw_memcpy(pHalData->bb_phy_reg_pg, pHalData->para_file_buf, rlen);
					pHalData->bb_phy_reg_pg_len = rlen;
				}
				else {
					DBG_871X("%s bb_phy_reg_pg alloc fail !\n",__FUNCTION__);
				}
			}
		}
	}
	else
	{
		if ((pHalData->bb_phy_reg_pg_len != 0) && (pHalData->bb_phy_reg_pg != NULL)) {
			_rtw_memcpy(pHalData->para_file_buf, pHalData->bb_phy_reg_pg, pHalData->bb_phy_reg_pg_len);
			rtStatus = _SUCCESS;
		}
		else {
			DBG_871X("%s(): Critical Error !!!\n",__FUNCTION__);
		}
	}

	if(rtStatus == _SUCCESS)
	{
		//DBG_871X("phy_ConfigBBWithPgParaFile(): read %s ok\n", pFileName);
		phy_ParseBBPgParaFile(Adapter, pHalData->para_file_buf);
	}
	else
	{
		DBG_871X("phy_ConfigBBWithPgParaFile(): Failed%s\n", pFileName);
	}

	return rtStatus;
}

#if (MP_DRIVER == 1 )

int
phy_ConfigBBWithMpParaFile(
	IN	PADAPTER	Adapter,
	IN	char* 		pFileName
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	int	rlen = 0, rtStatus = _FAIL;
	char	*szLine, *ptmp;
	u32	u4bRegOffset, u4bRegValue, u4bMove;

	if(!(Adapter->registrypriv.load_phy_file & LOAD_BB_MP_PARA_FILE))
		return rtStatus;

	_rtw_memset(pHalData->para_file_buf, 0, MAX_PARA_FILE_BUF_LEN);

	if ((pHalData->bb_phy_reg_mp_len == 0) && (pHalData->bb_phy_reg_mp == NULL))
	{
		rtw_merge_string(file_path, PATH_LENGTH_MAX, rtw_phy_file_path, pFileName);
	
		if (rtw_is_file_readable(file_path) == _TRUE)
		{
			rlen = rtw_retrive_from_file(file_path, pHalData->para_file_buf, MAX_PARA_FILE_BUF_LEN);
			if (rlen > 0)
			{
				rtStatus = _SUCCESS;
				pHalData->bb_phy_reg_mp = rtw_zmalloc(rlen);
				if(pHalData->bb_phy_reg_mp) {
					_rtw_memcpy(pHalData->bb_phy_reg_mp, pHalData->para_file_buf, rlen);
					pHalData->bb_phy_reg_mp_len = rlen;
				}
				else {
					DBG_871X("%s bb_phy_reg_mp alloc fail !\n",__FUNCTION__);
				}
			}
		}
	}
	else
	{
		if ((pHalData->bb_phy_reg_mp_len != 0) && (pHalData->bb_phy_reg_mp != NULL)) {
			_rtw_memcpy(pHalData->para_file_buf, pHalData->bb_phy_reg_mp, pHalData->bb_phy_reg_mp_len);
			rtStatus = _SUCCESS;
		}
		else {
			DBG_871X("%s(): Critical Error !!!\n",__FUNCTION__);
		}
	}

	if(rtStatus == _SUCCESS)
	{
		//DBG_871X("phy_ConfigBBWithMpParaFile(): read %s ok\n", pFileName);

		ptmp = pHalData->para_file_buf;
		for (szLine = GetLineFromBuffer(ptmp); szLine != NULL; szLine = GetLineFromBuffer(ptmp))
		{
			if(!IsCommentString(szLine))
			{
				// Get 1st hex value as register offset.
				if(GetHexValueFromString(szLine, &u4bRegOffset, &u4bMove))
				{
					if(u4bRegOffset == 0xffff)
					{ // Ending.
						break;
					}
					else if (u4bRegOffset == 0xfe || u4bRegOffset == 0xffe)
					{
						#ifdef CONFIG_LONG_DELAY_ISSUE
						rtw_msleep_os(50);
						#else
						rtw_mdelay_os(50);
						#endif
					}
					else if (u4bRegOffset == 0xfd)
					{
						rtw_mdelay_os(5);
					}
					else if (u4bRegOffset == 0xfc)
					{
						rtw_mdelay_os(1);
					}
					else if (u4bRegOffset == 0xfb)
					{
						rtw_udelay_os(50);
					}
					else if (u4bRegOffset == 0xfa)
					{
						rtw_udelay_os(5);
					}
					else if (u4bRegOffset == 0xf9)
					{
						rtw_udelay_os(1);
					}

					// Get 2nd hex value as register value.
					szLine += u4bMove;
					if(GetHexValueFromString(szLine, &u4bRegValue, &u4bMove))
					{
						//DBG_871X("[ADDR]%03lX=%08lX\n", u4bRegOffset, u4bRegValue);
						PHY_SetBBReg(Adapter, u4bRegOffset, bMaskDWord, u4bRegValue);

						// Add 1us delay between BB/RF register setting.
						rtw_udelay_os(1);
					}
				}
			}
		}
	}
	else
	{
		DBG_871X("phy_ConfigBBWithMpParaFile(): Failed%s\n", pFileName);
	}

	return rtStatus;
}

#endif

int
PHY_ConfigRFWithParaFile(
	IN	PADAPTER	Adapter,
	IN	char* 		pFileName,
	IN	u8			eRFPath
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	int	rlen = 0, rtStatus = _FAIL;
	char	*szLine, *ptmp;
	u32	u4bRegOffset, u4bRegValue, u4bMove;
	u16	i;
	char	*pBuf = NULL;
	u32	*pBufLen = NULL;

	if(!(Adapter->registrypriv.load_phy_file & LOAD_RF_PARA_FILE))
		return rtStatus;

	switch(eRFPath)
	{
		case ODM_RF_PATH_A:
			pBuf = pHalData->rf_radio_a;
			pBufLen = &pHalData->rf_radio_a_len;
			break;
		case ODM_RF_PATH_B:
			pBuf = pHalData->rf_radio_b;
			pBufLen = &pHalData->rf_radio_b_len;
			break;
		default:
			DBG_871X("Unknown RF path!! %d\r\n", eRFPath);
			break;			
	}

	_rtw_memset(pHalData->para_file_buf, 0, MAX_PARA_FILE_BUF_LEN);

	if ((*pBufLen == 0) && (pBuf == NULL))
	{
		rtw_merge_string(file_path, PATH_LENGTH_MAX, rtw_phy_file_path, pFileName);

		if (rtw_is_file_readable(file_path) == _TRUE)
		{
			rlen = rtw_retrive_from_file(file_path, pHalData->para_file_buf, MAX_PARA_FILE_BUF_LEN);
			if (rlen > 0)
			{
				rtStatus = _SUCCESS;
				pBuf = rtw_zmalloc(rlen);
				if(pBuf) {
					_rtw_memcpy(pBuf, pHalData->para_file_buf, rlen);
					*pBufLen = rlen;

					switch(eRFPath)
					{
						case ODM_RF_PATH_A:
							pHalData->rf_radio_a = pBuf;
							break;
						case ODM_RF_PATH_B:
							pHalData->rf_radio_b = pBuf;
							break;
						default:
							DBG_871X("Unknown eRFPath!! %d\r\n", eRFPath);
							rtw_mfree(pBuf, rlen);
							break;
					}
				}
				else {
					DBG_871X("%s(): eRFPath=%d  alloc fail !\n",__FUNCTION__,eRFPath);
				}
			}
		}
	}
	else
	{
		if ((*pBufLen != 0) && (pBuf != NULL)) {
			_rtw_memcpy(pHalData->para_file_buf, pBuf, *pBufLen);
			rtStatus = _SUCCESS;
		}
		else {
			DBG_871X("%s(): Critical Error !!!\n",__FUNCTION__);
		}
	}

	if(rtStatus == _SUCCESS)
	{
		//DBG_871X("%s(): read %s successfully\n", __FUNCTION__, pFileName);
	
		ptmp = pHalData->para_file_buf;
		for (szLine = GetLineFromBuffer(ptmp); szLine != NULL; szLine = GetLineFromBuffer(ptmp))
		{
			if(!IsCommentString(szLine))
			{
				// Get 1st hex value as register offset.
				if(GetHexValueFromString(szLine, &u4bRegOffset, &u4bMove))
				{
			 		if(u4bRegOffset == 0xfe || u4bRegOffset == 0xffe)
					{ // Deay specific ms. Only RF configuration require delay.												
						#ifdef CONFIG_LONG_DELAY_ISSUE
						rtw_msleep_os(50);
						#else
						rtw_mdelay_os(50);
						#endif
					}
					else if (u4bRegOffset == 0xfd)
					{
						//delay_ms(5);
						for(i=0;i<100;i++)
							rtw_udelay_os(MAX_STALL_TIME);
					}
					else if (u4bRegOffset == 0xfc)
					{
						//delay_ms(1);
						for(i=0;i<20;i++)
							rtw_udelay_os(MAX_STALL_TIME);
					}
					else if (u4bRegOffset == 0xfb)
					{
						rtw_udelay_os(50);
					}
					else if (u4bRegOffset == 0xfa)
					{
						rtw_udelay_os(5);
					}
					else if (u4bRegOffset == 0xf9)
					{
						rtw_udelay_os(1);
					}
					else if(u4bRegOffset == 0xffff)
					{
						break;					
					}
					
					// Get 2nd hex value as register value.
					szLine += u4bMove;
					if(GetHexValueFromString(szLine, &u4bRegValue, &u4bMove))
					{
						PHY_SetRFReg(Adapter, eRFPath, u4bRegOffset, bRFRegOffsetMask, u4bRegValue);
						
						// Temp add, for frequency lock, if no delay, that may cause
						// frequency shift, ex: 2412MHz => 2417MHz
						// If frequency shift, the following action may works.
						// Fractional-N table in radio_a.txt 
						//0x2a 0x00001		// channel 1
						//0x2b 0x00808		frequency divider.
						//0x2b 0x53333
						//0x2c 0x0000c
						rtw_udelay_os(1);
					}
				}
			}
		}
	}
	else
	{
		DBG_871X("%s(): Failed to read %s!\n", __FUNCTION__, pFileName);
	}

	return rtStatus;
}

VOID
initDeltaSwingIndexTables(
	PADAPTER	Adapter, 
	char*		Band, 
	char*		Path,
	char*		Sign,
	char*		Channel, 
	char*		Rate,
	char*		Data
)
{
	#define STR_EQUAL_5G(_band, _path, _sign, _rate, _chnl) \
		((strcmp(Band, _band) == 0) && (strcmp(Path, _path) == 0) && (strcmp(Sign, _sign) == 0) &&\
		(strcmp(Rate, _rate) == 0) && (strcmp(Channel, _chnl) == 0)\
	)
	#define STR_EQUAL_2G(_band, _path, _sign, _rate) \
		((strcmp(Band, _band) == 0) && (strcmp(Path, _path) == 0) && (strcmp(Sign, _sign) == 0) &&\
		(strcmp(Rate, _rate) == 0)\
	)
	
	#define STORE_SWING_TABLE(_array, _iteratedIdx) \
		for(token = strsep(&Data, delim); token != NULL; token = strsep(&Data, delim))\
		{\
			sscanf(token, "%d", &idx);\
			_array[_iteratedIdx++] = (u8)idx;\
		}\

	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	PODM_RF_CAL_T  	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
	u32	j = 0;
	char	*token;
	char	delim[] = ",";
	u32	idx = 0;
	
	//DBG_871X("===>initDeltaSwingIndexTables(): Band: %s;\nPath: %s;\nSign: %s;\nChannel: %s;\nRate: %s;\n, Data: %s;\n", 
	//	Band, Path, Sign, Channel, Rate, Data);
	
	if ( STR_EQUAL_2G("2G", "A", "+", "CCK") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKA_P, j);
	}
	else if ( STR_EQUAL_2G("2G", "A", "-", "CCK") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKA_N, j);
	}
	else if ( STR_EQUAL_2G("2G", "B", "+", "CCK") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKB_P, j);
	}
	else if ( STR_EQUAL_2G("2G", "B", "-", "CCK") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKB_N, j);
	}
	else if ( STR_EQUAL_2G("2G", "A", "+", "ALL") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_2GA_P, j);
	}
	else if ( STR_EQUAL_2G("2G", "A", "-", "ALL") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_2GA_N, j);
	}
	else if ( STR_EQUAL_2G("2G", "B", "+", "ALL") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_2GB_P, j);
	}
	else if ( STR_EQUAL_2G("2G", "B", "-", "ALL") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_2GB_N, j);
	}
	else if ( STR_EQUAL_5G("5G", "A", "+", "ALL", "0") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_5GA_P[0], j);
	}
	else if ( STR_EQUAL_5G("5G", "A", "-", "ALL", "0") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_5GA_N[0], j);
	}
	else if ( STR_EQUAL_5G("5G", "B", "+", "ALL", "0") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_5GB_P[0], j);
	}
	else if ( STR_EQUAL_5G("5G", "B", "-", "ALL", "0") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_5GB_N[0], j);
	}
	else if ( STR_EQUAL_5G("5G", "A", "+", "ALL", "1") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_5GA_P[1], j);
	}
	else if ( STR_EQUAL_5G("5G", "A", "-", "ALL", "1") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_5GA_N[1], j);
	}
	else if ( STR_EQUAL_5G("5G", "B", "+", "ALL", "1") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_5GB_P[1], j);
	}
	else if ( STR_EQUAL_5G("5G", "B", "-", "ALL", "1") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_5GB_N[1], j);
	}
	else if ( STR_EQUAL_5G("5G", "A", "+", "ALL", "2") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_5GA_P[2], j);
	}
	else if ( STR_EQUAL_5G("5G", "A", "-", "ALL", "2") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_5GA_N[2], j);
	}
	else if ( STR_EQUAL_5G("5G", "B", "+", "ALL", "2") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_5GB_P[2], j);
	}
	else if ( STR_EQUAL_5G("5G", "B", "-", "ALL", "2") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_5GB_N[2], j);
	}
	else if ( STR_EQUAL_5G("5G", "A", "+", "ALL", "3") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_5GA_P[3], j);
	}
	else if ( STR_EQUAL_5G("5G", "A", "-", "ALL", "3") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_5GA_N[3], j);
	}
	else if ( STR_EQUAL_5G("5G", "B", "+", "ALL", "3") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_5GB_P[3], j);
	}
	else if ( STR_EQUAL_5G("5G", "B", "-", "ALL", "3") )
	{
		STORE_SWING_TABLE(pRFCalibrateInfo->DeltaSwingTableIdx_5GB_N[3], j);
	}
	else
	{
 		DBG_871X("===>initDeltaSwingIndexTables(): The input is invalid!!\n");
	}
}

int
PHY_ConfigRFWithTxPwrTrackParaFile(
	IN	PADAPTER		Adapter,
	IN	char*	 		pFileName
)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	PDM_ODM_T			pDM_Odm = &pHalData->odmpriv;
	PODM_RF_CAL_T  		pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
	int	rlen = 0, rtStatus = _FAIL;
	char	*szLine, *ptmp;
	u32	i = 0, j = 0;
	char	c = 0;

	if(!(Adapter->registrypriv.load_phy_file & LOAD_RF_TXPWR_TRACK_PARA_FILE))
		return rtStatus;

	_rtw_memset(pHalData->para_file_buf, 0, MAX_PARA_FILE_BUF_LEN);

	if ((pHalData->rf_tx_pwr_track_len == 0) && (pHalData->rf_tx_pwr_track == NULL))
	{
		rtw_merge_string(file_path, PATH_LENGTH_MAX, rtw_phy_file_path, pFileName);
	
		if (rtw_is_file_readable(file_path) == _TRUE)
		{
			rlen = rtw_retrive_from_file(file_path, pHalData->para_file_buf, MAX_PARA_FILE_BUF_LEN);
			if (rlen > 0)
			{
				rtStatus = _SUCCESS;
				pHalData->rf_tx_pwr_track = rtw_zmalloc(rlen);
				if(pHalData->rf_tx_pwr_track) {
					_rtw_memcpy(pHalData->rf_tx_pwr_track, pHalData->para_file_buf, rlen);
					pHalData->rf_tx_pwr_track_len = rlen;
				}
				else {
					DBG_871X("%s rf_tx_pwr_track alloc fail !\n",__FUNCTION__);
				}
			}
		}
	}
	else
	{
		if ((pHalData->rf_tx_pwr_track_len != 0) && (pHalData->rf_tx_pwr_track != NULL)) {
			_rtw_memcpy(pHalData->para_file_buf, pHalData->rf_tx_pwr_track, pHalData->rf_tx_pwr_track_len);
			rtStatus = _SUCCESS;
		}
		else {
			DBG_871X("%s(): Critical Error !!!\n",__FUNCTION__);
		}
	}

	if(rtStatus == _SUCCESS)
	{
		//DBG_871X("%s(): read %s successfully\n", __FUNCTION__, pFileName);

		ptmp = pHalData->para_file_buf;
		for (szLine = GetLineFromBuffer(ptmp); szLine != NULL; szLine = GetLineFromBuffer(ptmp))
		{
			if ( ! IsCommentString(szLine) )
			{
				char	band[5]="", path[5]="", sign[5]  = "";
				char	chnl[5]="", rate[10]="";
				char	data[300]=""; // 100 is too small

				if (strlen(szLine) < 10 || szLine[0] != '[')
					continue;

				strncpy(band, szLine+1, 2); 
				strncpy(path, szLine+5, 1); 
				strncpy(sign, szLine+8, 1);

				i = 10; // szLine+10
				if ( ! ParseQualifiedString(szLine, &i, rate, '[', ']') ) {
					//DBG_871X("Fail to parse rate!\n");
				}
				if ( ! ParseQualifiedString(szLine, &i, chnl, '[', ']') ) {
					//DBG_871X("Fail to parse channel group!\n");
				}
				while ( szLine[i] != '{' && i < strlen(szLine))
					i++;
				if ( ! ParseQualifiedString(szLine, &i, data, '{', '}') ) {
					//DBG_871X("Fail to parse data!\n");
				}

				initDeltaSwingIndexTables(Adapter, band, path, sign, chnl, rate, data);
			}
		}
	}
	else
	{
		DBG_871X("%s(): Failed to read %s!\n", __FUNCTION__, pFileName);
	}
#if 0
	for (i = 0; i < DELTA_SWINGIDX_SIZE; ++i)
	{
		DBG_871X("pRFCalibrateInfo->DeltaSwingTableIdx_2GA_P[%d] = %d\n", i, pRFCalibrateInfo->DeltaSwingTableIdx_2GA_P[i]);
		DBG_871X("pRFCalibrateInfo->DeltaSwingTableIdx_2GA_N[%d] = %d\n", i, pRFCalibrateInfo->DeltaSwingTableIdx_2GA_N[i]);
		DBG_871X("pRFCalibrateInfo->DeltaSwingTableIdx_2GB_P[%d] = %d\n", i, pRFCalibrateInfo->DeltaSwingTableIdx_2GB_P[i]);
		DBG_871X("pRFCalibrateInfo->DeltaSwingTableIdx_2GB_N[%d] = %d\n", i, pRFCalibrateInfo->DeltaSwingTableIdx_2GB_N[i]);
		DBG_871X("pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKA_P[%d] = %d\n", i, pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKA_P[i]);
		DBG_871X("pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKA_N[%d] = %d\n", i, pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKA_N[i]);
		DBG_871X("pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKB_P[%d] = %d\n", i, pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKB_P[i]);
		DBG_871X("pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKB_N[%d] = %d\n", i, pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKB_N[i]);

		for (j = 0; j < 3; ++j)
		{
		    DBG_871X("pRFCalibrateInfo->DeltaSwingTableIdx_5GA_P[%d][%d] = %d\n", j, i, pRFCalibrateInfo->DeltaSwingTableIdx_5GA_P[j][i]);
		    DBG_871X("pRFCalibrateInfo->DeltaSwingTableIdx_5GA_N[%d][%d] = %d\n", j, i, pRFCalibrateInfo->DeltaSwingTableIdx_5GA_N[j][i]);
		    DBG_871X("pRFCalibrateInfo->DeltaSwingTableIdx_5GB_P[%d][%d] = %d\n", j, i, pRFCalibrateInfo->DeltaSwingTableIdx_5GB_P[j][i]);
		    DBG_871X("pRFCalibrateInfo->DeltaSwingTableIdx_5GB_N[%d][%d] = %d\n", j, i, pRFCalibrateInfo->DeltaSwingTableIdx_5GB_N[j][i]);
		}
	}
#endif
	return rtStatus;
}

int
phy_ParsePowerLimitTableFile(
  PADAPTER		Adapter,
  char*			buffer
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u32	i = 0, forCnt = 0;
	u8	loadingStage = 0, limitValue = 0, fraction = 0;
	char	*szLine, *ptmp;
	int	rtStatus = _SUCCESS;
	char band[10], bandwidth[10], rateSection[10],
		regulation[TXPWR_LMT_MAX_REGULATION_NUM][10], rfPath[10],colNumBuf[10];
	u8 	colNum = 0;

	DBG_871X("===>phy_ParsePowerLimitTableFile()\n" );

	if ( Adapter->registrypriv.RegDecryptCustomFile == 1 )
		phy_DecryptBBPgParaFile( Adapter, buffer);

	ptmp = buffer;
	for (szLine = GetLineFromBuffer(ptmp); szLine != NULL; szLine = GetLineFromBuffer(ptmp))
	{
		// skip comment 
		if ( IsCommentString( szLine ) ) {
			continue;
		}

		if( loadingStage == 0 ) {
			for ( forCnt = 0; forCnt < TXPWR_LMT_MAX_REGULATION_NUM; ++forCnt )
				_rtw_memset( ( PVOID ) regulation[forCnt], 0, 10 );
			_rtw_memset( ( PVOID ) band, 0, 10 );
			_rtw_memset( ( PVOID ) bandwidth, 0, 10 );
			_rtw_memset( ( PVOID ) rateSection, 0, 10 );
			_rtw_memset( ( PVOID ) rfPath, 0, 10 );
			_rtw_memset( ( PVOID ) colNumBuf, 0, 10 );

			if ( szLine[0] != '#' || szLine[1] != '#' )
				continue;

			// skip the space
			i = 2;
			while ( szLine[i] == ' ' || szLine[i] == '\t' )
				++i;

			szLine[--i] = ' '; // return the space in front of the regulation info

			// Parse the label of the table
			if ( ! ParseQualifiedString( szLine, &i, band, ' ', ',' ) ) {
				DBG_871X( "Fail to parse band!\n");
				return _FAIL;
			}
			if ( ! ParseQualifiedString( szLine, &i, bandwidth, ' ', ',' ) ) {
				DBG_871X("Fail to parse bandwidth!\n");
				return _FAIL;
			}
			if ( ! ParseQualifiedString( szLine, &i, rfPath, ' ', ',' ) ) {
				DBG_871X("Fail to parse rf path!\n");
				return _FAIL;
			}
			if ( ! ParseQualifiedString( szLine, &i, rateSection, ' ', ',' ) ) {
				DBG_871X("Fail to parse rate!\n");
				return _FAIL;
			}

			loadingStage = 1;
		}
		else if ( loadingStage == 1 )
		{
			if ( szLine[0] != '#' || szLine[1] != '#' )
				continue;

			// skip the space
			i = 2;
			while ( szLine[i] == ' ' || szLine[i] == '\t' )
				++i;

			if ( !eqNByte( (u8 *)(szLine + i), (u8 *)("START"), 5 ) ) {
				DBG_871X("Lost \"##   START\" label\n");
				return _FAIL;
			}

			loadingStage = 2;
		}
		else if ( loadingStage == 2 )
		{
			if ( szLine[0] != '#' || szLine[1] != '#' )
				continue;

			// skip the space
			i = 2;
			while ( szLine[i] == ' ' || szLine[i] == '\t' )
				++i;

			if ( ! ParseQualifiedString( szLine, &i, colNumBuf, '#', '#' ) ) {
				DBG_871X("Fail to parse column number!\n");
				return _FAIL;
			}

			if ( !GetU1ByteIntegerFromStringInDecimal( colNumBuf, &colNum ) )
				return _FAIL;

			if ( colNum > TXPWR_LMT_MAX_REGULATION_NUM ) {
				DBG_871X("unvalid col number %d (greater than max %d)\n", 
				          colNum, TXPWR_LMT_MAX_REGULATION_NUM );
				return _FAIL;
			}

			for ( forCnt = 0; forCnt < colNum; ++forCnt )
			{
				u8	regulation_name_cnt = 0;

				// skip the space
				while ( szLine[i] == ' ' || szLine[i] == '\t' )
					++i;

				while ( szLine[i] != ' ' && szLine[i] != '\t' && szLine[i] != '\0' )
					regulation[forCnt][regulation_name_cnt++] = szLine[i++];
				//DBG_871X("regulation %s!\n", regulation[forCnt]);

				if ( regulation_name_cnt == 0 ) {
					DBG_871X("unvalid number of regulation!\n");
					return _FAIL;
				}
			}

			loadingStage = 3;
		}
		else if ( loadingStage == 3 )
		{
			char	channel[10] = {0}, powerLimit[10] = {0};
			u8	cnt = 0;
			
			// the table ends
			if ( szLine[0] == '#' && szLine[1] == '#' ) {
				i = 2;
				while ( szLine[i] == ' ' || szLine[i] == '\t' )
					++i;

				if ( eqNByte( (u8 *)(szLine + i), (u8 *)("END"), 3 ) ) {
					loadingStage = 0;
					continue;
				}
				else {
					DBG_871X("Wrong format\n");
					DBG_871X("<===== phy_ParsePowerLimitTableFile()\n");
					return _FAIL;
				}
			}

			if ( ( szLine[0] != 'c' && szLine[0] != 'C' ) || 
				 ( szLine[1] != 'h' && szLine[1] != 'H' ) ) {
				DBG_871X("Meet wrong channel => power limt pair\n");
				continue;
			}
			i = 2;// move to the  location behind 'h'

			// load the channel number
			cnt = 0;
			while ( szLine[i] >= '0' && szLine[i] <= '9' ) {
				channel[cnt] = szLine[i];
				++cnt;
				++i;
			}
			//DBG_871X("chnl %s!\n", channel);
			
			for ( forCnt = 0; forCnt < colNum; ++forCnt )
			{
				// skip the space between channel number and the power limit value
				while ( szLine[i] == ' ' || szLine[i] == '\t' )
					++i;

				// load the power limit value
				cnt = 0;
				fraction = 0;
				_rtw_memset( ( PVOID ) powerLimit, 0, 10 );
				while ( ( szLine[i] >= '0' && szLine[i] <= '9' ) || szLine[i] == '.' )
				{
					if ( szLine[i] == '.' ){
						if ( ( szLine[i+1] >= '0' && szLine[i+1] <= '9' ) ) {
							fraction = szLine[i+1];
							i += 2;
						}
						else {
							DBG_871X("Wrong fraction in TXPWR_LMT.txt\n");
							return _FAIL;
						}

						break;
					}

					powerLimit[cnt] = szLine[i];
					++cnt;
					++i;
				}

				if ( powerLimit[0] == '\0' ) {
					powerLimit[0] = '6';
					powerLimit[1] = '3';
					i += 2;
				}
				else {
					if ( !GetU1ByteIntegerFromStringInDecimal( powerLimit, &limitValue ) )
						return _FAIL;

					limitValue *= 2;
					cnt = 0;
					if ( fraction == '5' )
						++limitValue;

					// the value is greater or equal to 100
					if ( limitValue >= 100 ) {
						powerLimit[cnt++] = limitValue/100 + '0';
						limitValue %= 100;

						if ( limitValue >= 10 ) {
							powerLimit[cnt++] = limitValue/10 + '0';
							limitValue %= 10;
						}
						else {
							powerLimit[cnt++] = '0';
						}

						powerLimit[cnt++] = limitValue + '0';
					}
					// the value is greater or equal to 10
					else if ( limitValue >= 10 ) {
						powerLimit[cnt++] = limitValue/10 + '0';
						limitValue %= 10;
						powerLimit[cnt++] = limitValue + '0';
					}
					// the value is less than 10 
					else
						powerLimit[cnt++] = limitValue + '0';

					powerLimit[cnt] = '\0';
				}

				//DBG_871X("ch%s => %s\n", channel, powerLimit);

				// store the power limit value
				PHY_SetTxPowerLimit( Adapter, (u8 *)regulation[forCnt], (u8 *)band,
					(u8 *)bandwidth, (u8 *)rateSection, (u8 *)rfPath, (u8 *)channel, (u8 *)powerLimit );

			}
		}
		else 
		{
			DBG_871X("Abnormal loading stage in phy_ParsePowerLimitTableFile()!\n");
			rtStatus = _FAIL;
			break;
		}
	}

	DBG_871X("<===phy_ParsePowerLimitTableFile()\n");
	return rtStatus;
}

int
PHY_ConfigRFWithPowerLimitTableParaFile(
	IN	PADAPTER	Adapter,
	IN	char*	 	pFileName
)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	int	rlen = 0, rtStatus = _FAIL;

	if(!(Adapter->registrypriv.load_phy_file & LOAD_RF_TXPWR_LMT_PARA_FILE))
		return rtStatus;

	_rtw_memset(pHalData->para_file_buf, 0, MAX_PARA_FILE_BUF_LEN);

	if ((pHalData->rf_tx_pwr_lmt_len == 0) && (pHalData->rf_tx_pwr_lmt == NULL))
	{
		rtw_merge_string(file_path, PATH_LENGTH_MAX, rtw_phy_file_path, pFileName);
	
		if (rtw_is_file_readable(file_path) == _TRUE)
		{
			rlen = rtw_retrive_from_file(file_path, pHalData->para_file_buf, MAX_PARA_FILE_BUF_LEN);
			if (rlen > 0)
			{
				rtStatus = _SUCCESS;
				pHalData->rf_tx_pwr_lmt = rtw_zmalloc(rlen);
				if(pHalData->rf_tx_pwr_lmt) {
					_rtw_memcpy(pHalData->rf_tx_pwr_lmt, pHalData->para_file_buf, rlen);
					pHalData->rf_tx_pwr_lmt_len = rlen;
				}
				else {
					DBG_871X("%s rf_tx_pwr_lmt alloc fail !\n",__FUNCTION__);
				}
			}
		}
	}
	else
	{
		if ((pHalData->rf_tx_pwr_lmt_len != 0) && (pHalData->rf_tx_pwr_lmt != NULL)) {
			_rtw_memcpy(pHalData->para_file_buf, pHalData->rf_tx_pwr_lmt, pHalData->rf_tx_pwr_lmt_len);
			rtStatus = _SUCCESS;
		}
		else {
			DBG_871X("%s(): Critical Error !!!\n",__FUNCTION__);
		}
	}

	if(rtStatus == _SUCCESS)
	{
		//DBG_871X("%s(): read %s ok\n", __FUNCTION__, pFileName);
		rtStatus = phy_ParsePowerLimitTableFile( Adapter, pHalData->para_file_buf );
	}
	else
	{
		DBG_871X("%s(): Failed to read %s!\n", __FUNCTION__, pFileName);
	}

	return rtStatus;
}
#endif


