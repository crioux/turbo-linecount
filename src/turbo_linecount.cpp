//
// Turbo Linecount
// Copyright 2015, Christien Rioux
// 
// MIT Licensed, see file 'LICENSE' for details
// 
///////////////////////////////////////////////

#include"turbo_linecount.h"
#include<algorithm>
#ifdef min
#undef min
#endif

///////////////////////////// Platform specific
#ifdef _WIN32

// Windows	
#define LCOPENFILE(name) CreateFile(name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)
#define LCCLOSEFILE(handle) CloseHandle(handle)
#define LCINVALIDHANDLE INVALID_HANDLE_VALUE
#define LCSETREALLASTERROR(err, errstr) { setLastError((err), (errstr)); }
#define MAP_FAILED NULL
typedef long long tlc_fileoffset_t;

#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))

// POSIX
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/fcntl.h>
#include<sys/mman.h>
#if (defined (__APPLE__) && defined (__MACH__))
#include <sys/sysctl.h>
typedef off_t tlc_fileoffset_t;
#define MMAP ::mmap
#define FSTAT ::fstat
#define STAT ::stat
#elif defined(__linux__)
typedef off64_t tlc_fileoffset_t;
#define MMAP ::mmap64
#define FSTAT ::fstat64
#define STAT ::stat64
#else
typedef off_t tlc_fileoffset_t;
#define MMAP ::mmap
#define FSTAT ::fstat
#define STAT ::stat
#endif

#define LCOPENFILE(name) ::open(name, O_RDONLY)
#define LCCLOSEFILE(handle) (::close(handle) != -1)
#define LCINVALIDHANDLE -1
#define LCSETREALLASTERROR(err, errstr) { int __err = errno; setLastError(__err, ::strerror(__err)); }
#define _tcsdup strdup

#endif

///////////////////////////// Line Count Class

BEGIN_TURBOLINECOUNT_NAMESPACE;

struct LCTHREADCONTEXT
{
	int thread_number;
	CLineCount *m_this;
};

CLineCount::CLineCount(PARAMETERS *parameters)
{

	// Set line count parameter defaults
	int cpucount;
	int allocationgranularity;
#ifdef _WIN32
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	cpucount = sysinfo.dwNumberOfProcessors;
	allocationgranularity = sysinfo.dwAllocationGranularity;
//#elif defined(__linux__)
#else
	cpucount = sysconf(_SC_NPROCESSORS_ONLN);
	allocationgranularity = sysconf(_SC_PAGESIZE);
//#elif (defined (__APPLE__) && defined (__MACH__))
//	mmsize_t count_len = sizeof(cpucount);
//	sysctlbyname("hw.logicalcpu", &cpucount, &count_len, NULL, 0);
//#else
//	cpucount = 1;
#endif
	m_parameters.threadcount = cpucount;
	m_parameters.buffersize = (1024 * 1024);
	
	// Override defaults if specified
	if (parameters)
	{
		if (parameters->buffersize != -1)
		{
			m_parameters.buffersize = parameters->buffersize;
			m_parameters.buffersize += (allocationgranularity - (m_parameters.buffersize % allocationgranularity)) % allocationgranularity;
		}
		if (parameters->threadcount != -1)
		{
			m_parameters.threadcount = parameters->threadcount;
		}
	}
	
	init();
}

CLineCount::~CLineCount()
{
	if (m_auto_close && m_opened)
	{
		LCCLOSEFILE(m_fh);
	}
}

void CLineCount::init(void)
{
	m_lasterror = 0;
	m_lasterrorstring = _T("");
	m_opened = false;
	m_auto_close = false;
	m_fh = LCINVALIDHANDLE;
	m_filesize = 0;
	m_actual_thread_count = 0;
#ifdef _WIN32
	m_filemapping = NULL;
#endif
	m_threads.clear();
	m_threadlinecounts.clear();
}

void CLineCount::setLastError(tlc_error_t lasterror, tlc_string_t lasterrorstring)
{
	m_lasterror = lasterror;
	m_lasterrorstring = lasterrorstring;
}

tlc_error_t CLineCount::lastError() const
{
	return m_lasterror;
}

tlc_string_t CLineCount::lastErrorString() const
{
	return m_lasterrorstring;
}

bool CLineCount::isOpened() const
{
	return m_opened;
}

bool CLineCount::open(tlc_filehandle_t fhandle, bool auto_close)
{
	if (m_opened)
	{
		setLastError(EEXIST, _T("file already opened"));
		return false;
	}

	m_fh = fhandle;
	m_opened = true;
	m_auto_close = auto_close;

	return true;
}

bool CLineCount::open(const TCHAR *filename)
{
	if (m_opened)
	{
		setLastError(EEXIST, _T("file already opened"));
		return false;
	}

	m_fh = LCOPENFILE(filename);
	if (m_fh == LCINVALIDHANDLE)
	{
		LCSETREALLASTERROR(ENOENT, _T("file could not be opened"));
		return false;
	}

	m_opened = true;
	m_auto_close = true;

	return true;
}

