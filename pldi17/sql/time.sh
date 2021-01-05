#!/bin/bash

for f in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 ; do

if grep -q "^R program:" benchmark$f.txt ;
then
    y=`grep "Total time:" benchmark$f.txt | cut -d ":" -f 2`
    echo "scale= 2; $y/1000" | bc -l
else echo 300
fi

done
