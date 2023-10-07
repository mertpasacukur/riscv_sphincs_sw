################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/tiny_aes/aes.c 

O_SRCS += \
../src/tiny_aes/aes.o 

OBJS += \
./src/tiny_aes/aes.o 

C_DEPS += \
./src/tiny_aes/aes.d 


# Each subdirectory must supply rules for building sources it contributes
src/tiny_aes/%.o: ../src/tiny_aes/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv64-unknown-elf-gcc -march=rv32imc -mabi=ilp32 -msmall-data-limit=8 -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


