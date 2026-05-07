#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <pthread.h>

#define NUM_THREADS get_nprocs()
#define RANGE 65535 // might be too small

typedef struct {
    int32_t key;
    char data[96]; 
} rec;

rec *A; // input
rec *B; // output

size_t numRecs;
int *globalCount;

typedef struct {
    int threadID;
    size_t start;
    size_t end;
    int *localCount;
} entryArg;

void* count(void* arg) {
    entryArg *thisThreadData = (entryArg*)arg;

    printf("Thread number %zu\n", thisThreadData.threadID);
    printf("Thread start: %zu\n", thisThreadData.start);
    printf("Thread end: %zu\n", thisThreadData.end);

    /*size_t len = sizeof(A) / sizeof(A[0]);
    int chunk = (int)((len + NUM_THREADS -1) / NUM_THREADS);
    int start = id * chunk;
    int end = (start + chunk > len) ? len : start + chunk;*/

    for (size_t i = thisThreadData->start; i < thisThreadData->end; i++) {
        int key = A[i].key;

        if (key >= 0 && key < RANGE) {
            t->localCount[key]__;
        }
    }

    
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    // create threads
    pthread_t threads[NUM_THREADS];
    entryArg args[NUM_THREADS];

    int f = open(argv[1], O_RDONLY);
    if (f == -1) {
        perror("Error");
        return 1;
    }

    struct stat data;
    if (fstat(f, &data) == -1) {
        perror("fstat error.");
        close(f);
        return 1;
    }

    // array to be sorted
    A = mmap(NULL, data.st_size, PROT_READ, MAP_PRIVATE, f, 0);
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

    for (long i = 0; i < NUM_THREADS; i++) {
        args[i].threadID = i;
        args[i]. start = i * chunk;
        args[i].end = (i + 1) * chunk;

        if (args[i].end > numRecs) {
            args[i].end = numRecs;
        }

        args[i].localCount = calloc(RANGE, sizeof(int));
        if (!args[i]. localCount) {
            perror("calloc error\n");
            return 1;
        }

        pthread_create(&threads[i], NULL, count, &args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    close(f);

    return 0;
}