#! /bin/bash

####################

FW=~/bs/firmware
DRIVER=~/bs/driver
IIO=/sys/bus/iio/devices/iio\:device0/
IIO_BUFFER=/dev/iio\:device0
BUF_LEN=500
FREQ="3000 30000 300000 800000 3000000 15000000"
TEST_CASES=6

#####################

echo "cd $FW"
cd $FW
echo "make install"
make install
echo "cd $DRIVER"
cd $DRIVER
echo "make load"
make load
echo "cd $IIO"
cd $IIO
echo "echo 1 > scan_elements/in_voltage0_en"
echo 1 > scan_elements/in_voltage0_en
echo "echo $BUF_LEN > buffer/length"
echo $BUF_LEN > buffer/length

for freq in $FREQ
do

echo "echo $freq > in_voltage0_sampling_frequency"
echo $freq > in_voltage0_sampling_frequency
echo "dmesg | tail -n3"
dmesg | tail -n3
dd if=$IIO_BUFFER of=/dev/null bs=2 1>&2  &
DD_PID=$!
echo "dd pid is $DD_PID"
echo "echo 1 > buffer/enable"
echo 1 > buffer/enable
echo "sleep 1"
sleep 1
echo "echo 0 > buffer/enable"
echo 0 > buffer/enable
echo "kill $DD_PID"

echo ""
echo "################################"
echo "############"
echo "For Frequency $freq"
kill -USR1 $DD_PID
sleep 1
kill $DD_PID 1>&2 
echo "############"
echo "################################"
echo ""

done


