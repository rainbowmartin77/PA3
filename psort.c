#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <pthread.h>

#define MAX (int)INT32_MAX
#define NUM_THREADS get_nprocs()
#define arrSize(x) (sizeof(x) / sizeof((x)[0]))

typedef struct {
    int32_t key;
    char data[96]; 
} rec;

rec *A, *B, *C; // global arrays

void* count(void* arg) {
    long id = (long)arg;

    printf("Thread number %ld\n", id);

    size_t len = sizeof(A) / sizeof(A[0]);
    int chunk = (int)((len + NUM_THREADS -1) / NUM_THREADS);
    int start = id * chunk;
    int end = (start + chunk > len) ? len : start + chunk;

    
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    // create threads
    pthread_t threads[NUM_THREADS];

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

    //size_t numRecs = data.st_size / sizeof(rec);
    

    
    for (long i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, count, (void *)i);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    close(f);

    return 0;
}