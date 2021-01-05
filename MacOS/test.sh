#!/bin/bash

ulimit -t 60

total=0
passed=0

#for p in 1 2 3 4 5 6 7 9 10 12 13 14 15 16 18 19 20 21 22 23 24 25 26 28 29 30   ; do 
#for p in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30   ; do 
for p in 13 1 20 23 24 25 26 2 30 3 4 6 ; do 
#./Morpheus ../benchmarks/pldi17/$p/input.json "/Users/utcs/Desktop/sandbox/github/Morpheus/specs/benchmarks/colrow/$p/*" 1 5 &> tmp
./Morpheus --benchmark ../benchmarks/pldi17/$p/input.json --specs "/Users/utcs/Desktop/sandbox/github/Morpheus/specs/L4/*" --fixsketch &> tmp
#./Morpheus ../benchmarks/pldi17/$p/input.json "/Users/utcs/Desktop/sandbox/github/Morpheus/specs/debug/*" 1 5 &> tmp

(( total++ ))

if grep -q "R program not found" tmp ; then
	if [ $p -lt 10 ]; then
		echo "$(tput setaf 4)[Morpheus]$(tput sgr 0) Benchmark $(tput bold)$p$(tput sgr 0)            $(tput setaf 1)[FAILED]$(tput sgr 0)"
	else
		echo "$(tput setaf 4)[Morpheus]$(tput sgr 0) Benchmark $(tput bold)$p$(tput sgr 0)           $(tput setaf 1)[FAILED]$(tput sgr 0)"
	fi
else
	if grep -q "R program" tmp ; then
		(( passed++ ))
		if [ $p -lt 10 ]; then
			echo "$(tput setaf 4)[Morpheus]$(tput sgr 0) Benchmark $(tput bold)$p$(tput sgr 0)            $(tput setaf 2)[OK]$(tput sgr 0)"
			else
			echo "$(tput setaf 4)[Morpheus]$(tput sgr 0) Benchmark $(tput bold)$p$(tput sgr 0)           $(tput setaf 2)[OK]$(tput sgr 0)"
		fi
	else 	
		if [ $p -lt 10 ]; then
			echo "$(tput setaf 4)[Morpheus]$(tput sgr 0) Benchmark $(tput bold)$p$(tput sgr 0)            $(tput setaf 1)[FAILED]$(tput sgr 0)"
			else
			echo "$(tput setaf 4)[Morpheus]$(tput sgr 0) Benchmark $(tput bold)$p$(tput sgr 0)           $(tput setaf 1)[FAILED]$(tput sgr 0)"
		fi
	fi
fi

rm -f tmp

done

if [ $passed -lt $total ] ; then
	echo "$(tput setaf 4)[Morpheus]$(tput sgr 0) Tests Passed           $(tput setaf 1)[$passed / $total]$(tput sgr 0)"
else
	echo "$(tput setaf 4)[Morpheus]$(tput sgr 0) Tests Passed           $(tput setaf 2)[$passed / $total]$(tput sgr 0)"
fi

