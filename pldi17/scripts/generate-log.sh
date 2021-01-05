#!/bin/bash

benchmark=$1
num=$2
paper=$3

#read URL
while read -r line ; do
	url=$line
done < $benchmark-url.txt

#read category
while read -r line ; do
	category=$line
done < $benchmark-cat.txt

#read num input tables
while read -r line ; do
	numtbl=$line
done < $benchmark-input.txt

echo "--------------------------------------------------------------------------------"
echo "| Stackoverflow                                                                |"
echo "--------------------------------------------------------------------------------"
echo ""
echo "URL= $url"
echo ""
echo "--------------------------------------------------------------------------------"
echo "| Benchmark                                                                    |"
echo "--------------------------------------------------------------------------------"
echo ""
echo "ID= $paper"
echo "Category= $category"
echo ""
echo "--------------------------------------------------------------------------------"
echo "| Input-Output Example                                                          |"
echo "--------------------------------------------------------------------------------"
echo ""

#R script for input
echo "library(dplyr)" > /tmp/morpheus.script 
echo "library(tidyr)" >> /tmp/morpheus.script 
echo "library(MorpheusData)" >> /tmp/morpheus.script 
echo "p"$num"_input1" >> /tmp/morpheus.script 
if [ $numtbl -eq 1 ] ; then 
	echo "Input:"
else
	echo "Input1:"
fi
echo ""
Rscript /tmp/morpheus.script 2> /dev/null

if [ $numtbl -eq 2 ] ; then
	echo "library(dplyr)" > /tmp/morpheus.script 
	echo "library(tidyr)" >> /tmp/morpheus.script 
	echo "library(MorpheusData)" >> /tmp/morpheus.script 
	echo "p"$num"_input2" >> /tmp/morpheus.script 
	echo ""
	echo "Input2:"
	echo ""
	Rscript /tmp/morpheus.script 2> /dev/null
fi

echo "library(dplyr)" > /tmp/morpheus.script 
echo "library(tidyr)" >> /tmp/morpheus.script 
echo "library(MorpheusData)" >> /tmp/morpheus.script 
echo "p"$num"_output1" >> /tmp/morpheus.script 
echo ""
echo "Output:"
echo ""
Rscript /tmp/morpheus.script 2> /dev/null

echo ""
echo "--------------------------------------------------------------------------------"
echo "| Synthesized Solution                                                         |"
echo "--------------------------------------------------------------------------------"
echo ""

if `grep -q timeout $benchmark-solution.txt` ; then
	echo "Morpheus timeout."
else

	echo "library(dplyr)" > /tmp/morpheus.script 
	echo "library(tidyr)" >> /tmp/morpheus.script 
	echo "library(MorpheusData)" >> /tmp/morpheus.script 

	while read -r line ; do 

		if [[ "$line" != "morpheus=as.data.frame(morpheus)" && "$line" != "morpheus" && "$line" != *"as.numeric"* ]] ; then

			if [[ "$line" == *"rename"* || "$line" == *"select(morpheus"* ]] ; then
				cmd=morpheus
			else
				cmd="tmp=$line"
			fi
			
			echo $line
			echo ""

			echo $line >> /tmp/morpheus.script
			cp /tmp/morpheus.script /tmp/morpheus.script2
			echo ""$cmd"" >> /tmp/morpheus.script2
			echo "tmp" >> /tmp/morpheus.script2
			Rscript /tmp/morpheus.script2 2> /dev/null
			echo ""
		fi	
	done < $benchmark-solution.txt

echo ""
echo "R program:"
echo ""
while read -r line ; do 
	if [[ "$line" != *"as.numeric"* ]] ; then
		echo $line
	fi
done < $benchmark-solution.txt
fi

echo ""
echo "--------------------------------------------------------------------------------"
echo "| Statistics                                                                   |"
echo "--------------------------------------------------------------------------------"
echo ""

if `grep -q timeout $benchmark-solution.txt` ; then
	echo "Morpheus timeout."
else

	declare -a stats

	c=1
	while read -r line ; do 
		stats[$c]=$line
		((c++))
	done < $benchmark-stats.txt

	echo "#sketches with SMT-based deduction= ${stats[1]}"
	echo "#sketches without SMT-based deduction= ${stats[2]}" 
	echo "#partial programs with partial evaluation= ${stats[3]}" 
	echo "#partial programs without partial evaluation= ${stats[4]}" 
	echo "Total time= ${stats[5]} s" 

fi