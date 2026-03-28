/**
 * Nintendo Alarmo content key bruteforce data dumper.
 * Created in 2024 by GaryOderNichts.
 */
#include <stdint.h>
#include <stm32h730xx.h>

static void CRYP_ProcessData(uint32_t* outData, uint32_t outCount, const uint32_t* inData, uint32_t inCount)
{
    // Enable CRYP
    CRYP->CR |= CRYP_CR_CRYPEN;

    uint32_t inOffset = 0;
    uint32_t outOffset = 0;
    while (outOffset < outCount) {
        // Push data if input FIFO isn't full
        while (inOffset < inCount && (CRYP->SR & CRYP_SR_IFNF)) {
            CRYP->DIN = inData[inOffset++];
        }

        // Read data if output FIFO isn't empty
        while (CRYP->SR & CRYP_SR_OFNE) {
            outData[outOffset++] = CRYP->DOUT;
        }
    }

    // Disable CRYP
    CRYP->CR &= ~CRYP_CR_CRYPEN;

    // Flush CRYP
    CRYP->CR |= CRYP_CR_FFLUSH;
}

static void CRYP_AES_CTR_Encrypt(uint32_t *counter, void *outData, uint32_t outSize, const void *inData, uint32_t inSize)
{
    // Setup datatype and algomode (AES-CTR)
    CRYP->CR &= ~(CRYP_CR_DATATYPE_0 | CRYP_CR_ALGOMODE_0 | CRYP_CR_ALGOMODE_1 | CRYP_CR_ALGOMODE_2);
    CRYP->CR |= CRYP_CR_ALGOMODE_AES_CTR;

    // Setup direction (Encrypt)
    CRYP->CR &= ~CRYP_CR_ALGODIR;

    // Setup key size (128)
    CRYP->CR &= ~CRYP_CR_KEYSIZE;

    // Clear lowest IV word
    CRYP->IV1RR = 0;

    CRYP_ProcessData((uint32_t*)outData, outSize / 4, (const uint32_t*)inData, inSize / 4);
}

int main(void)
{
    // Disable interrupts
    __asm__ __volatile__ ("cpsid i" : : : "memory");

    // Pointer to AXI SRAM where results are stored
    uint32_t *result_ptr = (uint32_t*) 0x24000000;

    // Buffer to hold encrypted data
    uint32_t encrypted[4];

    // Clear zero buffer
    uint32_t zeroes[4];
    for (int i = 0; i < 4; i++) {
        zeroes[i] = 0;
    }

    // store IV
    *result_ptr++ = __builtin_bswap32(CRYP->IV0LR);
    *result_ptr++ = __builtin_bswap32(CRYP->IV0RR);
    *result_ptr++ = __builtin_bswap32(CRYP->IV1LR);
    *result_ptr++ = 0; // for the counter just store 0

    for (int i = 0; i < 4; i++) {
        // Start blanking out parts of the key after the first round
        if (i > 0) {
            (&CRYP->K0LR)[4 + (i - 1)] = 0;
        }

        // Encrypt zeroes
        uint32_t counter = 0;
        CRYP_AES_CTR_Encrypt(&counter, encrypted, sizeof(encrypted), zeroes, sizeof(zeroes));

        // Store encrypted data
        *result_ptr++ = (encrypted[0]);
        *result_ptr++ = (encrypted[1]);
        *result_ptr++ = (encrypted[2]);
        *result_ptr++ = (encrypted[3]);
    }

    // All done
    __asm__ __volatile__("bkpt");
    while (1)
        ;
}
