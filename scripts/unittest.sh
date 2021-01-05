####################SketchTreeTest######################################
rm sketchTreeTest
rm DeductionTest
rm RTest
rm UtilTest

g++ -std=c++11 -I"../src/headers/" -I"../include/R/" -I"../include/Rcpp/" -I"../include/Rcpp/Rcpp/" -I"../include/Rinside/" -o SketchTree.o -c ../src/model/SketchTree.cpp
g++ -std=c++11 -I"../src/headers/" -I"../include/" -I"../include/R/" -I"../include/Rcpp/" -I"../include/Rcpp/Rcpp/" -I"../include/Rinside/" -o SketchTreeTest.o -c ../test/SketchTreeTest.cpp
g++ -o "sketchTreeTest" SketchTree.o SketchTreeTest.o ../lib/libgtest.a ../lib/libR.so ../lib/libRInside.a -g -Wall -Wextra -pthread

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

####################UtilTest######################################
g++ -std=c++11 -I"../src/headers/" -I"../include/z3" -I"../include/R/" -I"../include/Rcpp/" -I"../include/Rcpp/Rcpp/" -I"../include/Rinside/" -o UtilTest.o -c ../test/UtilTest.cpp
g++ -std=c++11 -o "UtilTest" UtilTest.o ../lib/libz3.so ../lib/libgtest.a ../lib/libR.so ../lib/libRInside.a -g -Wall -Wextra -pthread

./UtilTest


rm *.o
