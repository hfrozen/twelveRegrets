#include <stdio.h>
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

#include <Draft.h>
#include <TrainArch.h>
#include <Internal.h>

#define	MAX_BUF		2048
char	c_src[128];
char	c_dest[128];
char	c_buf[MAX_BUF];

char	c_odd[15];

long filesize(FILE *fp)
{
	long int orgpos;
	long fsize;

	orgpos = ftell(fp);
	fseek(fp, 0L, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, orgpos, SEEK_SET);
	return fsize;
}

int FindExec(char* fname)
{
	struct dinfo_s {
		procfs_debuginfo	info;
		char	pathbuf[1024];
	};
	//struct stat fs;
	//char path[256];
	char fn[256];

	DIR* dir = opendir("/dev");
	struct dirent* ent;
	while ((ent = readdir(dir)) != NULL) {
		//lstat(fn, &fs);
		int pid = atoi(ent->d_name);
		sprintf(fn, "/dev/%d/as", pid);
		if (pid <= 0)	continue;

		struct dinfo_s dinfo;
		int file;
		if ((file = open(fn, O_RDONLY | O_NONBLOCK)) >= 0) {
			int status = devctl(file, DCMD_PROC_MAPDEBUG_BASE,
					&dinfo, sizeof(dinfo), NULL);
			close(file);
			/*if (status == 0) {
				MSGLOG("Find:%s\r\n", dinfo.info.path);
			}*/
			if (status == 0 && !strcmp(dinfo.info.path, fname))
				return pid;
		}
	}
	return -1;
}

