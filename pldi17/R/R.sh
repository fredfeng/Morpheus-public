#!/bin/bash

for f in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 48 46 47 49 51 52 53 54 55 56 86 58 59 60 61 62 63 64 72 69 73 74 77 78 79 81 82 83 84 85 87 88 90 92 93 94 95 ; do

if grep -q "^R program:" benchmark$f.txt ;
then
    y=`grep "Perc" benchmark$f.txt | cut -d "=" -f 2 | cut -d ' ' -f 1`
    echo $y
else echo FALSE
fi

done
