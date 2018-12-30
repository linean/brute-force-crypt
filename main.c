#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <crypt.h>
#include <time.h>
#include <mpi.h>

# define TAG_RESULT 1
# define TAG_SUCCESS 3
# define MASTER_ID 0

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

int emptyBuf;
int slavesCount;
char *encryptedPassword;

long countPossibleCombinations(int passwordLength, int dictionarySize) {
    if (passwordLength < 1) return 0;
    if (passwordLength == 1) return dictionarySize;
    return countPossibleCombinations(passwordLength - 1, dictionarySize) * dictionarySize + dictionarySize;
}

char *generatePassword(long seed) {
    long passwordIndex = 0;
    long previousIndex = seed / dictionarySize - 1;
    char *password = calloc(maxPasswordLength, sizeof(char));

    while (previousIndex >= 0) {
        password[passwordIndex++] = (previousIndex > (dictionarySize - 1)) ?
                                    dictionary[previousIndex % dictionarySize] :
                                    dictionary[previousIndex];

        previousIndex = previousIndex / dictionarySize - 1;
    }

    password[passwordIndex] = dictionary[seed % dictionarySize];
    return password;
}

char *bruteForceCrypt(long startSeed, long endSeed) {
    int success;
    MPI_Request request;
    MPI_Status status;
    MPI_Irecv(&emptyBuf, 1, MPI_INT, MPI_ANY_SOURCE, TAG_SUCCESS, MPI_COMM_WORLD, &request);

    for (long seed = startSeed; seed < endSeed; seed++) {
        // check if any other process has found a solution
        MPI_Test(&request, &success, &status);
        if (success != 0) {
            break;
        }

        char *password = generatePassword(seed);
        if (strcmp(crypt(password, salt), encryptedPassword) == 0) {
            return password;
        }
        free(password);
    }

    return "";
}

void sendSuccessToOthers(int successSlaveId) {
    for (int slaveId = 1; slaveId <= slavesCount; slaveId++) {
        if (slaveId != successSlaveId) {
            MPI_Send(&emptyBuf, 1, MPI_INT, slaveId, TAG_SUCCESS, MPI_COMM_WORLD);
        }
    }
}


void configureMaster() {
    double startTime = MPI_Wtime();

    long possibleCombinations = countPossibleCombinations(maxPasswordLength, dictionarySize);
    long chunk = possibleCombinations / slavesCount;

    // send chunk size to all slaves
    MPI_Bcast(&chunk, 1, MPI_LONG, MASTER_ID, MPI_COMM_WORLD);

    char result[maxPasswordLength];
    MPI_Status status;

    // Wait for results from slaves
    for (int i = 1; i <= slavesCount; i++) {
        MPI_Recv(result, maxPasswordLength, MPI_CHAR, MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &status);

        // If result was success close other slaves and finish
        if (strlen(result) > 0) {
            sendSuccessToOthers(status.MPI_SOURCE);
            printf("Success! %s -> %s (%lf seconds, in slave: %d)\n",
                   encryptedPassword,
                   result,
                   MPI_Wtime() - startTime,
                   status.MPI_SOURCE
            );
            return;
        }
    }

    printf("Failed to brute force %s\n", encryptedPassword);
}


void configureSlave(int slaveId) {
    // Receive chunk size from master
    long chunk;
    MPI_Bcast(&chunk, 1, MPI_LONG, MASTER_ID, MPI_COMM_WORLD);

    // calculate seed range for given slaveId
    long startSeed = chunk * (slaveId - 1);
    long endSeed = chunk * slaveId;

    if (slaveId < slavesCount) {
        endSeed = endSeed - 1;
    }

    // Brute force password, send result to master
    char *result = bruteForceCrypt(startSeed, endSeed);
    MPI_Send(result, maxPasswordLength, MPI_CHAR, MASTER_ID, TAG_RESULT, MPI_COMM_WORLD);
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int processId;
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);

    int processCount;
    MPI_Comm_size(MPI_COMM_WORLD, &processCount);

    slavesCount = processCount - 1;
    encryptedPassword = argv[1];

    if (slavesCount < 1) {
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    if (processId == MASTER_ID) {
        configureMaster();
    } else {
        configureSlave(processId);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