int Copy(char* fname)
{
	char sbuf[128];
	char dbuf[128];
	char buf[1024];
	int fs, fd, size, tsize;

	sprintf(sbuf, "/nand/test/%s", fname);
	sprintf(dbuf, "/dos/c/%s", fname);
	printf("src = %s.\n", sbuf);
	printf("dest = %s.\n", dbuf);
	fs = open(sbuf, O_RDONLY);
	if (fs < 0)	return -1;
	fd = open(dbuf, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (fd < 0) {
		close(fs);
		return -2;
	}
	tsize = 0;
	do {
		size = read(fs, buf, 1024);
		if (size < 0) {
			close(fs);
			close(fd);
			return -3;
		}
		tsize += size;
		int res = write(fd, buf, size);
		if (res < 0) {
			close(fs);
			close(fd);
			return -4;
		}
	} while (size >= 1024);
	close(fs);
	close(fd);
	return tsize;
}

int CopyUsb()
{
	DIR* dir;
	struct dirent* ent;
	struct stat st;
	char buf[128];
	char usbc[128];

	int usb = open("/dev/hd1t6", O_RDONLY | O_NONBLOCK);
	if (usb >= 0) {
		close(usb);
		printf("find usb\n");
		dir = opendir("/dos/c");
		if (dir == NULL)	system("mount -tdos /dev/hd1t6 /dos/c");
		else	closedir(dir);
		strcpy(usbc, "/dos/c");
		int res = stat(usbc, &st);
		if (res < 0)	printf("usb stat error(%s).\n", strerror(errno));
		else {
			printf("nlink = %ld, blksize = %ld, nblock = %ld, ioblk = %ld, blockhi = %ld, block = %ld\n",
					(long)st.st_nlink, (long)st.st_blocksize, (long)st.st_nblocks, (long)st.st_blksize,
					(long)st.st_blocks_hi, (long)st.st_blocks);
		}
		int filelength = 0;
		dir = opendir("/nand/test");
		while ((ent = readdir(dir)) != NULL) {
			strcpy(buf, ent->d_name);
			if (buf[0] == 'D') {
				printf("find %s.\n", buf);
				sprintf(buf, "/nand/test/%s", ent->d_name);
				if (stat(buf, &st) < 0)	st.st_size = 0;
				printf("size %d\n", st.st_size);
				//int res = Copy(buf);
				//printf("copy res = %d.\n", res);
				filelength ++;
			}
		}
		closedir(dir);
		if (filelength > 0) {
			printf("found %d.\n", filelength);
			system("rm /nand/test/D*");
		}
	}
	else	printf("not found usb\n");
	return 0;
}

typedef struct _tagSEG {
	WORD	w[512];
} SEG;

typedef struct _tagSEGS {
	SEG		s[1024];
} SEGS;

SEGS	segs;
WORD max;

void print_time(uint64_t* pBegin)
{
	uint64_t end = ClockCycles();
	float cps = SYSPAGE_ENTRY(qtime)->cycles_per_sec;
	double sec = (double)(end - *pBegin) / (double)cps;
	WORD w = (WORD)(sec * 1000.f);
	printf("time %dms.\r\n", w);
	if (max < w)	max = w;
	*pBegin = end;
}

#define LENGTH	10

int mainB(int argc, char **argv)
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
		int res = 0;
		res = MsgSendPulse(coid, getprio(0), _PULSE_CODE_MINAVAIL, 0);
		uint64_t begin = ClockCycles();
		sleep(1);
		uint64_t end = ClockCycles();
		double sec = (double)(end - begin) / (double)cp;
		printf("res %d tick %f.\r\n", res, sec);
	}

	name_close(coid);
	printf("end\r\n");
	return EXIT_SUCCESS;

	//ARM_ARCH arm;
	//arm.b.alarm = TRUE;
	//BYTE m = (BYTE)arm.b.alarm;
	//printf("alarm is %d. alarm ext size is %d.\r\n", m, sizeof(ARM_EXTARCH));
	///return(0);

	struct stat buf;

	if (stat("/nfd/D120304031546/arm", &buf) > -1) {
		printf("size = %d.\r\n", buf.st_size);
		FILE* fp = fopen("/nfd/D120304031546/arm", "r");
		char buf[LENGTH];
		memset((PVOID)buf, 0, LENGTH);
		if (fp != NULL) {
			fseek(fp, 30, SEEK_SET);
			fread((PVOID)buf, LENGTH, 1, fp);
			fclose(fp);
			int n;
			for (n = 0; n < LENGTH; n ++) {
				printf(" %02X", buf[n]);
			}
			printf("\n");
		}
	}
	else	printf("stat error.\r\n");
	return(0);

	double db = 12345678.9f;
	DWORD dw = (DWORD)(db / 1000.f);
	WORD w1 = (WORD)((DWORD)db % 1000);
	printf("dw = %ld, w = %d\r\n", dw, w1);
	printf("int size is %d\r\n", sizeof(int));
	float cps = SYSPAGE_ENTRY(qtime)->cycles_per_sec;
	uint64_t s1 = ClockCycles();
	sleep(1);
	uint64_t s2 = ClockCycles();
	double sec = (double)(s2 - s1) / (double)cps;
	printf("time is %f\r\n", sec);

	printf("ATCRINFO size %d\r\n", sizeof(ATCRINFO));
	printf("SIZE_ATCSR is %d\r\n", SIZE_ATCSR);
	printf("ATORINFO size %d\r\n", sizeof(ATORINFO));
	printf("SIZE_ATOSR is %d\r\n", SIZE_ATOSR);
	printf("SIZE_ATCSR is %d\r\n", SIZE_ATCSR);
	printf("SIZE_ATCSA is %d\r\n", SIZE_ATCSA);
	printf("SIZE_ATOSR is %d\r\n", SIZE_ATOSR);
	printf("SIZE_ATOSA is %d\r\n", SIZE_ATOSA);
	printf("SIZE_SIVSR is %d\r\n", SIZE_SIVSR);
	printf("SIZE_SIVSA is %d\r\n", SIZE_SIVSA);
	printf("SIZE_ECUSR is %d\r\n", SIZE_ECUSR);
	printf("SIZE_ECUSA is %d\r\n", SIZE_ECUSA);
	printf("SIZE_CMSBSR is %d\r\n", sizeof(CMSBRINFO));
	printf("SIZE_CMSBSA is %d\r\n", sizeof(CMSBAINFO));
	printf("TBATC is %d\r\n", sizeof(TBATC));
	printf("TBATO is %d\r\n", sizeof(TBATO));
	printf("TBSIV is %d\r\n", sizeof(TBSIV));
	printf("TBV3F is %d\r\n", sizeof(TBV3F));
	printf("TBECU is %d\r\n", sizeof(TBECU));
	printf("TBCMSB is %d\r\n", sizeof(TBCMSB));

	printf("\r\nDUCJOINT size %d\r\n", sizeof(DUCJOINT));
	printf("DUOPERATEINFO size %d\r\n", sizeof(DUOPERATEINFO));
	printf("DUMONITINFO size %d\r\n", sizeof(DUMONITINFO));
	printf("DUSTATUSINFOA size %d\r\n", sizeof(DUSTATUSINFOA));
	printf("DUSTATUSINFOB size %d\r\n", sizeof(DUSTATUSINFOB));
	printf("DUENVINFO size %d\r\n", sizeof(DUENVINFO));
	//printf("DUUSBINFO size %d\r\n", sizeof(DUUSBINFO));
	printf("DUINSPECTINFO size %d\r\n", sizeof(DUINSPECTINFO));
	printf("DUTRACEINFO size %d\r\n", sizeof(DUTRACEINFO));
	printf("DUALARMLIST size %d\r\n", sizeof(DUALARMLIST));
	printf("DUCOMMMONIT size %d\r\n", sizeof(DUCOMMMONIT));
	printf("\r\ntcref size %d\r\n", sizeof(TCREFERENCE));
	printf("ccref size %d\r\n", sizeof(CCREFERENCE));

	char* p = DRV_FILE;
	printf("test string %s.\r\n", p);
	return 0;

	uint64_t begin = ClockCycles();
	WORD w = 0;
	max = 0;
	int n;
	for (n = 0; n < 1024; n ++) {
		FILE* fp = fopen("/nfd/testC.bin", "a+");
		if (fp != NULL) {
			printf("%d open ", n);
			print_time(&begin);
			//fread(&segs, sizeof(SEGS), 1, fp);
			//printf("read ");
			//print_time(&begin);
			int m;
			for (m = 0; m < 512; m ++) {
				segs.s[0].w[m] = w ++;
			}
			printf("%d index ", n);
			print_time(&begin);
			size_t num = fwrite(&segs, 2, 512, fp);
			printf("%d write %d ", n, num);
			print_time(&begin);
			fclose(fp);
			printf("%d close ", n);
			print_time(&begin);
		}
		else {
			printf("can not open/r/n");
			break;
		}
	}
	printf("max = %dms\r\n", max);
	return 0;
}

