#!/usr/bin/env python3
# Tool to work with Alarmo CIPH files
# Created by GaryOderNichts in 2024
import sys
import os
from Crypto.Cipher import AES
import hashlib
from key import AES_KEY, AES_IV

RSA_MOD = int.from_bytes([
    0xc1, 0xb5, 0x6f, 0xdf, 0x5f, 0xc9, 0xf8, 0x18, 0xf1, 0xd5, 0x08, 0x8f, 0x32, 0x5e, 0x47, 0xa1,
    0x3c, 0xcf, 0x60, 0xa7, 0x99, 0xf5, 0xae, 0x0a, 0xf0, 0xcd, 0xc9, 0x04, 0xd9, 0xca, 0xf0, 0x15,
    0x1c, 0xc6, 0x8e, 0x44, 0x96, 0x45, 0xd1, 0xe7, 0x41, 0x61, 0x13, 0x4a, 0x0c, 0xfe, 0xf7, 0x8c,
    0x28, 0xce, 0x90, 0x0b, 0x61, 0x2d, 0x83, 0x02, 0xa3, 0xe9, 0x70, 0x3b, 0x6a, 0x8c, 0x6b, 0xa0,
    0x42, 0x2d, 0x5b, 0xcf, 0xd1, 0x1b, 0xc2, 0x7a, 0x39, 0x99, 0x8f, 0x08, 0xeb, 0x06, 0x37, 0x81,
    0x23, 0xe3, 0xbe, 0xe4, 0xe3, 0x37, 0x7d, 0x82, 0xb4, 0x4b, 0x35, 0xc6, 0x43, 0xa7, 0x69, 0x23,
    0xbe, 0x3a, 0x5e, 0x3b, 0xa8, 0x6f, 0x8a, 0x92, 0xca, 0x43, 0x4f, 0x78, 0xb9, 0xfd, 0x62, 0xed,
    0x71, 0x6a, 0xf3, 0x89, 0x57, 0xfc, 0x0e, 0x72, 0x6d, 0x29, 0x9c, 0x50, 0x1c, 0x27, 0x60, 0xd5,
    0x0a, 0x58, 0x2e, 0x2f, 0xc4, 0x4a, 0x26, 0xff, 0x88, 0xf2, 0xac, 0x52, 0xcf, 0xcf, 0x56, 0x86,
    0x92, 0x75, 0x93, 0xea, 0xbe, 0x6b, 0x7c, 0x41, 0xab, 0x76, 0x3b, 0x17, 0x4e, 0x49, 0x5e, 0xc5,
    0xaa, 0xd3, 0x12, 0xed, 0x7d, 0x8b, 0x0b, 0x84, 0x7c, 0x32, 0x15, 0x94, 0x4a, 0x82, 0x17, 0xc9,
    0x80, 0x5c, 0x73, 0x1b, 0xcd, 0xf8, 0x4c, 0x5c, 0x4f, 0x8c, 0xfd, 0x3b, 0x71, 0x60, 0xe2, 0x5c,
    0x8a, 0x32, 0x3e, 0x0e, 0xab, 0xe1, 0x1a, 0xbd, 0x9b, 0xd2, 0x2f, 0x1a, 0x89, 0x8c, 0xdb, 0xe6,
    0xcf, 0xce, 0xb6, 0x24, 0x5e, 0x2e, 0x99, 0x94, 0xc1, 0xc0, 0xa5, 0xfa, 0x13, 0xff, 0x27, 0x14,
    0x28, 0xad, 0x63, 0xdf, 0x2e, 0x68, 0xa2, 0xf4, 0x7b, 0xfc, 0x4f, 0xe7, 0x22, 0x85, 0xcb, 0xd0,
    0x9c, 0x40, 0xfb, 0x3f, 0xa8, 0x25, 0x77, 0x10, 0x8b, 0x11, 0x43, 0x8a, 0x1b, 0xe2, 0xb3, 0x83,
], byteorder='big')

RSA_EXP = int.from_bytes([
    0x01, 0x00, 0x01,
], byteorder='big')

# https://lapo.it/asn1js/#MDEwDQYJYIZIAWUDBAIBBQAEIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
ASN1_PREFIX = b'\x30\x31\x30\x0d\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x01\x05\x00\x04\x20'

