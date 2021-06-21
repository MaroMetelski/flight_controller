#ifndef __BCM_SYS_H__
#define __BCM_SYS_H__

int bcm_sys_set_period(uint32_t period);
int bcm_sys_set_pin(uint32_t pin, uint32_t duty_cycle);
int bcm_sys_start(void);

#endif /* __BCM_SYS_H__ */
