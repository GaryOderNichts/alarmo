/**
 * Nintendo Alarmo content key bruteforcer.
 * Created in 2024 by GaryOderNichts.
 * 
 * Modified in 2025 by Minki the Avali to support fallback via OpenSSL when not using AES-NI.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#if (defined(__x86_64__) || defined(__i386__)) && !defined(USECRYPTO)
#include <wmmintrin.h>
#elif defined(__aarch64__) && defined(__ARM_FEATURE_AES) && !defined(USECRYPTO)
// ARMv8 Cryptographic Extension: back the x86 AES-NI intrinsics the code below
// uses with hardware AESE/AESMC, so the same aes128_encrypt runs natively on
// arm64 (e.g. Apple Silicon) without AES-NI, OpenSSL, or software AES.
#include <arm_neon.h>
typedef uint8x16_t __m128i;
#define _MM_SHUFFLE(z, y, x, w) (((z) << 6) | ((y) << 4) | ((x) << 2) | (w))
#define _mm_slli_si128(a, imm) vextq_u8(vdupq_n_u8(0), (a), 16 - (imm))
static inline __m128i _mm_loadu_si128(const __m128i *p) { return vld1q_u8((const uint8_t *)p); }
static inline void _mm_storeu_si128(__m128i *p, __m128i a) { vst1q_u8((uint8_t *)p, a); }
static inline __m128i _mm_xor_si128(__m128i a, __m128i b) { return veorq_u8(a, b); }
static inline __m128i _mm_shuffle_epi32(__m128i a, int imm)
{
    uint32x4_t v = vreinterpretq_u32_u8(a);
    uint32_t l[4] = { vgetq_lane_u32(v, 0), vgetq_lane_u32(v, 1),
                      vgetq_lane_u32(v, 2), vgetq_lane_u32(v, 3) };
    uint32_t r[4] = { l[imm & 3], l[(imm >> 2) & 3], l[(imm >> 4) & 3], l[(imm >> 6) & 3] };
    return vreinterpretq_u8_u32(vld1q_u32(r));
}
// aesenc(a,k)     = MixColumns(ShiftRows(SubBytes(a))) ^ k  (SubBytes/ShiftRows commute)
// aesenclast(a,k) = ShiftRows(SubBytes(a)) ^ k
static inline __m128i _mm_aesenc_si128(__m128i a, __m128i k) { return veorq_u8(vaesmcq_u8(vaeseq_u8(a, vdupq_n_u8(0))), k); }
static inline __m128i _mm_aesenclast_si128(__m128i a, __m128i k) { return veorq_u8(vaeseq_u8(a, vdupq_n_u8(0)), k); }
// _mm_aeskeygenassist has no single ARM instruction; it is only the (cold) key
// schedule, so implement it in software with the AES S-box.
static const uint8_t _aes_keygen_sbox[256] = {
    0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
    0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
    0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
    0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
    0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16,
};
static inline uint32_t _aes_subword(uint32_t x)
{
    return ((uint32_t)_aes_keygen_sbox[(x >> 24) & 0xff] << 24) |
           ((uint32_t)_aes_keygen_sbox[(x >> 16) & 0xff] << 16) |
           ((uint32_t)_aes_keygen_sbox[(x >> 8) & 0xff] << 8) |
           ((uint32_t)_aes_keygen_sbox[x & 0xff]);
}
static inline __m128i _mm_aeskeygenassist_si128(__m128i a, int rcon)
{
    uint32x4_t v = vreinterpretq_u32_u8(a);
    uint32_t x1 = vgetq_lane_u32(v, 1), x3 = vgetq_lane_u32(v, 3);
    uint32_t s1 = _aes_subword(x1), s3 = _aes_subword(x3);
    uint32_t r[4] = { s1, ((s1 >> 8) | (s1 << 24)) ^ (uint32_t)rcon,
                      s3, ((s3 >> 8) | (s3 << 24)) ^ (uint32_t)rcon };
    return vreinterpretq_u8_u32(vld1q_u32(r));
}
#else
#include <openssl/evp.h>
#endif

typedef struct {
    uint8_t iv[16];
    uint8_t encrypted_parts[4][16];
} aes_data_t;

static void aes128_encrypt(const uint8_t *key, const uint8_t *plaintext, uint8_t *ciphertext);
static int hex2bin(const char *str, void *bytes, size_t maxsize);

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <aes_data.bin or hex string>\n", argv[0]);
        return 1;
    }

    // Try to read in AES data from file
    aes_data_t data;
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        if (hex2bin(argv[1], &data, sizeof(data)) != sizeof(data)) {
            printf("Failed to open file or invalid hex string\n");
            return 1;
        }
    } else {
        if (fread(&data, 1, sizeof(data), f) != sizeof(data)) {
            fclose(f);
            printf("Failed to read file\n");
            return 1;
        }
        fclose(f);
    }

#if defined(USECRYPTO)
    // Initialize OpenSSL (required for some older versions)
    OpenSSL_add_all_algorithms();
#endif

    // Save start time
    time_t start_time = time(NULL);
    const uint8_t *iv = &data.iv[0];
    uint8_t full_key[16] = { 0 };

    for (int i = 3; i >= 0; i--) {
        // Start with the least blanked out key data
        const uint8_t *expected = &data.encrypted_parts[i][0];

        // Start guessing
        uint32_t key = 0;
        while (1) {
            // Fill in the key
            full_key[(i * 4) + 0] = (uint8_t) (key >> 24);
            full_key[(i * 4) + 1] = (uint8_t) (key >> 16);
            full_key[(i * 4) + 2] = (uint8_t) (key >> 8);
            full_key[(i * 4) + 3] = (uint8_t) (key);

            // Encrypting the IV should result in the expected data
            uint8_t result[16];
            aes128_encrypt(full_key, iv, result);

            if (memcmp(result, expected, 16) == 0) {
                // Found key
                printf("\nFound key part %d / 4: %08x\n", (4 - i), key);
                break;
            }

            // Make sure we don't overflow
            if (key == ~0) {
                printf("\nCouldn't find match\n");
                return 1;
            }

            // Update progress
            if ((key & 0xFFFF) == 0) {
                printf("\rBruteforcing key part %d / 4... %.1f%% (0x%08x / 0xffffffff)", (4 - i), (key / 4294967296.0) * 100.0f, key);
                fflush(stdout);
            }

            // Increment test key
            key++;
        }
    }

    // Print output
    printf("Found key! Took %ld seconds.\n", time(NULL) - start_time);
    printf("Key:\t");
    for (int i = 0; i < 16; i++) printf("%02x", full_key[i]);
    printf("\nIV:\t");
    for (int i = 0; i < 16; i++) printf("%02x", iv[i]);
    printf("\n");

    return 0;
}

static inline int char2bin(char c)
{
    if((c >= 'a') && (c <= 'f')) return c - 'a' + 10;
    if((c >= '0') && (c <= '9')) return c - '0';
    if((c >= 'A') && (c <= 'F')) return c - 'A' + 10;
    return -1;
}

static int hex2bin(const char *str, void *bytes, size_t maxsize)
{
    uint8_t *bytes_ptr = (uint8_t *)bytes;
    size_t bytes_in = 0;
    // Only accept hex pairs
    size_t len = strlen(str);

    if ((len & 1) != 0 || (len / 2) > maxsize)
        return -1;

    for (; str[0] != '\0'; str += 2) {
        // Check for invalid characters
        int char0 = char2bin(str[0]);
        int char1 = char2bin(str[1]);
        if (char0 == -1 || char1 == -1)
            return -1;

        bytes_ptr[bytes_in++] = (char0 << 4) | char1;
    }

    return bytes_in;
}

// Platform-specific AES implementation

#if ((defined(__x86_64__) || defined(__i386__)) || (defined(__aarch64__) && defined(__ARM_FEATURE_AES))) && !defined(USECRYPTO)

// Hardware-accelerated AES (x86 AES-NI, or ARMv8 AESE/AESMC via the shims above)
#define AES_128_key_exp(k, rcon) aes128_key_expansion(k, _mm_aeskeygenassist_si128(k, rcon))

static __m128i aes128_key_expansion(__m128i key, __m128i keygened)
{
    keygened = _mm_shuffle_epi32(keygened, _MM_SHUFFLE(3,3,3,3));
    key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
    key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
    key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
    return _mm_xor_si128(key, keygened);
}

static void aes128_encrypt(const uint8_t *key, const uint8_t *plaintext, uint8_t *ciphertext)
{
    // Start by loading the key
    __m128i aes_key_schedule[11]; // This was one byte short, since that byte wasnt read it didnt cause any errors but it did give me a warn on verbose build as there was no null byte.
    aes_key_schedule[0] = _mm_loadu_si128((const __m128i*) key);
    aes_key_schedule[1] = AES_128_key_exp(aes_key_schedule[0], 0x01);
    aes_key_schedule[2] = AES_128_key_exp(aes_key_schedule[1], 0x02);
    aes_key_schedule[3] = AES_128_key_exp(aes_key_schedule[2], 0x04);
    aes_key_schedule[4] = AES_128_key_exp(aes_key_schedule[3], 0x08);
    aes_key_schedule[5] = AES_128_key_exp(aes_key_schedule[4], 0x10);
    aes_key_schedule[6] = AES_128_key_exp(aes_key_schedule[5], 0x20);
    aes_key_schedule[7] = AES_128_key_exp(aes_key_schedule[6], 0x40);
    aes_key_schedule[8] = AES_128_key_exp(aes_key_schedule[7], 0x80);
    aes_key_schedule[9] = AES_128_key_exp(aes_key_schedule[8], 0x1B);
    aes_key_schedule[10] = AES_128_key_exp(aes_key_schedule[9], 0x36);

    __m128i m = _mm_loadu_si128((const __m128i *) plaintext);
    m = _mm_xor_si128(m, aes_key_schedule[0]);
    for (int i = 1; i < 10; ++i)
        m = _mm_aesenc_si128(m, aes_key_schedule[i]);
    m = _mm_aesenclast_si128(m, aes_key_schedule[10]);
    _mm_storeu_si128((__m128i *) ciphertext, m);
}

#else

// Fallback AES using OpenSSL EVP
static void aes128_encrypt(const uint8_t *key, const uint8_t *plaintext, uint8_t *ciphertext)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "EVP_CIPHER_CTX_new failed\n");
        exit(1);
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL) != 1) {
        fprintf(stderr, "EVP_EncryptInit_ex failed\n");
        EVP_CIPHER_CTX_free(ctx);
        exit(1);
    }

    EVP_CIPHER_CTX_set_padding(ctx, 0); // Disable padding

    int outlen = 0;
    if (EVP_EncryptUpdate(ctx, ciphertext, &outlen, plaintext, 16) != 1 || outlen != 16) {
        fprintf(stderr, "EVP_EncryptUpdate failed\n");
        EVP_CIPHER_CTX_free(ctx);
        exit(1);
    }

    EVP_CIPHER_CTX_free(ctx);
}
#endif

