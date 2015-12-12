# linecount
linecount 1.0 Copyright 2015, Christien Rioux

### Super-Fast Multi-Threaded Line Counter

*linecount* is a tool that simply counts the number of lines in a file, as fast as possible. It reads the file in large chunks into several threads and quickly scans the file for line endings.

Many times, you have to count the number of lines in text file on disk. The typical solution is to use 'wc -l' on the command line. 'wc' uses buffered streams to process the file, which has its advantages, but it is slower than direct memory mapped file access.

How much faster is *linecount*? About 10 times faster than `wc` and 5 times faster than the naive Python implementation.

To use *linecount*, just run the command line:

```
lc <file>
```
where *\<file\>* is the path to the file of which you'd like to count the lines. 

###Help
To get help with *linecount*:

```
lc -h
usage: lc [options] <file>
    -h  --help                         print this usage and exit
    -b  --buffersize <BUFFERSIZE>      size of buffer per-thread to use when reading (default is 1MB)
    -t  --threadcount <THREADCOUNT>    number of threads to use (defaults to number of cpu cores)
    -v  --version                      print version information and exit
```

###Building

To build *linecount*, we use *cmake*. Cmake 3.3.0 is the preferred version as of this release. For building just the command line utility on Windows, a Visual Studio 2013 solution file is also included.

```
cd build
cmake ..
make
make install
```

This will build and install the command line utility `lc`, a shared library `liblinecount`, a static library `liblinecount_static`, and a header file `linecount.h`.

Building *linecount* is known to be possible on

```
Windows 32/64 bit
Mac OS X
Linux
``` 

###Testing

Testing cmake against `wc` and `python` can be done with the test scripts. To generate some random test files, run `create_testfiles.sh`, and four test files,    one 10MB, one 100MB, one 1GB, and one 10GB file will be created. Feel free to delete these when you're done testing to save space.

To run the test, run `compare_testfiles.sh`. This will generate output as such:

```
Timing for lc
lc: test_10MB.txt 0.006s
lc: test_100MB.txt 0.015s
lc: test_1GB.txt 0.127s
lc: test_10GB.txt 1.196s
Timing for python
python: test_10MB.txt 0.025s
python: test_100MB.txt 0.084s
python: test_1GB.txt 0.661s
python: test_10GB.txt 6.165s
Timing for wc
wc: test_10MB.txt 0.012s
wc: test_100MB.txt 0.100s
wc: test_1GB.txt 0.933s
wc: test_10GB.txt 9.857s
``` 