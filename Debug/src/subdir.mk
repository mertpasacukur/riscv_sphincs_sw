################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/dbg_printf.c \
../src/main.c \
../src/myTime.c \
../src/my_uart.c \
../src/my_utils.c 

S_UPPER_SRCS += \
../src/crt0.S 

O_SRCS += \
../src/crt0.o 

OBJS += \
./src/crt0.o \
./src/dbg_printf.o \
./src/main.o \
./src/myTime.o \
./src/my_uart.o \
./src/my_utils.o 

S_UPPER_DEPS += \
./src/crt0.d 

C_DEPS += \
./src/dbg_printf.d \
./src/main.d \
./src/myTime.d \
./src/my_uart.d \
./src/my_utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross Assembler'
	riscv64-unknown-elf-gcc -march=rv32imc -mabi=ilp32 -msmall-data-limit=8 -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv64-unknown-elf-gcc -march=rv32imc -mabi=ilp32 -msmall-data-limit=8 -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


