#!/bin/bash

declare -a line0
declare -a line1
declare -a line2
declare -a line3
declare -a line4
declare -a line5
declare -a line6
declare -a line7
declare -a line8
declare -a line9

t=1

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

function csv() {


    echo "Category,Description,#,No deduction (#Solved),No deduction (Time),Spec 1 (#Solved),Spec 1 (Time),Spec 2 (#Solved),Spec 2 (Time)" > $fname

    for f in 1 2 3 4 5 6 7 8 9 0 ; do
        median=`Rscript -e 'd<-scan("stdin",quiet=TRUE)' -e 'cat(round(median(d),2), sep="\n")' < /tmp/morpheus.table.tmp$f`
	timeout=`grep 300 /tmp/morpheus.table.tmp$f -c`
	nb=`cat /tmp/morpheus.table.tmp$f | wc -l`
	case "$f" in
	    1) echo "C1,Reshaping dataframes from either 'long' to 'wide' or 'wide' to 'long',$nb,${line1[1]},${line1[2]},${line1[3]},${line1[4]},${line1[5]},${line1[6]}" >> $fname ;;
	    2) echo "C2,Arithmetic computations that produce values not present in the input tables,$nb,${line2[1]},${line2[2]},${line2[3]},${line2[4]},${line2[5]},${line2[6]}" >> $fname ;;
	    3) echo "C3,Combination of reshaping and string manipulation of cell contents,$nb,${line3[1]},${line3[2]},${line3[3]},${line3[4]},${line3[5]},${line3[6]}" >> $fname ;;
	    4) echo "C4,Reshaping and arithmetic computations,$nb,${line4[1]},${line4[2]},${line4[3]},${line4[4]},${line4[5]},${line4[6]}" >> $fname ;;
	    5) echo "C5,Combination of arithmetic computations and consolidation of information from multiple tables into a single table,$nb,${line5[1]},${line5[2]},${line5[3]},${line5[4]},${line5[5]},${line5[6]}" >> $fname ;;
	    6) echo "C6,Arithmetic computations and string manipulation tasks,$nb,${line6[1]},${line6[2]},${line6[3]},${line6[4]},${line6[5]},${line6[6]}" >> $fname ;;
	    7) echo "C7,Reshaping and consolidation tasks,$nb,${line7[1]},${line7[2]},${line7[3]},${line7[4]},${line7[5]},${line7[6]}" >> $fname ;;
	    8) echo "C8,Combination of reshaping and arithmetic computations and string manipulation,$nb,${line8[1]},${line8[2]},${line8[3]},${line8[4]},${line8[5]},${line8[6]}" >> $fname ;;
	    9) echo "C9,Combination of reshaping and arithmetic computations and consolidation,$nb,${line9[1]},${line9[2]},${line9[3]},${line9[4]},${line9[5]},${line9[6]}" >> $fname ;;
	    0) echo ",Total,$nb,${line0[1]},${line0[2]},${line0[3]},${line0[4]},${line0[5]},${line0[6]}" >> $fname ;;
	    *) ;;
	esac
    done

    echo "$(tput setaf 4)[Morpheus]$(tput sgr 0) created csv file: $(tput bold)$fname$(tput sgr 0)"

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


    for f in 1 2 3 4 5 6 7 8 9 0 ; do
	median=`Rscript -e 'd<-scan("stdin",quiet=TRUE)' -e 'cat(round(median(d),2), sep="\n")' < /tmp/morpheus.table.tmp$f`
	timeout=`grep 300 /tmp/morpheus.table.tmp$f -c`
	nb=`cat /tmp/morpheus.table.tmp$f | wc -l`
	case "$f" in
	    1) line1[$t]=$((nb-timeout)) ; line1[$((t+1))]=$median ;;
	    2) line2[$t]=$((nb-timeout)) ; line2[$((t+1))]=$median ;;
	    3) line3[$t]=$((nb-timeout)) ; line3[$((t+1))]=$median ;;
	    4) line4[$t]=$((nb-timeout)) ; line4[$((t+1))]=$median ;;
	    5) line5[$t]=$((nb-timeout)) ; line5[$((t+1))]=$median ;;
	    6) line6[$t]=$((nb-timeout)) ; line6[$((t+1))]=$median ;;
	    7) line7[$t]=$((nb-timeout)) ; line7[$((t+1))]=$median ;;
	    8) line8[$t]=$((nb-timeout)) ; line8[$((t+1))]=$median ;;
	    9) line9[$t]=$((nb-timeout)) ; line9[$((t+1))]=$median ;;
	    0) line0[$t]=$((nb-timeout)) ; line0[$((t+1))]=$median ;;
	    *) ;;
	esac	
    done

    t=$((t+2))
    
}

if [ "$#" -ne 2 ] ; then
    echo "Usage: ./table-morpheus-all.sh <output/dir> <output/file/csv>"
    echo "e.g. ./table-morpheus-all.sh logs table-all.csv"
    exit 1
fi

logo

dir=$1
fname=$2

dir="$1/no-deduction"
run

dir="$1/spec1"
run

dir="$1/spec2"
run

csv
