#!/bin/bash

if [ $# -lt 4 ]
  then
    echo "No arguments supplied, need to specify input directory with base files, average rows missing per file, max rows missing per file, and output directory for output files"
    exit -1
fi

./split $1/thousand.dat 1000 $2 $3 $4/thousand1.dat $4/thousand2.dat

./split $1/ten_thousand.dat 10000 $2 $3 $4/ten_thousand1.dat $4/hundred_thousand2.dat


./split $1/hundred_thousand.dat 100000 $2 $3 $4/hundred_thousand1.dat $4/hundred_thousand2.dat


echo "finished hundred_thousand"

./split $1/500_thousand.dat 500000 $2 $3 $4/500_thousand1.dat $4/500_thousand2.dat
 
./split $1/million.dat 1000000 $2 $3 $4/million1.dat $4/million2.dat 

echo "finished million"

./split $1/1.5_million.dat 1500000 $2 $3  $4/1.5_million1.dat $4/1.5_million2.dat 

 
./split $1/2_million.dat 2000000 $2 $3 $4/2_million1.dat $4/2_million2.dat 


echo "finished 2 million"

./split $1/2.5_million.dat 2500000 $2 $3 $4/2.5_million1.dat $4/2.5_million2.dat 

 
./split $1/3_million.dat 3000000 $2 $3 $4/3_million1.dat $4/3_million2.dat


echo "finished 3 million"

./split $1/3.5_million.dat 3500000 $2 $3 $4/3.5_million1.dat $4/3.5_million2.dat

 
./split $1/4_million.dat 4000000 $2 $3 $4/4_million1.dat $4/4_million2.dat


echo "finished 4 million"

./split $1/4.5_million.dat 4500000 $2 $3 $4/4.5_million1.dat $4/4.5_million2.dat

 
./split $1/5_million.dat 5000000 $2 $3 $4/5_million1.dat $4/5_million2.dat


echo "finished 5 million"

./split $1/5.5_million.dat 5500000 $2 $3 $4/5.5_million1.dat $4/5.5_million2.dat

 
./split $1/6_million.dat 6000000 $2 $3 $4/6_million1.dat $4/6_million2.dat


echo "finished 6 million"

./split $1/6.5_million.dat 6500000 $2 $3 $4/6.5_million1.dat $4/6.5_million2.dat

 
./split $1/7_million.dat 7000000 $2 $3 $4/7_million1.dat $4/7_million2.dat


echo "finished 7 million"

./split $1/7.5_million.dat 7500000 $2 $3 $4/7.5_million1.dat $4/7.5_million2.dat

 
./split $1/8_million.dat 8000000 $2 $3 $4/8_million1.dat $4/8_million2.dat


echo "finished 8 million"

./split $1/8.5_million.dat 8500000 $2 $3 $4/8.5_million1.dat $4/8.5_million2.dat

 
./split $1/9_million.dat 9000000 $2 $3 $4/9_million1.dat $4/9_million2.dat


echo "finished 9 million"

./split $1/9.5_million.dat 9500000 $2 $3 $4/9.5_million1.dat $4/9.5_million2.dat

 
./split $1/ten_million.dat 10000000 $2 $3 $4/ten_million1.dat $4/ten_million2.dat


echo "finished 10 million"   