def get_remaining_size(f):
    # Get file size
    offset = f.tell()
    f.seek(0, os.SEEK_END)
    size = f.tell() - offset
    f.seek(offset)
    return size

def verify_signature(signature, hash):
    result = pow(signature, RSA_EXP, RSA_MOD).to_bytes(0x100, byteorder='big')

    if result[0] != 0x00 or result[1] != 0x01:
        print('Invalid signature')
        return False

    # Check padding
    padding_length = len(result) - 0x36
    if result[0x02:0x02+padding_length] != b'\xff' * padding_length:
        print('Invalid signature padding')
        return False

    if result[len(result) - 0x34] != 0x00:
        print('Invalid signature')
        return False

    # Check ASN.1
    if result[len(result) - 0x33:len(result) - 0x20] != ASN1_PREFIX:
        print('Invalid ASN.1 prefix')
        return False

    if result[len(result) - 0x20:] != hash:
        print('Invalid SHA256 hash')
        print(f'Expected SHA256: {result[len(result) - 0x20:].hex()}\nActual SHA256: {hash.hex()}')
        return False

    return True

def verify(f):
    # Check header
    if f.read(8) != b'CIPH\x00\x00\x00\x00':
        print('Invalid file signature')
        sys.exit(1)

    # Get body size
    size = get_remaining_size(f)

    # Don't count signature
    size -= 0x100

    # Calculate body hash
    cipher = AES.new(AES_KEY, AES.MODE_CTR, nonce=AES_IV[:12])
    sha256 = hashlib.sha256()
    while size > 0:
        to_read = min(size, 0x10000)

        # Decrypt and update hash
        sha256.update(cipher.decrypt(f.read(to_read)))
        size -= to_read

    # Read and decrypt the signature
    signature = int.from_bytes(cipher.decrypt(f.read(0x100)), byteorder='big')

    if verify_signature(signature, sha256.digest()) != True:
        print('Error: Invalid signature')
        sys.exit(1)

    print('Verified successfully!')


def encrypt(f, outf):
    # Get size to encrypt
    size = get_remaining_size(f)

    # Add file signature
    outf.write(b'CIPH\x00\x00\x00\x00')

    cipher = AES.new(AES_KEY, AES.MODE_CTR, nonce=AES_IV[:12])
    while size > 0:
        to_read = min(size, 0x10000)

        # Encrypt and write to file
        outf.write(cipher.encrypt(f.read(to_read)))
        size -= to_read

    # Add dummy signature
    outf.write(b'\x00' * 0x100)

    outf.close()

def decrypt(f, outf):
    # Check header
    if f.read(8) != b'CIPH\x00\x00\x00\x00':
        print('Invalid file signature')
        sys.exit(1)

    # Get body size
    size = get_remaining_size(f)

    # Don't count signature
    size -= 0x100

    cipher = AES.new(AES_KEY, AES.MODE_CTR, nonce=AES_IV[:12])
    sha256 = hashlib.sha256()
    while size > 0:
        to_read = min(size, 0x10000)

        # Decrypt, update hash and write file
        decrypted = cipher.decrypt(f.read(to_read))
        sha256.update(decrypted)
        outf.write(decrypted)
        size -= to_read

    # Read and decrypt the signature
    signature_data = cipher.decrypt(f.read(0x100))
    signature = int.from_bytes(signature_data, byteorder='big')

    # TODO should the signature be added to the output file?
    # outf.write(signature_data)

    if verify_signature(signature, sha256.digest()) != True:
        print('Warning: Invalid signature')

    outf.close()


def print_usage(file):
    print(f'Usage: {file} verify/encrypt/decrypt infile outfile')
    sys.exit(1)

def main(argc, argv):
    if argc < 3:
        print_usage(argv[0])

    if len(AES_KEY) != 16 or len(AES_IV) < 12:
        print('Error: Update AES_KEY and AES_IV in key.py')
        sys.exit(1)

    f = open(argv[2], 'rb')

    if argv[1] == 'verify':
        verify(f)
    elif argv[1] == 'encrypt':
        if argc < 4:
            print_usage(argv[0])

        encrypt(f, open(sys.argv[3], 'wb'))
    elif argv[1] == 'decrypt':
        if argc < 4:
            print_usage(argv[0])

        decrypt(f, open(sys.argv[3], 'wb'))

    f.close()

if __name__ == '__main__':
    main(len(sys.argv), sys.argv)
