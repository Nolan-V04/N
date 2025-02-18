#include <iostream>
#include <bitset>
#include <cstring>

using namespace std;

// PC-1 table (Initial 56-bit permutation)
const int PC1[56] = {
    57, 49, 41, 33, 25, 17, 9,
    1, 58, 50, 42, 34, 26, 18,
    10, 2, 59, 51, 43, 35, 27,
    19, 11, 3, 60, 52, 44, 36,
    63, 55, 47, 39, 31, 23, 15,
    7, 62, 54, 46, 38, 30, 22,
    14, 6, 61, 53, 45, 37, 29,
    21, 13, 5, 28, 20, 12, 4
};

// PC-2 table (48-bit subkey permutation)
const int PC2[48] = {
    14, 17, 11, 24, 1, 5,
    3, 28, 15, 6, 21, 10,
    23, 19, 12, 4, 26, 8,
    16, 7, 27, 20, 13, 2,
    41, 52, 31, 37, 47, 55,
    30, 40, 51, 45, 33, 48,
    44, 49, 39, 56, 34, 53,
    46, 42, 50, 36, 29, 32
};

// Left shift schedule
const int SHIFT_SCHEDULE[16] = {
    1, 1, 2, 2, 2, 2, 2, 2,
    1, 2, 2, 2, 2, 2, 2, 1
};

// Function to extract a bit from a 64-bit key (represented as an array of unsigned chars)
bool ExtractBit(const unsigned char* data, int bitPosition) {
    return (data[bitPosition / 8] >> (7 - (bitPosition % 8))) & 1;
}

// Function to set a bit in the key
void ReplaceBit(unsigned char* data, int bitPosition, bool bitValue) {
    int byteIndex = bitPosition / 8;
    int bitIndex = 7 - (bitPosition % 8);
    if (bitValue)
        data[byteIndex] |= (1 << bitIndex);
    else
        data[byteIndex] &= ~(1 << bitIndex);
}

// Convert ASCII key to 64-bit binary
void ConvertKeyToBinary(const unsigned char* key, unsigned char* binaryKey) {
    memcpy(binaryKey, key, 8);  // Copy 8 bytes directly
}

// Function to generate 16 DES subkeys
unsigned char** CreateSubKey(const unsigned char* textKey) {
    unsigned char key[8] = {0};
    ConvertKeyToBinary(textKey, key);

    // Allocate space for 16 subkeys
    unsigned char** subkeys = new unsigned char*[16];
    for (int i = 0; i < 16; i++) {
        subkeys[i] = new unsigned char[6]();  // Subkey size is 6 bytes
    }

    // Step 1: Apply PC-1 to get a 56-bit key
    unsigned char permutedKey[7] = {0}; // 56 bits, but stored in 7 bytes

    for (int i = 0; i < 56; i++) {
        bool bit = ExtractBit(key, PC1[i] - 1);
        ReplaceBit(permutedKey, i, bit);
    }

    // Step 2: Split into C and D (28-bit halves)
    unsigned int C = 0, D = 0;
    for (int i = 0; i < 28; i++) {
        C = (C << 1) | ExtractBit(permutedKey, i);
        D = (D << 1) | ExtractBit(permutedKey, i + 28);
    }

    // Step 3: Generate 16 subkeys
    for (int round = 0; round < 16; round++) {
        // Perform left shift for C and D
        C = ((C << SHIFT_SCHEDULE[round]) | (C >> (28 - SHIFT_SCHEDULE[round]))) & 0x0FFFFFFF;
        D = ((D << SHIFT_SCHEDULE[round]) | (D >> (28 - SHIFT_SCHEDULE[round]))) & 0x0FFFFFFF;

        // Combine C and D back into a 56-bit value
        unsigned char combinedCD[7] = {0};
        for (int i = 0; i < 28; i++) {
            ReplaceBit(combinedCD, i, (C >> (27 - i)) & 1);
            ReplaceBit(combinedCD, i + 28, (D >> (27 - i)) & 1);
        }

        // Step 4: Apply PC-2 to get a 48-bit subkey
        for (int i = 0; i < 48; i++) {
            bool bit = ExtractBit(combinedCD, PC2[i] - 1);
            ReplaceBit(subkeys[round], i, bit);
        }
    }

    return subkeys;
}

int main() {
    unsigned char Key[] = "abcdefgh"; // Example key
    unsigned char** Keys = CreateSubKey(Key);

    // Print subkeys
    for (size_t k = 0; k < 16; k++) {
        printf("SubKey %2d: ", (int)k);
        for (size_t i = 0; i < 6; i++) {
            printf("%02x ", (unsigned short)Keys[k][i]);
        }
        printf("\n");
    }

    // Free allocated memory
    for (int i = 0; i < 16; i++) {
        delete[] Keys[i];
    }
    delete[] Keys;

    return 0;
}
