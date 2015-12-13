//
// Turbo Linecount
// Copyright 2015, Christien Rioux
// 
// MIT Licensed, see file 'LICENSE' for details
// 
///////////////////////////////////////////////

#ifndef __INC_TURBO_LINECOUNT_H
#define __INC_TURBO_LINECOUNT_H

#define TURBOLINECOUNT_VERSION_MAJOR 1
#define TURBOLINECOUNT_VERSION_MINOR 0

#ifdef __cplusplus

///////////////////////////////////////////// Headers

#include<string>
#include<vector>
#include<errno.h>

#if defined(__APPLE__) || defined(__linux__) || defined(__CYGWIN__) 
#define TLC_COMPATIBLE_UNIX 1
#endif

#ifdef _WIN32 // Windows

#include<Windows.h>
#include<tchar.h>

typedef errno_t tlc_error_t;

#elif defined(TLC_COMPATIBLE_UNIX)

#include<unistd.h>
#include<pthread.h>
#define _T(x) x

#ifdef _ERRNO_T
typedef errno_t tlc_error_t;
#elif defined(__error_t_defined)
typedef error_t tlc_error_t;
#else
typedef int tlc_error_t;
#endif

#else
#error Unsupported operating system.
#endif

///////////////////////////////////////////// Line Count Class

#define BEGIN_TURBOLINECOUNT_NAMESPACE namespace TURBOLINECOUNT {
#define END_TURBOLINECOUNT_NAMESPACE }

BEGIN_TURBOLINECOUNT_NAMESPACE;

////////////// Platform specific
#ifdef _WIN32 // Windows	
	
	#ifdef _UNICODE
		typedef std::wstring tlc_string_t;
	#else
		typedef std::string tlc_string_t;
	#endif
	
	typedef HANDLE tlc_filehandle_t;
	typedef long long int tlc_fileoffset_t;
	typedef tlc_fileoffset_t tlc_linecount_t;
	#define TLC_LINECOUNT_FMT "%I64d"

#elif defined(TLC_COMPATIBLE_UNIX) // Unix
	typedef char TCHAR;

	typedef std::string tlc_string_t;
	typedef int tlc_filehandle_t;
	
	#if (defined (__APPLE__) && defined (__MACH__))
		typedef off_t tlc_fileoffset_t;
		#define TLC_LINECOUNT_FMT "%lld"
	#elif defined(_LARGEFILE64_SOURCE)
		#if defined(__CYGWIN__)
			typedef _off64_t tlc_fileoffset_t;	
		#else
			typedef off64_t tlc_fileoffset_t;	
		#endif
		#ifdef __LP64__
			#define TLC_LINECOUNT_FMT "%ld"
		#else 
			#define TLC_LINECOUNT_FMT "%lld"
		#endif
	#else
		typedef off_t tlc_fileoffset_t;
		#define TLC_LINECOUNT_FMT "%d"
	#endif

	typedef tlc_fileoffset_t tlc_linecount_t;

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
	tlc_filehandle_t m_fh;
	tlc_error_t m_lasterror;
	tlc_string_t m_lasterrorstring;
	tlc_fileoffset_t m_filesize;
	PARAMETERS m_parameters;
	int m_actual_thread_count;
#ifdef _WIN32
	std::vector<HANDLE> m_threads;
	HANDLE m_filemapping;
#elif defined(TLC_COMPATIBLE_UNIX)
	std::vector<pthread_t> m_threads;
#endif
	std::vector<tlc_linecount_t> m_threadlinecounts;
	bool m_thread_fail;

private:

	void setLastError(tlc_error_t error, tlc_string_t lasterrorstring);
	void init();
	bool createThread(int thread_number);
#ifdef _WIN32
	friend DWORD WINAPI threadProc(LPVOID ctx);
#elif defined(TLC_COMPATIBLE_UNIX)
	friend void *threadProc(void *ctx);
#endif
	unsigned int countThread(int thread_number);

public:

	CLineCount(PARAMETERS *parameters=NULL);
	~CLineCount();

	bool isOpened() const;
	tlc_error_t lastError() const;
	tlc_string_t lastErrorString() const;

	bool open(tlc_filehandle_t fhandle, bool auto_close = false);
	bool open(const TCHAR * filename);
	bool close();

	bool countLines(tlc_linecount_t &linecount);
		
public:

	// Static utility functions
	static tlc_linecount_t LineCount(tlc_filehandle_t fhandle, tlc_error_t * error = NULL, tlc_string_t * errorstring = NULL);
	static tlc_linecount_t LineCount(const TCHAR *filename, tlc_error_t * error = NULL, tlc_string_t * errorstring = NULL);
};

END_TURBOLINECOUNT_NAMESPACE;

#endif


// C compatibility functions
#ifndef _NO_TURBO_LINECOUNT_C

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
	long long int turbo_linecount_handle(HANDLE fhandle, errno_t * error = NULL, TCHAR ** errorstring = NULL);
	long long int turbo_linecount_file(const TCHAR *filename, errno_t * error = NULL, TCHAR ** errorstring = NULL);
#elif defined(TLC_COMPATIBLE_UNIX)
	long long int turbo_linecount_handle(int fhandle, tlc_error_t * tlc_error = NULL, char ** errorstring = NULL);
	long long int turbo_linecount_file(const char *filename, tlc_error_t * error = NULL, char ** errorstring = NULL);
#endif


#ifdef __cplusplus
}
#endif

#endif

#endif
