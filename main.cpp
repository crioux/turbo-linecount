#include"linecount.h"
#ifdef _WIN32
#include<tchar.h>
#else
#define _tprintf printf
#define _ftprintf fprintf
#define _tcscmp strcmp
#define _ttoi atoi
#define _tcstoui64 strtoull
#define _T(x) x
#endif

using namespace LineCount;

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
	params.windowsize = -1;
	params.workercount = -1;

	TCHAR *filename = NULL;

	while (arg < argc)
	{
		if (_tcscmp(argv[arg], _T("--windowsize")) == 0)
		{
			arg++;
			if (arg == argc)
			{
				_ftprintf(stderr, _T("missing argument to --windowsize"));
				return 1;
			}

			_TCHAR *wsstr = argv[arg];

			// Check for size multipliers			
			size_t multiplier = 1;
			_TCHAR *lastchar = wsstr + (_tcslen(wsstr) - 1);
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

			_TCHAR *endptr;
			params.windowsize = ((size_t)_tcstoui64(argv[arg], &endptr, 10)) * multiplier;

		}
		else if (_tcscmp(argv[arg], _T("--workercount")) == 0)
		{
			arg++;
			if (arg == argc)
			{
				_ftprintf(stderr, _T("missing argument to --workercount"));
				return 1;
			}

			params.workercount = _ttoi(argv[arg]);
		}
		else
		{
			if (posparam == 0)
			{
				filename = argv[arg];
			}
			else
			{
				_ftprintf(stderr, _T("too many arguments"));
				return 1;
			}
			posparam++;
		}

		arg++;
	}

	if (posparam != 1)
	{
		_ftprintf(stderr, _T("missing required argument"));
		return 1;
	}
	
	// Create line count class
	CLineCount lc(&params);

	if (!lc.open(filename))
	{
		LCERROR err = lc.lastError();
		LCSTRING errstr = lc.lastErrorString();

		_ftprintf(stderr, _T("error (%d): %s\n"), err, errstr.c_str());
		return err;
	}

	// Count lines
	LCLINECOUNT count;
	if (!lc.countLines(count))
	{
		LCERROR err = lc.lastError();
		LCSTRING errstr = lc.lastErrorString();

		_ftprintf(stderr, _T("error (%d): %s\n"), err, errstr.c_str());
		return err;
	}

	// Display output
	_tprintf(_T(LCLINECOUNTFMT _T("\n")), count);
	
	return 0;
}