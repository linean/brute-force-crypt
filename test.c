#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

const int processesCount = 5;

void test(char encryptedPassword[]) {
    char command[128];
    snprintf(command, sizeof(command), "mpirun -q -n %d brute-force-mpi %s", processesCount, encryptedPassword);
    system(command);
}

/* gcc test.c -o test && ./test */
int main() {
    // Print some info about configuration
    printf("Processes count: %d\n", processesCount);
    printf("Brute forcing...\n");

    system("mpicc main.c -lcrypt -o brute-force-mpi"); // build main.c
    test("121PRpnQMYV3k"); // a
    test("12lEFLUCRu9F6"); // uLA
    test("12pVin.zzdyWc"); // n0gA
    test("12FzdOhc/dsSY"); // PI3s
    test("12j73m9qVJcjw"); // uChO
    test("12UeC5UlydX6A"); // wAgA
    test("12FBySa5z4k22"); // 5m0k
    printf("Done\n");
}
