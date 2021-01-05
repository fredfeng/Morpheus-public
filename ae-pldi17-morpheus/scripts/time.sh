#!/bin/bash

bench=$1

if grep -q "^R program:" $bench ;
then
    y=`grep "Total time:" $bench | cut -d ":" -f 2`
    echo "scale= 2; $y/1000" | bc -l
else echo 300
fi

