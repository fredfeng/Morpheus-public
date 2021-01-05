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
    rm -f /tmp/morpheus.table.tmp?
    timeout=0

    declare -a results
    p=1

    for f in {1..80} ; do
	y=`scripts/time.sh $dir/benchmark$f.txt`
	results[$p]=$y
	echo $y >> /tmp/morpheus.table.tmp0
	(( p++ ))
    done

    p=1
    while read line ; do
	case "$line" in
	    1) echo ${results[$p]} >> /tmp/morpheus.table.tmp1 ;;
            2) echo ${results[$p]} >> /tmp/morpheus.table.tmp2 ;;
	    3) echo ${results[$p]} >> /tmp/morpheus.table.tmp3 ;;
	    4) echo ${results[$p]} >> /tmp/morpheus.table.tmp4 ;;
	    5) echo ${results[$p]} >> /tmp/morpheus.table.tmp5 ;;
	    6) echo ${results[$p]} >> /tmp/morpheus.table.tmp6 ;;
	    7) echo ${results[$p]} >> /tmp/morpheus.table.tmp7 ;;
	    8) echo ${results[$p]} >> /tmp/morpheus.table.tmp8 ;;
	    9) echo ${results[$p]} >> /tmp/morpheus.table.tmp9 ;;
	    *) ;;
	esac
    (( p++ ))
    done < scripts/category-id.txt

    #echo "Category,Description,#,No deduction (#Solved),No deduction (Time),Spec 1 (#Solved),Spec 1 (Time),Spec 2 (#Solved),Spec 2 (Time)" > $fname
    echo "Category,Description,#,$dir (#Solved),$dir (Time)" > $fname
    
    for f in 1 2 3 4 5 6 7 8 9 0 ; do
	median=`Rscript -e 'd<-scan("stdin",quiet=TRUE)' -e 'cat(round(median(d),2), sep="\n")' < /tmp/morpheus.table.tmp$f`
	timeout=`grep 300 /tmp/morpheus.table.tmp$f -c`
	nb=`cat /tmp/morpheus.table.tmp$f | wc -l`
	case "$f" in
	    1) echo "C1,Reshaping dataframes from either 'long' to 'wide' or 'wide' to 'long',$nb,$((nb-timeout)),$median" >> $fname ;;
	    2) echo "C2,Arithmetic computations that produce values not present in the input tables,$nb,$((nb-timeout)),$median" >> $fname ;;
	    3) echo "C3,Combination of reshaping and string manipulation of cell contents,$nb,$((nb-timeout)),$median" >> $fname ;;
	    4) echo "C4,Reshaping and arithmetic computations,$nb,$((nb-timeout)),$median" >> $fname;;
	    5) echo "C5,Combination of arithmetic computations and consolidation of information from multiple tables into a single table,$nb,$((nb-timeout)),$median" >> $fname;;
	    6) echo "C6,Arithmetic computations and string manipulation tasks,$nb,$((nb-timeout)),$median" >> $fname;;
	    7) echo "C7,Reshaping and consolidation tasks,$nb,$((nb-timeout)),$median" >> $fname;;
	    8) echo "C8,Combination of reshaping and arithmetic computations and string manipulation,$nb,$((nb-timeout)),$median" >> $fname;;
	    9) echo "C9,Combination of reshaping and arithmetic computations and consolidation,$nb,$((nb-timeout)),$median" >> $fname;;
	    0) echo ",Total,$nb,$((nb-timeout)),$median" >> $fname;;
	    *) ;;
	esac	
    done

    echo "$(tput setaf 4)[Morpheus]$(tput sgr 0) created csv file: $(tput bold)$fname$(tput sgr 0)"
    
}

if [ "$#" -ne 2 ] ; then
    echo "Usage: ./table-morpheus-variant.sh <output/dir/variant> <output/file/csv>"
    echo "e.g. ./table-morpheus-variant.sh logs/spec2 table-spec2.csv"
    exit 1
fi

logo

dir=$1
fname=$2

run
