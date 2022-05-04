; Scca.s

.section	.bss

.extern		lm		;.rand	lm
.extern		cCare	;.rand	cCare

.equ	SYNC_MODE,		0x30
.equ	WAIT_RXCLOSE,	10
.equ	WAIT_RXCLOSEA,	8
.equ	WAIT_TXCLOSE,	2

.equ	BLOCK_SIZE,		256
.equ	BUF_SIZE,		122
.equ	BUFP_SIZE,		4

;.equ	_CSTATE,		lm
;.equ	_ASCCB,			(lm + 1)
.equ	_ARXB,			(lm + 1)
.equ	_ARXI,			(lm + 1 + BUF_SIZE + 0)
.equ	_ARXR,			(lm + 1 + BUF_SIZE + 1)
.equ	_ARXCW,			(lm + 1 + BUF_SIZE + 2)
.equ	_ATXB,			(lm + 1 + BUF_SIZE + BUFP_SIZE)
.equ	_ATXI,			(lm + 1 + BUF_SIZE * 2 + BUFP_SIZE + 0)
.equ	_ATXR,			(lm + 1 + BUF_SIZE * 2 + BUFP_SIZE + 1)
.equ	_ATXCW,			(lm + 1 + BUF_SIZE * 2 + BUFP_SIZE + 2)
.equ	_AMODE,			(lm + 1 + (BUF_SIZE + BUFP_SIZE) * 2)

.section	.text

.global		__vector_5
.global		__vector_6
.global		__vector_7
.global		__vector_8

__vector_5:
		push	r24
		push	r25
		in		r24, 0x3f
		cli
		push	r24
		eor		r25, r25
		in		r24, 0x3b
		push	r24
		ldi		r24, 0
		ldi		r25, 1
		call	SccExpress
		pop		r24
		out		0x3b, r24
		pop		r24
		out		0x3f, r24
		pop		r25
		pop		r24
		reti
;

__vector_6:
		push	r24
		push	r25
		in		r24, 0x3f
		cli
		push	r24
		eor		r25, r25
		in		r24, 0x3b
		push	r24
		ldi		r24, 1
		ldi		r25, 4
		call	SccExpress
		pop		r24
		out		0x3b, r24
		pop		r24
		out		0x3f, r24
		pop		r25
		pop		r24
		reti
;

__vector_7:
		push	r24
		push	r25
		in		r24, 0x3f
		cli
		push	r24
		eor		r25, r25
		in		r24, 0x3b
		push	r24
		ldi		r24, 2
		ldi		r25, 0x10
		call	SccExpress
		pop		r24
		out		0x3b, r24
		pop		r24
		out		0x3f, r24
		pop		r25
		pop		r24
		reti
;

__vector_8:
		push	r24
		push	r25
		in		r24, 0x3f
		cli
		push	r24
		eor		r25, r25
		in		r24, 0x3b
		push	r24
		ldi		r24, 3
		ldi		r25, 0x40
		call	SccExpress
		pop		r24
		out		0x3b, r24
		pop		r24
		out		0x3f, r24
		pop		r25
		pop		r24
		reti
;

;.equ	CID,			r24		; chip id = 0,1,2,3
;.equ	NI						; n
;.equ	CBID,			r25		; chip bit id = 1, 4, 0x10, 0x40
;.equ	BLK,			r22
;.equ	BLKL,			r22
;.equ	BLKH,			r23
;
;		r24 has cid,
;		r25 has bit index for cState,

SccExpress:
		push	r18
		push	r19
		push	r20
		push	r21
		push	r22
		push	r23
		push	r26
		push	r27
		push	r28
		push	r29
		push	r30
		push	r31

		in		r18, 0x16		; PORTB
		andi	r18, 0xfd
		out		0x18, r18

		mov		ZH, r24			; CID			; di << 8
		ori		ZH, 0x20		; z has 0x2n00

SccExpLoop:
		; SCCC(di, 0) = 3;		// rrf3
		ldi		r21, 3
		ldi		ZL, 1			; 0x2n01 -> ch. a,control
		st		Z, r21			; (0x2n01) = 3

		; state = SCCC(di, 0);
		ldi		r21, 0
		ld		r20, Z			; r20 = rr3
		cp		r20, r21
		brne	SccExpScan

		in		r18, 0x16		; portb
		ori		r18, 2
		out		0x18, r18

		pop		r31
		pop		r30
		pop		r29
		pop		r28
		pop		r27
		pop		r26
		pop		r23
		pop		r22
		pop		r21
		pop		r20
		pop		r19
		pop		r18
		ret

