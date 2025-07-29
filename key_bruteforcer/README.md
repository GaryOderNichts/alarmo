# Key Bruteforcer
This project contains a payload intended to be loaded with openocd, which dumps necessary data to bruteforce the AES key.  
It also contains a x86 PC tool to bruteforce the key.  

The old payload which bruteforces the key on the Alarmo itself can be found [here](./old/README.md).

## Usage

### Using the USB payload
- Build or download the latest USB payload.
- Run `make bruteforcer` (or `make bruteforcer-crypto` when on a non-x86 machine) to compile the PC tool.
- Hold down the confirm, back and notification button on the Alarmo at the same time.
- While holding down all three buttons, plug in the USB cable to your PC.  
  The dial button on top of the Alarmo should light up red and a drive should appear on the PC.
- Copy the `a.bin` from the USB payload to the newly appeared drive.
- Copy the `MarkFile` from the USB payload to the drive.  
  The alarmo should disconnect from the PC and a picture of a cat is displayed on the screen.
- Press the back button on the Alarmo to display the QR code.
- Scan the QR code and copy the containing text to your PC.
- Run `./bruteforcer <insert QR text here>` to start bruteforcing the key. This will take a few minutes.

### Using SWD
- Update the `FIRMWARE_VERSION` define in the `main.c` to match the software version the Alarmo is on.
- Run `make` to compile the payload and the PC tool.
- Boot the Alarmo normally and wait until you can see the clock screen.
- Connect to the debug interface using openocd and run the `halt` command.
- Run `load_image payload.elf` to load the payload into memory.
- Run `resume 0x20000000` to execute the payload. The necessary data will now be stored to memory.  
  Once the payload is done a breakpoint will be triggered.  
- Run `dump_image aes_data.bin 0x24000000 0x50` to dump the data to a file.
- Run `./bruteforcer aes_data.bin` to start bruteforcing the key. This will take a few minutes.
