#!/bin/bash

ulimit -t 300
spec3="$(PWD)/specs/spec3/*"
spec2="$(PWD)/specs/spec2/*"
spec1="$(PWD)/specs/spec1/*"
 
 #12 and 15 have issues with the input-output tables
for p in 1 2 3 4 5 6 7 9 10 13 14 16 18 19 20 21 22 23 24 25 26 28 29 30 ; do 

(time ./Morpheus ../benchmarks/pldi17/$p/input.json $spec3 1 4) &> output/spec3b/$p.txt

#(time ./Morpheus ../benchmarks/pldi17/$p/input.json "/Users/utcs/Desktop/sandbox/github/Morpheus/specs/evaluation/spec2/*" 1 4) &> output/spec2b/$p.txt

#(time ./Morpheus-weak ../benchmarks/pldi17/$p/input.json "/Users/utcs/Desktop/sandbox/github/Morpheus/specs/evaluation/spec3/*" 1 4) &> output/spec3a/$p.txt

#(time ./Morpheus-weak ../benchmarks/pldi17/$p/input.json "/Users/utcs/Desktop/sandbox/github/Morpheus/specs/evaluation/spec2/*" 1 4) &> output/spec2a/$p.txt

#(time ./Morpheus-weak ../benchmarks/pldi17/$p/input.json "/Users/utcs/Desktop/sandbox/github/Morpheus/specs/evaluation/spec1/*" 1 4) &> output/spec1/$p.txt

done