SccExpScan:
		mov		r23, r24	; CID	; (di * 2) << 8
		lsl		r23			; 0, 0x200, 0x400, 0x600
		eor		r22, r22

SccExpScanRxA:
		;if (state & 0x20) {	// rx ch. a
		sbrs	r20, 5
		rjmp	SccExpScanTxA

		;	BYTE c = SCC!!D!!(di, 0);
		ldi		ZL, 3			; 0x2n03, !! ch. a,data
		ld		r21, Z			; r21 = data

		;	if (!(lm.scc.cState & RXFBYDICH(di, 0)) && lm.scc.pt[di << 2].rx.ci < SIZE_SCCBUF) {
		lds		r18, lm			; lm.scc.cState,
		and		r18, r25		; cState & bit id
		breq	SccExpScanRxA1
		rjmp	SccExpScanTxA
SccExpScanRxA1:
		movw	Y, r22
		subi	YL, lo8(-(_ARXI))	; lm.scc.pt[n].rx.ci
		sbci	YH, hi8(-(_ARXI))
		ld		r18, Y			; r18 = ci;
		ldi		r19, BUF_SIZE
		cp		r18, r19
		brcs	SccExpReadA		; carry = 1 -> r18 < r19
		rjmp	SccExpScanTxA

SccExpReadA:
		;		pSccCh->RXB[pSccCh->RXI ++] = c;
		movw	X, r22
		eor		r19, r19
		add		XL, r18
		adc		XH, r19
		subi	XL, lo8(-(_ARXB))	; rx[ci]
		sbci	XH, hi8(-(_ARXB))
		st		X, r21
		inc		r18
		st		Y, r18

		;		if (pSccCh->cf.tp.b.mode == SCCMODE_SYNC) {
		movw	Y, r22
		subi	YL, lo8(-(_AMODE))
		sbci	YH, hi8(-(_AMODE))
		ld		r18, Y			; mode B4,5
		andi	r18, SYNC_MODE
		ldi		r19, SYNC_MODE
		eor		r18, r19
		brne	SccExpReadAsyncA

SccExpReadSyncA:
		;			SCCC(di, 0) = 1;
		ldi		r18, 1
		ldi		ZL, 1			; 0x2n01 -> ch. a,control
		st		Z, r18

		;			c = SCCC(di, 0);
		ld		r18, Z

		;#if	defined(_CARE_)
		;			if (c & 0x80) {
		sbrs	r18, 7
		rjmp	SccExpScanTxA
		;				if (pSccCh->RXCT == 0)
		movw	Y, r22
		subi	YL, lo8(-(_ARXCW))
		sbci	YH, hi8(-(_ARXCW))
		eor		r19, r19
		ld		r18, Y
		cp		r18, r19
		brne	SccExpReadSyncA1
		;					pSccCh->RXCT = WAITCLOSE;
		ldi		r18, WAIT_RXCLOSE
		st		Y, r18
		rjmp	SccExpScanTxA

SccExpReadSyncA1:
		;				else	cCare |= RXFBYDICH(di, 0);
		lds		r18, cCare
		or		r18, r25
		sts		cCare, r18
		rjmp	SccExpScanTxA

		;			}
		;#else
		;			if (c & 0x80)	pSccCh->RXCT = WAIT_RXCLOSE;
		;sbrs	r18, 7
		;rjmp	SccExpScanTxA
		;movw	Y, r22
		;subi	YL, lo8(-(_ARXCW))
		;sbci	YH, hi8(-(_ARXCW))
		;ldi		r18, WAIT_RXCLOSE
		;st		Y, r18
		;rjmp	SccExpScanTxA
		;#endif
		;		}

SccExpReadAsyncA:
		;	else {
		;		if (pSccCh->TXCT != 0 || pSccCh->RXR != 0)
		eor		r19, r19
		movw	Y, r22
		subi	YL, lo8(-(_ATXCW))
		sbci	YH,	hi8(-(_ATXCW))
		ld		r18, Y
		cp		r18, r19
		brne	SccExpReadAsyncA1
		movw	Y, r22
		subi	YL, lo8(-(_ARXR))
		sbci	YH, hi8(-(_ARXR))
		ld		r18, Y
		cp		r18, r19
		breq	SccExpReadAsyncA2

SccExpReadAsyncA1:
		;			-- pSccCh->RXI;
		movw	Y, r22
		subi	YL, lo8(-(_ARXI))
		sbci	YH, hi8(-(_ARXI))
		ld		r18, Y
		cp		r18, r19
		breq	SccExpScanTxA
		dec		r18
		st		Y, r18
		rjmp	SccExpScanTxA
				
SccExpReadAsyncA2:
		;		else	pSccCh->RXCT = WAIT_RXCLOSE;
		movw	Y, r22
		subi	YL, lo8(-(_ARXCW))
		sbci	YH, hi8(-(_ARXCW))
		ldi		r18, WAIT_RXCLOSEA
		st		Y, r18
		;		}
		;	}
		;}

