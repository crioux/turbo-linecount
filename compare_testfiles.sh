#!/bin/sh


lctest()
{
	OUT=`(time ./lc $1) 2>&1 | grep real | cut -f 2 | cut -c 3-`
	echo "lc: $1 $OUT"
	return 0
}

wctest()
{
	OUT=`(time wc -l $1) 2>&1 | grep real | cut -f 2 | cut -c 3-`
	echo "wc: $1 $OUT"
	return 0
}

pythontest()
{
	OUT=`(time echo "print str(sum(1 for line in open('$1'))) + ' $1'" | python) 2>&1 | grep real | cut -f 2 | cut -c 3-`
	echo "python: $1 $OUT"
	return 0
}

echo Timing for 'lc'
lctest test_10MB.txt
lctest test_100MB.txt
lctest test_1GB.txt
lctest test_10GB.txt

echo Timing for 'python'
pythontest test_10MB.txt 
pythontest test_100MB.txt
pythontest test_1GB.txt
pythontest test_10GB.txt

echo Timing for 'wc'
wctest test_10MB.txt
wctest test_100MB.txt
wctest test_1GB.txt
wctest test_10GB.txt
