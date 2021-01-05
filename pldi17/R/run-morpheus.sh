#!/bin/bash

#ulimit -t 300
declare -a pids
declare -a active

killall Morpheus
rm -f /tmp/the.lock

benchmark=$1
spec=$2
output=benchmark$1.txt
dir=$3
opt=$4
rm -f /tmp/morpheus.tmp1
rm -f /tmp/morpheus.tmp2
rm -f /tmp/morpheus.tmp3
rm -f /tmp/morpheus.tmp4
rm -f /tmp/morpheus.tmp5

./Morpheus $opt --benchmark ../../benchmarks/pldi17/$benchmark/input.json --specs "specs/$spec/*" --ngram sketches/size12-ngram.txt --sketchmap sketches/size12-map.txt &> /tmp/morpheus.tmp1 &
pids[1]=$!
./Morpheus $opt --benchmark ../../benchmarks/pldi17/$benchmark/input.json --specs "specs/$spec/*" --ngram sketches/size3-ngram.txt --sketchmap sketches/size3-map.txt &> /tmp/morpheus.tmp2 &
pids[2]=$!
./Morpheus $opt --benchmark ../../benchmarks/pldi17/$benchmark/input.json --specs "specs/$spec/*" --ngram sketches/size4-ngram.txt --sketchmap sketches/size4-map.txt &> /tmp/morpheus.tmp3 &
pids[3]=$!
./Morpheus $opt --benchmark ../../benchmarks/pldi17/$benchmark/input.json --specs "specs/$spec/*" --ngram sketches/size5-ngram.txt --sketchmap sketches/size5-map.txt &> /tmp/morpheus.tmp4 &
pids[4]=$!
./sleep.sh 300 &
pids[5]=$!

active[1]=1
active[2]=1
active[3]=1
active[4]=1
active[5]=1

end=1

while [ $end -eq 1 ] ; do

    while [[ -e /proc/${pids[1]} && -e /proc/${pids[2]} && -e /proc/${pids[3]} && -e /proc/${pids[4]} && -e /proc/${pids[5]} ]] ; do
	sleep 1
    done

    for f in {1..5} ; do
	if [ $f -eq 5 ] ; then
	    if [ ! -e /proc/${pids[5]} ] ; then
		lockfile -r 0 /tmp/the.lock || exit 1
		if [ ! -f $output ] ; then
		    echo "Morheus timeout."
		    echo "Morheus timeout." > $dir/$output
		    killall Morpheus
		    end=0
		    break
		fi
		rm -f /tmp/the.lock
	    fi
	elif [ ${active[$f]} -eq 1 ] ; then
	    if `grep -q "^R program:" /tmp/morpheus.tmp$f` ; then
		lockfile -r 0 /tmp/the.lock || exit 1
		if [ ! -f $output ] ; then
		    echo "Morheus thread $f found the solution."
		    echo "Morheus thread $f finished." > $dir/$output
		    cat /tmp/morpheus.tmp$f >> $dir/$output
		    killall Morpheus
		    kill -9 ${pids[5]}
		    end=0
		    break
		fi
		rm -f /tmp/the.lock
	    fi

	    if `grep -q "R program not found" /tmp/morpheus.tmp$f` ; then
		echo "Morpheus thread $f did not found the solution."
		active[$f]=0
		pids[$f]=${pids[5]}
	    fi
	fi
    done
done
    
