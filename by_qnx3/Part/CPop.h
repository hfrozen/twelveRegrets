/**
 * @file	CPop.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once
#include <pthread.h>
#include <semaphore.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <inttypes.h>

#include "CTool.h"

class CSemaphore
{
public:
	CSemaphore()
	{
		sem_init(&c_semaphore, 0, 0);
	}
	virtual ~CSemaphore()
	{
		sem_destroy(&c_semaphore);
	}

	int Init()
	{
		return sem_init(&c_semaphore, 0, 0);
	}

	int Post()
	{
		return sem_post(&c_semaphore);
	}

	int Wait()
	{
		return sem_wait(&c_semaphore);
	}

private:
	sem_t	c_semaphore;
};

class CMutex
{
public:
	CMutex()
	{
		pthread_mutex_init(&c_mutex, NULL);
	}
	virtual ~CMutex()
	{
		Destroy();
	}

	int Init()
	{
		return pthread_mutex_init(&c_mutex, NULL);
	}

	int Lock()
	{
		return pthread_mutex_lock(&c_mutex);
	}

	int Unlock()
	{
		return pthread_mutex_unlock(&c_mutex);
	}

	int Destroy()
	{
		return pthread_mutex_destroy(&c_mutex);
	}

private:
	pthread_mutex_t	c_mutex;
};

template <typename T>
class CCalm
{
public:
	CCalm()		{ cur = 0; buf = 0; deb = 0; cycle = 5; }

	T		cur;
	T		buf;
	BYTE	deb;
	BYTE	cycle;

	void	SetDebounceTime(BYTE cDeb)	{ cycle = cDeb; }
	bool	Stabilize(T present)
	{
		if (buf != present) {
			buf = present;
			deb = cycle;
		}
		else if (deb > 0 && -- deb == 0) {
			cur = buf;
			return true;
		}
		return false;
	}
};

template <typename T>
class CCouple
{
public:
	CCouple()	{ Clear(); }

	bool	bInit;
	bool	bAction;

	typedef struct _tagINDICATE {
		T	cur;
		T	buf;
	} _INDICATE;

	enum enINDIC {
		IND_LEFT = 0,
		IND_RIGHT,
		IND_MAX
	};

	typedef struct _tagORDER {
		_INDICATE	ind[IND_MAX];
		bool		bSide;
	} _ORDER;
	_ORDER	c_order;

	void Clear() {
		c_order.ind[IND_LEFT].cur = 0;	c_order.ind[IND_LEFT].buf = 0;
		c_order.ind[IND_RIGHT].cur = 0;	c_order.ind[IND_RIGHT].buf = 0;
		c_order.bSide = false;
		bAction = false;
		bInit = false;
	}

	void Init(T right) {
		Clear();
		c_order.ind[IND_RIGHT].cur = c_order.ind[IND_RIGHT].buf = right;
		bInit = true;
	}

	void Init(T left, T right) {
		Clear();
		c_order.ind[IND_LEFT].cur = c_order.ind[IND_LEFT].buf = left;
		c_order.ind[IND_RIGHT].cur = c_order.ind[IND_RIGHT].buf = right;
		bInit = true;
	}

	// 180511
	void Init(T left, T right, bool bSide) {
		Clear();
		c_order.ind[IND_LEFT].cur = c_order.ind[IND_LEFT].buf = left;
		c_order.ind[IND_RIGHT].cur = c_order.ind[IND_RIGHT].buf = right;
		c_order.bSide = bSide;
		bAction = true;
		bInit = true;
	}

	void InitSide(bool bSide) {
		c_order.bSide = bSide;
		bAction = true;
		bInit = true;
	}

	// 171027
	T Check(T left) {
		c_order.ind[IND_LEFT].cur = left;
		if (c_order.ind[IND_LEFT].buf != c_order.ind[IND_LEFT].cur) {
			c_order.ind[IND_LEFT].buf = c_order.ind[IND_LEFT].cur;
			c_order.bSide = false;
			bAction = true;
			return c_order.ind[IND_LEFT].cur;
		}
		return c_order.ind[IND_LEFT].cur;
	}

	T Check(T left, T right) {
		c_order.ind[IND_LEFT].cur = left;
		c_order.ind[IND_RIGHT].cur = right;
		if (c_order.ind[IND_LEFT].buf != c_order.ind[IND_LEFT].cur) {
			c_order.ind[IND_LEFT].buf = c_order.ind[IND_LEFT].cur;
			//c_order.ind[IND_TAIL].buf = c_order.ind[IND_TAIL].cur;
			c_order.bSide = false;
			bAction = true;
			return c_order.ind[IND_LEFT].cur;
		}
		else if (c_order.ind[IND_RIGHT].buf != c_order.ind[IND_RIGHT].cur) {
			c_order.ind[IND_RIGHT].buf = c_order.ind[IND_RIGHT].cur;
			c_order.bSide = true;
			bAction = true;
			return c_order.ind[IND_RIGHT].cur;
		}
		if (c_order.bSide)	return c_order.ind[IND_RIGHT].cur;
		else	return c_order.ind[IND_LEFT].cur;
	}

	bool GetSide()	{ return c_order.bSide; }
	bool GetAct()	{ return bAction; }
	bool IsInit()	{ return bInit; }
};

class CIntvMon
{
public:
	CIntvMon()		{ Initial(); }

private:
	bool		c_bInit;
	uint64_t	clk;

public:
	void	Initial()
	{
		c_bInit = false;
	}

	bool	IsSmall(double dbSec, const char* pMsg)
	{
		bool res = false;
		if (!c_bInit) {
			clk = ClockCycles();
			c_bInit = true;
		}
		else {
			uint64_t cur = ClockCycles();
			double us = (double)(cur - clk) / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;
			if (us < dbSec) {
				res = true;
				GETTOOL(pTool);
				pTool->Rollf("%s %fms!\n", pMsg, us * 1e3);
			}
			clk = cur;
		}
		return res;
	}

	bool	IsBig(double dbSec, const char* pMsg)
	{
		bool res = false;
		if (!c_bInit) {
			clk = ClockCycles();
			c_bInit = true;
		}
		else {
			uint64_t cur = ClockCycles();
			double us = (double)(cur - clk) / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;
			if (us > dbSec) {
				res = true;
				GETTOOL(pTool);
				pTool->Rollf("%s %fms!\n", pMsg, us * 1e3);
			}
			clk = cur;
		}
		return res;
	}
};
