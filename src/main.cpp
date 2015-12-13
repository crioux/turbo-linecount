//
// Turbo Linecount
// Copyright 2015, Christien Rioux
// 
// MIT Licensed, see file 'LICENSE' for details
// 
///////////////////////////////////////////////

#include"turbo_linecount.h"
#include<cstring>
#include<cstdio>

#ifdef _WIN32

#include<tchar.h>

#elif defined(TLC_COMPATIBLE_UNIX)

#include<stdlib.h>
#define _tprintf printf
#define _ftprintf fprintf
#define _tcscmp strcmp
#define _tcslen strlen
#define _ttoi atoi
#define _tcstoui64 strtoull
#define _T(x) x
#define TCHAR char

#endif

using namespace TURBOLINECOUNT;

//////////////////////////////////////////////////////


void help(const TCHAR *argv0)
{
	_ftprintf(stderr, _T("usage: %s [options] <file>\n"), argv0);
	_ftprintf(stderr, _T("    -h  --help                         print this usage and exit\n"));
	_ftprintf(stderr, _T("    -b  --buffersize <BUFFERSIZE>      size of buffer per-thread to use when reading (default is 1MB)\n"));   
	_ftprintf(stderr, _T("    -t  --threadcount <THREADCOUNT>    number of threads to use (defaults to number of cpu cores)\n"));
	_ftprintf(stderr, _T("    -v  --version                      print version information and exit\n"));
}


void version(void)
{
	_tprintf(_T("tlc (turbo-linecount) %d.%2.2d\nCopyright (c) 2015 Christien Rioux\n"), TURBOLINECOUNT_VERSION_MAJOR, TURBOLINECOUNT_VERSION_MINOR);
}

//////////////////////////////////////////////////////


#if defined(WIN32) && defined(_UNICODE)
int wmain(int argc, TCHAR **argv)
#else
int main(int argc, char **argv)
#endif
{
	// Parse parameters
	int arg = 1;
	int posparam = 0;
	
	CLineCount::PARAMETERS params;
	params.buffersize = -1;
	params.threadcount = -1;

	TCHAR *filename = NULL;

	if(argc==1)
	{
		help(argv[0]);
		exit(0);
	}

	while (arg < argc)
	{
		if (_tcscmp(argv[arg], _T("-h")) == 0 || _tcscmp(argv[arg], _T("--help")) == 0) 
		{
			help(argv[0]);
			exit(0);
		}
		else if (_tcscmp(argv[arg], _T("-v")) == 0 || _tcscmp(argv[arg], _T("--version")) == 0) 
		{
			version();
			exit(0);
		}
		else if (_tcscmp(argv[arg], _T("-b")) == 0 || _tcscmp(argv[arg], _T("--buffersize")) == 0)
		{
			arg++;
			if (arg == argc)
			{
				_ftprintf(stderr, _T("%s: missing argument to %s\n"), argv[0], argv[arg-1]);
				return 1;
			}

			TCHAR *wsstr = argv[arg];

			// Check for size multipliers			
			size_t multiplier = 1;
			TCHAR *lastchar = wsstr + (_tcslen(wsstr) - 1);
			if (*lastchar == _T('k') || *lastchar == _T('K'))
			{
				multiplier = 1024;
				lastchar = 0;
			}
			else if (*lastchar == _T('m') || *lastchar == _T('M'))
			{
				multiplier = 1024 * 1024;
				lastchar = 0;
			}
			else if (*lastchar == _T('g') || *lastchar == _T('G'))
			{
				multiplier = 1024 * 1024 * 1024;
				lastchar = 0;
			}

			TCHAR *endptr;
			params.buffersize = ((size_t)_tcstoui64(argv[arg], &endptr, 10)) * multiplier;

		}
		else if (_tcscmp(argv[arg], _T("-t")) == 0 || _tcscmp(argv[arg], _T("--threadcount")) == 0)
		{
			arg++;
			if (arg == argc)
			{
				_ftprintf(stderr, _T("%s: Missing argument to %s\n"), argv[0], argv[arg-1]);
				return 1;
			}

			params.threadcount = _ttoi(argv[arg]);
			if(params.threadcount<=0)
			{
				_ftprintf(stderr, _T("%s: Invalid thread count\n"), argv[0]);
				return 1;
			}
		}
		else
		{
			if (posparam == 0)
			{
				filename = argv[arg];
			}
			else
			{
				_ftprintf(stderr, _T("%s: Too many arguments\n"), argv[0]);
				return 1;
			}
			posparam++;
		}

		arg++;
	}

	if (posparam != 1)
	{
		_ftprintf(stderr, _T("%s: Missing required argument\n"), argv[0]);
		return 1;
	}
	
	// Create line count class
	CLineCount lc(&params);

	if (!lc.open(filename))
	{
		tlc_error_t err = lc.lastError();
		tlc_string_t errstr = lc.lastErrorString();

		_ftprintf(stderr, _T("%s: Error %d (%s)\n"), argv[0], err, errstr.c_str());
		return err;
	}

	// Count lines
	tlc_linecount_t count;
	if (!lc.countLines(count))
	{
		tlc_error_t err = lc.lastError();
		tlc_string_t errstr = lc.lastErrorString();

		_ftprintf(stderr, _T("%s: Error %d: (%s)\n"), argv[0], err, errstr.c_str());
		return err;
	}

	// Display output
	_tprintf(_T(TLC_LINECOUNT_FMT) _T("\n"), count);
	
	return 0;
}
