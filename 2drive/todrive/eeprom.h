#define PASS_UNSET 0xFFFFFFFF

char *eeprom_sern_read();
int eeprom_sern_valid();
void eeprom_sern_write(char *);
unsigned long eeprom_pass_read();
void eeprom_pass_write(unsigned long w);

void pass_setup();
int pass_check();

void sern_setup();
