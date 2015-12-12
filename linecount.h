#ifndef __INC_LINECOUNT_H
#define __INC_LINECOUNT_H

#define LINECOUNT_VERSION_MAJOR 1
#define LINECOUNT_VERSION_MINOR 0

///////////////////////////////////////////// Headers

////////////// Platform independent

#include<string>
#include<vector>
#include<errno.h>
#define BEGIN_LINECOUNT_NAMESPACE namespace LineCount {
#define END_LINECOUNT_NAMESPACE }

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

BEGIN_LINECOUNT_NAMESPACE;

////////////// Platform specific
#ifdef _WIN32 // Windows	
	
	#ifdef _UNICODE
		typedef std::wstring LCSTRING;
	#else
		typedef std::string LCSTRING;
	#endif
	typedef HANDLE LCFILEHANDLE;
	typedef errno_t LCERROR;
	typedef long long LCFILEOFFSET;
	typedef LCFILEOFFSET LCLINECOUNT;
	#define LCLINECOUNTFMT "%I64d"

#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)) // POSIX
	
	typedef std::string LCSTRING;
	typedef int LCFILEHANDLE;
	typedef errno_t LCERROR;
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
	LCERROR m_lasterror;
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

	void setLastError(LCERROR error, LCSTRING lasterrorstring);
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
	LCERROR lastError() const;
	LCSTRING lastErrorString() const;

	bool open(LCFILEHANDLE fhandle, bool auto_close = false);
	bool open(const TCHAR * filename);
	bool close();

	bool countLines(LCLINECOUNT &linecount);
		
public:

	// Static utility functions
	static LCLINECOUNT LineCount(LCFILEHANDLE fhandle, LCERROR * error = NULL, LCSTRING * errorstring = NULL);
	static LCLINECOUNT LineCount(const TCHAR *filename, LCERROR * error = NULL, LCSTRING * errorstring = NULL);
};

END_LINECOUNT_NAMESPACE;

#endif
