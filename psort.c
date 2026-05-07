#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sys/mman.h>

typedef struct {
    int32_t key;
    char data[96]; 
} rec;

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

    rec *mapped = mmap(NULL, data.st_size, PROT_READ, MAP_PRIVATE, f, 0);
    if (mapped == MAP_FAILED) {
        perror("mmap error");
        return 1;
    }

    // need number of records
    size_t numRecs = data.st_size / sizeof(rec);

    printf("%zu\n", numRecs);

    close(f);

    return 0;
}