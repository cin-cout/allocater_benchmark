#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define BENCH_DEV "/dev/bench"
#define MAX_OFFSET 4992
#define runs 50
#define LOOPS 10000000

#define zs_range_4096 4
#define zs_range_2048 6
#define xv_range_4096 5
#define xv_range_2048 7
#define zs 0
#define xv 1
#define v 2
#define k 3

char write_buf[] = "testing writing";

void big_range_bench(int fd, int mode, FILE *fptr, int range_min, int range_max){

    for (int i = 0; i <= runs; i++) {
        lseek(fd, range_min, SEEK_SET);
        printf("now %d %d\n", range_min, range_max);
        unsigned long long sz = write(fd, write_buf, mode);
        printf("%d %d %.3f\n", range_min, range_max, sz * 1e-3/(double)LOOPS);
        fprintf(fptr, "%d %d %.3f\n", range_min, range_max, sz * 1e-3/(double)LOOPS);
    }

}

void allocator_bench(int fd, int mode, FILE *fptr){

    for (int i = 0; i <= MAX_OFFSET; i+=64) {
        lseek(fd, i, SEEK_SET);
        printf("now %d %d\n", i, i+64);
        unsigned long long sz = write(fd, write_buf, mode);
        printf("%d %d %.3f\n", i, i+64, sz * 1e-3/(double)LOOPS);
        fprintf(fptr, "%d %d %.3f\n", i, i+64, sz * 1e-3/(double)LOOPS);
    }
}

int main()
{   
    FILE *fptr_za;
    FILE *fptr_zb;
    FILE *fptr_zs;
    FILE *fptr_xva;
    FILE *fptr_xvb;
    FILE *fptr_xvs;
    FILE *fptr_z;
    FILE *fptr_xv;
    FILE *fptr_v;
    FILE *fptr_k;
    
    fptr_za = fopen("./txt/zsmalloc_bench_range_all.txt", "w");
    fptr_zb = fopen("./txt/zsmalloc_bench_range_big.txt", "w");
    fptr_zs = fopen("./txt/zsmalloc_bench_range_small.txt", "w");
    fptr_xva = fopen("./txt/xvmalloc_bench_range_all.txt", "w");
    fptr_xvb = fopen("./txt/xvmalloc_bench_range_big.txt", "w");
    fptr_xvs = fopen("./txt/xvmalloc_bench_range_small.txt", "w");
    fptr_z = fopen("./txt/zsmalloc_bench.txt", "w");
    fptr_xv = fopen("./txt/xvmalloc_bench.txt", "w");
    fptr_v = fopen("./txt/vmalloc_bench.txt", "w");
    fptr_k = fopen("./txt/kmalloc_bench.txt", "w");

    int fd = open(BENCH_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }
    
    big_range_bench(fd, zs_range_4096, fptr_za, 0, 4096);
    big_range_bench(fd, zs_range_2048, fptr_zb, 2048, 4096);
    big_range_bench(fd, zs_range_2048, fptr_zs, 0, 2048);

    big_range_bench(fd, xv_range_4096, fptr_xva, 0, 4096);
    big_range_bench(fd, xv_range_2048, fptr_xvb, 2048, 4096);
    big_range_bench(fd, xv_range_2048, fptr_xvs, 0, 2048);

    allocator_bench(fd, zs, fptr_z);
    allocator_bench(fd, xv, fptr_xv);
    allocator_bench(fd, v, fptr_v);
    allocator_bench(fd, k, fptr_k);

    fclose(fptr_za);
    fclose(fptr_zb);
    fclose(fptr_zs);
    fclose(fptr_xva);
    fclose(fptr_xvb);
    fclose(fptr_xvs);
    fclose(fptr_z);
    fclose(fptr_xv);
    fclose(fptr_v);
    fclose(fptr_k);
    
    close(fd);
    return 0;
}