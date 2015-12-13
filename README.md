# turbo-linecount
turbo-linecount 1.0 Copyright 2015, Christien Rioux

### Super-Fast Multi-Threaded Line Counter

*turbo-linecount* is a tool that simply counts the number of lines in a file, as fast as possible. It reads the file in large chunks into several threads and quickly scans the file for line endings.

Many times, you have to count the number of lines in text file on disk. The typical solution is to use `wc -l` on the command line. `wc -l` uses buffered streams to process the file, which has its advantages, but it is slower than direct memory mapped file access. You can't 'pipe' to 

How much faster is *turbo-linecount*? About 8 times faster than `wc -l` and 5 times faster than the naive Python implementation.

To use *turbo-linecount*, just run the command line:

```
lc <file>
```

where *\<file\>* is the path to the file of which you'd like to count the lines. 

###Help
To get help with *turbo-linecount*:

```
lc -h
usage: lc [options] <file>
    -h  --help                         print this usage and exit
    -b  --buffersize <BUFFERSIZE>      size of buffer per-thread to use when reading (default is 1MB)
    -t  --threadcount <THREADCOUNT>    number of threads to use (defaults to number of cpu cores)
    -v  --version                      print version information and exit
```

###Building

To build *turbo-linecount*, we use *cmake*. Cmake 3.0.0 or higher is the preferred version as of this release. For simplified building on Windows, a Visual Studio 2013 solution file is also included.

To build with *cmake*:
```
cd build
cmake ..
make
make install
```

This will build and install the command line utility `tlc`, a shared library `libturbo_linecount`, a static library `libturbo_linecount_static`, and a header file `turbo_linecount.h`.

Building *turbo-linecount* is known to be possible on

```
Windows 32/64 bit
Mac OS X
Linux
Cygwin
``` 

### Testing

Testing cmake against `wc -l` and `python` can be done with the test scripts. To generate some random test files, run `create_testfiles.sh`, and four test files,    one 10MB, one 100MB, one 1GB, and one 10GB file will be created. Feel free to delete these when you're done testing to save space.

To run the test, run `compare_testfiles.sh`. This will generate output as such:

```
Timing for tlc
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

### Performance

Performance on Windows and Mac OS X is excellent for all file sizes. Performance on Linux and other operating systems is good, but can be better. Stay tuned.

* Macbook Pro (Retina, 15-inch Mid 2014)  
* 2.8 GHz Intel Core i7  
* 1TB SSD hard drive  
* 16GB Memory

| File Size | `tlc`  | `python`  | `wc -l` |
|-----------|---|---|---|---|---|
| 10MB      | 0.006s  | 0.025s (4.2x) | 0.012s (2.0x) |
| 100MB     | 0.015s  | 0.084s (5.6x) |  0.100s (6.7x) | 
| 1GB       | 0.127s  | 0.661s (5.2x) | 0.933s (7.3x) |
| 10GB      | 1.196s   | 6.165s (5.15x) | 9.857s (8.2x) |
