/**
 * @file	VarUnion.h
 * @brief
 * @details
 * @author
 * @date
 */
#pragma once

#ifdef	__cplusplus
extern "C" {
#endif

#if	!defined(_WIN32)
#define	_ARG36(\
		_00, _01, _02, _03, _04, _05, _06, _07, _08, _09,\
		_10, _11, _12, _13, _14, _15, _16, _17, _18, _19,\
		_20, _21, _22, _23, _24, _25, _26, _27, _28, _29,\
		_30, _31, _32, _33, _34, N, ...) N

#define	NARG36(...) _ARG36(__VA_ARGS__, 35, 34, 33, 32, 31, 30,\
						29, 28, 27, 26, 25, 24, 23, 22, 21, 20,\
						19, 18, 17, 16, 15, 14, 13, 12, 11, 10,\
						9,  8,  7,  6,  5,  4,  3,  2,  1,  0)
#else
#define	_ARG36(\
		_00, _01, _02, _03, _04, _05, _06, _07, _08, _09,\
		_10, _11, _12, _13, _14, _15, _16, _17, _18, _19,\
		_20, _21, _22, _23, _24, _25, _26, _27, _28, _29,\
		_30, _31, _32, _33, _34, N, ...) N

#define	__ARG36(args)	_ARG36 args

#define	NARG36(...) __ARG36((__VA_ARGS__, 35, 34, 33, 32, 31, 30,\
						29, 28, 27, 26, 25, 24, 23, 22, 21, 20,\
						19, 18, 17, 16, 15, 14, 13, 12, 11, 10,\
						9,  8,  7,  6,  5,  4,  3,  2,  1,  0))
#endif

#if	defined(__LITTLEENDIAN__) || (_WIN32)

#define	___BC3(name, b0, l0)\
	union {\
		struct {\
			BYTE b0 : l0;\
		} b;\
		BYTE a;\
	} name

#define	___BC5(name, b0, l0, b1, l1)\
	union {\
		struct {\
			BYTE b0 : l0;\
			BYTE b1 : l1;\
		} b;\
		BYTE a;\
	} name

#define	___BC7(name, b0, l0, b1, l1, b2, l2)\
	union {\
		struct {\
			BYTE b0 : l0;\
			BYTE b1 : l1;\
			BYTE b2 : l2;\
		} b;\
		BYTE a;\
	} name

#define	___BC9(name, b0, l0, b1, l1, b2, l2, b3, l3)\
	union {\
		struct {\
			BYTE b0 : l0;\
			BYTE b1 : l1;\
			BYTE b2 : l2;\
			BYTE b3 : l3;\
		} b;\
		BYTE a;\
	} name

#define	___BC11(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4)\
	union {\
		struct {\
			BYTE b0 : l0;\
			BYTE b1 : l1;\
			BYTE b2 : l2;\
			BYTE b3 : l3;\
			BYTE b4 : l4;\
		} b;\
		BYTE a;\
	} name

#define	___BC13(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5)\
	union {\
		struct {\
			BYTE b0 : l0;\
			BYTE b1 : l1;\
			BYTE b2 : l2;\
			BYTE b3 : l3;\
			BYTE b4 : l4;\
			BYTE b5 : l5;\
		} b;\
		BYTE a;\
	} name

#define	___BC15(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6)\
	union {\
		struct {\
			BYTE b0 : l0;\
			BYTE b1 : l1;\
			BYTE b2 : l2;\
			BYTE b3 : l3;\
			BYTE b4 : l4;\
			BYTE b5 : l5;\
			BYTE b6 : l6;\
		} b;\
		BYTE a;\
	} name

#define	___BC17(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7)\
	union {\
		struct {\
			BYTE b0 : l0;\
			BYTE b1 : l1;\
			BYTE b2 : l2;\
			BYTE b3 : l3;\
			BYTE b4 : l4;\
			BYTE b5 : l5;\
			BYTE b6 : l6;\
			BYTE b7 : l7;\
		} b;\
		BYTE a;\
	} name

#define	___BW3(name, b0, l0)\
	union {\
		struct {\
			WORD b0 : l0;\
		} b;\
		WORD a;\
	} name

#define	___BW5(name, b0, l0, b1, l1)\
	union {\
		struct {\
			WORD b0 : l0;\
			WORD b1 : l1;\
		} b;\
		WORD a;\
	} name

