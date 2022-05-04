#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <sys/statvfs.h>

#define	VERSION		4.17
#define	VersionHigh(n)	(BYTE)(n / 1)
#define	VersionLow(n)	(BYTE)((WORD)(n * 100) % 100)

typedef	unsigned char	BYTE;
typedef unsigned short	WORD;

enum  {	BA = 0, BB, BC, BD,
		BE, BF, BG, BH
};

#define	PARCELX(name, b7, b6, b5, b4, b3, b2, b1, b0)\
union {\
	struct {\
		BYTE	b0 : 1;\
		BYTE	b1 : 1;\
		BYTE	b2 : 1;\
		BYTE	b3 : 1;\
		BYTE	b4 : 1;\
		BYTE	b5 : 1;\
		BYTE	b6 : 1;\
		BYTE	b7 : 1;\
	} b;\
	BYTE	a;\
} name

#define	DOOR(name)	PARCELX(name, d4bp, d3bp, d2bp, d1bp, d4c, d3c, d2c, d1c)

typedef struct _tagInfo {
	BYTE	nID;
	WORD	nLoc;
	PARCELX	( ps, ar7, ar6, ar5, ar4, ar3, ar2, ar1, ar0);
	BYTE	nEdc;
} INFO, *PINFO;

#pragma pack(1)

typedef struct _tagSCRATCH {
	BYTE	p : 1;
	BYTE	g : 1;
} SCRATCH;

SCRATCH	scratch[65536];

#pragma pack()

/*typedef union {
	struct {
		BYTE	b0 : 1;
		BYTE	b1 : 1;
		BYTE	b2 : 1;
		BYTE	b3 : 1;
		BYTE	b4 : 1;
		BYTE	b5 : 1;
		BYTE	b6 : 1;
		BYTE	b7 : 1;
	} piece;
	BYTE	sheaf;
} TEST;
*/
char dest[8192];
char src[8192];

//#define	PARCH(p, x)	(p + sizeof(INFO) * x)
#define	PARCH(p, x)	(p + x)

#define	STEP	2

#define	SHIFTV(n)	((n < 0x10) ? 1 << n : 0)

void Gate(PINFO p, int n)
{
	(p + n)->nLoc = 1 << n;
	if (n & 1)	(p + n)->ps.b.ar6 = 1;
}

int main(int argc, char *argv[])
{
	std::cout << "Size of each variables." << std::endl;
	std::cout << "bool=" << sizeof(bool) << std::endl;
	std::cout << "char=" << sizeof(char) << std::endl;
	std::cout << "unsigned char=" << sizeof(unsigned char) << std::endl;
	std::cout << "short=" << sizeof(short) << std::endl;
	std::cout << "unsigned short=" << sizeof(unsigned short) << std::endl;
	std::cout << "int=" << sizeof(int) << std::endl;
	std::cout << "unsigned int=" << sizeof(unsigned int) << std::endl;
	std::cout << "long=" << sizeof(long) << std::endl;
	std::cout << "unsigned long=" << sizeof(unsigned long) << std::endl;
	std::cout << "int64_t=" << sizeof(int64_t) << std::endl;
	std::cout << "long long=" << sizeof(long long) << std::endl;
	std::cout << "float=" << sizeof(float) << std::endl;
	std::cout << "double=" << sizeof(double) << std::endl;
	std::cout << "long double=" << sizeof(long double) << std::endl;
	std::cout << "*void=" << sizeof(void*) << std::endl;
	std::cout << "*char=" << sizeof(char*) << std::endl;
	std::cout << "*int=" << sizeof(int*) << std::endl;
	std::cout << "*long=" << sizeof(long*) << std::endl;
	std::cout << "*float=" << sizeof(float*) << std::endl;
	std::cout << "*double=" << sizeof(double*) << std::endl;
	std::cout << "unsigned long=" << sizeof(unsigned long) << std::endl;
	std::cout << "fsblkcnt_t=" << sizeof(fsblkcnt_t) << std::endl;

	double db = 1234567.89f;
	printf("%d\r\n", (unsigned short)((unsigned long)db % 1000));

	typedef union _tagBWDTYPE {
		char	c[2];
		short	s;
	} _bwd;
	_bwd	bw;
	std::cout << sizeof(bw) << std::endl;

	bw.s = 0x1234;
	std::cout << (int)bw.c[0] << " " << (int)bw.c[1] << std::endl;
	std::cout << (int)((bw.s >> 8) & 0xff) << " " <<(int)(bw.s & 0xff) << std::endl;

	typedef union _tagForm {
		struct _tagSForm {
			char	n;
			short	s;
			int		i;
			double	db;
		} _real;
		char ns[sizeof(_tagSForm)];
	} _FORM;
	_FORM frm;

	frm.ns[0] = 0;
	frm._real.db = -1;
	std::cout << sizeof(frm) << std::endl;
	std::cout << sizeof(_FORM::_tagSForm) << std::endl;
	std::cout << sizeof(char) + sizeof(short) + sizeof(int) + sizeof(double) << std::endl;

//	TEST test;
//	test.sheaf = 0;

	INFO info[10];
	memset(info, 0, sizeof(INFO) * 10);
	PINFO pInfo = info;
	for (BYTE n = 0; n < 10; n ++) {
		(pInfo + n)->nID = n;
		Gate(pInfo, n);
	}

	printf("INFO\r\n");
	for (BYTE n = 0; n < 10; n ++)
		printf("%d, id=%d loc=%d ar=%X\r\n", n, info[n].nID, info[n].nLoc, info[n].ps.a);

	DOOR(left)[8];
	left[0].a = 0;
	left[0].b.d4c = 1;
	printf("door = %d.\r\n", left[0].a);

	clock_t stime = clock();
	for (int i = 0; i < 1000; i ++) {
		memset(src, 0, 8192);
		memcpy(dest, src, 8192);
	}
	clock_t etime = clock();
	printf("time:%f\r\n", (double)(etime - stime) / CLOCKS_PER_SEC);


	for (BYTE n = 0; n < 8; n ++) {
		printf("%d - %d\r\n", n, n ^ 1);
	}
	short w = -1024 * 10000 / 2048;
	printf("%d\r\n", w);
	printf("%d %d %d\r\n", 1, !1, ~1);
	printf("%d\r\n", sizeof(info[0].nLoc));
	char* p = src;
	printf("%d\r\n", sizeof(p));

#if	STEP >= 0
	printf("step = 0\r\n");
#if	STEP >= 1
	printf("step = 1\r\n");
#if	STEP >= 2
	printf("step = 2\r\n");
#if STEP >= 3
	printf("step = 3\r\n");
#if	STEP <= 4
	printf("step = 4\r\n");
#endif
#endif
#endif
#endif
#endif

	printf("scratch size = %d\n", sizeof(scratch));
	char bTest = 1;
	char bTest1 = !bTest;
	printf("test=%d test1=%d\n", bTest, bTest1);
	WORD sh = SHIFTV(0) | SHIFTV(0x10) | SHIFTV(7) | SHIFTV(0x10);
	printf("Shift = %X\n", sh);
	printf("Version high = %d low = %d\n", VersionHigh(VERSION), VersionLow(VERSION));
	printf("true = %d, false = %d\n", true, false);
	printf("true size = %d, false size = %d\n", sizeof(true), sizeof(false));



	return EXIT_SUCCESS;

}
