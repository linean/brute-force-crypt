#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <crypt.h>
#include <time.h>

const char salt[] = "123";
const char dictionary[] = {
        'A', 'B', 'C', 'D', 'E',
        'F', 'G', 'H', 'I', 'J',
        'K', 'L', 'M', 'N', 'O',
        'P', 'Q', 'R', 'S', 'T',
        'U', 'V', 'W', 'X', 'Y',
        'Z',
        'a', 'b', 'c', 'd', 'e',
        'f', 'g', 'h', 'i', 'j',
        'k', 'l', 'm', 'n', 'o',
        'p', 'q', 'r', 's', 't',
        'u', 'v', 'w', 'x', 'y',
        'z',
        '0', '1', '2', '3', '4',
        '5', '6', '7', '8', '9'
};
const int dictionarySize = sizeof(dictionary);
const int maxPasswordLength = 4;

long possibleCombinations;

long countPossibleCombinations(int passwordLength, int dictionarySize) {
    if (passwordLength < 1) return 0;
    if (passwordLength == 1) return dictionarySize;
    return countPossibleCombinations(passwordLength - 1, dictionarySize) * dictionarySize + dictionarySize;
}

char *generatePassword(long seed) {
    long passwordIndex = 0;
    long previousIndex = seed / dictionarySize - 1;
    char *password = malloc(maxPasswordLength);

    while (previousIndex >= 0) {
        password[passwordIndex++] = (previousIndex > (dictionarySize - 1)) ?
                                    dictionary[previousIndex % dictionarySize] :
                                    dictionary[previousIndex];

        previousIndex = previousIndex / dictionarySize - 1;
    }

    password[passwordIndex] = dictionary[seed % dictionarySize];
    return password;
}

void bruteForceCrypt(const char encryptedPassword[]) {
    clock_t start = clock();
    int success = 1;

    for (long seed = 0; seed < possibleCombinations; seed++) {
        if (success == 0) {
            break;
        }

        char *password = generatePassword(seed);
        if (strcmp(crypt(password, salt), encryptedPassword) == 0) {
            clock_t end = clock();
            double executionTime = (double) (end - start) / CLOCKS_PER_SEC;
            printf("Succeed to brute force! %s == %s (%lf seconds)\n", encryptedPassword, password, executionTime);
            success = 0;
        }
        free(password);
    }

    if (success != 0) {
        printf("Failed to brute force %s\n", encryptedPassword);
    }
}

/* gcc main.c -lcrypt -o main */
int main() {
//    printf("uLA - %s\n", crypt("uLA",salt));
//    printf("n0gA - %s\n", crypt("n0gA",salt));
//    printf("PI3s - %s\n", crypt("PI3s",salt));
//    printf("uChO - %s\n", crypt("uChO",salt));
//    printf("wAgA - %s\n", crypt("wAgA",salt));
//    printf("5m0k - %s\n", crypt("5m0k",salt));

    possibleCombinations = countPossibleCombinations(maxPasswordLength, dictionarySize);

    printf("Dictionary size:%d\n", dictionarySize);
    printf("Max password length:%d\n", maxPasswordLength);
    printf("Possible password combinations:%ld\n", possibleCombinations);
    printf("Brute forcing...\n");

    bruteForceCrypt("121PRpnQMYV3k");   // a
    bruteForceCrypt("12lEFLUCRu9F6");   // uLA
    bruteForceCrypt("12pVin.zzdyWc");   // n0gA
    bruteForceCrypt("12FzdOhc/dsSY");   // PI3s
    bruteForceCrypt("12j73m9qVJcjw");   // uChO
    bruteForceCrypt("12UeC5UlydX6A");   // wAgA
    bruteForceCrypt("12FBySa5z4k22");   // 5m0k

    printf("Done\n");
    return 0;
}
