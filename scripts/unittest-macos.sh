####################SketchTreeTest######################################
rm -f sketchTreeTest
rm DeductionTest
rm RTest

g++ -I"../src/headers/" -o SketchTree.o -c ../src/model/SketchTree.cpp
g++ -I"../src/headers/" -I"../include/" -o SketchTreeTest.o -c ../test/SketchTreeTest.cpp
g++ -o "sketchTreeTest" SketchTree.o SketchTreeTest.o ../lib/macos/libgtest.a -g -Wall -Wextra -pthread

./sketchTreeTest
####################DeductionTest######################################
g++ -std=c++11 -I"../src/headers/" -I"../include/z3" -I"../include/rapidjson/" -o Component.o -c ../src/model/Component.cpp
g++ -std=c++11 -I"../src/headers/" -I"../include/z3" -I"../include/rapidjson/" -I"../include/R/" -I"../include/Rcpp/" -I"../include/Rcpp/Rcpp/" -I"../include/Rinside/" -o Deduction.o -c ../src/deduction/Deduction.cpp
g++ -std=c++11 -I"../src/headers/" -I"../include/z3" -I"../include/rapidjson/" -I"../include/R/" -I"../include/Rcpp/" -I"../include/Rcpp/Rcpp/" -I"../include/Rinside/" -o DeductionTest.o -c ../test/DeductionTest.cpp
g++ -std=c++11 -o "DeductionTest" Deduction.o DeductionTest.o SketchTree.o Component.o ../lib/libgtest.a ../lib/libz3.so ../lib/libR.so ../lib/libRInside.a -g -Wall -Wextra -pthread

./DeductionTest

####################RTest######################################
g++ -std=c++11 -I"../src/headers/" -I"../include/R/" -I"../include/Rcpp/" -I"../include/Rcpp/Rcpp/" -I"../include/Rinside/" -o RTest.o -c ../test/RTest.cpp
g++ -std=c++11 -o "RTest" RTest.o ../lib/libgtest.a ../lib/libR.so ../lib/libRInside.a -g -Wall -Wextra -pthread

./RTest

rm *.o