bool CLineCount::close()
{
	if (!m_opened)
	{
		setLastError(EBADF, _T("file not opened"));
		return false;
	}

	bool ok = true;
	if (!LCCLOSEFILE(m_fh))
	{
		LCSETREALLASTERROR(EBADF, _T("unable to close file"));
		ok = false;
	}

	init();

	return ok;
}


#ifdef _WIN32
DWORD WINAPI threadProc(LPVOID ctx)
#else
void *threadProc(void *ctx)
#endif
{
	LCTHREADCONTEXT *lctctx = (LCTHREADCONTEXT*)ctx;
	lctctx->m_this->countThread(lctctx->thread_number);
	return NULL;
}

unsigned int CLineCount::countThread(int thread_number)
{
	tlc_fileoffset_t buffersize = (tlc_fileoffset_t)m_parameters.buffersize;
	tlc_fileoffset_t startoffset = buffersize * (tlc_fileoffset_t)thread_number;
	tlc_fileoffset_t stride = buffersize * m_actual_thread_count;
	tlc_fileoffset_t curoffset = startoffset;
	tlc_fileoffset_t lastmapsize = 0;
	tlc_linecount_t count = 0;
	void *mem = NULL;

	while (curoffset < m_filesize)
	{
		if (m_thread_fail)
		{

			return -1;
		}

		// Get best file mapping window size
		size_t mapsize = (size_t)std::min((m_filesize - curoffset), buffersize);

		// Map view of file
#ifdef _WIN32
		if (mem)
		{
			if (!UnmapViewOfFile(mem))
			{
				setLastError(EINVAL, _T("memory unmap failed"));
				m_thread_fail = true;
				return -1;
			}
		}
		mem = MapViewOfFile(m_filemapping, FILE_MAP_READ, (DWORD)(curoffset >> 32), (DWORD)curoffset, (SIZE_T)mapsize);
#else
		if (mem)
		{
			if(munmap(mem, lastmapsize) !=0)
			{
				LCSETREALLASTERROR(EINVAL, _T("memory unmap failed"));
				m_thread_fail = true;
				return -1;
			}
		}
		mem = MMAP(NULL, mapsize, PROT_READ, MAP_FILE | MAP_SHARED, m_fh, curoffset);
//		printf("%p %lld %lld\n",mem, mapsize, curoffset);
#endif		
		if (mem == MAP_FAILED)
		{
			LCSETREALLASTERROR(EINVAL, _T("memory map failed"));
			m_thread_fail = true;
			return -1;
		}

		// Count newlines in buffer
		tlc_fileoffset_t windowoffset = 0;
		size_t windowleft = mapsize;
		char *ptr = (char *)mem;
		while (windowleft > 0)
		{
			char *ptrnext = (char *)memchr(ptr, '\n', windowleft);
			if (ptrnext)
			{
				ptrnext++;
				count++;
				windowleft -= (ptrnext - ptr);
				ptr = ptrnext;
			}
			else
			{
				windowleft = 0;
			}
		}

		// See if we need to account for end of file not ending with line terminator
		if ((curoffset + mapsize) == m_filesize)
		{
			if (*((char *)mem + (mapsize - 1)) != '\n')
			{
				count++;
			}
		}

		// Move to next buffer
		curoffset += stride;
		lastmapsize = mapsize;
	}

	// Clean up memory map
#ifdef _WIN32
	if (mem)
	{
		if (!UnmapViewOfFile(mem))
		{
			setLastError(EINVAL, _T("memory unmap failed"));
			m_thread_fail = true;
			return -1;
		}
	}
#else
	if (mem)
	{
		if (munmap(mem, lastmapsize) != 0)
		{
			LCSETREALLASTERROR(EINVAL, _T("memory unmap failed"));
			m_thread_fail = true;
			return -1;
		}
	}
#endif

	// Save count for this thread
	m_threadlinecounts[thread_number] = count;

	return 0;
}

bool CLineCount::createThread(int thread_number)
{
	LCTHREADCONTEXT * ctx = new LCTHREADCONTEXT;
	ctx->m_this = this;
	ctx->thread_number = thread_number;
#ifdef _WIN32
	HANDLE hThread = CreateThread(NULL, 0, threadProc, ctx, 0, NULL);
	if(!hThread)
	{
		return false;
	}
#else
	pthread_t hThread;
	int ret = pthread_create(&hThread, NULL, threadProc, ctx);
	if (ret != 0)
	{
		return false;
	}
#endif
	m_threads[thread_number] = hThread;
	return true;
}

