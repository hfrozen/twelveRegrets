/**
 * @file	CString.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once
#include <string>
#include <vector>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

class CString
{
public:
	CString() {};
	CString(const char* p)	{ c_string = p; }
	~CString() {};

protected:
	std::string	c_string;

public:
	CString(const CString& cs)				{ c_string = cs.c_string; }
	void operator = (const CString& cs)		{ c_string = cs.c_string; }
	void operator = (const char* p)			{ if (p != NULL) c_string = p; }
	CString& operator += (const char* p)	{ if (p != NULL) c_string += p; return(*this); }

	bool operator == (const char* p)		{ return (c_string.compare(p) == 0); }
	operator const char* ()					{ return c_string.c_str(); }
	int GetLength()							{ return c_string.length(); }
	bool IsEmpty()							{ return c_string.empty(); }

	int Find(const char ch, int iBegin = 0)	{ return c_string.find(ch, iBegin); }

	int Find(const char* psz, int iBegin = 0)
	{
		if (psz == NULL)	return -1;
		return c_string.find(psz, iBegin);
	}

	int FindOneOf(const char* psz, int iBegin = 0)
	{
		if (psz == NULL)	return -1;
		while (*psz != '\0') {
			int pos = Find(*psz, iBegin);
			if (pos >= 0)	return pos;
			++ psz;
		}
		return -1;
	}

	friend CString operator + (const CString& str, const char* psz)
	{
		CString cs;
		cs.c_string = str.c_string;
		if (psz != NULL)
			cs.c_string = cs.c_string + psz;
		return cs;
	}

	CString Left(int iLength)
	{
		CString cs;
		try {
			cs.c_string = c_string.substr(0, iLength);
		}
		catch (...) {
		}
		return cs;
	}

	CString Mid(int iBegin, int iCount)
	{
		CString cs;
		try {
			cs.c_string = c_string.substr(iBegin, iCount);
		}
		catch (...) {
		}
		return cs;
	}

	CString Mid(int iBegin)
	{
		CString cs;
		try {
			cs.c_string = c_string.substr(iBegin);
		}
		catch (...) {
		}
		return cs;
	}

	int Delete(int iIndex, int iCount = 1)
	{
		try {
			c_string.erase(iIndex, iCount);
		}
		catch (...) {
		}
		return c_string.length();
	}

	int Insert(int iIndex, const char* psz)
	{
		try {
			if (psz != NULL)	c_string.insert(iIndex, psz);
		}
		catch (...) {
			c_string += psz;
		}
		return c_string.length();
	}

	int Insert(int iIndex, char ch)
	{
		try {
			c_string.insert(iIndex, 1, ch);
		}
		catch (...) {
			c_string += ch;
		}
		return c_string.length();
	}

	CString& TrimRight(char chTarget)
	{
		size_t size = c_string.length();
		for ( ; ; ) {
			if (size == 0)	break;
			if (c_string[size - 1] != chTarget)	break;
			else	c_string.erase(size -- - 1, 1);
		}
		return (*this);
	}

	CString& TrimRight(const char* pszTargets)
	{
		if (pszTargets == NULL)	return (*this);
		size_t size = c_string.length();
		for ( ; ; ) {
			if (size == 0)	break;
			if (strchr(pszTargets, c_string[size - 1]))	break;
			else	c_string.erase(size -- - 1, 1);
		}
		return (*this);
	}

	CString& TrimRight()
	{
		size_t size = c_string.length();
		for ( ; ; ) {
			if (size == 0)	break;
			if (c_string[size - 1] > ' ' && c_string[size - 1] <= '~')	break;
			else	c_string.erase(size -- - 1, 1);
		}
		return (*this);
	}

	CString& TrimLeft(char chTarget)
	{
		for ( ; ; ) {
			if (c_string[0] != chTarget)	break;
			else	c_string.erase(0, 1);
		}
		return (*this);
	}

	CString& TrimLeft(const char* pszTargets)
	{
		if (pszTargets == NULL)	return (*this);
		for ( ; ; ) {
			if (strchr(pszTargets, c_string[0]))	break;
			else	c_string.erase(0, 1);
		}
		return (*this);
	}

	CString TrimLeft()
	{
		for ( ; ; ) {
			if (c_string[0] > ' ' && c_string[0] <= '~')	break;
			else	c_string.erase(0, 1);
		}
		return (*this);
	}

	int Replace(const char* pszOld, const char* pszNew)
	{
		if (pszOld == NULL || pszNew == NULL)	return 0;

		int ret = 0;
		size_t size = 0;
		std::string::size_type offset = 0;
		for ( ; ; ) {
			offset = c_string.find(pszOld, offset);
			if (offset == std::string::npos)	break;
			else {
				if (size == 0) {
					size = strlen(pszOld);
					if (size == 0)	return 0;
				}
				c_string.replace(offset, size, pszNew);
				offset += size;
				ret ++;
			}
		}
		return ret;
	}

	void Format(const char* pszFormat, ...)
	{
		int leng = 0;
		va_list vl;
		va_start(vl, pszFormat);
		leng = vsnprintf(NULL, 0, pszFormat, vl) + 1;
		c_string.resize(leng, '\0');

		va_start(vl, pszFormat);
		if (vsnprintf(&c_string[0], leng, pszFormat, vl) > leng) {
			// error
		}
		va_end(vl);
	}
};
