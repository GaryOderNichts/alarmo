.PHONY: all clean bruteforcer bruteforcer-arm bruteforcer-crypto

all: bruteforcer
	arm-none-eabi-gcc -o payload.elf main.c start.S -mthumb -march=armv7e-m -mtune=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -nostartfiles -nodefaultlibs -T link.ld \
		-I../usb_payload/STM32CubeH7/Drivers/CMSIS/Include -I../usb_payload/STM32CubeH7/Drivers/CMSIS/Device/ST/STM32H7xx/Include

bruteforcer:
	gcc bruteforcer.c -maes -O3 -o bruteforcer

# Native arm64 (e.g. Apple Silicon): hardware AESE/AESMC, no OpenSSL needed.
bruteforcer-arm:
	$(CC) bruteforcer.c -O3 -march=armv8-a+crypto -o bruteforcer

bruteforcer-crypto:
	gcc bruteforcer.c -O3 -DUSECRYPTO -lcrypto -o bruteforcer

clean:
	rm -f bruteforcer payload.elf
