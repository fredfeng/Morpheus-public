################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/model/Benchmark.cpp \
../src/model/Component.cpp \
../src/model/SketchTree.cpp 

OBJS += \
./src/model/Benchmark.o \
./src/model/Component.o \
./src/model/SketchTree.o 

CPP_DEPS += \
./src/model/Benchmark.d \
./src/model/Component.d \
./src/model/SketchTree.d 


# Each subdirectory must supply rules for building sources it contributes
src/model/%.o: ../src/model/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -I"/Users/utcs/Desktop/sandbox/github/Morpheus/include/R" -I"/Users/utcs/Desktop/sandbox/github/Morpheus/include/Rcpp" -I"/Users/utcs/Desktop/sandbox/github/Morpheus/include/Rcpp/Rcpp" -I"/Users/utcs/Desktop/sandbox/github/Morpheus/include/rapidjson" -I"/Users/utcs/Desktop/sandbox/github/Morpheus/include/Rinside" -I"/Users/utcs/Desktop/sandbox/github/Morpheus/include/z3" -I"/Users/utcs/Desktop/sandbox/github/Morpheus/src/headers" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


