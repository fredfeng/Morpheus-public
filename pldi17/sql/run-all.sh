#!/bin/bash

for f in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 ; do
    echo $f ;
    ./run-morpheus.sh $f spec4 output/sql sql
done

for f in 1 2 3 4 5 ; do
    echo $f ;
    ./run-morpheus.sh $f spec4 output/sql-forum sql-forum
done
