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

function gplot() {

    echo "set terminal postscript enhanced color \"Helvetica\" 18" > $fname
    echo "set yrange [0:4000]" >> $fname
    echo "set xrange [1:80]" >> $fname
    echo "set key left top" >> $fname
    echo "set xlabel 'Benchmarks'" >> $fname
    echo "set ylabel 'Time'" >> $fname
    echo "" >> $fname
    echo "plot \"$plot/no-deduction.dat\" using 1:2 with linespoints pointsize 1.3 pointtype 1 lt 3 lc 4 title \"No deduction\",\\" >> $fname
    echo "\"$plot/spec1-no-pe.dat\" using 1:2 with linespoints pointsize 1.3 pointtype 10 lt 3 lc 6 title \"Spec 1 \(no p.e.\)\",\\" >> $fname
    echo "\"$plot/spec2-no-pe.dat\" using 1:2 with linespoints pointsize 1.3 pointtype 8 lt 3 lc 1 title \"Spec 2 \(no p.e.\)\",\\" >> $fname
    echo "\"$plot/spec1.dat\" using 1:2 with linespoints pointsize 1.3 pointtype 6 lt 3 lc 6 title \"Spec 1\",\\" >> $fname
    echo "\"$plot/spec2.dat\" using 1:2 with linespoints pointsize 1.3 pointtype 2 lt 3 lc 1 title \"Spec 2\"" >> $fname

}

if [ "$#" -ne 2 ] ; then
    echo "Usage: ./plot-morpheus-all.sh <output/dir> <plot/dir>"
    echo "e.g. ./plot-morpheus-all.sh logs plot"
    exit 1
fi

logo

dir=$1
plot=$2
fname="$plot/morpheus.gnuplot"

for f in no-deduction  spec1  spec1-no-pe  spec2  spec2-no-pe ; do
    rm -f /tmp/$f.morpheus.tmp
    for b in {1..80} ; do
	y=`scripts/time.sh $dir/$f/benchmark$b.txt`
	echo $y >> /tmp/$f.morpheus.tmp
    done
    sed -i '/300/d' /tmp/$f.morpheus.tmp
    sort -n /tmp/$f.morpheus.tmp > /tmp/$f.morpheus.sorted.tmp
    Rscript -e 'd<-scan("stdin",quiet=TRUE)' -e 'cat(round(cumsum(d),2), sep="\n")' < /tmp/$f.morpheus.sorted.tmp > /tmp/$f.morpheus.plot

    p=1
    rm -f $plot/$f.dat
    while read line ; do
	echo $p $line >> $plot/$f.dat
	(( p++ ))
    done < /tmp/$f.morpheus.plot
    rm -f /tmp/$f.morpheus.plot
    rm -f /tmp/$f.morpheus.tmp
    rm -f /tmp/$f.morpheus.sorted.tmp
done

gplot

gnuplot -persist $plot/morpheus.gnuplot > $plot/morpheus.ps
ps2pdf $plot/morpheus.ps
mv morpheus.pdf $plot/morpheus-plot.pdf
rm -f $plot/morpheus.ps

echo "$(tput setaf 4)[Morpheus]$(tput sgr 0) created plot file: $(tput bold)$plot/morpheus-plot.pdf$(tput sgr 0)"


