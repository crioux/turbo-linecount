//
// Turbo Linecount
// Copyright 2015, Christien Rioux
// 
// MIT Licensed, see file 'LICENSE' for details
// 
///////////////////////////////////////////////

#ifndef __INC_TURBO_LINECOUNT_H
#define __INC_TURBO_LINECOUNT_H

#define LINECOUNT_VERSION_MAJOR 1
#define LINECOUNT_VERSION_MINOR 0

#ifdef __cplusplus

///////////////////////////////////////////// Headers

////////////// Platform independent

#include<string>
#include<vector>
#include<errno.h>

#define BEGIN_TURBO_LINECOUNT_NAMESPACE namespace TURBO_LINECOUNT {
#define END_TURBO_LINECOUNT_NAMESPACE }

////////////// Platform specific

#ifdef _WIN32 // Windows	
#include<Windows.h>
#include<tchar.h>
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)) // POSIX
#include<unistd.h>
#include<pthread.h>
#define _T(x) x
#define TCHAR char
#endif

///////////////////////////////////////////// Line Count Class

BEGIN_TURBO_LINECOUNT_NAMESPACE;

////////////// Platform specific
#ifdef _WIN32 // Windows	
	
	#ifdef _UNICODE
		typedef std::wstring LCSTRING;
	#else
		typedef std::string LCSTRING;
	#endif

	typedef HANDLE LCFILEHANDLE;
	typedef long long LCFILEOFFSET;
	typedef LCFILEOFFSET LCLINECOUNT;
	#define LCLINECOUNTFMT "%I64d"

#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)) // POSIX
	
	typedef std::string LCSTRING;
	typedef int LCFILEHANDLE;
	#if (defined (__APPLE__) && defined (__MACH__))
		typedef off_t LCFILEOFFSET;
		#define LCLINECOUNTFMT "%lld"
	#elif defined(__linux__)
		typedef off64_t LCFILEOFFSET;
		#define LCLINECOUNTFMT "%lld"
	#else
		typedef off_t LCFILEOFFSET;
		#define LCLINECOUNTFMT "%d"
	#endif
	typedef LCFILEOFFSET LCLINECOUNT;

#endif


class CLineCount
{
public:

	struct PARAMETERS
	{
		size_t buffersize;
		int threadcount;
	};

private:

	bool m_opened;
	bool m_auto_close;
	LCFILEHANDLE m_fh;
	errno_t m_lasterror;
	LCSTRING m_lasterrorstring;
	LCFILEOFFSET m_filesize;
	PARAMETERS m_parameters;
	int m_actual_thread_count;
#ifdef _WIN32
	std::vector<HANDLE> m_threads;
	HANDLE m_filemapping;
#else
	std::vector<pthread_t> m_threads;
#endif
	std::vector<LCLINECOUNT> m_threadlinecounts;
	bool m_thread_fail;

private:

	void setLastError(errno_t error, LCSTRING lasterrorstring);
	void init();
	bool createThread(int thread_number);
#ifdef _WIN32
	friend DWORD WINAPI threadProc(LPVOID ctx);
#else
	friend void *threadProc(void *ctx);
#endif
	unsigned int countThread(int thread_number);

public:

	CLineCount(PARAMETERS *parameters=NULL);
	~CLineCount();

	bool isOpened() const;
	errno_t lastError() const;
	LCSTRING lastErrorString() const;

	bool open(LCFILEHANDLE fhandle, bool auto_close = false);
	bool open(const TCHAR * filename);
	bool close();

	bool countLines(LCLINECOUNT &linecount);
		
public:

	// Static utility functions
	static LCLINECOUNT LineCount(LCFILEHANDLE fhandle, errno_t * error = NULL, LCSTRING * errorstring = NULL);
	static LCLINECOUNT LineCount(const TCHAR *filename, errno_t * error = NULL, LCSTRING * errorstring = NULL);
};

END_TURBO_LINECOUNT_NAMESPACE;

#endif


// C compatibility functions
#ifndef _NO_TURBO_LINECOUNT_C

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
	long long turbo_linecount_handle(HANDLE fhandle, errno_t * error = NULL, TCHAR ** errorstring = NULL);
	long long turbo_linecount_file(const TCHAR *filename, errno_t * error = NULL, TCHAR ** errorstring = NULL);
#else
	long long turbo_linecount_handle(int fhandle, errno_t * error = NULL, char ** errorstring = NULL);
	long long turbo_linecount_file(const char *filename, errno_t * error = NULL, char ** errorstring = NULL);
#endif


#ifdef __cplusplus
}
#endif

#endif

#endif
