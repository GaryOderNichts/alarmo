.PHONY: all clean bruteforcer bruteforcer-crypto

all: bruteforcer
	arm-none-eabi-gcc -o payload.elf main.c start.S -mthumb -march=armv7e-m -mtune=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -nostartfiles -nodefaultlibs -T link.ld

bruteforcer:
	gcc bruteforcer.c -maes -O3 -o bruteforcer

bruteforcer-crypto:
	gcc bruteforcer.c -O3 -DUSECRYPTO -lcrypto -o bruteforcer

clean:
	rm -f bruteforcer payload.elf
