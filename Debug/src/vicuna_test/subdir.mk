################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/vicuna_test/opt_reduction.c \
../src/vicuna_test/opt_shiftrows.c \
../src/vicuna_test/opt_test.c \
../src/vicuna_test/opt_vlse.c \
../src/vicuna_test/test.c 

OBJS += \
./src/vicuna_test/opt_reduction.o \
./src/vicuna_test/opt_shiftrows.o \
./src/vicuna_test/opt_test.o \
./src/vicuna_test/opt_vlse.o \
./src/vicuna_test/test.o 

C_DEPS += \
./src/vicuna_test/opt_reduction.d \
./src/vicuna_test/opt_shiftrows.d \
./src/vicuna_test/opt_test.d \
./src/vicuna_test/opt_vlse.d \
./src/vicuna_test/test.d 


# Each subdirectory must supply rules for building sources it contributes
src/vicuna_test/%.o: ../src/vicuna_test/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv64-unknown-elf-gcc -march=rv32imc -mabi=ilp32 -msmall-data-limit=8 -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