#define	___BW7(name, b0, l0, b1, l1, b2, l2)\
	union {\
		struct {\
			WORD b0 : l0;\
			WORD b1 : l1;\
			WORD b2 : l2;\
		} b;\
		WORD a;\
	} name

#define	___BW9(name, b0, l0, b1, l1, b2, l2, b3, l3)\
	union {\
		struct {\
			WORD b0 : l0;\
			WORD b1 : l1;\
			WORD b2 : l2;\
			WORD b3 : l3;\
		} b;\
		WORD a;\
	} name

#define	___BW11(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4)\
	union {\
		struct {\
			WORD b0 : l0;\
			WORD b1 : l1;\
			WORD b2 : l2;\
			WORD b3 : l3;\
			WORD b4 : l4;\
		} b;\
		WORD a;\
	} name

#define	___BW13(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5)\
	union {\
		struct {\
			WORD b0 : l0;\
			WORD b1 : l1;\
			WORD b2 : l2;\
			WORD b3 : l3;\
			WORD b4 : l4;\
			WORD b5 : l5;\
		} b;\
		WORD a;\
	} name

#define	___BW15(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6)\
	union {\
		struct {\
			WORD b0 : l0;\
			WORD b1 : l1;\
			WORD b2 : l2;\
			WORD b3 : l3;\
			WORD b4 : l4;\
			WORD b5 : l5;\
			WORD b6 : l6;\
		} b;\
		WORD a;\
	} name

#define	___BW17(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7)\
	union {\
		struct {\
			WORD b0 : l0;\
			WORD b1 : l1;\
			WORD b2 : l2;\
			WORD b3 : l3;\
			WORD b4 : l4;\
			WORD b5 : l5;\
			WORD b6 : l6;\
			WORD b7 : l7;\
		} b;\
		WORD a;\
	} name

#define	___BW19(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7, b8, l8)\
	union {\
		struct {\
			WORD b0 : l0;\
			WORD b1 : l1;\
			WORD b2 : l2;\
			WORD b3 : l3;\
			WORD b4 : l4;\
			WORD b5 : l5;\
			WORD b6 : l6;\
			WORD b7 : l7;\
			WORD b8 : l8;\
		} b;\
		WORD a;\
	} name

#define	___BW21(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7, b8, l8, b9, l9)\
	union {\
		struct {\
			WORD b0 : l0;\
			WORD b1 : l1;\
			WORD b2 : l2;\
			WORD b3 : l3;\
			WORD b4 : l4;\
			WORD b5 : l5;\
			WORD b6 : l6;\
			WORD b7 : l7;\
			WORD b8 : l8;\
			WORD b9 : l9;\
		} b;\
		WORD a;\
	} name

#define	___BW23(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7, b8, l8, b9, l9, b10, l10)\
	union {\
		struct {\
			WORD b0 : l0;\
			WORD b1 : l1;\
			WORD b2 : l2;\
			WORD b3 : l3;\
			WORD b4 : l4;\
			WORD b5 : l5;\
			WORD b6 : l6;\
			WORD b7 : l7;\
			WORD b8 : l8;\
			WORD b9 : l9;\
			WORD b10 : l10;\
		} b;\
		WORD a;\
	} name

#define	___BW25(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7, b8, l8, b9, l9, b10, l10, b11, l11)\
	union {\
		struct {\
			WORD b0 : l0;\
			WORD b1 : l1;\
			WORD b2 : l2;\
			WORD b3 : l3;\
			WORD b4 : l4;\
			WORD b5 : l5;\
			WORD b6 : l6;\
			WORD b7 : l7;\
			WORD b8 : l8;\
			WORD b9 : l9;\
			WORD b10 : l10;\
			WORD b11 : l11;\
		} b;\
		WORD a;\
	} name

#define	___BW27(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7, b8, l8, b9, l9, b10, l10, b11, l11, b12, l12)\
	union {\
		struct {\
			WORD b0 : l0;\
			WORD b1 : l1;\
			WORD b2 : l2;\
			WORD b3 : l3;\
			WORD b4 : l4;\
			WORD b5 : l5;\
			WORD b6 : l6;\
			WORD b7 : l7;\
			WORD b8 : l8;\
			WORD b9 : l9;\
			WORD b10 : l10;\
			WORD b11 : l11;\
			WORD b12 : l12;\
		} b;\
		WORD a;\
	} name

