#ifndef __BT__
#define __BT__

void bt_setup();
int  bt_loop();
void bt_response(char *str);
int  bt_rx_pending();

#define BT_CMD_MAX_LEN 32

extern char bt_cmd[BT_CMD_MAX_LEN+1];

#endif
