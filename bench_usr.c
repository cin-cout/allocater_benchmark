#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define BENCH_DEV "/dev/bench"
#define MAX_OFFSET 16320
#define LOOPS 10000000

int main()
{
    unsigned long long sz;
    

    char write_buf[] = "testing writing";
    int offset = MAX_OFFSET;
    FILE *fptr_z;
    FILE *fptr_v;
    fptr_z = fopen("./txt/zsmalloc_bench.txt", "w");
    fptr_v = fopen("./txt/xvmalloc_bench.txt", "w");


    int fd = open(BENCH_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    for (int i = 0; i <= offset; i+=64) {
        lseek(fd, i, SEEK_SET);
        printf("now %d %d\n", i, i+64);
        sz = write(fd, write_buf, 0);
        printf("%d %d %.3f\n", i, i+64, sz * 1e-3/(double)LOOPS);
        fprintf(fptr_z, "%d %d %.3f\n", i, i+64, sz * 1e-3/(double)LOOPS);
    }

    for (int i = 0; i <= offset; i+=64) {
        lseek(fd, i, SEEK_SET);
        printf("now %d %d\n", i, i+64);
        sz = write(fd, write_buf, 1);
        printf("%d %d %.3f\n", i, i+64, sz * 1e-3/(double)LOOPS);
        fprintf(fptr_v, "%d %d %.3f\n", i, i+64, sz * 1e-3/(double)LOOPS);
    }

    fclose(fptr_z);
    fclose(fptr_v);
    close(fd);
    return 0;
}