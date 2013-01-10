#include <stdio.h>
#include <string.h>

typedef struct timeval {
    unsigned long tv_sec;
    unsigned long tv_usec;
} timeval_t;

typedef unsigned long time_t;

struct tm
{
  int tm_sec;			/* Seconds.	[0-60] (1 leap second) */
  int tm_min;			/* Minutes.	[0-59] */
  int tm_hour;			/* Hours.	[0-23] */
  int tm_mday;			/* Day.		[1-31] */
  int tm_mon;			/* Month.	[0-11] */
  int tm_year;			/* Year	- 1900.  */
  int tm_wday;			/* Day of week.	[0-6] */
  int tm_yday;			/* Days in year.[0-365]	*/
  int tm_isdst;			/* DST.		[-1/0/1]*/

  long int tm_gmtoff;		/* Seconds east of UTC.  */
  __const char *tm_zone;	/* Timezone abbreviation.  */
};


size_t strftime(char *s, size_t maxsize, const char *format, const struct tm *t);
char *ctime(const time_t *timer);
int get_uptime_secs();
struct tm *gmtime(const time_t *timer);
time_t get_cur_time();
struct tm *get_cur_time_tm();
void set_init_time(struct tm *t);
char *ctime_time();
char *ctime_time_hhmm();
char *ctime_date();
time_t mktime(struct tm *tmbuf);
char *asctime(const struct tm *tm);
