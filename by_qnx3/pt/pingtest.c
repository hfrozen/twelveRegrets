#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <process.h>
#include <errno.h>

/*char prot[] = {
		"PING 193.168.0.3 (193.168.0.3): 56 data bytes\
		64 bytes from 193.168.0.3: icmp_seq=0 ttl=128 time=4 ms\
		----193.168.0.3 PING Statistics----\
		1 packets transmitted, 1 packets received, 0% packet loss\
		round-trip min/avg/max = 4/4/4 ms   variance = 0 ms^2"
};

int FindStr(char* pDest, char* pSrc)
{
	int n;
	int leng = (int)(strlen(pDest) - strlen(pSrc) + 1);
	for (n = 0; n < leng; n ++) {
		printf("%d<%d -> %s\n\n\n", n, leng, pDest);
		if (!strncmp(pDest, pSrc, strlen(pSrc)))	return n;
		++ pDest;
	}
	return -1;
}
*/
int main(int argc, char *argv[])
{
	char sbuf[256];
	char dbuf[256];
	DIR* pDir = opendir("/h/sd/cus");
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;

			if (pEntry->d_name[0] == 'c') {
				strncpy(sbuf, pEntry->d_name, 256);
				strncpy(dbuf, pEntry->d_name, 256);
				dbuf[0] = 'a';
				printf("rename %s %s\n", sbuf, dbuf);
				rename(sbuf, dbuf);
			}
		}
		closedir(pDir);
	}
/*	struct stat sb;
	int rs = stat("/tmp/doc2", &sb);
	printf("/tmp/doc2 %s %d %d size %d\n", strerror(errno), rs, ENOTDIR, sb.st_size);
	return 0;

	int r = rename("/h/sd/cus/a19071516301234.zip", "/h/sd/cus/r19071516301234.zip");
	if (r < 0)	printf("%s\n", strerror(errno));
	else	printf("res = %d\n", r);
	return 0;

	int i = FindStr(prot, (char*)"packets received");
	printf("res = %d(%s)\n", i, &prot[i]);
	return 0;

	if (argc < 2) {
		printf("usage:%s ip\n", argv[0]);
		return -1;
	}
	char rbuf[256];
	memset(rbuf, 0, 256);
	sprintf(rbuf, "cd /tmp\r\nftp ftp://anonymous:@193.168.0.3/1234/a1907151635.zip");
	system(rbuf);
	return EXIT_SUCCESS;

	sprintf(rbuf, "/proc/boot/ping -c1 %s > /tmp/pingresult.txt", argv[1]);
	int ret = system(rbuf);
	printf("return %d\n", ret);
	return EXIT_SUCCESS;

	if (argc < 4) {
		printf("usage:%s ip dirname filename\n", argv[0]);
		return -1;
	}

	char sbuf[256];
	sprintf(sbuf, "ftp -u ftp://anonymous:@%s/%s/%s %s\n", argv[1], argv[2], argv[3], argv[3]);
	int res = system(sbuf);
	printf("return %d\n", res);
	return EXIT_SUCCESS;

	if (argc < 3) {
		printf("usage: %s ip_address port\n", argv[0]);
		return -1;
	}

	int sock;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed!");
		return -1;
	}

	printf("connect to %s\n", argv[1]);
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	inet_pton(AF_INET, argv[1], &saddr.sin_addr);
	saddr.sin_port = htons(atoi(argv[2]));
	if (connect(sock, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
		perror("connection failed!");
		return -1;
	}
	printf("connection success.\n");

	char buf[256];
	int n = 0;
	while (n < 1000) {
		sleep(1);
		sprintf(buf, "test string send %d", n ++);
		send(sock, buf, strlen(buf), 0);
	}
	close(sock);
*/
	return EXIT_SUCCESS;
}
