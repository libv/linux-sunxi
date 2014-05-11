/*
 *  drivers/arisc/interfaces/arisc_axp.c
 *
 * Copyright (c) 2012 Allwinner.
 * sunny (sunny@allwinnertech.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "../arisc_i.h"

typedef struct axp_isr
{
	arisc_cb_t   handler;
	void        *arg;
} axp_isr_t;

/* pmu isr node, record current pmu interrupt handler and argument */
axp_isr_t axp_isr_node;

/**
 * register call-back function, call-back function is for arisc notify some event to ac327,
 * axp interrupt for external interrupt NMI.
 * @func:  call-back function;
 * @para:  parameter for call-back function;
 *
 * @return: result, 0 - register call-back function successed;
 *                 !0 - register call-back function failed;
 * NOTE: the function is like "int callback(void *para)";
 *       this function will execute in system ISR.
 */
int arisc_axp_cb_register(arisc_cb_t func, void *para)
{
	if (axp_isr_node.handler) {
		if(func == axp_isr_node.handler) {
			ARISC_WRN("pmu interrupt handler register already\n");
			return 0;
		}
		/* just output warning message, overlay handler */
		ARISC_WRN("pmu interrupt handler register already\n");
		return -EINVAL;
	}
	axp_isr_node.handler = func;
	axp_isr_node.arg     = para;
	
	return 0;
}
EXPORT_SYMBOL(arisc_axp_cb_register);


/**
 * unregister call-back function.
 * @func:  call-back function which need be unregister;
 */
void arisc_axp_cb_unregister(arisc_cb_t func)
{
	if ((u32)(axp_isr_node.handler) != (u32)(func)) {
		/* invalid handler */
		ARISC_WRN("invalid handler for unreg\n\n");
		return ;
	}
	axp_isr_node.handler = NULL;
	axp_isr_node.arg     = NULL;
}
EXPORT_SYMBOL(arisc_axp_cb_unregister);

int arisc_disable_axp_irq(void)
{
	int					  result;
	struct arisc_message *pmessage;
	
	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}
	
	/* initialize message */
	pmessage->type       = ARISC_AXP_DISABLE_IRQ;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;
	
	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);
	
	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);
	
	return result;
}
EXPORT_SYMBOL(arisc_disable_axp_irq);

int arisc_enable_axp_irq(void)
{
	int					  result;
	struct arisc_message *pmessage;
	
	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}
	
	/* initialize message */
	pmessage->type       = ARISC_AXP_ENABLE_IRQ;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;
	
	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);
	
	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);
	
	return result;
}
EXPORT_SYMBOL(arisc_enable_axp_irq);

int arisc_axp_get_chip_id(unsigned char *chip_id)
{
	int                   i;
	int					  result;
	struct arisc_message *pmessage;
	
	/* allocate a message frame */
	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);
	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}
	
	/* initialize message */
	pmessage->type       = ARISC_AXP_GET_CHIP_ID;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;

	memset((void *)pmessage->paras, 0, 16);

	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* |paras[0]    |paras[1]    |paras[2]     |paras[3]      |
	 * |chip_id[0~3]|chip_id[4~7]|chip_id[8~11]|chip_id[12~15]|
	 */
	/* copy message readout data to user data buffer */
	for (i = 0; i < 4; i++) {
			chip_id[i] = (pmessage->paras[0] >> (i * 8)) & 0xff;
			chip_id[4 + i] = (pmessage->paras[1] >> (i * 8)) & 0xff;
			chip_id[8 + i] = (pmessage->paras[2] >> (i * 8)) & 0xff;
			chip_id[12 + i] = (pmessage->paras[3] >> (i * 8)) & 0xff;
	} 
	
	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);
	
	return result;
}
EXPORT_SYMBOL(arisc_axp_get_chip_id);

int arisc_axp_int_notify(struct arisc_message *pmessage)
{
	/* call pmu interrupt handler */
	if (axp_isr_node.handler == NULL) {
		ARISC_WRN("axp irq handler not install\n");
		return 1;
	}
	return (*(axp_isr_node.handler))(axp_isr_node.arg);
}