/*int main(int argc, char **argv)
{
	struct statvfs64 st;

	//if (statvfs64("/dos/c", &st) < 0) {
	if (statvfs64("/hd1", &st) < 0) {
		printf("stat error(%s)\n", strerror(errno));
		return EXIT_FAILURE;
	}
	else {
		printf("f_bsize=%ld, f_frsize=%ld, f_blocks=%ld, f_bfree=%ld, f_bavail=%ld, f_files=%ld, f_ffree=%ld\nf_favail=%ld, f_fsid=%ld, f_basetype=%s, f_flag=%ld, f_namemax=%ld\n",
				(long)st.f_bsize, (long)st.f_frsize, (long)st.f_blocks, (long)st.f_bfree, (long)st.f_bavail, (long)st.f_files,
				(long)st.f_ffree, (long)st.f_favail, (long)st.f_fsid, st.f_basetype, (long)st.f_flag, (long)st.f_namemax);
		//printf("size = %ld, nlink = %ld, blksize = %ld, nblock = %ld, ioblk = %ld, block = %ld\n",
		//			st.f_size, (long)st.st_nlink, (long)st.st_blocksize, (long)st.st_nblocks,
					// (intmax_t)st.st_size, (long)st.st_nlink, (long)st.st_blocksize, (long)st.st_nblocks,
					// (long)st.st_blksize, (long)st.st_blocks);
		//return EXIT_SUCCESS;
		printf("free size = %lld.\n", (int64_t)(st.f_bsize * st.f_bfree));
	}
	int i = rename("/hd1/current", "/hd1/testA");
	printf("rename result = %d\n", i);
	//FILE* fp = fopen("current/oper", "a+");
	//if (fp != NULL) {
	//	fclose(fp);
	//	return 0;
	//}
	return -1;*/

	/*char dn[256];
	sprintf(dn, "%s", "/dos/c/TC1");
	printf("[BEIL] :Build %s.\r\n", dn);
	DIR* pDir = opendir(dn);
	int res = 0;
	if (pDir == NULL) {
		res = mkdir(dn, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (res != 0) {
			printf("can not create %s\n", dn);
		}
	}
	else {
		closedir(pDir);
	}

	if (res == 0) {
		sprintf(dn, "%s", "/dos/c/TC1/test");
		printf("[BEIL] :Build %s.\r\n", dn);
		pDir = opendir(dn);
		if (pDir == NULL) {
			int res = mkdir(dn, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
			if (res != 0) {
				printf("can not create %s\n", dn);
			}
		}
		else {
			closedir(pDir);
		}
	}*/

	//int res;
	/*int res = unlink("/tmp/oper");
	if (res != 0)
		printf("unlink error (%s).\n", strerror(errno));
	char fn[256];
	strcpy(fn, "this is test2.\n");
	FILE* fp = fopen("/tmp/oper1", "a+");
	if (fp != NULL) {
		size_t ret = fwrite(fn, strlen(fn), 1, fp);
		if (strlen(fn) != ret)
			printf("write error:%d-%s\n", ret, strerror(errno));
		else	printf("write ok\n");
		fclose(fp);
	}
	else	printf("can not open file-%s\n", strerror(errno));*/

	/*time_t cur = time(NULL);
	struct tm* lc = localtime(&cur);
	sprintf(fn, "/nand/usb/D%02d%02d%02d%02d%02d%02d",
			lc->tm_year % 100, lc->tm_mon, lc->tm_mday, lc->tm_hour, lc->tm_min, lc->tm_sec);
	printf("mkdir \"%s\"\n", fn);
	if ((res = mkdir(fn, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) != 0)
		printf("mkdir error %s\n", strerror(errno));*/

	/*DIR* dir = opendir("/nand/usb");
	struct dirent* ent;
	long long min = 991231235959ll;
	while ((ent = readdir(dir)) != NULL) {
		strcpy(fn, ent->d_name);
		long long v = 0;
		if (fn[0] == 'D') {
			v = atoll(&fn[1]);
			if (min > v)	min = v;
		}
		printf("%s %lld\n", fn, v);
	}
	printf("find %lld\n", min);
	sprintf(fn, "/nand/usb/D%012lld", min);
	dir = opendir(fn);
	char buf[256];
	while (ent = readdir(dir)) {
		if (ent->d_name[0] == '.')	continue;
		sprintf(buf, "%s/%s", fn, ent->d_name);
		printf("remove \"%s\"\n", buf);
		if ((res = remove(buf)) != 0)
			printf("remove error %s\n", strerror(errno));
	}
	sprintf(fn, "/nand/usb/D%012lld", min);
	printf("rmdir \"%s\"\n", fn);
	if ((res = remove(fn)) != 0)
		printf("rmdir error %s\n", strerror(errno));*/
	//printf("CCFRAME %d\n", sizeof(CCFIRMREAL));
	//printf("DUFRAME %d\n", sizeof(SRDUCINFO));

	/*char fn[256];
	DIR* dir = opendir("/dev");
	struct dirent* ent;
	while ((ent = readdir(dir)) != NULL) {
		strcpy(fn, ent->d_name);
		if (strlen(fn) > 4) {
			fn[4] = 0;
			if (!strcmp(fn, "hd1t")) {
				strcpy(fn, ent->d_name);
				printf("%s\n", fn);
			}
		}
	}
	closedir(dir);
	printf("search end.\n");*/
	/*printf("find %lld\n", min);
	sprintf(fn, "/nand/usb/D%012lld", min);
	dir = opendir(fn);
	char buf[256];
	while (ent = readdir(dir)) {
		if (ent->d_name[0] == '.')	continue;
		sprintf(buf, "%s/%s", fn, ent->d_name);
		printf("remove \"%s\"\n", buf);
		if ((res = remove(buf)) != 0)
			printf("remove error %s\n", strerror(errno));
	}
	sprintf(fn, "/nand/usb/D%012lld", min);
	printf("rmdir \"%s\"\n", fn);
	if ((res = remove(fn)) != 0)
		printf("rmdir error %s\n", strerror(errno));*/

	//return 0;
