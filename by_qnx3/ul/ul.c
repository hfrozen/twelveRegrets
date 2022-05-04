#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/procfs.h>

int main(int argc, char *argv[])
{
	char buf[256];
	char sbuf[256];
	char dbuf[256];
	DIR* pDir = opendir("/h/sd/arc");
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;

			if (pEntry->d_name[0] == 'c') {
				strncpy(buf, pEntry->d_name, 256);
				sprintf(sbuf, "/h/sd/arc/%s", buf);
				buf[0] = 'a';
				sprintf(dbuf, "/h/sd/arc/%s", buf);
				printf("rename %s %s\n", sbuf, dbuf);
				int res = rename(sbuf, dbuf);
				if (res != 0)	printf("rename error %s\n", strerror(errno));
			}
		}
		closedir(pDir);
	}
	return 0;
}
