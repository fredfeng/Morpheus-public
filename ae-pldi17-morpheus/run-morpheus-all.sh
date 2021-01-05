#!/bin/bash

function logo() {

    echo "$(tput setaf 4)          __  __                  _$(tput sgr 0)"
    echo "$(tput setaf 4)         |  \/  |                | |$(tput sgr 0)"
    echo "$(tput setaf 4)         | \  / | ___  _ __ _ __ | |__   ___ _   _ ___$(tput sgr 0)"
    echo "$(tput setaf 4)         | |\/| |/ _ \| '__| '_ \| '_ \ / _ \ | | / __|$(tput sgr 0)"
    echo "$(tput setaf 4)         | |  | | (_) | |  | |_) | | | |  __/ |_| \__ \ $(tput sgr 0)"
    echo "$(tput setaf 4)         |_|  |_|\___/|_|  | .__/|_| |_|\___|\__,_|___/$(tput sgr 0)"
    echo "$(tput setaf 4)                           | |$(tput sgr 0)"
    echo "$(tput setaf 4)                           |_|$(tput sgr 0)"

}

function run() {
    for f in benchmarks/* ; do

	id=`echo $f | cut -d '/' -f 2`
	./run-morpheus.sh $f/input.json $spec $output/benchmark$id.txt $options &> /dev/null

	if [ ! -f  $output/benchmark$id.txt ] ; then
	    echo "$(tput setaf 4)[Morpheus ($label)]$(tput sgr 0) Benchmark $(tput bold)$id$(tput sgr 0)            $(tput setaf 1)[FAILED]$(tput sgr 0)"
	elif grep -q "^R program:" $output/benchmark$id.txt ; then
	    echo "$(tput setaf 4)[Morpheus ($label)]$(tput sgr 0) Benchmark $(tput bold)$id$(tput sgr 0)            $(tput setaf 2)[OK]$(tput sgr 0)"
	else
	    echo "$(tput setaf 4)[Morpheus ($label)]$(tput sgr 0) Benchmark $(tput bold)$id$(tput sgr 0)            $(tput setaf 1)[FAILED]$(tput sgr 0)"
	fi
	
    done
}

logo

# run spec2
spec="specs/spec2"
output="output/spec2"
options=""
label="Spec 2"
run

spec="specs/spec2"
output="output/spec2-no-pe"
options="--disablepe"
label="Spec 2 [no pe]"
run

spec="specs/spec1"
output="output/spec1"
options=""
label="Spec 1"
run

spec="specs/spec1"
output="output/spec1-no-pe"
options="--disablepe"
label="Spec 1 [no pe]"
run

spec="specs/no-deduction"
output="output/no-deduction"
options=""
label="No deduction"
run

