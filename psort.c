#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <pthread.h>

#define MAX = 4294967296;
#define arrSize(x) (sizeof(x) / sizeof((x)[0]))

typedef struct {
    int32_t key;
    char data[96]; 
} rec;

typedef struct {
    rec *arr;
    size_t start;
    size_t end;
} threadRecArray;

void* count(void* arg) {
    // Array A - to be sorted
    rec* thisChunk = (rec*)arg;
    // size of array A
    int n = arrSize(thisChunk);
    int32_t max = INT32_MAX;

    
}

int main(int argc, char* argv[]) {
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
    rec *mapped = mmap(NULL, data.st_size, PROT_READ, MAP_PRIVATE, f, 0);
    if (mapped == MAP_FAILED) {
        perror("mmap error");
        return 1;
    }

    size_t numRecs = data.st_size / sizeof(rec);

    // number of threads - available processors available
    int numThreads = get_nprocs();
    // records per thread
    int threadChunk = (int)(data.st_size / numThreads);
    // array of mapped array chunks to go to each thread
    threadRecArray arr[numThreads];

    // create threads
    pthread_t threads[numThreads];
    for (int i = 0; i < numThreads; i++) {
        arr[i].arr = mapped;
        arr[i].start = i * threadChunk;
        arr[i].end = (i == numThreads - 1) ? numRecs : (i + 1) * threadChunk;
        pthread_create(&threads[i], NULL, count, &arr[i]);
    }

    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    
    
    

    close(f);

    return 0;
}