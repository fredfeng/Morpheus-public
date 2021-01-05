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

if [ "$#" -lt 3 ]; then
    echo "Usage: ./run-morpheus.sh <path/to/benchmark/json> <path/to/dir/spec> <path/to/output/file> <options (optional)>"
    echo "Options: --disablepe (disables partial evaluation)"
    echo "e.g. ./run-morpheus.sh benchmarks/1/input.json specs/spec2 output.txt"
    exit 1
fi

declare -a pids
declare -a active

logo

killall Morpheus &> /dev/null
rm -f /tmp/the.lock

benchmark="$1"
spec="$2"
output="$3"
opt="$4"
rm -f /tmp/morpheus.tmp1
rm -f /tmp/morpheus.tmp2
rm -f /tmp/morpheus.tmp3
rm -f /tmp/morpheus.tmp4
rm -f /tmp/morpheus.tmp5
if [ -f $output ] ; then
    rm -f $output ;
fi

./Morpheus $opt --benchmark $benchmark --specs "$spec/*" --ngram ngram/size12-ngram.txt --sketchmap ngram/size12-map.txt &> /tmp/morpheus.tmp1 &
pids[1]=$!
./Morpheus $opt --benchmark $benchmark --specs "$spec/*" --ngram ngram/size3-ngram.txt --sketchmap ngram/size3-map.txt &> /tmp/morpheus.tmp2 &
pids[2]=$!
./Morpheus $opt --benchmark $benchmark --specs "$spec/*" --ngram ngram/size4-ngram.txt --sketchmap ngram/size4-map.txt &> /tmp/morpheus.tmp3 &
pids[3]=$!
./Morpheus $opt --benchmark $benchmark --specs "$spec/*" --ngram ngram/size5-ngram.txt --sketchmap ngram/size5-map.txt &> /tmp/morpheus.tmp4 &
pids[4]=$!
./scripts/sleep.sh 300 &
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
		    #echo "Morheus timeout."
		    echo "Morheus timeout." > $output
		    killall Morpheus &> /dev/null
		    end=0
		    break
		fi
		rm -f /tmp/the.lock
	    fi
	elif [ ${active[$f]} -eq 1 ] ; then
	    if `grep -q "^R program:" /tmp/morpheus.tmp$f` ; then
		lockfile -r 0 /tmp/the.lock || exit 1
		if [ ! -f $output ] ; then
		    #echo "Morheus thread $f found the solution."
		    echo "$(tput setaf 4)[Morpheus]$(tput sgr 0) output file: $(tput bold)$output$(tput sgr 0)"
		    echo "Morheus thread $f finished." > $output
		    cat /tmp/morpheus.tmp$f >> $output
		    killall Morpheus &> /dev/null
		    kill -9 ${pids[5]} &> /dev/null
		    end=0
		    break
		fi
		rm -f /tmp/the.lock
	    fi

	    if `grep -q "R program not found" /tmp/morpheus.tmp$f` ; then
		#echo "Morpheus thread $f did not found the solution."
		active[$f]=0
		pids[$f]=${pids[5]}
	    fi
	fi
    done
done
    
