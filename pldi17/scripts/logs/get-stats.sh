#!/bin/bash

fname=$1

grep "sketches with deduction:" $fname | cut -d ':' -f 2 | cut -d ' ' -f 2
grep "sketches without deduction:" $fname | cut -d ':' -f 2 | cut -d ' ' -f 2
grep "predicates with deduction:" $fname | cut -d ':' -f 2 | cut -d ' ' -f 2
grep "predicates without deduction" $fname | cut -d ':' -f 2 | cut -d ' ' -f 2

y=`grep "Total time:" $fname | cut -d ":" -f 2`
echo "scale= 2; $y/1000" | bc -l

