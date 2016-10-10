#!/bin/bash
insmod calculator.ko
`dmesg | grep -m 1 'mknod' | awk -F "'" {'print $2'}`

function check {
    echo $1 > /proc/first
    echo $2 > /proc/operand
    echo $3 > /proc/second
    if [ "$4" == "`cat /dev/result`" ]; then
        echo "True!"
    else
        echo "False!"
    fi
}

check 3 + 2 "  5"
check 20 + 2 " 22"
check 77 + 33 "110"

check 20 - 18 "  2"
check 44 - 33 " 11"
check 2 - 1 "  1"

check 55 / 11 "  5"
check 33 / 11 "  3"
check 21 / 7 "  3"

check 20 p 3 " 60"
check 31 p 3 " 93"
check 44 p 1 " 44"

rmmod calculator
rm /dev/result
