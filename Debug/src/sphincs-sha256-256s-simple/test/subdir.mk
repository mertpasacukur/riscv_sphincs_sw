################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sphincs-sha256-256s-simple/test/benchmark.c \
../src/sphincs-sha256-256s-simple/test/fors.c \
../src/sphincs-sha256-256s-simple/test/spx.c \
../src/sphincs-sha256-256s-simple/test/wots.c 

OBJS += \
./src/sphincs-sha256-256s-simple/test/benchmark.o \
./src/sphincs-sha256-256s-simple/test/fors.o \
./src/sphincs-sha256-256s-simple/test/spx.o \
./src/sphincs-sha256-256s-simple/test/wots.o 

C_DEPS += \
./src/sphincs-sha256-256s-simple/test/benchmark.d \
./src/sphincs-sha256-256s-simple/test/fors.d \
./src/sphincs-sha256-256s-simple/test/spx.d \
./src/sphincs-sha256-256s-simple/test/wots.d 


# Each subdirectory must supply rules for building sources it contributes
src/sphincs-sha256-256s-simple/test/%.o: ../src/sphincs-sha256-256s-simple/test/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv64-unknown-elf-gcc -march=rv32imc -mabi=ilp32 -msmall-data-limit=8 -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


