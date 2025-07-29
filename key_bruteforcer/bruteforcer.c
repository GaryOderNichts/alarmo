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

    time_t start_time = time(NULL);
    const uint8_t *iv = &data.iv[0];
    uint8_t full_key[16] = { 0 };

    for (int i = 3; i >= 0; i--) {
        const uint8_t *expected = &data.encrypted_parts[i][0];
        uint32_t key = 0;
        while (1) {
            full_key[(i * 4) + 0] = (uint8_t) (key >> 24);
            full_key[(i * 4) + 1] = (uint8_t) (key >> 16);
            full_key[(i * 4) + 2] = (uint8_t) (key >> 8);
            full_key[(i * 4) + 3] = (uint8_t) (key);

            uint8_t result[16];
            aes128_encrypt(full_key, iv, result);

            if (memcmp(result, expected, 16) == 0) {
                printf("\nFound key part %d / 4: %08x\n", (4 - i), key);
                break;
            }

            if (key == ~0) {
                printf("\nCouldn't find match\n");
                return 1;
            }

            if ((key & 0xFFFF) == 0) {
                printf("\rBruteforcing key part %d / 4... %.1f%% (0x%08x / 0xffffffff)", (4 - i), (key / 4294967296.0) * 100.0f, key);
                fflush(stdout);
            }

            key++;
        }
    }

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
    size_t len = strlen(str);

    if ((len & 1) != 0 || (len / 2) > maxsize)
        return -1;

    for (; str[0] != '\0'; str += 2) {
        int char0 = char2bin(str[0]);
        int char1 = char2bin(str[1]);
        if (char0 == -1 || char1 == -1)
            return -1;

        bytes_ptr[bytes_in++] = (char0 << 4) | char1;
    }

    return bytes_in;
}

// Platform-specific AES implementation

#if (defined(__x86_64__) || defined(__i386__)) && !defined(USECRYPTO)

// x86-accelerated AES using AES-NI
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
    __m128i aes_key_schedule[11];
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

