#!bin/bash

CMAKE=/home/s1583091/cmake-3.4.3-Linux-x86_64/bin/cmake

mkdir -p build && cd build
rm -rf ./*

THRIFT_ROOT=/exports/applications/apps/community/VELaSSCo/lib/thrift/
BOOST_BASE=/exports/applications/apps/community/VELaSSCo/build/boost
$CMAKE -DTHRIFT_HOME:PATH=$THRIFT_ROOT -DBOOST_ROOT:PATH=$BOOST_BASE ..

