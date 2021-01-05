################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/deduction/Deduction.cpp \
../src/deduction/Synthesizer.cpp \
../src/deduction/SketchEncoding.cpp  \
../src/deduction/Predicate.cpp  \
../src/deduction/SketchFill.cpp

OBJS += \
./src/deduction/Deduction.o \
./src/deduction/Synthesizer.o \
./src/deduction/SketchEncoding.o \
./src/deduction/Predicate.o \
./src/deduction/SketchFill.o

CPP_DEPS += \
./src/deduction/Deduction.d \
./src/deduction/Synthesizer.d \
./src/deduction/SketchEncoding.d \
./src/deduction/Predicate.d \
./src/deduction/SketchFill.d


# Each subdirectory must supply rules for building sources it contributes
src/deduction/%.o: ../src/deduction/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -I"/Users/utcs/Desktop/sandbox/github/Morpheus/include/minisat/core" -I"/Users/utcs/Desktop/sandbox/github/Morpheus/include/minisat/mtl" -I"/Users/utcs/Desktop/sandbox/github/Morpheus/include/R" -I"/Users/utcs/Desktop/sandbox/github/Morpheus/include/Rcpp" -I"/Users/utcs/Desktop/sandbox/github/Morpheus/include/Rcpp/Rcpp" -I"/Users/utcs/Desktop/sandbox/github/Morpheus/include/rapidjson" -I"/Users/utcs/Desktop/sandbox/github/Morpheus/include/Rinside" -I"/Users/utcs/Desktop/sandbox/github/Morpheus/include/z3" -I"/Users/utcs/Desktop/sandbox/github/Morpheus/src/headers" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