bool CLineCount::countLines(tlc_linecount_t & linecount)
{
	// Determine file size
#ifdef _WIN32
	LARGE_INTEGER li;
	if (!GetFileSizeEx(m_fh, &li))
	{
		LCSETREALLASTERROR(EBADF, _T("unable to get file size"));
		return false;
	}
	m_filesize = li.QuadPart;
#else
	struct STAT statbuf;
	if(FSTAT(m_fh,&statbuf)!=0)
	{
		LCSETREALLASTERROR(EBADF, _T("unable to get file size"));
		return false;
	}
	m_filesize = statbuf.st_size;
#endif

	// Exit now for empty files
	if (m_filesize == 0)
	{
		linecount = 0;
		return true;
	}

	// Figure out actual thread count
	tlc_fileoffset_t windowcount = (m_filesize + (m_parameters.buffersize - 1)) / m_parameters.buffersize;
	if (windowcount < (tlc_fileoffset_t) m_parameters.threadcount)
	{
		m_actual_thread_count = (int)windowcount;
	}
	else
	{
		m_actual_thread_count = m_parameters.threadcount;
	}

//	printf("act: %d\n",m_actual_thread_count);

#ifdef _WIN32
	// Prepare file mapping
	m_filemapping = CreateFileMapping(m_fh, NULL, PAGE_READONLY, 0, 0, NULL);
#endif

	// Spin up threads
	m_threads.resize(m_actual_thread_count);
	m_threadlinecounts.resize(m_actual_thread_count);
	m_thread_fail = false;
	for (int i = 0; i < m_actual_thread_count; i++)
	{
		if (!createThread(i))
		{
			setLastError(ECHILD, _T("failed to create counting thread"));
			m_thread_fail = true;
		}
	}

	// Wait for threads to complete
	int complete = 0;
	int errors = 0;
	for (int i = 0; i < m_actual_thread_count; i++)
	{
		bool success = false;
		if (m_threads[i] != NULL)
		{
#ifdef _WIN32
			success = (WaitForSingleObject(m_threads[i], INFINITE) == WAIT_OBJECT_0);
#else
			success = pthread_join(m_threads[i], NULL) == 0;
#endif
		}

		if (success)
		{
			complete++;
		}
		else
		{
			errors++;
		}
	}

#ifdef _WIN32
	// Clean up file mapping
	CloseHandle(m_filemapping);
#endif

	if (m_thread_fail)
	{
		return false;
	}

	if (complete != m_actual_thread_count)
	{
		setLastError(ECHILD, _T("thread join failed"));
		return false;
	}

	// Sum up thread line counts and return
	linecount = 0;
	for (int i = 0; i < m_actual_thread_count; i++)
	{
		linecount += m_threadlinecounts[i];
	}
	
	return true;
}

// Static helpers
tlc_linecount_t CLineCount::LineCount(tlc_filehandle_t fhandle, tlc_error_t * error, tlc_string_t *errorstring)
{
	CLineCount lc;
	if (!lc.open(fhandle))
	{
		if (error)
		{
			*error = lc.lastError();
		}
		if (errorstring)
		{
			*errorstring = lc.lastErrorString();
		}

		return -1;
	}

	tlc_linecount_t count;
	if (!lc.countLines(count))
	{
		if (error)
		{
			*error = lc.lastError();
		}
		if (errorstring)
		{
			*errorstring = lc.lastErrorString();
		}
		return -1;
	}

	*error = 0;
	*errorstring = _T("");

	return count;
}

tlc_linecount_t CLineCount::LineCount(const TCHAR *filename, tlc_error_t * error, tlc_string_t *errorstring)
{
	CLineCount lc;
	if (!lc.open(filename))
	{
		if (error)
		{
			*error = lc.lastError();
		}
		if (errorstring)
		{
			*errorstring = lc.lastErrorString();
		}

		return -1;
	}

	tlc_linecount_t count;
	if (!lc.countLines(count))
	{
		if (error)
		{
			*error = lc.lastError();
		}
		if (errorstring)
		{
			*errorstring = lc.lastErrorString();
		}
		return -1;
	}

	*error = 0;
	*errorstring = _T("");

	return count;
}

END_TURBOLINECOUNT_NAMESPACE;


///////////////////////////// C Linkage

#ifndef _NO_TURBOLINECOUNT_C

#ifdef _WIN32
long long turbo_linecount_handle(HANDLE fhandle, tlc_error_t * error, TCHAR ** errorstring)
#else
long long turbo_linecount_handle(int fhandle, tlc_error_t * error, char ** errorstring)
#endif
{
	TURBOLINECOUNT::tlc_string_t errstr;

	long long linecount = TURBOLINECOUNT::CLineCount::LineCount(fhandle, error, &errstr);

	if (errorstring)
	{
		*errorstring = _tcsdup(errstr.c_str());
	}

	return linecount;
}

#ifdef _WIN32
long long turbo_linecount_file(const TCHAR *filename, tlc_error_t * error, TCHAR ** errorstring)
#else
long long turbo_linecount_file(const char *filename, tlc_error_t * error, char ** errorstring)
#endif
{
	TURBOLINECOUNT::tlc_string_t errstr;

	long long linecount = TURBOLINECOUNT::CLineCount::LineCount(filename, error, &errstr);

	if (errorstring)
	{
		*errorstring = _tcsdup(errstr.c_str());
	}

	return linecount;
}

#endif
