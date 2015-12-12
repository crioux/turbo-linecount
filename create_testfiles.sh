#!/bin/bash

scdd if=/dev/urandom of=test1.txt bs=1000000 count=10
dd if=/dev/urandom of=test2.txt bs=1000000 count=100
dd if=/dev/urandom of=test3.txt bs=1000000 count=1000
cat test3.txt test3.txt test3.txt test3.txt test3.txt test3.txt test3.txt test3.txt test3.txt test3.txt > test4.txt