#define	___BW29(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7, b8, l8, b9, l9, b10, l10, b11, l11, b12, l12, b13, l13)\
	union {\
		struct {\
			WORD b0 : l0;\
			WORD b1 : l1;\
			WORD b2 : l2;\
			WORD b3 : l3;\
			WORD b4 : l4;\
			WORD b5 : l5;\
			WORD b6 : l6;\
			WORD b7 : l7;\
			WORD b8 : l8;\
			WORD b9 : l9;\
			WORD b10 : l10;\
			WORD b11 : l11;\
			WORD b12 : l12;\
			WORD b13 : l13;\
		} b;\
		WORD a;\
	} name

#define	___BW31(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7, b8, l8, b9, l9, b10, l10, b11, l11, b12, l12, b13, l13, b14, l14)\
	union {\
		struct {\
			WORD b0 : l0;\
			WORD b1 : l1;\
			WORD b2 : l2;\
			WORD b3 : l3;\
			WORD b4 : l4;\
			WORD b5 : l5;\
			WORD b6 : l6;\
			WORD b7 : l7;\
			WORD b8 : l8;\
			WORD b9 : l9;\
			WORD b10 : l10;\
			WORD b11 : l11;\
			WORD b12 : l12;\
			WORD b13 : l13;\
			WORD b14 : l14;\
		} b;\
		WORD a;\
	} name

#define	___BW33(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7, b8, l8, b9, l9, b10, l10, b11, l11, b12, l12, b13, l13, b14, l14, b15, l15)\
	union {\
		struct {\
			WORD b0 : l0;\
			WORD b1 : l1;\
			WORD b2 : l2;\
			WORD b3 : l3;\
			WORD b4 : l4;\
			WORD b5 : l5;\
			WORD b6 : l6;\
			WORD b7 : l7;\
			WORD b8 : l8;\
			WORD b9 : l9;\
			WORD b10 : l10;\
			WORD b11 : l11;\
			WORD b12 : l12;\
			WORD b13 : l13;\
			WORD b14 : l14;\
			WORD b15 : l15;\
		} b;\
		WORD a;\
	} name

#elif	defined(__BIGENDIAN__)

#define	___BC3(name, b0, l0)\
	union {\
		struct {\
			BYTE b0 : l0;\
		} b;\
		BYTE a;\
	} name

#define	___BC5(name, b0, l0, b1, l1)\
	union {\
		struct {\
			BYTE b1 : l1;\
			BYTE b0 : l0;\
		} b;\
		BYTE a;\
	} name

#define	___BC7(name, b0, l0, b1, l1, b2, l2)\
	union {\
		struct {\
			BYTE b2 : l2;\
			BYTE b1 : l1;\
			BYTE b0 : l0;\
		} b;\
		BYTE a;\
	} name

#define	___BC9(name, b0, l0, b1, l1, b2, l2, b3, l3)\
	union {\
		struct {\
			BYTE b3 : l3;\
			BYTE b2 : l2;\
			BYTE b1 : l1;\
			BYTE b0 : l0;\
		} b;\
		BYTE a;\
	} name

#define	___BC11(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4)\
	union {\
		struct {\
			BYTE b4 : l4;\
			BYTE b3 : l3;\
			BYTE b2 : l2;\
			BYTE b1 : l1;\
			BYTE b0 : l0;\
		} b;\
		BYTE a;\
	} name

#define	___BC13(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5)\
	union {\
		struct {\
			BYTE b5 : l5;\
			BYTE b4 : l4;\
			BYTE b3 : l3;\
			BYTE b2 : l2;\
			BYTE b1 : l1;\
			BYTE b0 : l0;\
		} b;\
		BYTE a;\
	} name

#define	___BC15(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6)\
	union {\
		struct {\
			BYTE b6 : l6;\
			BYTE b5 : l5;\
			BYTE b4 : l4;\
			BYTE b3 : l3;\
			BYTE b2 : l2;\
			BYTE b1 : l1;\
			BYTE b0 : l0;\
		} b;\
		BYTE a;\
	} name

