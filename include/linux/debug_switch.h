#ifndef __DEBUG_SWITCH__
#define __DEBUG_SWITCH__

extern int log_switch;
 
#define AWPRINTR(LOG) do{if (log_switch == 1) printk("AWKLABEL#"LOG);}while(0)

#endif