SccExpScanTxA:
		;if (state & 0x10) {
		sbrs	r20, 4
		rjmp	SccExpScanEsA

		;	if (pSccCh->TXI < pSccCh->TXR)
		movw	Y, r22
		subi	YL, lo8(-(_ATXI))
		sbci	YH, hi8(-(_ATXI))
		ld		r18, Y
		movw	X, r22
		subi	XL, lo8(-(_ATXR))
		sbci	XH, hi8(-(_ATXR))
		ld		r19, X
		cp		r18, r19
		brsh	SccExpWriteA

		;		SCCD(di, 0) = pSccCh->TXB[pSccCh->TXI ++];
		eor		r19, r19		; r18 = txi
		movw	X, r22
		add		XL, r18
		adc		XH, r19
		subi	XL, lo8(-(_ATXB))
		sbci	XH, hi8(-(_ATXB))
		ld		r19, X
		ldi		ZL, 3			; 0x2n03, ch. a,data
		st		Z, r19
		inc		r18
		st		Y, r18
		rjmp	SccExpScanEsA

SccExpWriteA:
		;	else {
		;		SCCC(di, 0) = 0x28;
		ldi		r19, 0x28
		ldi		ZL, 1			; 0x2n01, ch. a,control
		st		Z, r19

		;		pSccCh->TXCT = WAIT_TXCLOSE;
		movw	Y, r22
		subi	YL, lo8(-(_ATXCW))
		sbci	YH, hi8(-(_ATXCW))
		ldi		r19, WAIT_TXCLOSE
		st		Y, r19
		;	}
		;}

SccExpScanEsA:
		;if (state & 8)		SCCC(di, 0) = 0x10;	// reset ext/status int
		sbrs	r20, 3
		rjmp	SccExpScanRxB
		ldi		r19, 0x10
		ldi		ZL, 1			; 0x2n01, ch. a,control
		st		Z, r19

