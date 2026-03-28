.PHONY: all clean bruteforcer

all: bruteforcer
	arm-none-eabi-gcc -o payload.elf main.c start.S -mthumb -march=armv7e-m -mtune=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -nostartfiles -nodefaultlibs -T link.ld \
		-I../usb_payload/STM32CubeH7/Drivers/CMSIS/Include -I../usb_payload/STM32CubeH7/Drivers/CMSIS/Device/ST/STM32H7xx/Include

bruteforcer:
	gcc bruteforcer.c -maes -O3 -o bruteforcer

clean:
	rm -f bruteforcer payload.elf
