// RTC-related functions
void rtc_setup();
void rtc_dump();
struct tm *rtc_get_tm();
char *rtc_ctime_time();
char *rtc_ctime_hhmm();
char *rtc_ctime_date();
char *rtc_ctime();
int rtc_set_tm(struct tm *t);

// main clock setup function - adjusts the RTC
void clock_setup();
