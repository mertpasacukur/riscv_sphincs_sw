################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sphincs-sha256-256s-simple/PQCgenKAT_sign.c \
../src/sphincs-sha256-256s-simple/address.c \
../src/sphincs-sha256-256s-simple/fors.c \
../src/sphincs-sha256-256s-simple/hash_sha256.c \
../src/sphincs-sha256-256s-simple/rng.c \
../src/sphincs-sha256-256s-simple/sha256.c \
../src/sphincs-sha256-256s-simple/sign.c \
../src/sphincs-sha256-256s-simple/thash_sha256_simple.c \
../src/sphincs-sha256-256s-simple/utils.c \
../src/sphincs-sha256-256s-simple/wots.c 

OBJS += \
./src/sphincs-sha256-256s-simple/PQCgenKAT_sign.o \
./src/sphincs-sha256-256s-simple/address.o \
./src/sphincs-sha256-256s-simple/fors.o \
./src/sphincs-sha256-256s-simple/hash_sha256.o \
./src/sphincs-sha256-256s-simple/rng.o \
./src/sphincs-sha256-256s-simple/sha256.o \
./src/sphincs-sha256-256s-simple/sign.o \
./src/sphincs-sha256-256s-simple/thash_sha256_simple.o \
./src/sphincs-sha256-256s-simple/utils.o \
./src/sphincs-sha256-256s-simple/wots.o 

C_DEPS += \
./src/sphincs-sha256-256s-simple/PQCgenKAT_sign.d \
./src/sphincs-sha256-256s-simple/address.d \
./src/sphincs-sha256-256s-simple/fors.d \
./src/sphincs-sha256-256s-simple/hash_sha256.d \
./src/sphincs-sha256-256s-simple/rng.d \
./src/sphincs-sha256-256s-simple/sha256.d \
./src/sphincs-sha256-256s-simple/sign.d \
./src/sphincs-sha256-256s-simple/thash_sha256_simple.d \
./src/sphincs-sha256-256s-simple/utils.d \
./src/sphincs-sha256-256s-simple/wots.d 


# Each subdirectory must supply rules for building sources it contributes
src/sphincs-sha256-256s-simple/%.o: ../src/sphincs-sha256-256s-simple/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv64-unknown-elf-gcc -march=rv32imc -mabi=ilp32 -msmall-data-limit=8 -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


