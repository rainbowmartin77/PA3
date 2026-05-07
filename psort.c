#include <stdio.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
    int f = open(argv[0], O_RDONLY);
    if (f == -1) {
        perror("Error");
        return 1;
    }



    return 0;
}