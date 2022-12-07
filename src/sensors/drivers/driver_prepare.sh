#! /bin/sh

echo $1
driver_name1="${1}_driver"
driver_name=`echo "${driver_name1}" | tr '[a-z]' '[A-Z]'`
driver_case="${1}Driver"
driver_file_name="${1}_driver.h"
driver_cpp_name="${1}_driver.cpp"

echo $driver_name
echo $driver_file_name
echo $driver_case

#sed "s/WATERBEAR_DRIVER_TEMPLATE/$driver_name/g" driver_template.h > $driver_file_name
#sed -i "s/DriverTemplate/$driver_case/g" test_driver.h
sed "s/DriverTemplate/$driver_case/g" driver_template.h > $driver_file_name

sed -i.bak "s/WATERBEAR_DRIVER_TEMPLATE/$driver_name/g" $driver_file_name
sed -i.bak "s/DRIVER_TEMPLATE/$driver_name/g" $driver_file_name


sed "s/DriverTemplate/$driver_case/g" driver_template.cpp > $driver_cpp_name

sed -i.bak "s/driver_template/$driver_name1/g" $driver_cpp_name

find . -name "*.bak" -print0 | xargs -0 rm