#define	___BC17(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7)\
	union {\
		struct {\
			BYTE b7 : l7;\
			BYTE b6 : l6;\
			BYTE b5 : l5;\
			BYTE b4 : l4;\
			BYTE b3 : l3;\
			BYTE b2 : l2;\
			BYTE b1 : l1;\
			BYTE b0 : l0;\
		} b;\
		BYTE a;\
	} name

#define	___BW3(name, b0, l0)\
	union {\
		struct {\
			WORD b0 : l0;\
		} b;\
		WORD a;\
	} name

#define	___BW5(name, b0, l0, b1, l1)\
	union {\
		struct {\
			WORD b1 : l1;\
			WORD b0 : l0;\
		} b;\
		WORD a;\
	} name

#define	___BW7(name, b0, l0, b1, l1, b2, l2)\
	union {\
		struct {\
			WORD b2 : l2;\
			WORD b1 : l1;\
			WORD b0 : l0;\
		} b;\
		WORD a;\
	} name

#define	___BW9(name, b0, l0, b1, l1, b2, l2, b3, l3)\
	union {\
		struct {\
			WORD b3 : l3;\
			WORD b2 : l2;\
			WORD b1 : l1;\
			WORD b0 : l0;\
		} b;\
		WORD a;\
	} name

#define	___BW11(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4)\
	union {\
		struct {\
			WORD b4 : l4;\
			WORD b3 : l3;\
			WORD b2 : l2;\
			WORD b1 : l1;\
			WORD b0 : l0;\
		} b;\
		WORD a;\
	} name

#define	___BW13(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5)\
	union {\
		struct {\
			WORD b5 : l5;\
			WORD b4 : l4;\
			WORD b3 : l3;\
			WORD b2 : l2;\
			WORD b1 : l1;\
			WORD b0 : l0;\
		} b;\
		WORD a;\
	} name

#define	___BW15(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6)\
	union {\
		struct {\
			WORD b6 : l6;\
			WORD b5 : l5;\
			WORD b4 : l4;\
			WORD b3 : l3;\
			WORD b2 : l2;\
			WORD b1 : l1;\
			WORD b0 : l0;\
		} b;\
		WORD a;\
	} name

#define	___BW17(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7)\
	union {\
		struct {\
			WORD b7 : l7;\
			WORD b6 : l6;\
			WORD b5 : l5;\
			WORD b4 : l4;\
			WORD b3 : l3;\
			WORD b2 : l2;\
			WORD b1 : l1;\
			WORD b0 : l0;\
		} b;\
		WORD a;\
	} name

#define	___BW19(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7, b8, l8)\
	union {\
		struct {\
			WORD b8 : l8;\
			WORD b7 : l7;\
			WORD b6 : l6;\
			WORD b5 : l5;\
			WORD b4 : l4;\
			WORD b3 : l3;\
			WORD b2 : l2;\
			WORD b1 : l1;\
			WORD b0 : l0;\
		} b;\
		WORD a;\
	} name

#define	___BW21(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7, b8, l8, b9, l9)\
	union {\
		struct {\
			WORD b9 : l9;\
			WORD b8 : l8;\
			WORD b7 : l7;\
			WORD b6 : l6;\
			WORD b5 : l5;\
			WORD b4 : l4;\
			WORD b3 : l3;\
			WORD b2 : l2;\
			WORD b1 : l1;\
			WORD b0 : l0;\
		} b;\
		WORD a;\
	} name

#define	___BW23(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7, b8, l8, b9, l9, b10, l10)\
	union {\
		struct {\
			WORD b10 : l10;\
			WORD b9 : l9;\
			WORD b8 : l8;\
			WORD b7 : l7;\
			WORD b6 : l6;\
			WORD b5 : l5;\
			WORD b4 : l4;\
			WORD b3 : l3;\
			WORD b2 : l2;\
			WORD b1 : l1;\
			WORD b0 : l0;\
		} b;\
		WORD a;\
	} name

#define	___BW25(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7, b8, l8, b9, l9, b10, l10, b11, l11)\
	union {\
		struct {\
			WORD b11 : l11;\
			WORD b10 : l10;\
			WORD b9 : l9;\
			WORD b8 : l8;\
			WORD b7 : l7;\
			WORD b6 : l6;\
			WORD b5 : l5;\
			WORD b4 : l4;\
			WORD b3 : l3;\
			WORD b2 : l2;\
			WORD b1 : l1;\
			WORD b0 : l0;\
		} b;\
		WORD a;\
	} name