//}

void GetFileQuantity(PSZ dir, BYTE fileID, PDIRINFO pDi)
{
	DIR* pDir = opendir(dir);
	if (pDir == NULL) {
		pDi->quantity = -1;
		return;
	}
	struct dirent* pEnt;
	char buf[128];
	pDi->quantity = 0;
	pDi->size = 0L;
	while ((pEnt = readdir(pDir)) != NULL) {
		strcpy(buf, pEnt->d_name);
		if (buf[0] == fileID) {
			++ pDi->quantity;
			sprintf(buf, "%s/%s/arm", dir, pEnt->d_name);
			printf("%s\t", buf);
			struct stat st;
			if (stat(buf, &st) < 0) {
				st.st_size = 0;
				printf("%s\n", strerror(errno));
			}
			else {
				pDi->size += (long)st.st_size;
				printf("%d\r\n", st.st_size);
			}
			sprintf(buf, "%s/%s/oper", dir, pEnt->d_name);
			printf("%s\t", buf);
			if (stat(buf, &st) < 0) {
				st.st_size = 0;
				printf("%s\n", strerror(errno));
			}
			else {
				pDi->size += (long)st.st_size;
				printf("%d\r\n", st.st_size);
			}
		}
	}
	closedir(pDir);
}

/*int main(int argc, char **argv)
{
	BUCKET bucket;
	DIRINFO di;
	GetFileQuantity((PSZ)"/nand/archives", 'D', &di);
	if (di.quantity > 0) {
		bucket.respond = 1;
		bucket.src.size.length = (WORD)di.quantity;
		bucket.src.size.sizeLow = (DWORD)di.size;
		bucket.progress = 0;
		printf("Find %d files (%ld).\r\n", di.quantity, di.size);
		long tsize = 0;
		DIR* pDir = opendir("/nand/archives");
		if (pDir != NULL) {
			struct dirent* pEnt;
			while ((pEnt = readdir(pDir)) != NULL) {
				strcpy(c_src, pEnt->d_name);
				if (c_src[0] == 'D') {
					sprintf(c_src, "/nand/archives/%s/arm", pEnt->d_name);
					sprintf(c_dest, "/dos/c/%s/arm", pEnt->d_name);
					int fs = open(c_src, O_RDONLY);
					if (fs > -1) {
						int fd = open(c_dest, O_RDWR | O_CREAT | O_TRUNC,
								S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
						if (fd > -1) {
							int size;
							do {
								size = read(fs, c_buf, MAX_BUF);
								if (size > -1) {
									write(fd, c_buf, size);
									tsize += (long)size;
								}
								bucket.progress = (WORD)(tsize * 100L / di.size);
							} while (size >= MAX_BUF);
							close (fd);
						}
						else	printf("Can not open %s.\r\n", c_dest);
						close (fs);
					}
					else	printf("Can not open %s.\r\n", c_src);

					sprintf(c_src, "/nand/archives/%s/oper", pEnt->d_name);
					sprintf(c_dest, "/dos/c/%s/oper", pEnt->d_name);
					fs = open(c_src, O_RDONLY);
					if (fs > -1) {
						int fd = open(c_dest, O_RDWR | O_CREAT | O_TRUNC,
								S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
						if (fd > -1) {
							int size;
							do {
								size = read(fs, c_buf, MAX_BUF);
								if (size > -1) {
									write(fd, c_buf, size);
									tsize += (long)size;
								}
								bucket.progress = (WORD)(tsize * 100L / di.size);
							} while (size >= MAX_BUF);
							close (fd);
						}
						else	printf("Can not open %s.\r\n", c_dest);
						close (fs);
					}
					else	printf("Can not open %s.\r\n", c_src);
				}
			}
		}
		closedir(pDir);
		bucket.respond = 2;
		//if (msg == BEILCMD_DOWNLOADDELETE)
		//	system("rm /nand/archives/D*");
	}
	else	bucket.respond = 3;
	printf("End.\r\n");

	return 0;
}
*/
