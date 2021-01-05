
  __  __                  _                    
 |  \/  |                | |                   
 | \  / | ___  _ __ _ __ | |__   ___ _   _ ___ 
 | |\/| |/ _ \| '__| '_ \| '_ \ / _ \ | | / __|
 | |  | | (_) | |  | |_) | | | |  __/ |_| \__ \
 |_|  |_|\___/|_|  | .__/|_| |_|\___|\__,_|___/
                   | |                         
                   |_|                         

Morpheus: Component-based Synthesis of Table Consolidation and Transformation Tasks from Examples

+------------------------------------------------------------------------------+
Morpheus is a novel example-driven synthesis tool
for automating a large class of data preparation tasks that
arise in data science. Given a set of input tables and an output 
table, Morpheus synthesizes a table transformation                
program that performs the desired task. Our approach is not
restricted to a fixed set of DSL constructs and can synthesize
programs from an arbitrary set of components, including
higher-order combinators.                             
+------------------------------------------------------------------------------+

+------------------------------------------------------------------------------+
|                                                                              |
| A. Reproducing the results from "Component-based Synthesis of Table          |
|    Consolidationand Transformation Tasks from Examples"                      | 
|                                                                              |
+------------------------------------------------------------------------------

All experiments in the paper were conducted using an Intel 
Xeon(R) computer with an E5-2640 v3 CPU and 32G of memory running Ubuntu 16.04.

Morpheus can run either on Linux or Mac OS X distributions (it is not fully tested under Windows). 

A.0 Check artifacts/solutions/benchmark$ID.txt for detail descriptions of all benchmarks

A.1 Steps to reproduce the results from figure 16 

1) Run the script ./run-morpheus-all.sh
will solve all benchmarks from the paper with all variants (this will be very time-consuming. Might take up to ~12 hours!)

2) After completion, the logs files from this experiment are available on the directory 'output'
Make sure script ./run-morpheus-all.sh has finished before you continue!

3) Run this script to generate the table in figure 16:
./table-morpheus-all.sh <output/dir> <output/file/csv>

For instance, running 
./table-morpheus-all.sh output figure16.csv 
will consolidate your results into figure16.csv

Running:
./table-morpheus-all.sh logs paper-figure16.csv 
will reproduce figure16 in the original paper. 

A.2 Steps to generate the results in figure 17 

1) Run this script to plot the results in figure 17:
./plot-morpheus-all.sh <output/dir> <plot/dir>

For instance, running 
./plot-morpheus-all.sh output aec
will plot your results into aec/morpheus-plot.pdf

Running:
./table-morpheus-all.sh logs paper
will generate paper/morpheus-plot.pdf, which is figure17 in current paper.

A.3 Steps to run one single variant on all 80 benchmarks 

1) Run the script ./run-morpheus-variant.sh will display all variants:
Usage: ./run-morpheus-variant.sh <variant-id>
Variant: 1=spec2, 2=spec2-no-pe, 3=spec1, 4=spec1-no-pe, 5=no deduction

2) For instance, option 1 is the fastest option which solves all benchmarks with our most precise spec (spec2) and it could terminate within an hour:
./run-morpheus-variant.sh 1

3) You could also run option 5 to turn off the deduction (Might take up to > 3 hours!)
./run-morpheus-variant.sh 5 

4) Finally you can get a summary of the output in s2.csv by specifying the folder of your output (output/spec2):
./table-morpheus-variant.sh output/spec2/ s2.csv


+------------------------------------------------------------------------------+
|                                                                              |
| B. How to solve an individual benchmark with Morpheus?                       | 
|                                                                              | 
+------------------------------------------------------------------------------

B.1 To solve a problem from our existing benchmarks:

Usage: ./run-morpheus.sh <path/to/benchmark/json> <path/to/dir/spec> <path/to/output/file> <options (optional)>
Options: --disablepe (disables partial evaluation)

For instance, running
./run-morpheus.sh benchmarks/1/input.json specs/spec2 out.txt
will solve benchmark 1 (using spec2) and dump the output to out.txt

B.2 To solve a new problem the user must provide:

1) All input and output tables are in CSV format. 
  i) all CSV files are separated by "|" and the last CSV should be the output table
  ii) The entire string of specifying the input-output should be quoted!

The directory 'example/' contains a new benchmark that performs reshaping:
  
  i)  input1.csv: input table.
  ii) output1.csv: output table.

2) Running the following script will solve this new example in seconds: 
./run-morpheus.sh "example/input1.csv|example/output1.csv" specs/spec2/ out.txt

B.3 You can also solve a benchmark with different options (different spec, with or without partial evaluation, etc). 


+------------------------------------------------------------------------------+
|                                                                              |
| C. Directory Structure                                                       |
|                                                                              |
+------------------------------------------------------------------------------

+-- README : this file
+-- Morpheus : Morpheus binary
+-- Morpheus_PLDI17.pdf : Current paper
+-- benchmarks : directory with all 80 benchmarks, including .json files and input-output examples in .csv.
+-- artifacts : original data in the paper. Include solutions, links of stackoverflow, table and plot in figure 16, 17, respectively.
|	+-- stackoverflow.txt: original stackoverflow urls for all benchmarks
+-- specs : specifications for all high-order components
|	+-- no_deduction : No deduction. i.e., specs using 'true' for all components.
|	+-- spec1 : specs using rows and columns.
|	+-- spec2 : specs using cardinality and number of groups (more precise than spec1).
+-- example : directory with a new example
|	+-- example_input.csv : input table for a new example
|	+-- example_output.csv : input table for a new example
+-- logs : directory with the logs of the experiment done for the paper
+-- output : directory that will contain the output of the experiment of 'run-all.sh'
+-- run-all.sh : script to run all benchmarks
+-- run-morpheus.sh : script to run Morpheus with an individual 
+-- run-morpheus-variant.sh : script to run Morpheus on different variants
+-- table-morpheus-all.sh : script to generate figure 16 in the paper
+-- table-morpheus-variant.sh : script to generate each column in figure 16
+-- plot-morpheus-all.sh : script to plot figure 17 in the paper
+-- lib : Third-party library
+-- ngram : Offline sketch rankings based on our 2-gram model
+-- scripts : Our internal scripts 


+------------------------------------------------------------------------------+
|                                                                              |
| D. Limitations                                                               |
|                                                                              |
+------------------------------------------------------------------------------

The current version of Morpheus has a few limitations:

- Morpheus only accepts a single input-output example
- Currently all first-order components (e.g., sum, avg, +, <, etc.) are hard-coded and are not easily configurable
- The parallelism in Morpheus is done in a naive way by launching multiple processes with different parameters

We plan to address these limitations in our next version of Morpheus and release it before the PLDI conference. 
To make Morpheus easier to use, we plan to release Morpheus as a R library and make it accessible to millions of R users.