#define	___BW27(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7, b8, l8, b9, l9, b10, l10, b11, l11, b12, l12)\
	union {\
		struct {\
			WORD b12 : l12;\
			WORD b11 : l11;\
			WORD b10 : l10;\
			WORD b9 : l9;\
			WORD b8 : l8;\
			WORD b7 : l7;\
			WORD b6 : l6;\
			WORD b5 : l5;\
			WORD b4 : l4;\
			WORD b3 : l3;\
			WORD b2 : l2;\
			WORD b1 : l1;\
			WORD b0 : l0;\
		} b;\
		WORD a;\
	} name

#define	___BW29(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7, b8, l8, b9, l9, b10, l10, b11, l11, b12, l12, b13, l13)\
	union {\
		struct {\
			WORD b13 : l13;\
			WORD b12 : l12;\
			WORD b11 : l11;\
			WORD b10 : l10;\
			WORD b9 : l9;\
			WORD b8 : l8;\
			WORD b7 : l7;\
			WORD b6 : l6;\
			WORD b5 : l5;\
			WORD b4 : l4;\
			WORD b3 : l3;\
			WORD b2 : l2;\
			WORD b1 : l1;\
			WORD b0 : l0;\
		} b;\
		WORD a;\
	} name

#define	___BW31(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7, b8, l8, b9, l9, b10, l10, b11, l11, b12, l12, b13, l13, b14, l14)\
	union {\
		struct {\
			WORD b14 : l14;\
			WORD b13 : l13;\
			WORD b12 : l12;\
			WORD b11 : l11;\
			WORD b10 : l10;\
			WORD b9 : l9;\
			WORD b8 : l8;\
			WORD b7 : l7;\
			WORD b6 : l6;\
			WORD b5 : l5;\
			WORD b4 : l4;\
			WORD b3 : l3;\
			WORD b2 : l2;\
			WORD b1 : l1;\
			WORD b0 : l0;\
		} b;\
		WORD a;\
	} name

#define	___BW33(name, b0, l0, b1, l1, b2, l2, b3, l3, b4, l4, b5, l5, b6, l6, b7, l7, b8, l8, b9, l9, b10, l10, b11, l11, b12, l12, b13, l13, b14, l14, b15, l15)\
	union {\
		struct {\
			WORD b15 : l15;\
			WORD b14 : l14;\
			WORD b13 : l13;\
			WORD b12 : l12;\
			WORD b11 : l11;\
			WORD b10 : l10;\
			WORD b9 : l9;\
			WORD b8 : l8;\
			WORD b7 : l7;\
			WORD b6 : l6;\
			WORD b5 : l5;\
			WORD b4 : l4;\
			WORD b3 : l3;\
			WORD b2 : l2;\
			WORD b1 : l1;\
			WORD b0 : l0;\
		} b;\
		WORD a;\
	} name

#else
	#error	Not specified __BIGENDIAN__ or __LITTILEENDIAN__ !!!
#endif

#if	!defined(_WIN32)

#define __BCB(n)	___BC##n
#define	__BCA(n)	__BCB(n)
#define	__BC(...)	__BCA(NARG36(__VA_ARGS__))(__VA_ARGS__)

#define	__BWB(n)	___BW##n
#define	__BWA(n)	__BWB(n)
#define	__BW(...)	__BWA(NARG36(__VA_ARGS__))(__VA_ARGS__)

#else

#define __BCD(n)	___BC##n
#define	__BCC(n)	__BCD(n)
#define	__BCB(n)	__BCC(n)
#define	__BCA(x, y)	x y
#define	__BC(...)	__BCA(__BCB(NARG36(__VA_ARGS__)),(__VA_ARGS__))

#define __BWD(n)	___BW##n
#define	__BWC(n)	__BWD(n)
#define	__BWB(n)	__BWC(n)
#define	__BWA(x, y)	x y
#define	__BW(...)	__BWA(__BWB(NARG36(__VA_ARGS__)),(__VA_ARGS__))

#endif

#ifdef	__cplusplus
}
#endif
