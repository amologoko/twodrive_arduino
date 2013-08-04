#ifndef __ELM_H__
#define __ELM_H__

int elm_setup(int rx_pin, int tx_pin);
int elm_cmd(char *cmd, int send_at, int timeout_ms);
void elm_prius_lock(int on);


#endif __ELM_H__
