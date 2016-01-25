#!/bin/bash

mkdir -p ./bin
OUTPUT_FILE=bin/hbase_thrift_client
rm -f $OUTPUT_FILE

THRIFT_BASE=/exports/applications/apps/community/VELaSSCo/lib/thrift/lib/cpp
THRIFT_INCLUDE=$THRIFT_BASE/include
THRIFT_LIB=$THRIFT_BASE/lib
THRIFT_HBASE_GEN=/home/s1583091/git/HPRS/thrift/hbase_thrift/gen-cpp

BOOST_BASE=/exports/applications/apps/community/VELaSSCo/build/boost
BOOST_INCLUDE=$BOOST_BASE/include

HBASE_THRIFT_FILES=" $THRIFT_HBASE_GEN/Hbase_types.cpp $THRIFT_HBASE_GEN/Hbase_constants.cpp $THRIFT_HBASE_GEN/Hbase.cpp "

g++ -std=c++11 -Wall -I$THRIFT_INCLUDE -I$THRIFT_HBASE_GEN  -L$THRIFT_LIB -isystem $BOOST_INCLUDE -lthrift -o $OUTPUT_FILE hbase_thrift_client.cpp $HBASE_THRIFT_FILES

if [ -e "$OUTPUT_FILE" ]
then
  echo "BUILD SUCCESFULL"
else
  echo "!!!ERROR BUILD UNSUCCESFULL!!!!"
fi



#echo $STR
