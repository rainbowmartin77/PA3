#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <pthread.h>
#include <stdlib.h>
#include <inttypes.h>

#define NUM_THREADS get_nprocs()
#define BIT 256
//#define RANGE 65535 // might be too small

typedef struct {
    int32_t key;
    char data[96]; 
} rec;

rec *A; // input & output
rec *B; // swapping array

size_t numRecs;

typedef struct {
    int threadID;
    size_t start;
    size_t end;
    int shift;
    int *localCount;
} entryArg;

void* count(void* arg) {
    entryArg *thisThreadData = (entryArg*)arg;

    for (size_t i = thisThreadData->start; i < thisThreadData->end; i++) {
        uint32_t key = (uint32_t)A[i].key;
        int DIG = (key >> thisThreadData->shift) & 0xFF;
        thisThreadData->localCount[DIG]++;
    }

    printf("Thread number %d\n", thisThreadData->threadID);
    printf("Thread %d start: %zu\n", thisThreadData->threadID, thisThreadData->start);
    printf("Thread %d end: %zu\n", thisThreadData->threadID, thisThreadData->end);

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    // create threads
    pthread_t threads[NUM_THREADS];
    entryArg args[NUM_THREADS];

    int f = open(argv[1], O_RDONLY);
    if (f == -1) {
        perror("Open error");
        return 1;
    }

    struct stat data;
    if (fstat(f, &data) == -1) {
        perror("fstat error.");
        close(f);
        return 1;
    }

    // array to be sorted
    A = mmap(NULL, data.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, f, 0);
    if (A == MAP_FAILED) {
        perror("mmap error");
        return 1;
    }

    numRecs = data.st_size / sizeof(rec);

    printf("Records: %zu\n", numRecs);
    printf("Threads: %d\n", NUM_THREADS);

    B = malloc(numRecs * sizeof(rec));
    if (!B) {
        perror("malloc B failed\n");
        return 1;
    }

    int chunk = (numRecs + NUM_THREADS - 1) / NUM_THREADS;
    printf("Chunk size: %d\n", chunk);

    for (int p = 0; p < 4; p++) {

        int shift = p * 8;

        for (int i = 0; i < NUM_THREADS; i++) {
            args[i].threadID = i;
            args[i].start = i * chunk;
            args[i].end = ((i + 1) * chunk);

            if (args[i].end > numRecs) {
                args[i].end = numRecs;
            }
            args[i].shift = shift;

            args[i].localCount = calloc(BIT, sizeof(int));
            if (!args[i].localCount) {
                perror("calloc error\n");
                exit(1);
            } 

            pthread_create(&threads[i], NULL, count, &args[i]);
            printf("test\n");
        }

        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

        printf("Test after joining\n");

        int globalCount[BIT] = {0};
    
        printf("Test after calloc for B\n");

        // sum the counts of each key value
        for (int x = 0; x < NUM_THREADS; x++) {
            for (int j = 0; j < BIT; j++) {
                globalCount[j] += args[x].localCount[j];
            }
        }

        printf("Test after accumulating all counts\n");

        // count how many elements per key
        for (int m = 1; m < BIT; m++) {
            globalCount[m] += globalCount[m-1];
        }

        // place each key into output array index
        for (long i = numRecs - 1; i >= 0; i--) {
            uint32_t key = (uint32_t)A[i].key;
            int dig = (key >> shift) & 0xFF;

            int outputIndex = --globalCount[dig];
            B[outputIndex] = A[i];
        }

        for (int t = 0; t < NUM_THREADS; t++) {
            free(args[t].localCount);
        }

        rec *temp = A;
        A = B;
        B = temp;
    }

    for (size_t f = 0; f < numRecs; f++) {
        printf("%" PRId32 "\n", A[f].key);
    }

    free(B);
    munmap(A, data.st_size);
    close(f);

    return 0;
}