SccExpScanRxB:
		inc		r23			; 0, 0x200, 0x400, 0x600 -> 0x100, 0x300, 0x500, 0x700
		lsl		r25			; cbid = 1, 4, 0x10, 0x40 -> 2, 8, 0x20, 0x80
		;if (state & 4) {	// rx ch. b
		sbrs	r20, 2
		rjmp	SccExpScanTxB

		;	BYTE c = SCCD(di, 1);
		ldi		ZL, 2			; 0x2n02, ch. b,data
		ld		r21, Z

		;	if (!(lm.scc.cState & RXFBYDICH(di, 1)) && pSccCh->RXI < SIZE_SCCBUF) {
		lds		r18, lm
		and		r18, r25
		breq	SccExpScanRxB1
		rjmp	SccExpScanTxB
SccExpScanRxB1:
		movw	Y, r22
		subi	YL, lo8(-(_ARXI))
		sbci	YH, hi8(-(_ARXI))
		ld		r18, Y
		ldi		r19, BUF_SIZE
		cp		r18, r19
		brcs	SccExpReadB		; carry = 1
		rjmp	SccExpScanTxB

SccExpReadB:
		;		pSccCh->RXB[pSccCh->RXI ++] = c;
		movw	X, r22
		eor		r19, r19
		add		XL, r18
		adc		XH, r19
		subi	XL, lo8(-(_ARXB))
		sbci	XH, hi8(-(_ARXB))
		st		X, r21
		inc		r18
		st		Y, r18

		;		if (pSccCh->cf.tp.b.mode == SCCMODE_SYNC) {
		movw	Y, r22
		subi	YL, lo8(-(_AMODE))
		sbci	YH, hi8(-(_AMODE))
		ld		r18, Y
		andi	r18, SYNC_MODE
		ldi		r19, SYNC_MODE
		eor		r18, 19
		brne	SccExpReadAsyncB

SccExpReadSyncB:
		;			SCCC(di, 1) = 1;
		ldi		r18, 1
		ldi		ZL, 0			; 0x2n00 -> ch. b, control
		st		Z, r18

		;			c = SCCC(di, 1);
		ld		r18, Z

		;#if	defined(_CARE_)
		;			if (c & 0x80) {
		sbrs	r18, 7
		rjmp	SccExpScanTxB

		;				if (pSccCh->RXCT == 0)
		movw	Y, r22
		subi	YL, lo8(-(_ARXCW))
		sbci	YH, hi8(-(_ARXCW))
		eor		r19, r19
		ld		r18, Y
		cp		r18, r19
		brne	SccExpReadSyncB1

		;					pSccCh->RXCT = WAIT_RXCLOSE;
		ldi		r18, WAIT_RXCLOSE
		st		Y, r18
		rjmp	SccExpScanTxB

SccExpReadSyncB1:
		;				else	cCare |= RXFBYDICH(di, 1);
		lds		r18, cCare
		or		r18, r25
		sts		cCare, r18
		rjmp	SccExpScanTxB

		;			}
		;#else
		;			if (c & 0x80)	pSccCh->RXCT = WAIT_RXCLOSE;
		;sbrs	r18, 7
		;rjmp	SccExpScanTxB
		;movw	Y, r22
		;subi	YL, lo8(-(_ARXCW))
		;sbci	YH, hi8(-(_ARXCW))
		;ldi		r18, WAIT_RXCLOSE
		;st		Y, r18
		;rjmp	SccExpScanTxB
		;#endif
		;		}

SccExpReadAsyncB:
		;		else {
		;			if (pSccCh->TXCT != 0 || pSccCh->RXR != 0)
		eor		r19, r19
		movw	Y, r22
		subi	YL, lo8(-(_ATXCW))
		sbci	YH, hi8(-(_ATXCW))
		ld		r18, Y
		cp		r18, r19
		brne	SccExpReadAsyncB1
		movw	Y, r22
		subi	YL, lo8(-(_ARXR))
		sbci	YH, hi8(-(_ARXR))
		ld		r18, Y
		cp		r18, r19
		breq	SccExpReadAsyncB2

SccExpReadAsyncB1:
		;				-- pSccCh->RXI;
		movw	Y, r22
		subi	YL, lo8(-(_ARXI))
		sbci	YH, hi8(-(_ARXI))
		ld		r18, Y
		cp		r18, r19
		breq	SccExpScanTxB
		dec		r18
		st		Y, r18
		rjmp	SccExpScanTxB

SccExpReadAsyncB2:
		;			else	pSccCh->RXCT = WAIT_RXCLOSE;
		movw	Y, r22
		subi	YL, lo8(-(_ARXCW))
		sbci	YH, hi8(-(_ARXCW))
		ldi		r18, WAIT_RXCLOSEA
		st		Y, r18
		;		}
		;	}
		;}

SccExpScanTxB:
		;if (state & 2) {
		sbrs	r20, 1
		rjmp	SccExpScanEsB

		;	if (pSccCh->TXI < pSccCh->TXR)
		movw	Y, r22
		subi	YL, lo8(-(_ATXI))
		sbci	YH, hi8(-(_ATXI))
		ld		r18, Y
		movw	X, r22
		subi	XL, lo8(-(_ATXR))
		sbci	XH, hi8(-(_ATXR))
		ld		r19, X
		cp		r18, r19
		brsh	SccExpWriteB

		;		SCCD(di, 1) = pSccCh->TXB[pSccCh->TXI ++];
		eor		r19, r19
		movw	X, r22
		add		XL, r18
		adc		XH, r19
		subi	XL, lo8(-(_ATXB))
		sbci	XH,	hi8(-(_ATXB))
		ld		r19, X
		ldi		ZL, 2			; 0x2n02, ch. b, data
		st		Z, r19
		inc		r18
		st		Y, r18
		rjmp	SccExpScanEsB

SccExpWriteB:
		;	else {
		;		SCCC(di, 1) = 0x28;
		ldi		r19, 0x28
		ldi		ZL, 0			; 0x2n00, ch. b,control
		st		Z, r19

		;		pSccCh->TXCT = WAIT_TXCLOSE;
		movw	Y, r22
		subi	YL, lo8(-(_ATXCW))
		sbci	YH, hi8(-(_ATXCW))
		ldi		r19, WAIT_TXCLOSE
		st		Y, r19
		;	}
		;}

SccExpScanEsB:
		;if (state & 1)		SCCC(di, 1) = 0x10;	// reset ext/status int
		sbrs	r20, 0
		rjmp	SccExpEnd
		ldi		r19, 0x10
		ldi		ZL, 0			; 0x2n00, ch. b,control
		st		Z, r19

		;//SCCC(di, 1) = 0x38;
	;} while (state != 0);
SccExpEnd:
		rjmp	SccExpLoop
;
;

