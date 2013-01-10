#ifndef __BT__
#define __BT__

void bt_setup();
int  bt_loop();
void bt_response(char *str);
int  bt_rx_pending();

#define CMD_MAX_LEN 32

extern char bt_cmd[CMD_MAX_LEN+1];

#endif
