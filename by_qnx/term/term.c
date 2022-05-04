#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <string.h>
#include <errno.h>
//#include <devctl.h>
#include <fcntl.h>
#include <sys/procfs.h>
#include <sys/neutrino.h>
#include <inttypes.h>
#include <time.h>

#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <unistd.h>
#include <ctype.h>

#define	MSGLOG(fmt, ...) \
do {\
	printt();\
	printf(fmt, ##__VA_ARGS__);\
} while(0)

void printt()
{
	time_t cur = time(NULL);
	struct tm* lc = localtime(&cur);
	printf("%02d:%02d:%02d", lc->tm_hour, lc->tm_min, lc->tm_sec);
}

#define	SIZE_TA		10
const char ta[SIZE_TA] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J' };

double RoundedOff(double db)
{
	double quotient = db / 10.f;
	double remainder = fmodf(fabs(db), 10.f);
	if (remainder >= 5.f) {
		if (db > 0.f)	quotient += 1.0f;
		else if (db < 0.f)	quotient -= 1.0f;
	}
	return quotient;
}

int main(int argc, char **argv)
{
	MSGLOG("[FIRST]This is message%d.\r\n", 1);
	/*MSGLOG("[FIRST]This is message%d.\r\n", 2);
	MSGLOG("[FIRST]This is message%d.\r\n", 3);

	MSGLOG("int size is %d.\r\n", sizeof(int));
	MSGLOG("short size is %d.\r\n", sizeof(short));
	MSGLOG("unsigned long size is %d.\r\n", sizeof(unsigned long));
	MSGLOG("int64_t size is %d.\r\n", sizeof(int64_t));

	unsigned short n;
	for (n = SIZE_TA; n > 0; n --) {
		MSGLOG("%c.\r\n", ta[n - 1]);
	}
	MSGLOG("toascii %X\r\n", toascii(1));
	MSGLOG("toascii %X\r\n", toascii(0xa));*/

	//for (double db = 1.0f; db < 100.f; db += 1.0f)
	int n;
	for (n = 1; n < 100; n ++) {
		int res = RoundedOff(n);
		printf("%d %d\r\n", n, res);
	}

	return 0;
}
/*int main(int argc, char **argv)
{
	int coid = name_open("Stall_func", 0);
	if (coid < 0) {
		printf("Can not open stall.\r\n");
		exit(EXIT_FAILURE);
	}

	printf("stall OK!\r\nstart...\r\n");
	float cp = SYSPAGE_ENTRY(qtime)->cycles_per_sec;
	int i;
	for (i = 0; i < 10; i ++) {
		int res;
		res = MsgSendPulse(coid, getprio(0), _PULSE_CODE_MINAVAIL, 0);
		uint64_t begin = ClockCycles();
		sleep(1);
		uint64_t end = ClockCycles();
		double sec = (double)(end - begin) / (double)cp;
		printf("res %d tick %f.\r\n", res, sec);
	}

	name_close(coid);
	printf("end\r\n");
	return 0;
}
*/

